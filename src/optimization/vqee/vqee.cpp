// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/optimization/vqee/vqee.hpp"


namespace qb::vqee {

  // Split a Pauli into multiple sub-Paulis according to a max number of terms constraint.
  std::vector<std::shared_ptr<xacc::quantum::PauliOperator>> VQEE::splitPauli(std::shared_ptr<xacc::quantum::PauliOperator> &in_pauli, int nTermsPerSplit) const {
    std::vector<std::shared_ptr<xacc::quantum::PauliOperator>> subPaulis;
    std::map<std::string, xacc::quantum::Term> terms;
    for (auto termIt = in_pauli->begin(); termIt != in_pauli->end(); ++termIt) {
      terms.emplace(*termIt);
      if (terms.size() >= nTermsPerSplit) {
        subPaulis.emplace_back( std::make_shared<xacc::quantum::PauliOperator>(terms) );
        terms.clear();
      }
    }
    if (!terms.empty()) {
      assert(subPaulis.size() * nTermsPerSplit + terms.size() == in_pauli->nTerms());
      subPaulis.emplace_back( std::make_shared<xacc::quantum::PauliOperator>(terms) );
    }
    return subPaulis;
  }

  std::shared_ptr<xacc::Accelerator> VQEE::getAccelerator(const std::string& accName) const {
    // 3 of 4: accelerator - qpp: "vqe-mode"=true is non-stochastic
    xacc::HeterogeneousMap accParams{{"n-virtual-qpus", params_.nWorker}, {"vqe-mode", params_.isDeterministic}, {"shots", params_.nShots}, {"threads", params_.nThreadsPerWorker}};
    //printf("accParams got %i threads\n",accParams.get<int>("threads"));
    std::shared_ptr<xacc::Accelerator> accelerator = xacc::getAccelerator(accName, accParams);
    if (isParallel_) {
      accelerator = xacc::getAcceleratorDecorator("hpc-virtualization", accelerator, accParams); // wrap accelerator with hpc-decorator to inttroduce MPI parallelism
    }
    if (isRoot_) {
      std::cout << "Accelerator: " << accelerator->name() << std::endl;
    }
    return accelerator;
  }

  std::shared_ptr<xacc::CompositeInstruction> VQEE::getAnsatz() const {
    // 1 of 4: ansatz from XACC qasm string
    std::shared_ptr<xacc::CompositeInstruction> ansatz;
    if (params_.ansatz) {
    //std::cout << "found provided ansatz" << std::endl;
      ansatz = params_.ansatz;
    }
    else {
      //std::cout << "using provided circuit string" << std::endl;
      xacc::qasm(params_.circuitString);
      ansatz = xacc::getCompiled("ansatz");
    }
    //  std::cout << "ansatz:\n" << ansatz->toString() << std::endl;
    if (isRoot_) {
      //std::cout << "\nAnsatz:\n" << ansatz->toString() << std::endl;
      std::cout << "Parameters: " << ansatz->getVariables() << std::endl;
      std::cout << "Ansatz depth: " << ansatz->depth() << std::endl;
    }
    return ansatz;
  }

  std::shared_ptr<xacc::Observable> VQEE::getObservable() {
    // 2 of 4: observable from string
    std::shared_ptr<xacc::Observable> observable = std::make_shared<xacc::quantum::PauliOperator>();
    observable->fromString(params_.pauliString);
    return observable;
  }

  void VQEE::optimize(){ 
  // Here VQE is called with a decorated accelerator. The decorator adds pre- and post-processing around the actual accelerator execution. 
  // This is used to introduce MPI parallelism, i.e partitioning and distributing the vector of instructions (base curcuit + Pauli terms) 
  // and return reduce of the results. Number of MPI processes and threads can be choosen as needed.

    std::shared_ptr<xacc::Accelerator> accelerator{getAccelerator(params_.acceleratorName)};  // 1 of 4: accelerator
    std::shared_ptr<xacc::CompositeInstruction> ansatz{getAnsatz()};        // 2 of 4: ansatz from XACC qasm string

    if (!params_.partitioned) {
      std::shared_ptr<xacc::Observable> observable{getObservable()};          // 3 of 4: observable from string
      
      // 4 of 4: optimiser
      std::shared_ptr<xacc::Optimizer> optimizer = xacc::getOptimizer("nlopt");
      optimizer->setOptions(xacc::HeterogeneousMap{std::make_pair("initial-parameters", params_.theta),
                                                  std::make_pair("nlopt-optimizer",    "cobyla"), // ["nelder-mead", "l-bfgs" or "cobyla"], default: "cobyla"
                                                  std::make_pair("nlopt-maxeval",      params_.maxIters),
                                                  std::make_pair("nlopt-ftol",         params_.tolerance)});

      // instantiate XACC VQE
      std::shared_ptr<xacc::Algorithm> vqe = xacc::getAlgorithm("vqe");
      vqe->initialize({{"ansatz", ansatz},
                      {"accelerator", accelerator},
                      {"observable", observable},
                      {"optimizer", optimizer}});

      // Allocate some qubits and execute
      xacc::qbit buffer = xacc::qalloc(params_.nQubits);
      vqe->execute(buffer);

      // read out buffer 
      params_.energies       = (*buffer)["params-energy" ].as<std::vector<double>>();
      params_.theta          = (*buffer)["opt-params"    ].as<std::vector<double>>();
      params_.optimalValue   = (*buffer)["opt-val"       ].as<double>();
    } 
    else {
      const int nOptVars = ansatz->nVariables();

      // 2 of 4: observable from string
      auto Ham_pauli = std::make_shared<xacc::quantum::PauliOperator>(params_.pauliString);
      const int MAX_TERMS_PER_OBSERVE =  8*params_.nWorker*params_.nThreadsPerWorker; // size of batches, adjust to needs
      auto subPaulis = splitPauli(Ham_pauli, MAX_TERMS_PER_OBSERVE);

      auto q = xacc::qalloc(params_.nQubits);

      // Function to optimize:
      xacc::OptFunction f(
        [&](const std::vector<double> &x, std::vector<double> &g) {
          assert(x.size() == nOptVars);
          std::vector<double> subEnergies;
          int totalProcess = 0;

          auto evaled = (*ansatz)(x);

          for (auto &obs : subPaulis) {
            auto vqe = xacc::getAlgorithm("vqe", {{"ansatz", evaled},
                                                  {"observable", std::dynamic_pointer_cast<xacc::Observable>(obs)},
                                                  {"accelerator", accelerator}});

            xacc::set_verbose(false);
            const double energy = vqe->execute(q, {})[0];
            xacc::set_verbose(true);
            const bool isRank0 = q->hasExtraInfoKey("rank") ? ((*q)["rank"].as<int>() == 0) : true;
            totalProcess += obs->nTerms();

            if (isRank0) xacc::info("Processed " + std::to_string(totalProcess) + " / " + std::to_string(Ham_pauli->nTerms()));
            subEnergies.emplace_back(energy);
          }

          const double total_energy = std::accumulate(subEnergies.begin(), subEnergies.end(), decltype(subEnergies)::value_type(0));
          params_.energies.emplace_back(total_energy);
          
          if (xacc::verbose) {
            std::stringstream ss;
            ss << "[Rank" << GetRank() << "] " << "E(" << x << ") = " << total_energy;
            xacc::info(ss.str());
          }
          return total_energy;
        },
        nOptVars
      );

      // Run optimization:
      auto optimizer = xacc::getOptimizer("nlopt", {{"initial-parameters", params_.theta}});
      std::tie(params_.optimalValue, params_.theta) = optimizer->optimize(f);
    }

    if (isRoot_) {
      std::cout << "\nMin energy = "       << params_.optimalValue << "\n"
                << "Optimal parameters = " << params_.theta        << std::endl;
    }
  } // end of optimize()

} // end of namespace qb::vqee
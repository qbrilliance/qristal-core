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
      ansatz = params_.ansatz;
    }
    else {
      xacc::qasm(params_.circuitString);
      ansatz = xacc::getCompiled("ansatz");
    }
    if (isRoot_) {
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

  size_t VQEE::getOptimumIterationE() {
    vqe_iteration_data xacc_opt;
    xacc_opt.energy = params_.optimalValue;
    xacc_opt.params = params_.theta;

    auto optimum_iteration = std::find(params_.iterationData.begin(), params_.iterationData.end(), xacc_opt);
    if (optimum_iteration == params_.iterationData.end()) {
      throw std::range_error("Could not find the iteration where the optimum occurred");
    } else {
      size_t index = std::distance(params_.iterationData.begin(), optimum_iteration);
      return index;
    }
  }
 
  void VQEE::generateThetaEnergyVis(const std::string in_title, const size_t in_start_elem, const int in_scale, const int in_width, const int in_precision) {
    double scale_factor_theta = in_scale/(2*M_PI);
    std::stringstream ss;    
    if (!params_.plain) ss << "\033[0m"; // Switch text colour to default
    ss << "\n";
    int iterations = params_.iterationData.size();
    size_t iteration_optimum_e = getOptimumIterationE();    
    double energy_at_optimum = params_.iterationData.at(iteration_optimum_e).energy;

    size_t use_last = (params_.tail > 0) ? (iterations-params_.tail) : 0;
    if (params_.blocked) {
      int iteration_i = use_last;
      for (size_t i = use_last; i < params_.iterationData.size(); i++) {
        iteration_i++; // the first iteration is at index 1    
        if ((params_.iterationData.at(i)).energy == energy_at_optimum) {
          if (!params_.plain) ss << "\033[1;32m"; // Switch text colour to green bold
        }     
        if (iteration_i == (iteration_optimum_e + 1)) {
          if (!params_.plain) ss << "\033[1;31m"; // Switch text colour to red bold
        }
        ss << "Iteration " << iteration_i << "\n";

        // Output the energy for iteration_i
        double scale_factor = in_scale/std::abs(params_.iterationData.at(0).energy);
        int bar_scale = 2*in_scale;
        int bar_val = std::floor(params_.iterationData.at(i).energy * scale_factor);
        if ((bar_scale + bar_val) < 0) {
          bar_scale*=2;
          ss << "Rescaling bars..." << "\n";
        }
        ss << "Energy      ";
        if (iteration_i == (iteration_optimum_e+1)) {
          ss << std::setw(in_width) << " **|";
        } else {
          ss << std::setw(in_width) << "   |";
        }
        for (size_t j = 0; j < (bar_scale + bar_val); ++j) {
          ss << "#";
        }
        ss << " " << std::setprecision(in_precision) << (params_.iterationData.at(i)).energy << "\n"; 

        // Output the theta elements for the iteration_i
        if (params_.showTheta) {
          size_t use_elements_n = (params_.limitThetaN > 0) ? params_.limitThetaN : params_.theta.size();
          if (use_elements_n > params_.theta.size()) {
            throw std::range_error("Requested number of elements exceeds the limit of theta elements");
          }
          if (use_elements_n > 0)
            ss << "Theta"
               << "\n";
          for (size_t subpl = in_start_elem; subpl < use_elements_n; ++subpl) {
            double current_val = (params_.iterationData.at(i).params).at(subpl);
            int bar_val = std::floor(current_val * scale_factor_theta);
            ss << std::left << "       Element " << std::setw(4) << subpl;
            if (iteration_i == (iteration_optimum_e + 1)) {
              ss << std::setw(in_width) << " **|";
            }
            else {
              ss << std::setw(in_width) << "   |";
            }
            for (size_t j = 0; j < (in_scale + bar_val); ++j) {
              ss << "#";
            }
            ss << " " << std::setprecision(in_precision) << (current_val / M_PI) << "*pi"
               << "\n";
          }
        }
        if (!params_.plain) ss << "\033[0m"; // Switch text colour to default
      }
      if (!params_.plain) ss << "\033[0m"; // Switch text colour to default
    }
    else {
      if (params_.showTheta) {
        size_t use_elements_n = (params_.limitThetaN > 0) ? params_.limitThetaN : params_.theta.size();
        if (use_elements_n > params_.theta.size()) {
          throw std::range_error("Requested number of elements exceeds the limit of theta elements");
        }
        for (size_t subpl = in_start_elem; subpl < use_elements_n; ++subpl) {
          ss << in_title << ", Element " << subpl << ", " << iterations << " iterations\n";
          int iteration_i = use_last;
          //
          // Generate trace for theta
          for (size_t i = use_last; i < params_.iterationData.size(); i++) {
            iteration_i++; // the first iteration is at index 1
            if (iteration_i == (iteration_optimum_e + 1)) {
              if (!params_.plain)
                ss << "\033[1;31m"; // Switch text colour to red bold
            }
            double current_val = (params_.iterationData.at(i).params).at(subpl);
            int bar_val = std::floor(current_val * scale_factor_theta);
            ss << std::left << "Iteration " << std::setw(4) << iteration_i;
            if (iteration_i == (iteration_optimum_e + 1)) {
              ss << std::setw(in_width) << " **|";
            }
            else {
              ss << std::setw(in_width) << "   |";
            }
            for (size_t j = 0; j < (in_scale + bar_val); ++j) {
              ss << "#";
            }
            ss << " " << std::setprecision(in_precision) << (current_val / M_PI) << "*pi"
               << "\n";
            if (!params_.plain)
              ss << "\033[0m"; // Switch text colour to default
          }
          if (!params_.plain)
            ss << "\033[0m"; // Switch text colour to default
        }
      }
      if (!params_.plain) ss << "\033[0m"; // Switch text colour to default    
      ss << generateEnergyVis(params_.energies, "Energy");
    }
    if (!params_.plain) ss << "\033[0m"; // Switch text colour to default
    params_.vis = ss.str();
  }

  std::string VQEE::generateEnergyVis(const std::vector<double> in_val, const std::string in_title, const int in_stride, const int in_scale,  const int in_width, const int in_precision) {
    double scale_factor = in_scale/std::abs(in_val.at(0));
    int bar_scale = 2*in_scale;
    std::stringstream ss;
    if (!params_.plain) ss << "\033[0m"; // Switch text colour to default

    int iterations = in_val.size()/in_stride;    
    size_t iteration_optimum = getOptimumIterationE();
    double energy_at_optimum = in_val.at(iteration_optimum);
    size_t use_last = (params_.tail > 0) ? (iterations-params_.tail) : 0;

    for (size_t subpl = 0; subpl < in_stride; ++subpl) {        
      ss << in_title << " element " << subpl << ", " << iterations << " iterations\n";
      size_t iteration_i = use_last;
      for (size_t i = subpl+use_last*in_stride; i < in_val.size(); i += in_stride) {
        iteration_i++;  // the first iteration is at index 1        
        int bar_val = std::floor(in_val.at(i)*scale_factor);
        if ((bar_scale + bar_val) < 0) {
          bar_scale*=2;
          ss << "Rescaling bars..." << "\n";
        }       
        if (in_val.at(i) == energy_at_optimum) {
          if (!params_.plain) ss << "\033[1;32m"; // Switch text colour to green bold
        }       
        if (iteration_i == (iteration_optimum+1)) {
          if (!params_.plain) ss << "\033[1;31m"; // Switch text colour to red bold
        }
        ss << std::left << "Iteration " << std::setw(4) << iteration_i;
        if (iteration_i == (iteration_optimum+1)) {
          ss << std::setw(in_width) << " **|";
        } else {
          ss << std::setw(in_width) << "   |";
        }
        for (size_t j = 0; j < (bar_scale + bar_val); ++j) {
          ss << "#";
        }
        ss << " " << std::setprecision(in_precision) << in_val.at(i) << "\n"; 
        if (!params_.plain) ss << "\033[0m"; // Switch text colour to default
      }
      if (!params_.plain) ss << "\033[0m"; // Switch text colour to default
    }    
    if (!params_.plain) ss << "\033[0m"; // Switch text colour to default
    return ss.str();
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
      std::shared_ptr<xacc::Optimizer> optimizer;
      if (params_.algorithm == "nelder-mead") {
          qb::vqee::NelderMeadNLO opt_nlmd = qb::vqee::NelderMeadNLO(params_.theta, 
                                                               params_.maxIters,
                                                               params_.tolerance,
                                                               YAML::Load(params_.extraOptions));
          optimizer = opt_nlmd.get();
      } else if (params_.algorithm =="l-bfgs"){
          qb::vqee::LbfgsNLO opt_lbfgs = qb::vqee::LbfgsNLO(params_.theta,
                                                       params_.maxIters,
                                                       params_.tolerance,
                                                       YAML::Load(params_.extraOptions));
          optimizer = opt_lbfgs.get();
      } else if (params_.algorithm =="adam"){
          qb::vqee::AdamMLP opt_adam = qb::vqee::AdamMLP(params_.theta,
                                                       params_.maxIters,
                                                       params_.tolerance,
                                                       YAML::Load(params_.extraOptions));
          optimizer = opt_adam.get();
      }
      else {
          // This is the default when no algorithm is specified
          optimizer = xacc::getOptimizer("nlopt");
          optimizer->setOptions(xacc::HeterogeneousMap{std::make_pair("initial-parameters", params_.theta),
                                                  std::make_pair("nlopt-optimizer",    "cobyla"), // default: "cobyla"
                                                  std::make_pair("nlopt-maxeval",      params_.maxIters),
                                                  std::make_pair("nlopt-ftol",         params_.tolerance)});
      }
 
      // instantiate XACC VQE
      std::shared_ptr<xacc::Algorithm> vqe;
      if ((params_.isDeterministic) && (accelerator->name() != "hpc-virtualization")) {
        // Handle the case where a state-vector simulator is the back-end,
        // whereby expectation can be calculated from linear algebra    
        vqe = xacc::getAlgorithm("vqe-gen");
      }
      else {   
        // Handle the general case where expectation is found from multiple shot outcomes   
        vqe = xacc::getAlgorithm("vqe");
      }
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
      
      const size_t nIters = params_.energies.size();
      const size_t step = (buffer->nChildren()) / nIters;
      size_t stepidx = 0;

      // params_.iterationData
      params_.iterationData.clear();
      for (auto &childBuff : buffer->getChildren()) {
        if (stepidx % step == 0) {
          if (childBuff->hasExtraInfoKey("parameters")) {
            std::vector<double> param = (*childBuff)["parameters"].as<std::vector<double>>();            
            vqe_iteration_data vid;
            vid.energy = params_.energies.at(stepidx/step);
            vid.params = param;
            params_.iterationData.emplace_back(vid);
          }
        }
        stepidx++;
      }
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
      if (params_.enableVis) {
        generateThetaEnergyVis("theta");
      }
      std::cout << "\nMin energy = "       << params_.optimalValue << "\n"
                << "Optimal parameters = " << params_.theta        << std::endl;
    }
  } // end of optimize()

} // end of namespace qb::vqee
// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#ifndef _QB_VQEE_
#define _QB_VQEE_

// from std lib
#include <string>
#include <future>
#include <chrono>
#include <thread>
#include <cassert>
#include <fstream>
#include <memory>
#include <numeric>

// from xacc lib
#include "PauliOperator.hpp"
#include "ObservableTransform.hpp"
#include "Utils.hpp"
#include "xacc.hpp"
#include "xacc_observable.hpp"
#include "xacc_service.hpp"
#include "AcceleratorDecorator.hpp"

// from qb lib
#include "qb/core/optimization/vqee/mpi_wrapper.hpp"
#include "qb/core/optimization/vqee/case_generator.hpp"

namespace qb::vqee {

/// Overload to allow `std::cout << vector`
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec){
  os<<'[';
  for(auto& elem : vec) {
    os<<elem<< ", ";
  }
  os << "\b\b]"; // Remove ", " from last element and close vector with "]". One character at the end is still to be overwritten
  return os;
}

/// Variational Quantum Eigensolver (VQE) hybrid quantum-classical algorithm
class VQEE {
private:
  const bool  isRoot_ {GetRank() == 0}; // is MPI master process? 
  const bool  isParallel_ {GetSize() >1};
  Params&     params_;

public:

  /// Constructor that accepts qb::vqee::Params
  VQEE(Params& params) : params_{params} {}

// - - - - - - member functions - - - - - - //
private: 
  /// Split a Pauli into multiple sub-Paulis according to a max number of terms constraint.
  std::vector<std::shared_ptr<xacc::quantum::PauliOperator>> splitPauli(std::shared_ptr<xacc::quantum::PauliOperator> &in_pauli, int nTermsPerSplit) {
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

  /// Select a backend simulator or QPU
  std::shared_ptr<xacc::Accelerator> getAccelerator(const std::string& accName) {
    // 3 of 4: accelerator - qpp: "vqe-mode"=true is non-stochastic
    xacc::HeterogeneousMap accParams{{"n-virtual-qpus", params_.nWorker}, {"vqe-mode", params_.isDeterministic}, {"shots", params_.nShots}, {"threads", params_.nThreadsPerWorker}};
    //printf("accParams got %i threads\n",accParams.get<int>("threads"));
    std::shared_ptr<xacc::Accelerator> accelerator = xacc::getAccelerator(accName, accParams);
    if (isParallel_) {
      accelerator = xacc::getAcceleratorDecorator("hpc-virtualization", accelerator, accParams); // wrap accelerator with hpc-decorator to inttroduce MPI parallelism
    }
    return accelerator;
  }

  /// Define an ansatz with associated parameters for VQE
  std::shared_ptr<xacc::CompositeInstruction> getAnsatz() {
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
      std::cout << "\ngetVariables: " << ansatz->getVariables() << std::endl;
    }
    return ansatz;
  }

  /// Define a Hamiltonian for VQE
  std::shared_ptr<xacc::Observable> getObservable() {
    // 2 of 4: observable from string
    std::shared_ptr<xacc::Observable> observable = std::make_shared<xacc::quantum::PauliOperator>();
    observable->fromString(params_.pauliString);
    return observable;
  }

public:

  /// Run the VQE algorithm
  void optimize(){ 
  // Here VQE is called with a decorated accelerator. The decorator adds pre- and post-processing aroung the actual accelerator execution. 
  // This is used to introduce MPI parallelism, i.e partitioning and distributing the vector of instructions (base curcuit + Pauli terms) 
  // and return reduce of the results. Number of MPI processes and threads can be choosen as needed.

    std::shared_ptr<xacc::Accelerator> accelerator{getAccelerator("qpp")};  // 1 of 4: accelerator
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
    } else{
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
      std::cout << "Min energy = " << params_.optimalValue << "\n";
      std::cout << "Optimal parameters = " << params_.theta << "\n";
    }
  } // end of optimize()
};// end of class VQEE

} // end of namespace qb::vqee

#endif // _QB_VQEE_

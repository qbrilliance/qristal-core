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
  // Split a Pauli into multiple sub-Paulis according to a max number of terms constraint.
  std::vector<std::shared_ptr<xacc::quantum::PauliOperator>> splitPauli(std::shared_ptr<xacc::quantum::PauliOperator> &in_pauli, int nTermsPerSplit) const;

  // helper constructors/initializers 
  std::shared_ptr<xacc::Accelerator> getAccelerator(const std::string& accName) const;
  std::shared_ptr<xacc::CompositeInstruction> getAnsatz() const;
  std::shared_ptr<xacc::Observable> getObservable();

public:
  // const int getAnsatzDepth() const { return getAnsatz()->depth(); }

  void optimize(); // setup and optimize vqe problem

};// end of class VQEE

} // end of namespace qb::vqee

#endif // _QB_VQEE_

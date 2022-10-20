/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qbOS {

// Subtraction circuit
// This circuit is used to subtract two qubit registers

// Inputs:
// qubits_larger: indices of first qubit register. This register will contain the result of the subtraction.
// qubits_smaller: indices of second qubit register

class Subtraction : public xacc::quantum::Circuit {
public:
  Subtraction() : xacc::quantum::Circuit("Subtraction") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(Subtraction);
};
} // namespace qbOS
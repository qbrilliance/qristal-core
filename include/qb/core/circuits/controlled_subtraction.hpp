/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qbOS {

// Controlled subtraction circuit
// This circuit is used to subtract two qubit registers
// with optional control qubits

// Inputs:
// qubits_larger: indices of first qubit register. This register will contain the result of the subtraction.
// qubits_smaller: indices of second qubit register.
// controls_on: indices of control qubits conditional on being |1> (optional)
// controls_off: indices of control qubits conditional on being |0> (optional)

class ControlledSubtraction : public xacc::quantum::Circuit {
public:
  ControlledSubtraction() : xacc::quantum::Circuit("ControlledSubtraction") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(ControlledSubtraction);
};
} // namespace qbOS
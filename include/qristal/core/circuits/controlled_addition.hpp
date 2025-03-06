/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qristal {

// Controlled addition circuit
// This circuit is used to add two qubit registers
// with optional control qubits

// Inputs:
// qubits_adder: indices of first qubit register
// qubits_sum: indices of second qubit register. This register will contain the sum after the addition
// c_in: carry-in bit
// flags_on: indices of control qubits conditional on being |1> (optional)
// flags_off: indices of control qubits conditional on being |0> (optional)

class ControlledAddition : public xacc::quantum::Circuit {
public:
  ControlledAddition() : xacc::quantum::Circuit("ControlledAddition") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(ControlledAddition);
};
}
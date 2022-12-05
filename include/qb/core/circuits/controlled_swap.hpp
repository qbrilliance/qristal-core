/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qb {

// Controlled SWAP circuit
// This circuit is used to swap two qubit registers
// with optional control qubits

// Inputs:
// qubits_a: indices of first qubit register
// qubits_b: indices of second qubit register
// flags_on: indices of control qubits conditional on being |1> (optional)
// flags_off: indices of control qubits conditional on being |0> (optional)

class ControlledSwap : public xacc::quantum::Circuit {
public:
  ControlledSwap() : xacc::quantum::Circuit("ControlledSwap") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(ControlledSwap);
};
} // namespace qb
/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qb {

// Equality checker circuit:
// We want to check whether or not two quantum registers are equal

// Inputs:
// qubits_a: the first register of qubits
// qubits_b: the second register of qubits
// flag: an additional qubit that will be flipped to |1> if and only if a=b

// Optional inputs: qubit registers
// use_ancilla: bool, use ancilla for mcx decomposition?
// qubits_ancilla: the register of ancilla qubits if use_ancilla = true

// Output: A circuit object that inputs |a>|b>|0> and returns |a>|b>|flag>
// where |flag> = |1> if a=b and |0> otherwise

class EqualityChecker : public xacc::quantum::Circuit {
public:
  EqualityChecker() : xacc::quantum::Circuit("EqualityChecker") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(EqualityChecker);
};
} // namespace qb
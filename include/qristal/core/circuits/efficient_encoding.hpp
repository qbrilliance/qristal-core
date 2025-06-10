/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include <Circuit.hpp>
namespace qristal {

// Efficient encoding circuit:
// We want to entangle a "score" to each state in a superposition
// I.e., we assume an input state |++...+>|00...0> and we want to 
// inscribe a score on the second register conditional on the state in
// the first register

// Inputs:
// scoring_function: a function that inputs a state and outputs the score corresponding to that state
// num_state_qubits: the number of qubits in the first register
// num_scoring_qubits: the number of qubits in the second register

// Optional inputs: qubit registers
// state_qubits: the indices of the state qubits
// scoring_qubits: the indices of the scoring qubits
// Default register is |state_qubits>|scoring_qubits>

// Optional input: qubit ordering
// is_LSB: [bool] should the qubits be encoded in LSB? 
// Default is true

// Optional input: ancilla
// use_ancilla: [bool] should ancilla be used to decompose mcx gates?
// Default is false

// Output: A circuit object that builds the entangled state |state>|score>

class EfficientEncoding : public xacc::quantum::Circuit {
public:
  EfficientEncoding() : xacc::quantum::Circuit("EfficientEncoding") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(EfficientEncoding);
};
}
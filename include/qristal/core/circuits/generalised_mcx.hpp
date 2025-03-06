/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qristal {

// Generalised MCX gate

// Performs an X gate on the target qubit condtional on:
// controls_on being in the |1> state, and 
// controls_off being in the |0> state

// Inputs:
// target_qubit: index of target qubit (int)
// controls_on: indices of the control qubits conditional on being |1> (std::vector<int>)
// controls_off: indices of the control qubits conditional on being |0> (std::vector<int>)

class GeneralisedMCX : public xacc::quantum::Circuit {
public:
  GeneralisedMCX() : xacc::quantum::Circuit("GeneralisedMCX") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(GeneralisedMCX);
};
}
/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qristal {

/// Multiply circuit:
/// Given two n-bit numbers |a> and |b>, this circuit
/// performs the operation |a>|b>|0> -> |a>|b>|a*b>
/// with possible control bits

// qubits_a: first register
// qubits_b: second register
// qubits_result: result register
// qubit_ancilla: single ancilla required
// is_LSB: a and b given with LSB ordering?
// controls_on: control bits conditional on being |1>
// controls_off: control bits conditional on being |0>

class ControlledMultiplication : public xacc::quantum::Circuit {
public:
  ControlledMultiplication() : xacc::quantum::Circuit("ControlledMultiplication") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(ControlledMultiplication);
};
}
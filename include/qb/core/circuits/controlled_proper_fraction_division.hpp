/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qbOS {

// Proper Fraction Division Circuit
// This circuit is used to divide two quantum registers
// a/b where a <= b

// qubits_numerator: qubits encoding the numerator
// qubits_denominator: qubits encoding the denominator
// qubits_ancilla: ancilla required for the division
// qubits_fraction: the qubits that will store the result
// is_LSB: boolean option indicating whether LSB ordering is used
// controls_on: control qubits conditional on being |1>
// controls_off: control qubits conditional on being |0> 

class ControlledProperFractionDivision : public xacc::quantum::Circuit {
public:
  ControlledProperFractionDivision() : xacc::quantum::Circuit("ControlledProperFractionDivision") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(ControlledProperFractionDivision);
};
} // namespace qbOS
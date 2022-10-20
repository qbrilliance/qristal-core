/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
#include "comparator.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <optional>
#include <vector>
namespace qbOS {

// Quantum Decoder Kernel Circuit

// This circuit is used by the quantum decoder to form beam classes from the original prepared state
// I.e., |String>|StringMetric> -> |Beam>|BeamMetric> (simplified)

// Inputs:
// qubits_string: The register encoding the strings
// qubits_metric: The register encoding the metrics
// qubits_ancilla_adder: The register containing additional qubits to form total_metric
// qubits_init_null: The register of qubits used to mark if a letter is null (one per letter)
// qubits_init_repeat: The register of qubits used to mark if a letter is a repeat (one per letter)
// qubits_superfluous_flags: The register of qubits used to mark trailing letters as superfluous (one per letter)
// qubits_beam_metric: The register of qubits that will contain the final beam metric 
// total_metric: The register of qubits encoding the sum of metrics contained in qubits_metric
// total_metric_copy: A copy of the total_metric register used for the amplitude estimation adder
// evaluation_bits: The register of qubits used to store the output of the amplitude estimation 
// precision_bits: A list of the number of precision qubits used per metric qubit during amplitude estimation
// qubits_ancilla_pool: The register of qubits used as ancilla

class DecoderKernel : public xacc::quantum::Circuit {
public:
  DecoderKernel() : xacc::quantum::Circuit("DecoderKernel") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(DecoderKernel);
};
} // namespace qbOS
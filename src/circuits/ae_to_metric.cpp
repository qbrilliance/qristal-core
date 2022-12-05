/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/ae_to_metric.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <CompositeInstruction.hpp>
#include <assert.h>
#include <bitset>
#include <memory>
#include <vector>

namespace qb {
bool AEtoMetric::expand(const xacc::HeterogeneousMap &runtimeOptions) {
  // Inputs:

  // Circuit
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  if (!runtimeOptions.keyExists<std::vector<int>>("evaluation_bits")) {
    return false;
  }
  std::vector<int> evaluation_bits =
      runtimeOptions.get<std::vector<int>>("evaluation_bits");

  if (!runtimeOptions.keyExists<std::vector<int>>("precision_bits")) {
    return false;
  }
  std::vector<int> precision_bits =
      runtimeOptions.get<std::vector<int>>("precision_bits");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_beam_metric")) {
    return false;
  }
  std::vector<int> qubits_beam_metric =
      runtimeOptions.get<std::vector<int>>("qubits_beam_metric");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla")) {
    return false;
  }
  std::vector<int> qubits_ancilla =
      runtimeOptions.get<std::vector<int>>("qubits_ancilla");
//   assert(
//       qubits_ancilla.size() ==
//       3 * (int)*std::max_element(precision_bits.begin(), precision_bits.end()) +
//           1);

  if (!runtimeOptions.keyExists<int>("qubits_beam_metric_ones_idx")) {
    return false;
  }
  int ones_idx = runtimeOptions.get<int>("qubits_beam_metric_ones_idx");

  int num_metric_bits = precision_bits.size();

  for (int i = 0; i < num_metric_bits; i++) {

    // Square a_i
    std::vector<int> buffer;
    for (int j = 0; j < precision_bits[i]; j++) {
      buffer.push_back(qubits_ancilla[j]);
    }
    int bits_so_far = 0;
    for (int j = 0; j < i; j++) {
      bits_so_far += precision_bits[j];
    }
    std::vector<int> current_eval_bits;
    for (int j = 0; j < precision_bits[i]; j++) {
      current_eval_bits.push_back(evaluation_bits[bits_so_far + j]);
    }
    for (int j = 0; j < current_eval_bits.size(); j++) {
      addInstruction(gateRegistry->createInstruction(
          "CX", {static_cast<unsigned long>(current_eval_bits[j]),
                 static_cast<unsigned long>(buffer[j])}));
    }
    std::vector<int> amp_squared;
    for (int j = 0; j < 2 * buffer.size(); j++) {
      amp_squared.push_back(qubits_ancilla[precision_bits[i] + j]);
    }
    auto square = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Multiplication"));
    const bool expand_ok_square =
        square->expand({{"qubits_a", current_eval_bits},
                        {"qubits_b", buffer},
                        {"qubits_result", amp_squared},
                        {"qubit_ancilla",
                         qubits_ancilla[precision_bits[i] + 2 * buffer.size()]},
                        {"is_LSB", true}});
    assert(expand_ok_square);
    addInstruction(square);

    // Shift it by 2^i and add to result register
    int min_col_amp_squared = i - 2 * precision_bits[i];
    int min_col_beam_metric = -ones_idx;
    std::vector<int> adder_bits;
    std::vector<int> qubits_beam_metric_target;
    int first_target_idx;
    if (min_col_amp_squared < min_col_beam_metric) {
        for (int j = std::abs(min_col_beam_metric-min_col_amp_squared); j < amp_squared.size(); j++) {
            adder_bits.push_back(amp_squared[j]);
        }
        first_target_idx = 0;
        // for (int j = 0; j < adder_bits.size() + 1; j++) {
        //     qubits_beam_metric_target.push_back(qubits_beam_metric[j]);
        // }
    } else {
        adder_bits = amp_squared;
        // for (int j = 0; j < adder_bits.size() + 1; j++) {
        //     qubits_beam_metric_target.push_back(qubits_beam_metric[j + min_col_amp_squared - min_col_beam_metric]);
        // }
        first_target_idx = std::abs(min_col_amp_squared - min_col_beam_metric);
    }
    for (int j = first_target_idx; j < qubits_beam_metric.size(); j++) {
        qubits_beam_metric_target.push_back(qubits_beam_metric[j]);
    }
    int temp = 1;
    while (adder_bits.size() < qubits_beam_metric_target.size() - 1) {
        adder_bits.push_back(qubits_ancilla[temp + precision_bits[i] + 2*buffer.size()]);
        temp++;
    }

    assert(adder_bits.size() + 1 == qubits_beam_metric_target.size());
    if (adder_bits.size() > 0) {
      auto adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("RippleCarryAdder"));
      const bool expand_ok_adder =
          adder->expand({{"sum_bits", qubits_beam_metric_target},
                         {"adder_bits", adder_bits},
                         {"c_in", qubits_ancilla[qubits_ancilla.size() - 1]}});
      assert(expand_ok_adder);
      addInstruction(adder);
    }

    // Undo squaring of a_i
    auto inv_square = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("InverseCircuit"));
    const bool expand_ok_inv_square =
        inv_square->expand({{"circ", square}});
    assert(expand_ok_inv_square);
    addInstruction(inv_square);

    for (int j = 0; j < current_eval_bits.size(); j++) {
      addInstruction(gateRegistry->createInstruction(
          "CX", {static_cast<unsigned long>(current_eval_bits[j]),
                 static_cast<unsigned long>(buffer[j])}));
    }
  }

  return true;
}

const std::vector<std::string> AEtoMetric::requiredKeys() { return {}; }
} // namespace qb

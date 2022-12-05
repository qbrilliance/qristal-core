/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/phase_estimation.hpp"
#include "qb/core/circuit_builder.hpp"
#include "CommonGates.hpp"
#include "xacc_service.hpp"
#include <assert.h>
#include <optional>
#include "IRProvider.hpp"
#include <vector>
#include <string>

namespace qb {

bool PhaseEstimation::expand(const xacc::HeterogeneousMap &runtimeOptions) {
  // Inputs:
  // num_evaluation_qubits: number of evaluation qubits
  // unitary: The unitary circuit which will be repeated and controlled.
  if (!runtimeOptions.keyExists<int>("num_evaluation_qubits")) {
    return false;
  }
  const auto num_evaluation_qubits =
      runtimeOptions.get<int>("num_evaluation_qubits");
  if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
          "unitary")) {
    return false;
  }
  auto unitary = xacc::as_shared_ptr(
      runtimeOptions.getPointerLike<xacc::CompositeInstruction>("unitary"));
  assert(unitary->nInstructions() > 0);
  assert(num_evaluation_qubits > 0);
  // If a qubit ordering is not provided, assume the state qubits: 0 -> (n - 1), where n is the number of qubits of the unitary
  const auto qubits_state_reg = qb::uniqueBitsQD(unitary);
  assert(qubits_state_reg.size() > 0);

  std::vector<int> trial_qubits = {};
  if (runtimeOptions.keyExists<std::vector<int>>("trial_qubits")) {
      trial_qubits = runtimeOptions.get<std::vector<int>>("trial_qubits");
  }
  if (trial_qubits.size() == 0) {
      for (auto i : qubits_state_reg) {
          trial_qubits.push_back(i);
      }
  }

  std::vector<int> evaluation_qubits = {};
  if (runtimeOptions.keyExists<std::vector<int>>("evaluation_qubits")) {
      evaluation_qubits = runtimeOptions.get<std::vector<int>>("evaluation_qubits");
  }
  if (evaluation_qubits.size() == 0) {
      int j = 0;
      while (evaluation_qubits.size() < num_evaluation_qubits) {
      if (std::find(trial_qubits.begin(), trial_qubits.end(), j) == trial_qubits.end()) {
          evaluation_qubits.push_back(j);
      }
      j++;
      }
  }


  // Apply hadamards on evaluation qubits
  for (int i = 0; i < num_evaluation_qubits; ++i) {
    const auto qb_idx = evaluation_qubits[i];
    addInstruction(std::make_shared<xacc::quantum::Hadamard>(qb_idx));
  }
  // Controlled powers
  // Handle optional 'global-phase' settings:
  const std::optional<double> global_phase =
      runtimeOptions.keyExists<double>("global-phase")
          ? runtimeOptions.get<double>("global-phase")
          : std::optional<double>();
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  for (int i = 0; i < num_evaluation_qubits; ++i) {
    const int ctrl_qb_idx = evaluation_qubits[i];
    const auto power = 1 << i;
    for (int j = 0; j < power; ++j) {
      auto controlled_U = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      controlled_U->expand({{"U", unitary}, {"control-idx", ctrl_qb_idx}});
      addInstruction(controlled_U);
      if (global_phase.has_value()) {
        //addInstruction(gateRegistry->createInstruction("Z", ctrl_qb_idx));
         addInstruction(gateRegistry->createInstruction(
             "Rz", {static_cast<size_t>(ctrl_qb_idx)}, {global_phase.value()}));
      }
    }
  }

  // IQFT on evaluation_qubits register
  auto iqft = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("iqft"));
  iqft->expand({{"nq", num_evaluation_qubits}});

  // Need to remap qubit operands of these instruction to the evaluation_qubits
  // register range
  for (auto &inst : iqft->getInstructions()) {
    auto bits = inst->bits();
    for (auto &bit : bits) {
      bit = evaluation_qubits[bit];
    }
    auto new_inst = inst->clone();
    new_inst->setBits(bits);
    addInstruction(new_inst);
  }
  return true;
}

const std::vector<std::string> PhaseEstimation::requiredKeys() {
  return {"num_evaluation_qubits", "unitary"};
}
} // namespace qb

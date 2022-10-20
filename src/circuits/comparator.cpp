/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/comparator.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <optional>
#include <vector>

namespace qbOS {
bool Comparator::expand(const xacc::HeterogeneousMap &runtimeOptions) {
  // Inputs:
  // BestScore: the current best score that we want to compare to
  // num_scoring_qubits: the number of qubits used to encode a score, n
  // trial_score_qubits: the indices of the qubits encoding the trial score
  // (default 0 to n-1) flag_qubit: the index of the flag qubit (default n)
  // best_score_qubits: the indices of the qubits encoding the best score
  // (default n+1 to 2n) ancilla_qubits: the indices of the ancilla qubits
  // (defualt 2n+1 to 5n-2) as_oracle: [bool] convert the comparator to an
  // oracle via phase kickback? (defualt False) is_LSB: [bool] is the input
  // trial score in LSB qubit ordering? (default True)
  if (!runtimeOptions.keyExists<int>("BestScore")) {
    return false;
  }
  int BestScore = runtimeOptions.get<int>("BestScore");
  assert(BestScore >= 0);

  if (!runtimeOptions.keyExists<int>("num_scoring_qubits")) {
    return false;
  }
  const int n = runtimeOptions.get<int>("num_scoring_qubits");
  assert(n > 0);

  std::vector<int> trial_score_qubits = {};
  int flag_qubit = -1;
  std::vector<int> best_score_qubits = {};
  std::vector<int> ancilla_qubits = {};

  if (runtimeOptions.keyExists<std::vector<int>>("trial_score_qubits")) {
    trial_score_qubits =
        runtimeOptions.get<std::vector<int>>("trial_score_qubits");
  }
  if (runtimeOptions.keyExists<int>("flag_qubit")) {
    flag_qubit = runtimeOptions.get<int>("flag_qubit");
  }
  if (runtimeOptions.keyExists<std::vector<int>>("best_score_qubits")) {
    best_score_qubits =
        runtimeOptions.get<std::vector<int>>("best_score_qubits");
  }
  if (runtimeOptions.keyExists<std::vector<int>>("ancilla_qubits")) {
    ancilla_qubits = runtimeOptions.get<std::vector<int>>("ancilla_qubits");
  }
  bool as_oracle = false;
  if (runtimeOptions.keyExists<bool>("as_oracle")) {
    as_oracle = runtimeOptions.get<bool>("as_oracle");
  }
  bool is_LSB = true;
  if (runtimeOptions.keyExists<bool>("is_LSB")) {
    is_LSB = runtimeOptions.get<bool>("is_LSB");
  }
  if (ancilla_qubits.size() == 3 * n - 2 && flag_qubit != -1) {
    ancilla_qubits.insert(ancilla_qubits.begin(), flag_qubit);
  }
  std::vector<int> controls_on;
  std::vector<int> controls_off;
  if (runtimeOptions.keyExists<std::vector<int>>("controls_on")) {
    controls_on = runtimeOptions.get<std::vector<int>>("controls_on");
  }
  if (runtimeOptions.keyExists<std::vector<int>>("controls_off")) {
    controls_off = runtimeOptions.get<std::vector<int>>("controls_off");
  }
  bool controlled = controls_on.size() > 0 || controls_off.size() > 0;

  // Total number of qubits in the circuit will be 5n - 1
  // If default, register assumed to be of the form
  // |TrialState>|flag>|BestScore>|ancilla> where |flag> is an ancilla qubit
  // that will be |1> if TrialScore > BestScore and |0> otherwise
  if (as_oracle) {
    if (trial_score_qubits.size() == 0) {
      for (int i = 0; i < n; i++) {
        trial_score_qubits.push_back(i);
      }
    }
    assert(trial_score_qubits.size() == n);
    if (best_score_qubits.size() == 0) {
      for (int i = 0; i < n; i++) {
        best_score_qubits.push_back(n + i);
      }
    }
    assert(best_score_qubits.size() == n);
    if (ancilla_qubits.size() == 0) {
      for (int i = 0; i < 3 * n - 1; i++) {
        ancilla_qubits.push_back(2 * n + i);
      }
    }
    assert(ancilla_qubits.size() == 3 * n - 1);
    if (flag_qubit == -1) {
      flag_qubit = 5 * n - 1;
    }
  }
  if (!as_oracle) {
    if (trial_score_qubits.size() == 0) {
      for (int i = 0; i < n; i++) {
        trial_score_qubits.push_back(i);
      }
    }
    if (flag_qubit == -1) {
      flag_qubit = n;
    }
    if (best_score_qubits.size() == 0) {
      for (int i = 0; i < n; i++) {
        best_score_qubits.push_back(n + 1 + i);
      }
    }
    assert(best_score_qubits.size() == n);
    if (ancilla_qubits.size() == 0) {
      for (int i = 0; i < 3 * n - 2; i++) {
        ancilla_qubits.push_back(2 * n + 1 + i);
      }
      ancilla_qubits.insert(ancilla_qubits.begin(), flag_qubit);
    }
    assert(ancilla_qubits.size() == 3 * n - 1);
  }

  if (is_LSB) {
    reverse(trial_score_qubits.begin(), trial_score_qubits.end());
  }

  // Encode BestScore as a bitstring
  std::string BestScore_binary =
      std::bitset<sizeof(BestScore)>(BestScore).to_string();
  std::string BestScore_binary_n = BestScore_binary.substr(
      BestScore_binary.size() < n ? 0 : BestScore_binary.size() - n);

  // Prepare |BestScore>
  for (int i = 0; i < n; i++) {
    if (BestScore_binary_n[i] == '1') {
      addInstruction(std::make_shared<xacc::quantum::X>(best_score_qubits[i]));
    }
  }

  // Perform Comparator
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  if (as_oracle == true) {
    addInstruction(gateRegistry->createInstruction("X", flag_qubit));
    addInstruction(gateRegistry->createInstruction("H", flag_qubit));
  }

  for (int i = 0; i < n; i++) {
    addInstruction(std::make_shared<xacc::quantum::X>(best_score_qubits[i]));
  }

  for (int i = 0; i < n; i++) {
    auto controlled_U_2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    auto cnot_gate_2 = gateRegistry->createComposite("cnot_gate_2");
    cnot_gate_2->addInstruction(gateRegistry->createInstruction(
        "X", ancilla_qubits[i]));
    std::vector<int> controls2 = {trial_score_qubits[i], best_score_qubits[i]};
    controlled_U_2->expand(
        {{"U", cnot_gate_2}, {"control-idx", controls2}});
    addInstruction(controlled_U_2);
  }

  for (int i = 0; i < n; i++) {
    addInstruction(std::make_shared<xacc::quantum::X>(best_score_qubits[i]));
  }

  for (int i = 0; i < n; i++) {
    addInstruction(std::make_shared<xacc::quantum::X>(trial_score_qubits[i]));
  }

  for (int i = 0; i < n; i++) {
    auto controlled_U_3 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    auto cnot_gate_3 = gateRegistry->createComposite("cnot_gate_3");
    cnot_gate_3->addInstruction(gateRegistry->createInstruction(
        "X", ancilla_qubits[n + i]));
    std::vector<int> controls3 = {trial_score_qubits[i], best_score_qubits[i]};
    controlled_U_3->expand(
        {{"U", cnot_gate_3}, {"control-idx", controls3}});
    addInstruction(controlled_U_3);
  }

  for (int i = 0; i < n; i++) {
    addInstruction(std::make_shared<xacc::quantum::X>(trial_score_qubits[i]));
  }

  for (int i = 0; i < n; i++) {
    addInstruction(std::make_shared<xacc::quantum::X>(ancilla_qubits[i]));
  }

  for (int i = 0; i < n; i++) {
    addInstruction(std::make_shared<xacc::quantum::X>(ancilla_qubits[n + i]));
  }

  for (int i = 0; i < n - 1; i++) {
    auto controlled_U_5 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    auto cnot_gate_5 = gateRegistry->createComposite("cnot_gate_5");
    std::vector<int> controls5 = {ancilla_qubits[i], ancilla_qubits[n+i]};
    cnot_gate_5->addInstruction(gateRegistry->createInstruction(
        "X", ancilla_qubits[2 * n + i]));
    controlled_U_5->expand(
        {{"U", cnot_gate_5}, {"control-idx", controls5}});
    addInstruction(controlled_U_5);
  }

  for (int i = 0; i < n; i++) {
    addInstruction(std::make_shared<xacc::quantum::X>(ancilla_qubits[i]));
  }

  for (int i = 0; i < n; i++) {
    addInstruction(std::make_shared<xacc::quantum::X>(ancilla_qubits[n + i]));
  }

  for (int i = 0; i < n - 1; i++) {
    auto controlled_U_6 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    auto cnot_gate_6 = gateRegistry->createComposite("cnot_gate_6");
    cnot_gate_6->addInstruction(gateRegistry->createInstruction(
        "X", ancilla_qubits[n - 2 - i]));
    std::vector<int> controls6 = {ancilla_qubits[n-1-i], ancilla_qubits[3*n-2-i]};
    controlled_U_6->expand(
        {{"U", cnot_gate_6}, {"control-idx", controls6}});
    addInstruction(controlled_U_6);

    auto controlled_U_7 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    auto cnot_gate_7 = gateRegistry->createComposite("cnot_gate_7");
    cnot_gate_7->addInstruction(gateRegistry->createInstruction(
        "X", ancilla_qubits[2 * n - 2 - i]));
    std::vector<int> controls7 = {ancilla_qubits[2*n-1-i], ancilla_qubits[3*n-2-i]};
    controlled_U_7->expand(
        {{"U", cnot_gate_7}, {"control-idx", controls7}});
    addInstruction(controlled_U_7);
  }

  for (int i = 0; i < n; i++) {
    if (BestScore_binary_n[i] == '1') {
      addInstruction(std::make_shared<xacc::quantum::X>(best_score_qubits[i]));
    }
  }

  if (as_oracle == true) {
    auto mark_flag = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    auto xf = gateRegistry->createComposite("uf");
    xf->addInstruction(gateRegistry->createInstruction("X", flag_qubit));
    std::vector<int> controls = {ancilla_qubits[0]};
    if (controlled) {
      for (int q = 0; q < controls_on.size(); q++) {
        controls.push_back(controls_on[q]);
      }
      for (int q = 0; q < controls_off.size(); q++) {
        controls.push_back(controls_off[q]);
      }
    }
    mark_flag->expand({{"U", xf}, {"control-idx", controls}});
    addInstruction(mark_flag);

    for (int i = 0; i < n; i++) {
      if (BestScore_binary_n[i] == '1') {
        addInstruction(
            std::make_shared<xacc::quantum::X>(best_score_qubits[i]));
      }
    }

    for (int j = 0; j < n - 1; j++) {
      int i = n - 2 - j;
      auto controlled_U_8 =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
      auto cnot_gate_8 = gateRegistry->createComposite("cnot_gate_8");
      cnot_gate_8->addInstruction(gateRegistry->createInstruction(
          "X", ancilla_qubits[n - 2 - i]));
      std::vector<int> controls8 = {ancilla_qubits[n-1-i], ancilla_qubits[3*n-2-i]};
      controlled_U_8->expand(
          {{"U", cnot_gate_8}, {"control-idx", controls8}});
      addInstruction(controlled_U_8);

      auto controlled_U_9 =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
      auto cnot_gate_9 = gateRegistry->createComposite("cnot_gate_9");
      cnot_gate_9->addInstruction(gateRegistry->createInstruction(
          "X", ancilla_qubits[2 * n - 2 - i]));
      std::vector<int> controls9 = {ancilla_qubits[2*n-1-i], ancilla_qubits[3*n-2-i]};
      controlled_U_9->expand(
          {{"U", cnot_gate_9}, {"control-idx", controls9}});
      addInstruction(controlled_U_9);
    }

    for (int i = 0; i < n; i++) {
      addInstruction(std::make_shared<xacc::quantum::X>(ancilla_qubits[i]));
    }

    for (int i = 0; i < n; i++) {
      addInstruction(std::make_shared<xacc::quantum::X>(ancilla_qubits[n + i]));
    }

    for (int j = 0; j < n - 1; j++) {
      int i = n - 2 - j;
      auto controlled_U_10 =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
      auto cnot_gate_10 = gateRegistry->createComposite("cnot_gate_10");
      cnot_gate_10->addInstruction(gateRegistry->createInstruction(
          "X", ancilla_qubits[2 * n + i]));
      std::vector<int> controls10 = {ancilla_qubits[i], ancilla_qubits[n+i]};
      controlled_U_10->expand(
          {{"U", cnot_gate_10}, {"control-idx", controls10}});
      addInstruction(controlled_U_10);
    }

    for (int i = 0; i < n; i++) {
      addInstruction(std::make_shared<xacc::quantum::X>(trial_score_qubits[i]));
    }

    for (int i = 0; i < n; i++) {
      addInstruction(std::make_shared<xacc::quantum::X>(ancilla_qubits[i]));
    }

    for (int i = 0; i < n; i++) {
      addInstruction(std::make_shared<xacc::quantum::X>(ancilla_qubits[n + i]));
    }

    for (int j = 0; j < n; j++) {
      int i = n - 1 - j;
      auto controlled_U_11 =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
      auto cnot_gate_11 = gateRegistry->createComposite("cnot_gate_11");
      cnot_gate_11->addInstruction(gateRegistry->createInstruction(
          "X", ancilla_qubits[n + i]));
      std::vector<int> controls11 = {trial_score_qubits[i], best_score_qubits[i]};
      controlled_U_11->expand(
          {{"U", cnot_gate_11}, {"control-idx", controls11}});
      addInstruction(controlled_U_11);
    }

    for (int i = 0; i < n; i++) {
      addInstruction(std::make_shared<xacc::quantum::X>(best_score_qubits[i]));
    }

    for (int i = 0; i < n; i++) {
      addInstruction(std::make_shared<xacc::quantum::X>(trial_score_qubits[i]));
    }

    for (int j = 0; j < n; j++) {
      int i = n - 1 - j;
      auto controlled_U_12 =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
      auto cnot_gate_12 = gateRegistry->createComposite("cnot_gate_12");
      cnot_gate_12->addInstruction(gateRegistry->createInstruction(
          "X", ancilla_qubits[i]));
      std::vector<int> controls12 = {best_score_qubits[i], trial_score_qubits[i]};
      controlled_U_12->expand(
          {{"U", cnot_gate_12}, {"control-idx", controls12}});
      addInstruction(controlled_U_12);
    }

    for (int i = 0; i < n; i++) {
      addInstruction(std::make_shared<xacc::quantum::X>(best_score_qubits[i]));
    }

    for (int i = 0; i < n; i++) {
      if (BestScore_binary_n[i] == '1') {
        addInstruction(
            std::make_shared<xacc::quantum::X>(best_score_qubits[i]));
      }
    }

    addInstruction(gateRegistry->createInstruction("H", flag_qubit));
    addInstruction(gateRegistry->createInstruction("X", flag_qubit));
  }

  for (int i = 0; i < n; i++) {
    if (BestScore_binary_n[i] == '1') {
      addInstruction(std::make_shared<xacc::quantum::X>(best_score_qubits[i]));
    }
  }

  if (is_LSB) {
    reverse(trial_score_qubits.begin(), trial_score_qubits.end());
  }

  return true;
}

const std::vector<std::string> Comparator::requiredKeys() {
  return {"BestScore", "num_scoring_qubits"};
}

} // namespace qbOS

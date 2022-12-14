/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/q_prime_unitary.hpp"
#include "CommonGates.hpp"
#include "xacc_service.hpp"
//#include <array>
#include <assert.h>
#include <bits/c++config.h>
#include <iostream>
#include <ostream>
//#include <bits/c++config.h>
//#include <cstddef>
//#include <iterator>
//#include <optional>
//#include <ostream>
#include "IRProvider.hpp"

namespace qb {
bool QPrime::expand(const xacc::HeterogeneousMap &runtimeOptions) {
  std::cout << "Defining QPrime" << std::endl;
  // Inputs:
  // Iteration: which letter are we up to
  // qubits_ancilla_metric: ancillary qubits assigned to next letter probability
  // qubits_ancilla_letter: ancillary qubits assigned to next letter
  // qubits_next_metric: qubits assigned to next letter probabilities
  // qubits_next_letter: qubits assigned to next letter
  if (!runtimeOptions.keyExists<int>("iteration")) {
    return false;
  }
  int Iteration = runtimeOptions.get<int>("iteration");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_metric")) {
    return false;
  }
  const auto qubits_metric =
      runtimeOptions.get<std::vector<int>>("qubits_metric");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_string")) {
    return false;
  }
  const auto qubits_string =
      runtimeOptions.get<std::vector<int>>("qubits_string");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_next_metric")) {
    return false;
  }
  const auto qubits_next_metric =
      runtimeOptions.get<std::vector<int>>("qubits_next_metric");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_next_letter")) {
    return false;
  }
  const auto qubits_next_letter =
      runtimeOptions.get<std::vector<int>>("qubits_next_letter");

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  // Assert statements here?
  assert(qubits_metric.size() > 0);
  assert(qubits_string.size() > 0);
  assert(qubits_next_metric.size() > 0);
  assert(qubits_next_letter.size() > 0);

  int num_qubits_next_metric = qubits_next_metric.size();
  std::vector<int> qubits_current_metric = {};
  for (int i = 0; i < num_qubits_next_metric; i++) {
    qubits_current_metric.push_back(qubits_metric[Iteration * num_qubits_next_metric + i]);
  }
  int num_qubits_next_letter = qubits_next_letter.size();
  std::vector<int> qubits_current_letter = {};
  for (int i = 0; i < num_qubits_next_letter; i++) {
    qubits_current_letter.push_back(qubits_string[Iteration * num_qubits_next_letter + i]);
  }

  // Apply CNOT to probability qubits
  for (int qindex = 0; qindex < qubits_current_metric.size(); ++qindex) {
    const std::size_t qubit_current_metric = qubits_current_metric[qindex];
    const std::size_t qubit_next_metric = qubits_next_metric[qindex];
    addInstruction(gateRegistry->createInstruction(
        "CX", std::vector<std::size_t>{qubit_current_metric,
                                       qubit_next_metric}));
    // addInstruction(std::make_shared<xacc::quantum::CNOT>(qubit_ancilla_prob,qubit_next_letter_probabilities));
  }
  // Apply CNOT to letter qubits
  for (int qindex = 0; qindex < qubits_current_letter.size(); ++qindex) {
    const std::size_t qubit_current_letter = qubits_current_letter[qindex];
    const std::size_t qubit_next_letter = qubits_next_letter[qindex];
    addInstruction(gateRegistry->createInstruction(
        "CX",
        std::vector<std::size_t>{qubit_current_letter, qubit_next_letter}));
    // addInstruction(std::make_shared<xacc::quantum::CNOT>(qubit_ancilla_letter,qubit_next_letter));
  }

  return true;
}

const std::vector<std::string> QPrime::requiredKeys() {
  return {"qubits_metric", "qubits_string",
             "qubits_next_metric", "qubits_next_letter"};
}
} // namespace qb

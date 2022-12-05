/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/uq_prime_unitary.hpp"
#include "CommonGates.hpp"
#include "xacc_service.hpp"
//#include <array>
#include <assert.h>
#include <bits/c++config.h>
#include <iostream>
#include <ostream>
//#include <cstddef>
//#include <iterator>
//#include <optional>
//#include <ostream>
#include "IRProvider.hpp"

namespace qb{
bool UQPrime::expand(const xacc::HeterogeneousMap &runtimeOptions) {
  std::cout << "Defining UQPrime" << std::endl;
  // Inputs:
  // qubits_ancilla_prob: ancillary qubits assigned to next letter probability
  // qubits_ancilla_letter: ancillary qubits assigned to next letter
  // qubits_next_letter_probabilities: qubits assigned to next letter probabilities
  // qubits_next_letter: qubits assigned to next letter
  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla_metric")) {
    return false;
  }
  const auto qubits_ancilla_metric =
      runtimeOptions.get<std::vector<int>>("qubits_ancilla_metric");


  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla_letter")) {
    return false;
  }
  const auto qubits_ancilla_letter =
      runtimeOptions.get<std::vector<int>>("qubits_ancilla_letter");


  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_next_letter_metric")) {
    return false;
  }
  const auto qubits_next_letter_metric =
      runtimeOptions.get<std::vector<int>>("qubits_next_letter_metric");


  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_next_letter")) {
    return false;
  }
  const auto qubits_next_letter =
      runtimeOptions.get<std::vector<int>>("qubits_next_letter");

  std::cout << "gateregistry\n";

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  // Assert statements here?
  assert(qubits_ancilla_metric.size() > 0);
  assert(qubits_ancilla_letter.size() > 0);
  assert(qubits_next_letter_metric.size() > 0);
  assert(qubits_next_letter.size() > 0);

  // Apply CNOT to probability qubits
  for (int qindex = 0; qindex < qubits_ancilla_metric.size(); ++qindex) {
    const std::size_t qubit_ancilla_metric = qubits_ancilla_metric[qindex];
    const std::size_t qubit_next_letter_metric = qubits_next_letter_metric[qindex];
    const std::size_t qubit_ancilla_letter = qubits_ancilla_letter[qindex];
    const std::size_t qubit_next_letter = qubits_next_letter[qindex];
    addInstruction(gateRegistry->createInstruction("CX",std::vector<std::size_t>
        {qubit_next_letter_metric,qubit_ancilla_metric}));
    addInstruction(std::make_shared<xacc::quantum::CNOT>(qubit_ancilla_metric,qubit_next_letter_metric));
  }
  // Apply CNOT to letter qubits
  for (int qindex = 0; qindex < qubits_ancilla_letter.size(); ++qindex) {
    const std::size_t qubit_ancilla_letter = qubits_ancilla_letter[qindex] ;
    const std::size_t qubit_next_letter = qubits_next_letter[qindex];
    addInstruction(gateRegistry->createInstruction("CX",std::vector<std::size_t> {qubit_next_letter,qubit_ancilla_letter}));
    addInstruction(std::make_shared<xacc::quantum::CNOT>(qubit_ancilla_letter,qubit_next_letter));
  }

  return true;

}

const std::vector<std::string> UQPrime::requiredKeys() {
    return {"qubits_ancilla_metric", "qubits_ancilla_letter",
        "qubits_next_letter_metric", "qubits_next_letter"};
}

} // namespace qb

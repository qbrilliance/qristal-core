// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/circuits/u_prime_unitary.hpp"
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>
#include <iostream>


TEST(UPrimeTester_1, checkSimple) {
  std::cout << "UPrimeTester1:\n";
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  //auto oracle = gateRegistry->createComposite("T_oracle");
  //oracle->addInstruction(gateRegistry->createInstruction("T", 0));
  const std::vector<int> qubits_ancilla_prob = {0,1};
  const std::vector<int> qubits_ancilla_letter = {2,3};
  const std::vector<int> qubits_next_letter_metric = {4,5};
  const std::vector<int> qubits_next_letter = {6,7};
  int key[8];
  for (int qindex = 0; qindex < 2; qindex++){
    key[qindex] = qubits_ancilla_prob[qindex];
    key[qindex+2] = qubits_ancilla_letter[qindex];
    key[qindex+4] = qubits_next_letter_metric[qindex];
    key[qindex+6] = qubits_next_letter[qindex];
  }
  const int *max_key = std::max_element(std::begin(key), std::end(key));
  //int *max_key = std::max_element(std::begin(key), std::end(key));
  //const int max_key = key.max();
  //const int min_key = min(key);
  //const int offset_key = max_key + min_key;

  auto u_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("UPrime"));
  std::cout << "expand\n";
  //const bool expand_ok =
  xacc::HeterogeneousMap map = {{"iteration",0},{"qubits_metric", qubits_ancilla_prob}, {"qubits_string", qubits_ancilla_letter},
        {"qubits_next_metric", qubits_next_letter_metric},{"qubits_next_letter", qubits_next_letter}};
  u_prime->expand(map);
  //EXPECT_TRUE(expand_ok);
  // Simulation test:
  // Construct the full circuit, include state prep (eigen state of |1>);
  std::cout << "u_prime_test\n";
  auto u_prime_test = gateRegistry->createComposite("sim_uprime");
  int qubit_next_letter_probabilities;
  for (int qindex = 0; qindex < std::size(qubits_ancilla_prob) ; qindex++){
      qubit_next_letter_probabilities = qubits_next_letter_metric[qindex];
      u_prime_test->addInstruction(gateRegistry->createInstruction("X", qubit_next_letter_probabilities));
  }
  u_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_next_letter[0]));
  // Add phase estimation:
  u_prime_test->addInstructions(u_prime->getInstructions());
  // Measure evaluation qubits:
  for (int i = 0; i <= *max_key; ++i) {
    u_prime_test->addInstruction(gateRegistry->createInstruction("Measure", i));
  }
  std::cout << "HOWDY: UPrime circuit:\n";
  std::cout << u_prime_test->toString() << '\n';
  // Sim:
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(*max_key + 1);
  acc->execute(buffer, u_prime_test);
  buffer->print();
  std::cout << u_prime_test->toString() << '\n';
  //std::cout << u_prime->toString() << '\n';
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
  EXPECT_EQ(buffer->getMeasurementCounts()["11101110"], 1024);
}

TEST(UPrimeTester_2, checkSimple) {
  std::cout << "UPrimeTester2:\n";
  // Test QPE: Oracle(|State>) = exp(i*Phase)*|State>
  // and we need to estimate that Phase value.
  // The Oracle in this case is a T gate and the eigenstate is |1>
  // i.e. T|1> = exp(i*pi/4)|1>
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  //auto oracle = gateRegistry->createComposite("T_oracle");
  //oracle->addInstruction(gateRegistry->createInstruction("T", 0));
  const std::vector<int> qubits_ancilla_prob = {0,1};
  const std::vector<int> qubits_ancilla_letter = {2,3};
  const std::vector<int> qubits_next_letter_probabilities = {4,5};
  const std::vector<int> qubits_next_letter = {6,7};
  int key[8];
  for (int qindex = 0; qindex < 2; qindex++){
    key[qindex] = qubits_ancilla_prob[qindex];
    key[qindex+2] = qubits_ancilla_letter[qindex];
    key[qindex+4] = qubits_next_letter_probabilities[qindex];
    key[qindex+6] = qubits_next_letter[qindex];
  }
  const int *max_key = std::max_element(std::begin(key), std::end(key));
  //int *max_key = std::max_element(std::begin(key), std::end(key));
  //const int max_key = key.max();
  //const int min_key = min(key);
  //const int offset_key = max_key + min_key;

  auto u_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("UPrime"));const bool expand_ok = u_prime->expand(
      {{"iteration",0},{"qubits_metric", qubits_ancilla_prob}, {"qubits_string", qubits_ancilla_letter},
        {"qubits_next_metric", qubits_next_letter_probabilities},{"qubits_next_letter", qubits_next_letter}});
  EXPECT_TRUE(expand_ok);
  //auto q_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
  //    xacc::getService<xacc::Instruction>("QPrime"));
  //auto u_prime_test = std::dynamic_pointer_cast<xacc::quantum::Circuit>(
    //  xacc::getService<xacc::Instruction>("UPrimeTest"));
  //const bool expand_ok = q_prime.expand(
    //  {{"unitary", oracle}, {"num_evaluation_qubits", max_key}});
  //EXPECT_TRUE(expand_ok);
  // Simulation test:
  // Construct the full circuit, include state prep (eigen state of |1>);
  auto u_prime_test = gateRegistry->createComposite("sim_uprime");
  int qubit_next_letter_probabilities;
  for (int qindex = 0; qindex < std::size(qubits_ancilla_prob) ; qindex++){
      qubit_next_letter_probabilities = qubits_next_letter_probabilities[qindex];
      u_prime_test->addInstruction(gateRegistry->createInstruction("X", qubit_next_letter_probabilities));
  }
  u_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_ancilla_letter[0]));
  u_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_next_letter[0]));
  u_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_ancilla_prob[1]));
  // Add phase estimation:
  u_prime_test->addInstructions(u_prime->getInstructions());
  // Measure evaluation qubits:
  for (int i = 0; i <= *max_key; ++i) {
    u_prime_test->addInstruction(gateRegistry->createInstruction("Measure", i));
  }
  std::cout << "HOWDY: UPrime circuit2:\n";
  std::cout << u_prime_test->toString() << '\n';
  // Sim:
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(*max_key + 1);
  acc->execute(buffer, u_prime_test);
  buffer->print();
  std::cout << u_prime_test->toString() << '\n';
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
  EXPECT_EQ(buffer->getMeasurementCounts()["10001110"], 1024);    // 10100100
}



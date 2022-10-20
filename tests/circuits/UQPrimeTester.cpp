// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>
#include "../uq_prime_unitary.hpp"
#include <iostream>
#include <bitset>


TEST(UQPrimeTester_1, checkSimple) {
  std::cout << "UQPrimeTester1:\n";
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
  
  auto uq_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("UQPrime"));
  std::cout << "expand\n";
  //const bool expand_ok = 
  xacc::HeterogeneousMap map = {{"qubits_ancilla_metric", qubits_ancilla_prob}, {"qubits_ancilla_letter", qubits_ancilla_letter},
        {"qubits_next_letter_metric", qubits_next_letter_metric},{"qubits_next_letter", qubits_next_letter}};
  std::cout << "map " <<"\n";
  const bool expand_ok = uq_prime->expand(map);
  EXPECT_TRUE(expand_ok);
  // Simulation test:
  // Construct the full circuit, include state prep (eigen state of |1>);
  std::cout << "uq_prime_test\n";
  auto uq_prime_test = gateRegistry->createComposite("sim_uprime");
  int qubit_next_letter_probabilities;
  for (int qindex = 0; qindex < std::size(qubits_ancilla_prob) ; qindex++){
      qubit_next_letter_probabilities = qubits_next_letter_metric[qindex];
      uq_prime_test->addInstruction(gateRegistry->createInstruction("X", qubit_next_letter_probabilities));
  }
  uq_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_next_letter[0]));
  // Add phase estimation:
  uq_prime_test->addInstructions(uq_prime->getInstructions());
  // Measure evaluation qubits:
  for (int i = 0; i <= *max_key; ++i) {
    uq_prime_test->addInstruction(gateRegistry->createInstruction("Measure", i));
  }
  std::cout << "HOWDY: UPrime circuit:\n";
  std::cout << uq_prime_test->toString() << '\n';
  // Sim:
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(*max_key + 1);
  acc->execute(buffer, uq_prime_test);
  buffer->print();
  std::cout << uq_prime_test->toString() << '\n';
  //std::cout << u_prime->toString() << '\n';
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
  EXPECT_EQ(buffer->getMeasurementCounts()["11100000"], 1024);
}

TEST(UQPrimeTester_2, checkSimple) {
  std::cout << "UQPrimeTester2:\n";
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
  
  auto uq_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("UQPrime"));
  std::cout << "expand\n";
  //const bool expand_ok = 
  xacc::HeterogeneousMap map = {{"qubits_ancilla_metric", qubits_ancilla_prob}, {"qubits_ancilla_letter", qubits_ancilla_letter},
        {"qubits_next_letter_metric", qubits_next_letter_metric},{"qubits_next_letter", qubits_next_letter}};
  std::cout << "map " <<"\n";
  const bool expand_ok = uq_prime->expand(map);
  EXPECT_TRUE(expand_ok);
  // Simulation test:
  // Construct the full circuit, include state prep (eigen state of |1>);
  std::cout << "uq_prime_test\n";
  auto uq_prime_test = gateRegistry->createComposite("sim_uprime");
  int qubit_next_letter_probabilities;
  for (int qindex = 0; qindex < std::size(qubits_ancilla_prob) ; qindex++){
      qubit_next_letter_probabilities = qubits_next_letter_metric[qindex];
      uq_prime_test->addInstruction(gateRegistry->createInstruction("X", qubit_next_letter_probabilities));
  }
  uq_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_next_letter[0]));
  uq_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_ancilla_prob[0]));    // If ancilla not zero then input not reset
  // Add phase estimation:
  uq_prime_test->addInstructions(uq_prime->getInstructions());
  // Measure evaluation qubits:
  for (int i = 0; i <= *max_key; ++i) {
    uq_prime_test->addInstruction(gateRegistry->createInstruction("Measure", i));
  }
  std::cout << "HOWDY: UPrime circuit:\n";
  //std::cout << uq_prime_test->toString() << '\n';
  // Sim:
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(*max_key + 1);
  acc->execute(buffer, uq_prime_test);
  buffer->print();
  std::cout << uq_prime_test->toString() << '\n';
  //std::cout << u_prime->toString() << '\n';
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
  EXPECT_EQ(buffer->getMeasurementCounts()["01101000"], 1024);
}



int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

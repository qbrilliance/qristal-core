// Copyright (c) Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
//#include <array>
#include <gtest/gtest.h>
#include "../q_prime_unitary.hpp"


TEST(QPrimeTester_1, checkSimple) {
  std::cout << "QPrimeTester 1:\n";
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  //auto oracle = gateRegistry->createComposite("T_oracle");
  //oracle->addInstruction(gateRegistry->createInstruction("T", 0));
  const std::vector<int> qubits_ancilla_prob = {0,1};
  const std::vector<int> qubits_ancilla_letter = {2,3};
  const std::vector<int> qubits_next_letter_probabilities = {4,5};
  const std::vector<int> qubits_next_letter = {6,7};
  int key[8];
  std::cout << "QPrimeTester 1:a\n";
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
     
  //auto q_prime = xacc::getService<xacc::CompositeInstruction>("QPrime");
  //auto q_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    //  xacc::getService<xacc::Instruction>("QPrime"));
  std::shared_ptr<xacc::quantum::QPrime> q_prime =
      std::make_shared<xacc::quantum::QPrime>();
  xacc::HeterogeneousMap map = {{"qubits_ancilla_metric", std::vector<int>(qubits_ancilla_prob) }, {"qubits_ancilla_letter", std::vector<int>(qubits_ancilla_letter)},
        {"qubits_next_letter_metric", std::vector<int>(qubits_next_letter_probabilities)},{"qubits_next_letter", std::vector<int>(qubits_next_letter)}};
  //auto q_prime_test = std::dynamic_pointer_cast<xacc::quantum::Circuit>(
    //  xacc::getService<xacc::Instruction>("QPrimeTest"));
  //const bool expand_ok = 
  q_prime->expand(map);
  //EXPECT_TRUE(expand_ok);
  // Simulation test:
  // Construct the full circuit, include state prep (eigen state of |1>);
  auto q_prime_test = gateRegistry->createComposite("sim_qprime");
  int qubit_ancilla_prob;
  int qubit_next_letter_probabilities;
  for (int qindex = 0; qindex < std::size(qubits_ancilla_prob) ; qindex++){
      qubit_ancilla_prob = qubits_ancilla_prob[qindex];
      qubit_next_letter_probabilities = qubits_next_letter_probabilities[qindex];
      q_prime_test->addInstruction(gateRegistry->createInstruction("X", qubit_ancilla_prob));
      q_prime_test->addInstruction(gateRegistry->createInstruction("X", qubit_next_letter_probabilities));
      //circuit->addInstruction(gateRegistry->createInstruction("X", qubit));
  }
  q_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_ancilla_letter[0]));
  q_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_next_letter[0]));
  // Add phase estimation:
  q_prime_test->addInstructions(q_prime->getInstructions());
  // Measure evaluation qubits:
  for (int i = 0; i <= *max_key; ++i) {
    q_prime_test->addInstruction(gateRegistry->createInstruction("Measure", i));
  }
  std::cout << "HOWDY: QPrime circuit:\n";
  std::cout << q_prime_test->toString() << '\n';
  // Sim:
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(*max_key+1);
  acc->execute(buffer, q_prime_test);
  buffer->print();
  std::cout << q_prime_test->toString() << '\n';
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
  EXPECT_EQ(buffer->getMeasurementCounts()["11100000"], 1024);
}


TEST(QPrimeTester_2, checkSimple) {
  std::cout << "QPrimeTester2:\n";
  // Test QPE: Oracle(|State>) = exp(i*Phase)*|State>
  // and we need to estimate that Phase value.
  // The Oracle in this case is a T gate and the eigenstate is |1>
  // i.e. T|1> = exp(i*pi/4)|1>
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
     
  std::shared_ptr<xacc::quantum::QPrime> q_prime =
      std::make_shared<xacc::quantum::QPrime>();
  //auto q_prime = xacc::getService<xacc::CompositeInstruction>("QPrime");
  const bool expand_ok = q_prime->expand(
      {{"qubits_ancilla_metric", std::vector<int>(qubits_ancilla_prob) }, {"qubits_ancilla_letter", std::vector<int>(qubits_ancilla_letter)},
        {"qubits_next_letter_metric", std::vector<int>(qubits_next_letter_metric)},{"qubits_next_letter", std::vector<int>(qubits_next_letter)}});
  //auto q_prime_test = std::dynamic_pointer_cast<xacc::quantum::Circuit>(xacc::getService<xacc::Instruction>("QPrimeTest"));
  //const bool expand_ok = q_prime.expand(
    //  {{"unitary", oracle}, {"num_evaluation_qubits", max_key}});
  //EXPECT_TRUE(expand_ok);
  // Simulation test:
  // Construct the full circuit, include state prep (eigen state of |1>);
  auto q_prime_test = gateRegistry->createComposite("sim_qprime");
  int qubit_ancilla_prob;
  int qubit_next_letter_metric;
  for (int qindex = 0; qindex < std::size(qubits_ancilla_prob) ; qindex++){
      qubit_ancilla_prob = qubits_ancilla_prob[qindex];
      qubit_next_letter_metric = qubits_next_letter_metric[qindex];
      q_prime_test->addInstruction(gateRegistry->createInstruction("X", qubit_ancilla_prob));
      q_prime_test->addInstruction(gateRegistry->createInstruction("X", qubit_next_letter_metric));
      //circuit->addInstruction(gateRegistry->createInstruction("X", qubit));
  }
  q_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_ancilla_letter[0]));
  q_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_next_letter[0]));
  q_prime_test->addInstruction(gateRegistry->createInstruction("X", qubits_ancilla_prob[1]));
  // Add phase estimation:
  q_prime_test->addInstructions(q_prime->getInstructions());
  // Measure evaluation qubits:
  for (int i = 0; i <= *max_key; ++i) {
    q_prime_test->addInstruction(gateRegistry->createInstruction("Measure", i));
  }
  std::cout << "HOWDY: QPrime circuit2:\n";
  std::cout << q_prime_test->toString() << '\n';
  // Sim:
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(*max_key + 1);
  acc->execute(buffer, q_prime_test);
  buffer->print();
  std::cout << q_prime_test->toString() << '\n';
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
  EXPECT_EQ(buffer->getMeasurementCounts()["10100100"], 1024);    // 10100100
}


int main(int argc, char **argv) {
  std::cout << "HOWDY: QPrime circuit:\n";
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

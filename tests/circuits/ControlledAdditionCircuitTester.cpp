// Copyright (c) Quantum Brilliance Pty Ltd
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <Circuit.hpp>
#include <gtest/gtest.h>
////////////////////////
// Other include statements
////////////////////////

// In this test we add q1 = |10> to q2 = |000> conditional on a flag.
// Recall that the register being added in to has to have more qubits than the
// register being added to allow for overflow.

// So, the expected output of q2 is 000 if the flag is off and 100 if the flag is on

TEST(ControlledAdditionCircuitTester, FlagOn) {

  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto test_circ = gateRegistry->createComposite("test_circ");
  std::vector<int> qubits_adder = {0,1};
  std::vector<int> qubits_sum = {
      2,3,4}; // qubit 3 is the overflow qubit required by the ripple carry adder
  int c_in = 5;
  std::vector<int> flag = {6};

  // Prepare initial state
  test_circ->addInstruction(
      gateRegistry->createInstruction("X", qubits_adder[0]));

  // Prepare the flag to be on
  test_circ->addInstruction(gateRegistry->createInstruction("X", flag[0]));

  // Perform the conditional addition
  auto controlled_addition =
      std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("ControlledAddition"));
  const bool expand_ok =
      controlled_addition->expand({{"qubits_adder", qubits_adder},
                                   {"qubits_sum", qubits_sum},
                                   {"flags_on", flag},
                                   {"c_in", c_in}});
  EXPECT_TRUE(expand_ok);
  test_circ->addInstructions(controlled_addition->getInstructions());

  // Measure the final sum
  for (int i = 0; i < qubits_sum.size(); i++) {
    test_circ->addInstruction(
        gateRegistry->createInstruction("Measure", qubits_sum[i]));
  }

  std::cout << test_circ->toString();

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////

  auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(qubits_adder.size() + qubits_sum.size() + 1 + flag.size());
  acc->execute(buffer, test_circ);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////

  buffer->print();
  EXPECT_EQ(buffer->getMeasurementCounts()["100"], 1024);

}

TEST(ControlledAdditionCircuitTester, FlagOff) {

  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto test_circ = gateRegistry->createComposite("test_circ");
  std::vector<int> qubits_adder = {0,1};
  std::vector<int> qubits_sum = {
      2,3,4}; // qubit 2 is the overflow qubit required by the ripple carry adder
  int c_in = 5;
  std::vector<int> flag = {6};

  // Prepare initial state
  test_circ->addInstruction(
      gateRegistry->createInstruction("X", qubits_adder[0]));

  // In this case we want to check we get no addition happening when the flag is off
  // So no flag preparation required

  // Perform the conditional addition
  auto controlled_addition =
      std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("ControlledAddition"));
  const bool expand_ok =
      controlled_addition->expand({{"qubits_adder", qubits_adder},
                                   {"qubits_sum", qubits_sum},
                                   {"flags_on", flag},
                                   {"c_in", c_in}});
  EXPECT_TRUE(expand_ok);
  test_circ->addInstructions(controlled_addition->getInstructions());

  // Measure the final sum
  for (int i = 0; i < qubits_sum.size(); i++) {
    test_circ->addInstruction(
        gateRegistry->createInstruction("Measure", qubits_sum[i]));
  }

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////

  auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(qubits_adder.size() + qubits_sum.size() + 1 + flag.size());
  acc->execute(buffer, test_circ);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////

  buffer->print();
  EXPECT_EQ(buffer->getMeasurementCounts()["000"], 1024);

}


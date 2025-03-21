// Copyright (c) Quantum Brilliance Pty Ltd
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "Circuit.hpp"
#include <gtest/gtest.h>
#include <bitset>
#include <memory>

TEST(CompareBeamOracleGeneralCircuitTester, checkSimple1) {
/////////////////////////////// Define circuit /////////////////////////////////
  int q0 = 0;
  int q1 = 1;
  int q2 = 2;
  std::vector<int> FA = {3,4};
  std::vector<int> FB = {5,6};
  std::vector<int> SA = {7,8,9,10}; //Initialize SA = |0000>
  std::vector<int> SB = {11,12,13,14}; //Initialize SB = |0000>

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("sim");
  //Initialize FA and FB as |11>
  for (int i = 0; i < FA.size(); i++){
    circuit->addInstruction(gateRegistry->createInstruction("X", FA[i]));
    circuit->addInstruction(gateRegistry->createInstruction("X", FB[i]));
  }

  //Beam checker
  auto beam_check = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("CompareBeamOracle"));
  const bool expand_ok = beam_check->expand({{"simplified", false}, {"q0", q0}, {"q1", q1}, {"q2", q2},
                                             {"FA", FA}, {"FB", FB},
                                             {"SA", SA}, {"SB", SB}});
  EXPECT_TRUE(expand_ok);
  circuit->addInstructions(beam_check->getInstructions());

  //Measure flags q0, q1 and q2 to see if conditions are met
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q0));
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q1));
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q2));

/////////////////////////////// Run circuit /////////////////////////////////

  auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(3 + FA.size() + FB.size() + SA.size() + SB.size());
  std::cout<<circuit->toString()<<std::endl;
  buffer->print();
  acc->execute(buffer, circuit);

/////////////////////////////// Check results /////////////////////////////////

  //q1 should be 1 if condition 1 is fulfilled
  //q2 should be 1 if condition 1 is fulfilled
  //q0 should be 1 if both qubits q1 and q2 are 1
  EXPECT_EQ(buffer->getMeasurementCounts()["111"], 1024);
}

TEST(CompareBeamOracleGeneralCircuitTester, checkSimple2) {
/////////////////////////////// Define circuit /////////////////////////////////
  int q0 = 0;
  int q1 = 1;
  int q2 = 2;
  std::vector<int> FA = {3,4};
  std::vector<int> FB = {5,6};
  std::vector<int> SA = {7,8,9,10}; //Initialize SA = |0000>
  std::vector<int> SB = {11,12,13,14}; //Initialize SB = |0000>

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("sim");
  circuit->addInstruction(gateRegistry->createInstruction("X", FA[1])); //Initialize FA as |01>
  for (int i = 0; i < FB.size(); i++){
    circuit->addInstruction(gateRegistry->createInstruction("X", FB[i])); //Initialize FB as |11>
  }

  //Beam checker
  auto beam_check = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("CompareBeamOracle"));
  const bool expand_ok = beam_check->expand({{"simplified", false}, {"q0", q0}, {"q1", q1}, {"q2", q2},
                                             {"FA", FA}, {"FB", FB},
                                             {"SA", SA}, {"SB", SB}});
  EXPECT_TRUE(expand_ok);
  circuit->addInstructions(beam_check->getInstructions());

  //Measure flags q0, q1 and q2 to see if conditions are met
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q0));
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q1));
//  circuit->addInstruction(gateRegistry->createInstruction("Measure", q2));

/////////////////////////////// Run circuit /////////////////////////////////

  auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(3 + FA.size() + FB.size() + SA.size() + SB.size());
//  std::cout<<circuit->toString()<<std::endl;
  acc->execute(buffer, circuit);

/////////////////////////////// Check results /////////////////////////////////

  //q1 should be 1 if condition 1 is fulfilled
  //q2 should be 1 if condition 1 is fulfilled
  //q0 should be 1 if both qubits q1 and q2 are 1
  EXPECT_EQ(buffer->getMeasurementCounts()["00"], 1024);
}

TEST(CompareBeamOracleGeneralCircuitTester, checkSimple3) {
/////////////////////////////// Define circuit /////////////////////////////////
  int q0 = 0;
  int q1 = 1;
  int q2 = 2;
  std::vector<int> FA = {3,4};
  std::vector<int> FB = {5,6};
  std::vector<int> SA = {7,8,9,10};
  std::vector<int> SB = {11,12,13,14};

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("sim");
  circuit->addInstruction(gateRegistry->createInstruction("X", FA[1])); //Initialize FA as |01>
  circuit->addInstruction(gateRegistry->createInstruction("X", FB[1])); //Initialize FB as |01>
  for (int i = 1; i < SA.size(); i++){
    circuit->addInstruction(gateRegistry->createInstruction("X", SA[i])); //Initialize SA as |0111>
    circuit->addInstruction(gateRegistry->createInstruction("X", SB[i])); //Initialize SB as |0111>
  }

  //Beam checker
  auto beam_check = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("CompareBeamOracle"));
  const bool expand_ok = beam_check->expand({{"simplified", false}, {"q0", q0}, {"q1", q1}, {"q2", q2},
                                             {"FA", FA}, {"FB", FB},
                                             {"SA", SA}, {"SB", SB}});
  EXPECT_TRUE(expand_ok);
  circuit->addInstructions(beam_check->getInstructions());

  //Measure flags q0, q1 and q2 to see if conditions are met
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q0));
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q1));
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q2));

/////////////////////////////// Run circuit /////////////////////////////////

  auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(3 + FA.size() + FB.size() + SA.size() + SB.size());
//  std::cout<<circuit->toString()<<std::endl;
  acc->execute(buffer, circuit);
buffer->print();

/////////////////////////////// Check results /////////////////////////////////

  //q1 should be 1 if condition 1 is fulfilled
  //q2 should be 1 if condition 1 is fulfilled
  //q0 should be 1 if both qubits q1 and q2 are 1
  EXPECT_EQ(buffer->getMeasurementCounts()["111"], 1024);
}

TEST(CompareBeamOracleGeneralCircuitTester, checkSimple4) {
/////////////////////////////// Define circuit /////////////////////////////////
  int q0 = 0;
  int q1 = 1;
  int q2 = 2;
  std::vector<int> FA = {3,4};
  std::vector<int> FB = {5,6};
  std::vector<int> SA = {7,8,9,10};
  std::vector<int> SB = {11,12,13,14};

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("sim");
  circuit->addInstruction(gateRegistry->createInstruction("X", FA[1])); //Initialize FA as |01>
  circuit->addInstruction(gateRegistry->createInstruction("X", FB[1])); //Initialize FB as |01>
  for (int i = 0; i < 2; i++){
    circuit->addInstruction(gateRegistry->createInstruction("X", SA[i])); //Initialize SA as |1100>
    circuit->addInstruction(gateRegistry->createInstruction("X", SB[i])); //Initialize SB as |1100>
  }

  //Beam checker
  auto beam_check = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("CompareBeamOracle"));
  const bool expand_ok = beam_check->expand({{"simplified", false}, {"q0", q0}, {"q1", q1}, {"q2", q2},
                                             {"FA", FA}, {"FB", FB},
                                             {"SA", SA}, {"SB", SB}});
  EXPECT_TRUE(expand_ok);
  circuit->addInstructions(beam_check->getInstructions());

  //Measure flags q0, q1 and q2 to see if conditions are met
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q0));
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q1));
  circuit->addInstruction(gateRegistry->createInstruction("Measure", q2));

/////////////////////////////// Run circuit /////////////////////////////////

  auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(3 + FA.size() + FB.size() + SA.size() + SB.size());
//  std::cout<<circuit->toString()<<std::endl;
  acc->execute(buffer, circuit);
//  buffer->print();

/////////////////////////////// Check results /////////////////////////////////

  //q1 should be 1 if condition 1 is fulfilled
  //q2 should be 1 if condition 1 is fulfilled
  //q0 should be 1 if both qubits q1 and q2 are 1
  EXPECT_EQ(buffer->getMeasurementCounts()["111"], 1024);
}


// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/circuit_builder.hpp>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <Circuit.hpp>
#include <GateModifier.hpp>
#include <boost/math/special_functions/next.hpp>
#include <gtest/gtest.h>
#include <memory>

TEST(SuperpositionAdderCircuitTester, check1) {
  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto ae_state_prep_circ = gateRegistry->createComposite("state_prep");

  int q0 = 0;
  int q1 = 1;
  int q2 = 2;
  std::vector<int> qubits_string = {3,4};
  std::vector<int> qubits_metric = {5,6};
  std::vector<int> qubits_superfluous_flags = {7,8};
  std::vector<int> qubits_beam_metric = {9,10,11,12,13,14};
  std::vector<int> qubits_ancilla = {15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36};

  // Let's define the state
  // |string>|metric>|flags> = |00>|11>|11> + |10>|10>|01> + |10>|11>|01> + |11>|01>|01>

  // Strings
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("H", qubits_string[0]));
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("H", qubits_string[1]));

  // Metrics
  auto mcx00a = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx00a->expand(
      {{"target", qubits_metric[0]}, {"controls_off", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx00a);

  auto mcx00b = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx00b->expand(
      {{"target", qubits_metric[1]}, {"controls_off", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx00b);

  auto mcx10a = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx10a->expand(
      {{"target", qubits_metric[1]}, {"controls_off", std::vector<int> {qubits_string[1]}},
      {"controls_on", std::vector<int> {qubits_string[0]}}});
  ae_state_prep_circ->addInstruction(mcx10a);

  auto mcx01a = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx01a->expand(
      {{"target", qubits_metric[0]}, {"controls_off", std::vector<int> {qubits_string[0]}},
      {"controls_on", std::vector<int> {qubits_string[1]}}});
  ae_state_prep_circ->addInstruction(mcx01a);

  auto mcx01b = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx01b->expand(
      {{"target", qubits_metric[1]}, {"controls_off", std::vector<int> {qubits_string[0]}},
      {"controls_on", std::vector<int> {qubits_string[1]}}});
  ae_state_prep_circ->addInstruction(mcx01b);

  auto mcx11a = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx11a->expand(
      {{"target", qubits_metric[1]}, {"controls_on", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx11a);

  // Flags
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("X", qubits_superfluous_flags[1]));

  auto mcx00c = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx00c->expand(
      {{"target", qubits_superfluous_flags[0]}, {"controls_off", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx00c);

  // String -> beam
  auto swap = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("ControlledSwap"));
  std::vector<int> swap_flags_on = {qubits_metric[0], qubits_metric[1], qubits_superfluous_flags[1]};
  std::vector<int> swap_flags_off = {qubits_superfluous_flags[0]};
  swap->expand({{"qubits_a", std::vector<int> {qubits_string[0]}},
                {"qubits_b", std::vector<int> {qubits_string[1]}},
                {"flags_on", swap_flags_on},
                {"flags_off", swap_flags_off}});
  ae_state_prep_circ->addInstruction(swap);

  auto circ = gateRegistry->createComposite("circ");
  circ->addInstructions(ae_state_prep_circ->getInstructions());

  auto ae_adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("SuperpositionAdder"));
  const bool expand_ok_ae = ae_adder->expand({
      {"q0", q0}, {"q1", q1}, {"q2", q2},
      {"qubits_flags", qubits_superfluous_flags},
      {"qubits_string", qubits_string},
      {"qubits_metric", qubits_metric},
      {"ae_state_prep_circ", ae_state_prep_circ},
      {"qubits_ancilla", qubits_ancilla},
      {"qubits_beam_metric", qubits_beam_metric}});
  EXPECT_TRUE(expand_ok_ae);
  circ->addInstructions(ae_adder->getInstructions());

  // Measure
//   for (int i = 0; i < qubits_beam_metric.size(); i++)
//     circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_beam_metric[i]));
  for (int i = 0; i < qubits_string.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_string[i]));
//   for (int i = 0; i < qubits_metric.size(); i++)
//     circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_metric[i]));
  for (int i = 0; i < qubits_superfluous_flags.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_superfluous_flags[i]));

  circ->addInstruction(gateRegistry->createInstruction("Measure", q0));

  for (int i = 9; i < 15; i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", i));

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////
//   std::cout << test_circ->toString() << "\n";
  std::cout << "reached execution\n";
  auto acc = xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(37);
  acc->execute(buffer, circ);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////
  buffer->print();
}

TEST(SuperpositionAdderCircuitTester, check2) {
  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto ae_state_prep_circ = gateRegistry->createComposite("state_prep");

  int q0 = 0;
  int q1 = 1;
  int q2 = 2;
  std::vector<int> qubits_string = {3,4};
  std::vector<int> qubits_metric = {5,6,7};
  std::vector<int> qubits_superfluous_flags = {8,9};
  std::vector<int> qubits_beam_metric = {10,11,12,13,14,15,16};
  std::vector<int> qubits_ancilla;
  for (int q = 17; q < 60; q++) {
      qubits_ancilla.push_back(q);
  }

  // We will set up the state |string>|metric>|flags> =
  // |00>|101>|11> + |10>|011>|01> + |10>|100>|01> + |11>|111>|01>

  // strings
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("H", qubits_string[0]));
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("H", qubits_string[1]));

  // metrics
  auto mcx00a = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx00a->expand({{"target", qubits_metric[0]},
                                {"controls_off", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx00a);

  auto mcx00b = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx00b->expand({{"target", qubits_metric[2]},
                                {"controls_off", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx00b);

  auto mcx10a = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx10a->expand({{"target", qubits_metric[1]},
                                {"controls_off", std::vector<int> {qubits_string[1]}},
                                {"controls_on", std::vector<int> {qubits_string[0]}}});
  ae_state_prep_circ->addInstruction(mcx10a);

  auto mcx10b = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx10b->expand({{"target", qubits_metric[2]},
                                {"controls_off", std::vector<int> {qubits_string[1]}},
                                {"controls_on", std::vector<int> {qubits_string[0]}}});
  ae_state_prep_circ->addInstruction(mcx10b);

  auto mcx01a = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx01a->expand({{"target", qubits_metric[0]},
                                {"controls_off", std::vector<int> {qubits_string[0]}},
                                {"controls_on", std::vector<int> {qubits_string[1]}}});
  ae_state_prep_circ->addInstruction(mcx01a);

  auto mcx11a = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx11a->expand({{"target", qubits_metric[0]},
                                {"controls_on", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx11a);

  auto mcx11b = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx11b->expand({{"target", qubits_metric[1]},
                                {"controls_on", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx11b);

  auto mcx11c = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx11c->expand({{"target", qubits_metric[2]},
                                {"controls_on", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx11c);

  // flags
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("X", qubits_superfluous_flags[1]));

  auto mcx00c = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx00c->expand({{"target", qubits_superfluous_flags[0]},
                                {"controls_off", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx00c);

  // string to beam
  auto swap = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("ControlledSwap"));
  std::vector<int> flags_on_swap = {qubits_metric[0], qubits_superfluous_flags[1]};
  std::vector<int> flags_off_swap = {qubits_metric[1], qubits_metric[2], qubits_superfluous_flags[0]};
  swap->expand({{"qubits_a", std::vector<int> {qubits_string[0]}},
                {"qubits_b", std::vector<int> {qubits_string[1]}},
                {"flags_on", flags_on_swap},
                {"flags_off", flags_off_swap}});
  ae_state_prep_circ->addInstruction(swap);

  auto circ = gateRegistry->createComposite("circ");
  circ->addInstruction(ae_state_prep_circ);

  auto ae_adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("SuperpositionAdder"));
  const bool expand_ok_ae = ae_adder->expand({
      {"q0", q0}, {"q1", q1}, {"q2", q2},
      {"qubits_flags", qubits_superfluous_flags},
      {"qubits_string", qubits_string},
      {"qubits_metric", qubits_metric},
      {"ae_state_prep_circ", ae_state_prep_circ},
      {"qubits_ancilla", qubits_ancilla},
      {"qubits_beam_metric", qubits_beam_metric}});
  EXPECT_TRUE(expand_ok_ae);
  circ->addInstruction(ae_adder);

  // Measure
  for (int i = 0; i < qubits_beam_metric.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_beam_metric[i]));
  for (int i = 0; i < qubits_string.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_string[i]));
  for (int i = 0; i < qubits_metric.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_metric[i]));
  for (int i = 0; i < qubits_superfluous_flags.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_superfluous_flags[i]));

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////
//   std::cout << test_circ->toString() << "\n";
  std::cout << "reached execution\n";
  auto acc = xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(60);
  acc->execute(buffer, circ);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////
  buffer->print();
}


// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/circuit_builder.hpp>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <Circuit.hpp>
#include <GateModifier.hpp>
#include <gtest/gtest.h>
#include <memory>

TEST(MeanValueFinderCircuitTester, checksimple) {
  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circ = gateRegistry->createComposite("circ");

  int indicator = 0;
  std::vector<int> qubits_superposition = {1,2};
  std::vector<int> qubits_mean = {3,4,5,6};
  std::vector<int> qubits_ancilla;
  for (int i = 7; i <= 22; i++)
    qubits_ancilla.push_back(i);

  // Generate state |a> = |00> + |10> + |01> + |11>
  auto ae_state_prep_circ = gateRegistry->createComposite("ae_state_prep_circ");
  for (int i = 0; i < qubits_superposition.size(); i++)
    ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("H", qubits_superposition[i]));

  // Add AE state prep circuit to circ
  circ->addInstructions(ae_state_prep_circ->getInstructions());

  // Entangle the indicator state |i_k> with elements |a_k> of the state |a>.
  auto indicator_state_prep = gateRegistry->createComposite("indicator_state_prep");
  auto isp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  // Set |i_k> = 1 for |a_k> = |11>.
  isp->expand({{"target", indicator}, {"controls_on", qubits_superposition}});
  indicator_state_prep->addInstruction(isp);
  circ->addInstructions(indicator_state_prep->getInstructions());

//  auto indicator_state_prep = gateRegistry->createComposite("indicator_state_prep");
//  auto isp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
//  isp->expand({{"target", indicator},
//               {"controls_on", std::vector<int> {qubits_superposition[0]}},
//               {"controls_off", std::vector<int> {qubits_superposition[1]}}});
//  indicator_state_prep->addInstruction(isp);
//  circ->addInstructions(indicator_state_prep->getInstructions());

  std::shared_ptr<xacc::CompositeInstruction> sp_clone =
      xacc::ir::asComposite(ae_state_prep_circ->clone());
  std::shared_ptr<xacc::CompositeInstruction> ip_clone =
      xacc::ir::asComposite(indicator_state_prep->clone());

  auto mean = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("MeanValueFinder"));
  const bool expand_ok_mean = mean->expand({
      {"qubits_superposition", qubits_superposition},
      {"qubits_superposition_state_prep", sp_clone},
      {"qubits_mean", qubits_mean},
      {"qubits_ancilla", qubits_ancilla},
      {"qubit_indicator", indicator},
      {"qubit_indicator_state_prep", ip_clone}});
  EXPECT_TRUE(expand_ok_mean);
  circ->addInstruction(mean);

  // Measure
  circ->addInstruction(gateRegistry->createInstruction("Measure", indicator));
  for (int i = 0; i < qubits_superposition.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_superposition[i]));
  for (int i = 0; i < qubits_mean.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_mean[i]));

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////
  std::cout << "arrived at execute\n";
  auto acc = xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
  auto buffer = xacc::qalloc((int)qubits_ancilla[qubits_ancilla.size()-1]);
  acc->execute(buffer, circ);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////
  buffer->print();
}


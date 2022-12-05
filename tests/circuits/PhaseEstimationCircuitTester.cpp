// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "Circuit.hpp"
#include <gtest/gtest.h>
#include <vector>

TEST(PhaseEstimationCircuitTester, checkSimple) {
  // Test QPE: Oracle(|State>) = exp(i*Phase)*|State>
  // and we need to estimate that Phase value.
  // The Oracle in this case is a T gate and the eigenstate is |1>
  // i.e. T|1> = exp(i*pi/4)|1>
  const int bits_precision = 3;
  std::vector<int> evaluation_qubits = {0,1,3};
  std::vector<int> state_qubits = {2};
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto oracle = gateRegistry->createComposite("T_oracle");
  oracle->addInstruction(gateRegistry->createInstruction("T", state_qubits[0]));
  auto qpe = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("PhaseEstimation"));
  const bool expand_ok = qpe->expand(
      {{"unitary", oracle}, {"num_evaluation_qubits", bits_precision}, {"evaluation_qubits", evaluation_qubits}, {"trial_qubits", state_qubits}});
  EXPECT_TRUE(expand_ok);
  // Simulation test:
  // Construct the full circuit, include state prep (eigen state of |1>);
  auto circuit = gateRegistry->createComposite("sim_qpe");
  // State prep: eigen state of the oracle
  circuit->addInstruction(gateRegistry->createInstruction("X", state_qubits[0]));
  // Add phase estimation:
  circuit->addInstructions(qpe->getInstructions());
  // Measure evaluation qubits:
  for (int i = 0; i < bits_precision; ++i) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", evaluation_qubits[i]));
  }
  std::cout << "HOWDY: QPE circuit:\n";
  std::cout << circuit->toString() << '\n';
  // Sim:
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(bits_precision + 1);
  acc->execute(buffer, circuit);
  buffer->print();
  // EXPECTED: only "100" bitstring
  // phi_est = 1/8 (denom = 8 since we have 3 bits)
  // => phi = 2pi * 1/8 = pi/4 (the expected phase of T gate)
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
  EXPECT_EQ(buffer->getMeasurementCounts()["100"], 1024);
}

TEST(PhaseEstimationCircuitTester, checkGeneralRotationOracle) {
  // Test QPE: Oracle(|State>) = exp(i*Phase)*|State>
  // and we need to estimate that Phase value.
  // The Oracle in this case is a general U1 rotation
  // i.e. U1(theta)|1> = exp(i*theta)|1>
  // Test value: -5pi/8
  const double theta = -5.0 * M_PI / 8.0;
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto oracle = gateRegistry->createComposite("U1_oracle");
  oracle->addInstruction(gateRegistry->createInstruction("U1", {0}, {theta}));
  // Use more bits for precision
  const int bits_precision = 4;
  auto qpe = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("PhaseEstimation"));
  const bool expand_ok = qpe->expand(
      {{"unitary", oracle}, {"num_evaluation_qubits", bits_precision}});
  EXPECT_TRUE(expand_ok);
  // Simulation test:
  // Construct the full circuit, include state prep (eigen state of |1>);
  auto circuit = gateRegistry->createComposite("sim_qpe");
  // State prep: eigen state of the oracle
  circuit->addInstruction(gateRegistry->createInstruction("X", 0));
  // Add phase estimation:
  circuit->addInstructions(qpe->getInstructions());
  // Measure evaluation qubits:
  for (int i = 0; i < bits_precision; ++i) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", i + 1));
  }
  std::cout << "HOWDY: QPE circuit:\n";
  std::cout << circuit->toString() << '\n';
  // Sim:
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(bits_precision + 1);
  acc->execute(buffer, circuit);
  buffer->print();
  // Expected to get 4 bits of 1101 (LSB) = 11(decimal):
  // phi_est = 11/16 (denom = 16 since we have 4 bits)
  // => phi = 2pi * 11/16 = 11pi/8 = 2pi - 5pi/8
  // i.e. we estimate the -5*pi/8 angle...
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 1);
  EXPECT_EQ(buffer->getMeasurementCounts()["1101"], 1024);
}


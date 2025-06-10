// Copyright (c) Quantum Brilliance Pty Ltd
#include <Circuit.hpp>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <gtest/gtest.h>

TEST(AmplitudeAmplificationTester, checkSimple) {
  // Grover to search for '11' state
  // => oracle == CZ gate since CZ |11> = -|11>
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto oracle = gateRegistry->createComposite("cz_oracle");
  oracle->addInstruction(gateRegistry->createInstruction("CZ", {0, 1}));
  auto amplitude_amplification =
      std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("AmplitudeAmplification"));
  const bool expand_ok = amplitude_amplification->expand({{"oracle", oracle}});
  EXPECT_TRUE(expand_ok);
  std::cout << "HOWDY:\n" << amplitude_amplification->toString() << "\n";
  for (int i = 0; i < 2; ++i) {
    amplitude_amplification->addInstruction(
        gateRegistry->createInstruction("Measure", i));
  }
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(2);
  acc->execute(buffer, amplitude_amplification);
  buffer->print();
  // Amplify the expected state (11)
  EXPECT_TRUE(buffer->getMeasurementCounts()["11"] = 1024);
}

TEST(AmplitudeAmplificationTester, checkThreeQubits) {
  // Checking the result of this paper:
  // Complete 3-Qubit Grover search on a programmable quantum computer
  // https://www.nature.com/articles/s41467-017-01904-7
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto oracle = gateRegistry->createComposite("cz_cz_oracle");
  // Oracle contains 2 CZ gates
  oracle->addInstruction(gateRegistry->createInstruction("CZ", {0, 2}));
  oracle->addInstruction(gateRegistry->createInstruction("CZ", {1, 2}));
  auto amplitude_amplification =
      std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("AmplitudeAmplification"));
  const bool expand_ok = amplitude_amplification->expand({{"oracle", oracle}});
  EXPECT_TRUE(expand_ok);
  //std::cout << "HOWDY:\n" << amplitude_amplification->toString() << "\n";
  for (int i = 0; i < 3; ++i) {
    amplitude_amplification->addInstruction(
        gateRegistry->createInstruction("Measure", i));
  }
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(3);
  acc->execute(buffer, amplitude_amplification);
  buffer->print();
  // Amplify only 2 states
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 2);
}


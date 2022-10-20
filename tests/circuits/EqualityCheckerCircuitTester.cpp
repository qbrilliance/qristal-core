// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <bitset>
#include <gtest/gtest.h>
#include <memory>

TEST(EqualityCheckerCircuitTester, noancilla) {
  // Test Equality Checker
  // Compare all pairs of bit strings of length 3

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  std::vector<int> qubits_a = {0, 1, 2};
  std::vector<int> qubits_b = {3, 4, 5};
  int flag = 6;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      auto circuit = gateRegistry->createComposite("sim");
      // Prepare states a = i, b = j
      std::string bin_i = std::bitset<3>(i).to_string();
      std::string bin_i_3 =
          bin_i.substr(bin_i.size() < 3 ? 0 : bin_i.size() - 3);
      std::string bin_j = std::bitset<3>(j).to_string();
      std::string bin_j_3 =
          bin_j.substr(bin_j.size() < 3 ? 0 : bin_j.size() - 3);
      for (int k = 0; k < 3; k++) {
        if (bin_i[k] == '1') {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", qubits_a[k]));
        }
        if (bin_j[k] == '1') {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", qubits_b[k]));
        }
      }
      // Equality checker
      auto eq = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EqualityChecker"));
      const bool expand_ok = eq->expand(
          {{"qubits_a", qubits_a}, {"qubits_b", qubits_b}, {"flag", flag}});
      EXPECT_TRUE(expand_ok);
      circuit->addInstructions(eq->getInstructions());
      // Measure flag
      circuit->addInstruction(gateRegistry->createInstruction("Measure", flag));
      // Run
      auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
      auto buffer = xacc::qalloc(7);
      acc->execute(buffer, circuit);
      // Check results
      if (i == j) {
        EXPECT_EQ(buffer->getMeasurementCounts()["1"], 1024);
      } else {
        EXPECT_EQ(buffer->getMeasurementCounts()["0"], 1024);
      }
    }
  }
}

TEST(EqualityCheckerCircuitTester, ancilla) {
  // Same test but using ancilla

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  std::vector<int> qubits_a = {0, 1, 2};
  std::vector<int> qubits_b = {3, 4, 5};
  int flag = 6;
  std::vector<int> qubits_ancilla = {7, 8};
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      auto circuit = gateRegistry->createComposite("sim");
      // Prepare states a = i, b = j
      std::string bin_i = std::bitset<3>(i).to_string();
      std::string bin_i_3 =
          bin_i.substr(bin_i.size() < 3 ? 0 : bin_i.size() - 3);
      std::string bin_j = std::bitset<3>(j).to_string();
      std::string bin_j_3 =
          bin_j.substr(bin_j.size() < 3 ? 0 : bin_j.size() - 3);
      for (int k = 0; k < 3; k++) {
        if (bin_i[k] == '1') {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", qubits_a[k]));
        }
        if (bin_j[k] == '1') {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", qubits_b[k]));
        }
      }
      // Equality checker
      auto eq = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EqualityChecker"));
      const bool expand_ok = eq->expand({{"qubits_a", qubits_a},
                                         {"qubits_b", qubits_b},
                                         {"flag", flag},
                                         {"use_ancilla", true},
                                         {"qubits_ancilla", qubits_ancilla}});
      EXPECT_TRUE(expand_ok);
      circuit->addInstructions(eq->getInstructions());
      // Measure flag
      circuit->addInstruction(gateRegistry->createInstruction("Measure", flag));
      // Run
      auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
      auto buffer = xacc::qalloc(9);
      acc->execute(buffer, circuit);
      // Check results
      if (i == j) {
        EXPECT_EQ(buffer->getMeasurementCounts()["1"], 1024);
      } else {
        EXPECT_EQ(buffer->getMeasurementCounts()["0"], 1024);
      }
    }
  }
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}
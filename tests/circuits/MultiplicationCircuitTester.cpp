// Copyright (c) Quantum Brilliance Pty Ltd
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <Circuit.hpp>
#include <bitset>
#include <gtest/gtest.h>
#include <memory>

TEST(MultiplicationCircuitTester, check_integer1) {
  // Helper functions
  // int to binary function
  std::function<std::string(int, int)> binary = [&](int i, int num_qubits) {
    std::string i_binary = std::bitset<8 * sizeof(i)>(i).to_string();
    std::string i_binary_n = i_binary.substr(
        i_binary.size() < num_qubits ? 0 : i_binary.size() - num_qubits);
    reverse(i_binary_n.begin(), i_binary_n.end());
    return i_binary_n;
  };

  // binary to int function
  std::function<int(std::string)> integer = [&](std::string str) {
    reverse(str.begin(), str.end());
    int integer = std::stoi(str, 0, 2);
    return integer;
  };

  /////////////////////////////// Define circuit ////////////////////////////////////
  std::vector<int> qubits_a = {0, 1};            // n qubits
  std::vector<int> qubits_b = {2, 3};            // n qubits
  std::vector<int> qubits_result = {4, 5, 6, 7}; // 2n qubits
  int qubit_ancilla = 8;                         // 1 qubit
  bool is_LSB = true;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
      auto circuit = gateRegistry->createComposite("circuit");

      std::string a_bin = binary(i, (int)qubits_a.size());
      std::string b_bin = binary(j, (int)qubits_b.size());

      for (int k = 0; k < a_bin.size(); k++) {
        char b = a_bin[k];
        if (b == '1') {
          circuit->addInstruction(gateRegistry->createInstruction("X", qubits_a[k]));
        }
      }
      for (int k = 0; k < b_bin.size(); k++) {
        char b = b_bin[k];
        if (b == '1') {
          circuit->addInstruction(gateRegistry->createInstruction("X", qubits_b[k]));
        }
      }

      auto multiply = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("Multiplication"));
      const bool expand_ok = multiply->expand({{"qubits_a", qubits_a},
                                               {"qubits_b", qubits_b},
                                               {"qubits_result", qubits_result},
                                               {"qubit_ancilla", qubit_ancilla},
                                               {"is_LSB", is_LSB}});
      EXPECT_TRUE(expand_ok);
      circuit->addInstructions(multiply->getInstructions());

      // Measure
      for (int i = 0; i < qubits_a.size(); i++) {
        circuit->addInstruction(gateRegistry->createInstruction("Measure", qubits_a[i]));
      }
      for (int i = 0; i < qubits_b.size(); i++) {
        circuit->addInstruction(gateRegistry->createInstruction("Measure", qubits_b[i]));
      }
      for (int i = 0; i < qubits_result.size(); i++) {
        circuit->addInstruction(gateRegistry->createInstruction("Measure", qubits_result[i]));
      }
      circuit->addInstruction(gateRegistry->createInstruction("Measure", qubit_ancilla));

      /////////////////////////////// Run circuit ////////////////////////////////////

      auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
      auto buffer = xacc::qalloc(9);
      acc->execute(buffer, circuit);

      /////////////////////////////// Check results ////////////////////////////////////

      std::cout << "i = " << i << "\n";
      std::cout << "j = " << j << "\n";
      std::cout << "i*j = " << i*j << "\n";

      int answer = i*j;
      std::string bin_answer = binary(answer, (int)qubits_result.size());
      std::string expected_output = a_bin + b_bin + bin_answer + '0';

      buffer->print();
      auto measurements = buffer->getMeasurementCounts();
      EXPECT_EQ(measurements.size(), 1);
      EXPECT_EQ(measurements[expected_output], 1024);
    }
  }
}

TEST(MultiplicationCircuitTester, check_integer2) {
  /////////////////////////////// Define circuit ////////////////////////////////////
  std::vector<int> qubits_a = {0,1}; // n qubits
  std::vector<int> qubits_b = {2,3}; // n qubits
  std::vector<int> qubits_result = {4,5,6,7}; // 2n qubits
  int qubit_ancilla = 8; // 1 qubit
  bool is_LSB = true;

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("circuit");

  circuit->addInstruction(gateRegistry->createInstruction("X", qubits_a[0]));
  circuit->addInstruction(gateRegistry->createInstruction("X", qubits_a[1]));
  circuit->addInstruction(gateRegistry->createInstruction("X", qubits_b[0]));
  circuit->addInstruction(gateRegistry->createInstruction("X", qubits_b[1]));

  auto multiply = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("Multiplication"));
  const bool expand_ok = multiply->expand({{"qubits_a", qubits_a},
                                           {"qubits_b", qubits_b},
                                           {"qubits_result", qubits_result},
                                           {"qubit_ancilla", qubit_ancilla},
                                           {"is_LSB", is_LSB}});
  EXPECT_TRUE(expand_ok);
  circuit->addInstructions(multiply->getInstructions());

  // Measure
  for (int i = 0; i < qubits_a.size(); i++)
    circuit->addInstruction(gateRegistry->createInstruction("Measure", qubits_a[i]));
  for (int i = 0; i < qubits_b.size(); i++)
    circuit->addInstruction(gateRegistry->createInstruction("Measure", qubits_b[i]));
  for (int i = 0; i < qubits_result.size(); i++)
    circuit->addInstruction(gateRegistry->createInstruction("Measure", qubits_result[i]));
  circuit->addInstruction(gateRegistry->createInstruction("Measure", qubit_ancilla));

  /////////////////////////////// Run circuit ////////////////////////////////////

  auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(9);
  acc->execute(buffer, circuit);

  /////////////////////////////// Check results ////////////////////////////////////

  buffer->print();
  EXPECT_EQ(buffer->getMeasurementCounts()["111110010"], 1024);

}


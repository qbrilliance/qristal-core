#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <bitset>
#include <gtest/gtest.h>
////////////////////////
// Other include statements
////////////////////////

TEST(CompareGTCircuitTester, CheckGrid) {

  // Helper functions

  // int to binary function
  std::function<std::string(int, int)> binary = [&](int i, int num_qubits) {
    std::string i_binary = std::bitset<8*sizeof(i)>(i).to_string();
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

  // This test performs all possible 5-qubit bitstring comparisons

  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {

      //////////////////////////////////////
      // Define circuit
      //////////////////////////////////////

      // State prep
      auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
      auto circuit = gateRegistry->createComposite("circuit");

      std::vector<int> qubits_a = {0, 1, 2, 3, 4};
      std::vector<int> qubits_b = {5, 6, 7, 8, 9};
      int qubit_flag = 10;
      int qubit_ancilla = 11;

      std::string a_bin = binary(i, (int)qubits_a.size());
      std::string b_bin = binary(j, (int)qubits_b.size());

      for (int k = 0; k < a_bin.size(); k++) {
        char b = a_bin[k];
        if (b == '1') {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", qubits_a[k]));
        }
      }
      for (int k = 0; k < b_bin.size(); k++) {
        char b = b_bin[k];
        if (b == '1') {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", qubits_b[k]));
        }
      }

      // Prepare the module
      auto CompareGT = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("CompareGT"));
      const bool expand_ok =
          CompareGT->expand({{"qubits_b", qubits_b},
                               {"qubits_a", qubits_a},
                               {"qubit_flag", qubit_flag},
                               {"qubit_ancilla", qubit_ancilla}});
      EXPECT_TRUE(expand_ok);

      // Add the module to the circuit
      circuit->addInstructions(CompareGT->getInstructions());

      // Add measurements
      for (auto bit : qubits_a) {
        circuit->addInstruction(
            gateRegistry->createInstruction("Measure", bit));
      }
      for (auto bit : qubits_b) {
        circuit->addInstruction(
            gateRegistry->createInstruction("Measure", bit));
      }
      circuit->addInstruction(gateRegistry->createInstruction("Measure", qubit_flag));
      circuit->addInstruction(gateRegistry->createInstruction("Measure", qubit_ancilla));

      //////////////////////////////////////
      // Run circuit
      //////////////////////////////////////

      auto accelerator = xacc::getAccelerator("qsim", {{"shots", 1024}});
      auto buffer =
          xacc::qalloc((int)qubits_a.size() + (int)qubits_b.size() + 2);
      accelerator->execute(buffer, circuit);

      //////////////////////////////////////
      // Check results
      //////////////////////////////////////

      std::string expected_output = a_bin + b_bin;
      if (i>j) {
          expected_output += '1';
      } else {
          expected_output += '0';
      }
      expected_output += '0';

    //   std::cout << "i = " << i << "\n";
    //   std::cout << "j = " << j << "\n";
    //   buffer->print();

      auto measurements = buffer->getMeasurementCounts();
      EXPECT_EQ((int)measurements.size(), 1);
      EXPECT_EQ(measurements[expected_output], 1024);
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
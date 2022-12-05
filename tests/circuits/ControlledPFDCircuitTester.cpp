// #include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>
#include <bitset>
////////////////////////
// Other include statements
////////////////////////

TEST(cPFDCircuitTester, CheckGrid) {

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

  for (int c = 0; c < 4; c++) {
    for (int i = 0; i < 8; i++) {
      for (int j = i+1; j < 8; j++) {

        int precision = 3;

        //////////////////////////////////////
        // Define circuit
        //////////////////////////////////////

        // Define the circuit we want to run
        auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
        auto circuit = gateRegistry->createComposite("circuit");

        // Prepare the module
        std::vector<int> qubits_numerator = {0, 1, 2};
        std::vector<int> qubits_denominator = {3, 4, 5};
        std::vector<int> qubits_fraction = {6, 7, 8};
        std::vector<int>
            qubits_ancilla; // Length = 2*precision + 1
        for (int k = 9; k < 16; k++) {
          qubits_ancilla.push_back(k);
        }
        std::vector<int> controls_on = {16};
        std::vector<int> controls_off = {17};

        if (c == 1) {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", controls_on[0]));
        }
        if (c == 2) {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", controls_off[0]));
        }
        if (c == 3) {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", controls_on[0]));
          circuit->addInstruction(
              gateRegistry->createInstruction("X", controls_off[0]));
        }

        std::string numerator_bin = binary(i, (int)qubits_numerator.size());
        std::string denominator_bin = binary(j, (int)qubits_denominator.size());

        for (int k = 0; k < numerator_bin.size(); k++) {
          char b = numerator_bin[k];
          if (b == '1') {
            circuit->addInstruction(
                gateRegistry->createInstruction("X", qubits_numerator[k]));
          }
        }
        for (int k = 0; k < denominator_bin.size(); k++) {
          char b = denominator_bin[k];
          if (b == '1') {
            circuit->addInstruction(
                gateRegistry->createInstruction("X", qubits_denominator[k]));
          }
        }

        auto pfd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>(
                "ControlledProperFractionDivision"));
        const bool expand_ok =
            pfd->expand({{"qubits_numerator", qubits_numerator},
                         {"qubits_denominator", qubits_denominator},
                         {"qubits_fraction", qubits_fraction},
                         {"qubits_ancilla", qubits_ancilla},
                         {"controls_on", controls_on},
                         {"controls_off", controls_off}});
        EXPECT_TRUE(expand_ok);

        // Add the module to the circuit
        circuit->addInstructions(pfd->getInstructions());

        // Add measurements
        for (auto bit : qubits_numerator) {
          circuit->addInstruction(
              gateRegistry->createInstruction("Measure", bit));
        }
        for (auto bit : qubits_denominator) {
          circuit->addInstruction(
              gateRegistry->createInstruction("Measure", bit));
        }
        for (auto bit : qubits_fraction) {
          circuit->addInstruction(
              gateRegistry->createInstruction("Measure", bit));
        }
        for (auto bit : qubits_ancilla) {
          circuit->addInstruction(
              gateRegistry->createInstruction("Measure", bit));
        }

        //////////////////////////////////////
        // Run circuit
        //////////////////////////////////////

        auto accelerator =
            xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
        auto buffer = xacc::qalloc(18);
        accelerator->execute(buffer, circuit);

        //////////////////////////////////////
        // Check results
        //////////////////////////////////////

        // Print the buffer following the execution.
        // It provides a lot of information such as measurement results.
        // buffer->print();

        // Get just the measurement counts.
        // This will return a dictionary type object {"measurement" ; counts}
        auto measurements = buffer->getMeasurementCounts();
        EXPECT_EQ(measurements.size(), 1);

        if (c == 1) {
          float classical_result;
          if (j != 0) {
            classical_result = (float)i / (float)j;
          } else {
            classical_result = 0;
          }
          std::string classical_result_string;
          for (int k = 1; k <= precision; k++) {
            if (classical_result - (1.0 / (1 << k)) >= 0) {
              classical_result_string += '1';
              classical_result = classical_result - (1.0 / (1 << k));
            } else {
              classical_result_string += '0';
            }
          }

          reverse(classical_result_string.begin(), classical_result_string.end());

          std::string expected_output =
              numerator_bin + denominator_bin + classical_result_string;
          for (auto bit : qubits_ancilla) {
            expected_output += '0';
          }

          EXPECT_EQ(measurements[expected_output], 1024);
        } else {
          std::string expected_output = numerator_bin + denominator_bin;
          for (auto bit : qubits_fraction) {
            expected_output += '0';
          }
          for (auto bit : qubits_ancilla) {
            expected_output += '0';
          }
          EXPECT_EQ(measurements[expected_output], 1024);
        }
      }
    }
  }
}


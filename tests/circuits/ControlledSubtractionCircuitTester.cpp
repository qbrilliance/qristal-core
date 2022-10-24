#include "xacc.hpp"
#include "xacc_service.hpp"
#include "Circuit.hpp"
#include <gtest/gtest.h>
#include <bitset>
////////////////////////
// Other include statements
////////////////////////

TEST(ControlledSubtractionCircuitTester, CheckGrid) {

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

  // This test performs all valid 5-qubit subtractions i - j
for (int c = 0; c < 4; c++) {
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j <= i; j++) {

      //////////////////////////////////////
      // Define circuit
      //////////////////////////////////////

      // State prep
      auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
      auto circuit = gateRegistry->createComposite("circuit");

      std::vector<int> qubits_larger = {0, 1, 2, 3, 4};
      std::vector<int> qubits_smaller = {5, 6, 7, 8, 9};
      std::vector<int> control_on = {10};
      std::vector<int> control_off = {11};

      std::string larger_bin = binary(i, (int)qubits_larger.size());
      std::string smaller_bin = binary(j, (int)qubits_smaller.size());
      std::string controls_bin = binary(c, 2);

      for (int k = 0; k < larger_bin.size(); k++) {
        char b = larger_bin[k];
        if (b == '1') {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", qubits_larger[k]));
        }
      }
      for (int k = 0; k < smaller_bin.size(); k++) {
        char b = smaller_bin[k];
        if (b == '1') {
          circuit->addInstruction(
              gateRegistry->createInstruction("X", qubits_smaller[k]));
        }
      }
      if (controls_bin[0] == '1') {
        circuit->addInstruction(
            gateRegistry->createInstruction("X", control_on[0]));
      }
      if (controls_bin[1] == '1') {
        circuit->addInstruction(
            gateRegistry->createInstruction("X", control_off[0]));
      }

      // Prepare the module
      auto CSubtraction = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("ControlledSubtraction"));
      const bool expand_ok =
          CSubtraction->expand({{"qubits_smaller", qubits_smaller},
                               {"qubits_larger", qubits_larger},
                               {"controls_on", control_on},
                               {"controls_off", control_off}});
      EXPECT_TRUE(expand_ok);

      // Add the module to the circuit
      circuit->addInstructions(CSubtraction->getInstructions());

      // Add measurements
      for (auto bit : qubits_larger) {
        circuit->addInstruction(
            gateRegistry->createInstruction("Measure", bit));
      }
      for (auto bit : qubits_smaller) {
        circuit->addInstruction(
            gateRegistry->createInstruction("Measure", bit));
      }

      //////////////////////////////////////
      // Run circuit
      //////////////////////////////////////

      auto accelerator = xacc::getAccelerator("qsim", {{"shots", 1024}});
      auto buffer =
          xacc::qalloc((int)qubits_larger.size() + (int)qubits_smaller.size() + 2);
      accelerator->execute(buffer, circuit);

      //////////////////////////////////////
      // Check results
      //////////////////////////////////////

      if (c == 1) {
        int expected_result = i - j;
        std::string expected_result_bin =
            binary(expected_result, (int)qubits_larger.size());

        std::string expected_output = expected_result_bin + smaller_bin;

        //   std::cout << "i " << i << "\n";
        //   std::cout << "j " << j << "\n";
        //   std::cout << "exp " << expected_output << "\n";
        //   std::cout << "circ:\n" << circuit->toString() << "\n";
        //   buffer->print();

        auto measurements = buffer->getMeasurementCounts();
        EXPECT_EQ((int)measurements.size(), 1);
        EXPECT_EQ(measurements[expected_output], 1024);
      } else {

      std::string expected_output = larger_bin + smaller_bin;
      // buffer->print();

      auto measurements = buffer->getMeasurementCounts();
      EXPECT_EQ((int)measurements.size(), 1);
      EXPECT_EQ(measurements[expected_output], 1024);
      }
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

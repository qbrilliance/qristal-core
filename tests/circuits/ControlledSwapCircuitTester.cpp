// Copyright (c) Quantum Brilliance Pty Ltd
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <Circuit.hpp>
#include <gtest/gtest.h>
////////////////////////
// Other include statements
////////////////////////

TEST(ControlledSwapCircuitTester, checkstring) {

  // In this test we define a string of length 4 from the alphabet {a,b}.
  // Entangled to this register is a flag qubit indicating which letters are a
  // "b". Conditional on this b-flag, the letter is moved form its current place
  // to the end of the string by a series of controlled swap gates. So the
  // expected output is a string with any b's moved to the end.

  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto test_circ = gateRegistry->createComposite("test_circ");
  std::vector<int> qubits_string = {0, 1, 2, 3};
  std::vector<int> b_flags = {4, 5, 6, 7};

  std::vector<std::string> alphabet = {
      "a", "b"}; // "a" represented by |0> and "b" represented by |1>
  std::string initial_string = "baba";
  std::cout << "The input string is " << initial_string << "\n";

  // Prepare initial string entangled with b flags

  for (int i = 0; i < initial_string.length(); i++) {
    if (initial_string[i] == 'b') {
      test_circ->addInstruction(
          gateRegistry->createInstruction("X", qubits_string[i]));
    }
  }

  for (int i = 0; i < initial_string.length(); i++) {
    test_circ->addInstruction(gateRegistry->createInstruction(
        "CX",
        std::vector<std::size_t>{static_cast<unsigned long>(qubits_string[i]),
                                 static_cast<unsigned long>(b_flags[i])}));
  }

  // Perform controlled swaps
  for (int i = 0; i < initial_string.length(); i++) {
    int k = initial_string.length() - 1 - i;
    for (int j = k; j < initial_string.length()-1; j++) {
      std::vector<int> qubits_a = {qubits_string[j]};
      std::vector<int> qubits_b = {qubits_string[j+1]};
      std::vector<int> flags_on = {b_flags[k]};
      auto controlled_swap =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("ControlledSwap"));
      const bool expand_ok =
          controlled_swap->expand({{"qubits_a", qubits_a},
                                   {"qubits_b", qubits_b},
                                   {"flags_on", flags_on}});
      EXPECT_TRUE(expand_ok);
      test_circ->addInstructions(controlled_swap->getInstructions());
    }
  }

  // Measure the final string
  for (int i = 0; i < qubits_string.size(); i++) {
      test_circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_string[i]));
  }

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////

  auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(qubits_string.size() + b_flags.size());
  acc->execute(buffer, test_circ);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////

  int num_b = 0;
  for (char letter : initial_string) {
      if (letter == 'b') {
          num_b = num_b + 1;
      }
  }

  std::string expected_output;
  for (int i = 0; i < initial_string.size() - num_b; i++) {
      expected_output.push_back('a');
  }
  for (int i = 0; i < num_b; i++) {
      expected_output.push_back('b');
  }

  std::cout << "The expected output is " << expected_output << "\n";

  std::string measured_output;
  assert(buffer->getMeasurements().size() == 1);
  auto measurement = buffer->getMeasurements().front();
  for (int i = 0; i < measurement.size(); i++) {
      if (measurement[i] == '0') {
          measured_output.push_back('a');
      } else {
          measured_output.push_back('b');
      }
  }

  std::cout << "The measured output is " << measured_output << "\n";

  std::string expected_measurement;
  for (int i = 0; i < expected_output.length(); i++) {
      if (expected_output[i] == 'a') {
          expected_measurement.push_back('0');
      } else {
          expected_measurement.push_back('1');
      }
  }
  EXPECT_EQ(buffer->getMeasurementCounts()[expected_measurement], 1024);

}


// Copyright (c) Quantum Brilliance Pty Ltd
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "Circuit.hpp"
#include <gtest/gtest.h>
////////////////////////
// Other include statements
////////////////////////

// In this test we use generalised mcx to
// perform mcx on all possible 3-qubit bit strings (|000>,...,|111>)
// with all combinations of control qubit conditions ((on,on),...,(off,off))

// Expected outcomes:

// - When the control qubits are required to be (on,on) then we expect
// |110> -> |111>, |111> -> |110>, and all other bit strings remain unchanged

// - When the control qubits are required to be (on,off) then we expect
// |100> -> |101>, |101> -> |100>, and all other bit strings remain unchanged

// - When the control qubits are required to be (off,on) then we expect
// |010> -> |011>, |011> -> |010>, and all other bit strings remain unchanged

// - When the control qubits are required to be (off,off) then we expect
// |000> -> |001>, |001> -> |000>, and all other bit strings remain unchanged

TEST(GeneralisedMCXCircuitTester, checkgrid) {

  //////////////////////////////////////
  // Run test
  //////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  std::vector<int> controls = {0,1};
  int target = 2;

  std::vector<std::vector<int>> conditions = {{0,0}, {0,1}, {1,0}, {1,1}};
  std::vector<std::string> input_bitstrings = {"000", "001", "010", "011", "100", "101", "110", "111"};

  for (std::vector<int> condition : conditions) {
      for (std::string input_bitstring : input_bitstrings) {
          auto circ = gateRegistry->createComposite("circ");

          // Prepare input bitstring
          for (int i = 0; i < input_bitstring.length(); i++) {
              if (input_bitstring[i] == '1') {
                  circ->addInstruction(gateRegistry->createInstruction("X", i));
              }
          }

          // Add the generalised mcx
          std::vector<int> controls_on;
          std::vector<int> controls_off;
          for (int i = 0; i < condition.size(); i++) {
              if (condition[i] == 0) {
                  controls_off.push_back(controls[i]);
              } else {
                  controls_on.push_back(controls[i]);
              }
          }
          auto generalised_mcx =
              std::dynamic_pointer_cast<xacc::CompositeInstruction>(
                  xacc::getService<xacc::Instruction>("GeneralisedMCX"));
          const bool expand_ok =
              generalised_mcx->expand({{"target", target},
                                           {"controls_on", controls_on},
                                           {"controls_off", controls_off}});
          EXPECT_TRUE(expand_ok);
          circ->addInstructions(generalised_mcx->getInstructions());

          // Measurements
          for (int i = 0; i < input_bitstring.size(); i++) {
              circ->addInstruction(gateRegistry->createInstruction("Measure", i));
          }

          std::cout << "circ:\n" << circ->toString() << "\n";

          // Execute the circuit
          auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
          auto buffer = xacc::qalloc(1+controls.size());
          acc->execute(buffer, circ);

          // Check results
          std::string expected_output;
          expected_output.push_back(input_bitstring[0]);
          expected_output.push_back(input_bitstring[1]);
          char a;
          char b;
          if (condition[0] == 0) {a = '0';} else {a = '1';}
          if (condition[1] == 0) {b = '0';} else {b = '1';}
          if (input_bitstring[0] == a && input_bitstring[1] == b) {
              if (input_bitstring[2] == '0') {expected_output.push_back('1');} else {expected_output.push_back('0');}
          } else {
              expected_output.push_back(input_bitstring[2]);
          }
          EXPECT_EQ(buffer->getMeasurementCounts()[expected_output], 1024);
      }
  }
}


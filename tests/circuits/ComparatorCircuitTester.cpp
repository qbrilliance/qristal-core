// Copyright (c) Quantum Brilliance Pty Ltd
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <Circuit.hpp>
#include <gtest/gtest.h>
#include <bitset>

TEST(ComparatorCircuitTester, checkGrid) {
  // Test Comparator: input a bitstring to compare to BestScore.
  // If input > BestScore, flag qubit should return |1>.
  // Otherwise flag qubit should return |0>.
  for (int k = 0; k < 4; k++) {
      for (int j = 0; j < 4; j++) {
          // k will be used as the trial score
          // j will be used as the current best score
          auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
          auto BestScore = j;
          auto num_scoring_qubits = 3;
          auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("Comparator"));
          const bool expand_ok = comp->expand(
              {{"BestScore", BestScore}, {"num_scoring_qubits", num_scoring_qubits}, {"is_LSB", false}});
          EXPECT_TRUE(expand_ok);

          // Simulation test:
          // Construct the full circuit including preparation of input trial score
          auto circuit = gateRegistry->createComposite("sim_comp");
          // Trial score = k
          std::string TrialScore_binary = std::bitset<sizeof(k)>(k).to_string();
          std::string TrialScore_binary_n = TrialScore_binary.substr(TrialScore_binary.size() < num_scoring_qubits ? 0 : TrialScore_binary.size() - num_scoring_qubits);
          for (int i = 0; i < num_scoring_qubits; i++) {
              if (TrialScore_binary_n[i] == '1') {
                  circuit->addInstruction(gateRegistry->createInstruction("X",i));
              }
          }
          // Add comp:
          circuit->addInstructions(comp->getInstructions());
          // Measure scoring qubit:
          circuit->addInstruction(gateRegistry->createInstruction("Measure", num_scoring_qubits));
          // std::cout << "HOWDY: Comparator circuit:\n";
          // std::cout << circuit->toString() << '\n';
          // Sim:
          auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
          auto buffer = xacc::qalloc(5*num_scoring_qubits-1);
          acc->execute(buffer, circuit);
          //buffer->print();
          // EXPECTED: '1' bitstring iff k>j
          if (k>j) {
              EXPECT_EQ(buffer->getMeasurementCounts()["1"], 1024);
          }
          else {
              EXPECT_EQ(buffer->getMeasurementCounts()["0"], 1024);
          }
      }
  }
}

// TEST(ComparatorCircuitTester, checkGrid_inputqubits) {
//   // Test Comparator: input a bitstring to compare to BestScore.
//   // If input > BestScore, flag qubit should return |1>.
//   // Otherwise flag qubit should return |0>.
//   for (int k = 0; k < 4; k++) {
//       for (int j = 0; j < 4; j++) {
//           // k will be used as the trial score
//           // j will be used as the current best score
//           auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
//           auto BestScore = j;
//           auto num_scoring_qubits = 2;
//           std::vector<int> trial_score_qubits = {1,2};
//           int flag_qubit = 0;
//           std::vector<int> best_score_qubits = {3,7};
//           std::vector<int> ancilla_qubits = {4,5,6,8};
//           auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//               xacc::getService<xacc::Instruction>("Comparator"));
//           const bool expand_ok = comp->expand(
//               {{"BestScore", BestScore}, {"num_scoring_qubits", num_scoring_qubits},{"trial_score_qubits", trial_score_qubits},{"flag_qubit",flag_qubit},{"best_score_qubits",best_score_qubits}, {"ancilla_qubits",ancilla_qubits}});
//           EXPECT_TRUE(expand_ok);

//           // Simulation test:
//           // Construct the full circuit including preparation of input trial score
//           auto circuit = gateRegistry->createComposite("sim_comp");
//           // Trial score = k
//           std::string TrialScore_binary = std::bitset<sizeof(k)>(k).to_string();
//           std::string TrialScore_binary_n = TrialScore_binary.substr(TrialScore_binary.size() < num_scoring_qubits ? 0 : TrialScore_binary.size() - num_scoring_qubits);
//           for (int i = 0; i < num_scoring_qubits; i++) {
//               if (TrialScore_binary_n[i] == '1') {
//                   circuit->addInstruction(gateRegistry->createInstruction("X",trial_score_qubits[i]));
//               }
//           }
//           // Add comp:
//           circuit->addInstructions(comp->getInstructions());
//           // Measure scoring qubit:
//           circuit->addInstruction(gateRegistry->createInstruction("Measure", flag_qubit));
//           std::cout << "HOWDY: Comparator circuit:\n";
//           std::cout << circuit->toString() << '\n';
//           // Sim:
//           auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
//           auto buffer = xacc::qalloc(5*num_scoring_qubits-1);
//           acc->execute(buffer, circuit);
//           buffer->print();
//           // EXPECTED: '1' bitstring iff k>j
//           if (k>j) {
//               EXPECT_EQ(buffer->getMeasurementCounts()["1"], 1024);
//           }
//           else {
//               EXPECT_EQ(buffer->getMeasurementCounts()["0"], 1024);
//           }
//       }
//   }
// }


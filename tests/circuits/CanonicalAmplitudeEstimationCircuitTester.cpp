// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>

// TEST(AmplitudeEstimationCircuitTester, checkSimple) {
//   // Test the example here:
//   // https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
//   // i.e., estimate the probability p = 0.2 of the state:
//   // sqrt(1-p)|0> + sqrt(p)|1>
//   constexpr double p = 1;
//   const double theta_p = 2 * std::asin(std::sqrt(p));
//   auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
//   // A circuit
//   auto state_prep = gateRegistry->createComposite("A");
//   state_prep->addInstruction(
//       gateRegistry->createInstruction("Ry", {1}, {theta_p}));
//   // Q circuit
//   auto grover_op = gateRegistry->createComposite("Q");
//   grover_op->addInstruction(
//       gateRegistry->createInstruction("Ry", {1}, {2.0 * theta_p}));
//   const int bits_precision = 1;
//   auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
//   const bool expand_ok =
//       ae->expand({{"state_preparation_circuit", state_prep},
//                   {"grover_op_circuit", grover_op},
//                   {"num_evaluation_qubits", bits_precision},
//                   {"num_state_qubits", 1},
//                   {"num_trial_qubits", 1}});
//   EXPECT_TRUE(expand_ok);
//   // Simulation test:
//   auto circuit = gateRegistry->createComposite("sim_ae");
//   // Add amplitude estimation:
//   circuit->addInstructions(ae->getInstructions());
//   // Measure evaluation qubits:
//   for (int i = 0; i < bits_precision; ++i) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure", i));
//   }
//   std::cout << "HOWDY: Canonical amplitude estimation circuit:\n";
//   std::cout << circuit->toString() << '\n';
//   auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
//   auto buffer = xacc::qalloc(bits_precision + 1);
//   acc->execute(buffer, circuit);
//   buffer->print();
// }

// TEST(AmplitudeEstimationCircuitTester, checkfordecoder) {
//   // Test the example here:
//   // https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
//   // i.e., estimate the probability p = 0.2 of the state:
//   // sqrt(1-p)|0> + sqrt(p)|1>
//   constexpr double p = 0.2;
//   const double theta_p = 2 * std::asin(std::sqrt(p));
//   auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
//   // A circuit
//   auto state_prep = gateRegistry->createComposite("A");
//   state_prep->addInstruction(
//       gateRegistry->createInstruction("Ry", {0}, {theta_p}));
//   // Q circuit
//   auto grover_op = gateRegistry->createComposite("Q");
//   grover_op->addInstruction(
//       gateRegistry->createInstruction("Ry", {0}, {2.0 * theta_p}));
//   const int bits_precision = 3;
//   std::vector<int> trial_qubits = {0};
//   std::vector<int> evaluation_qubits = {1,2,3};
//   auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
//   const bool expand_ok =
//       ae->expand({{"state_preparation_circuit", state_prep},
//                   {"grover_op_circuit", grover_op},
//                   {"num_evaluation_qubits", bits_precision},
//                   {"num_state_qubits", 1},
//                   {"num_trial_qubits", 1},
//                   {"trial_qubits", trial_qubits},
//                   {"evaluation_qubits", evaluation_qubits}});
//   EXPECT_TRUE(expand_ok);
//   // Simulation test:
//   auto circuit = gateRegistry->createComposite("sim_ae");
//   // Add amplitude estimation:
//   circuit->addInstructions(ae->getInstructions());
//   // Measure evaluation qubits:
//   for (int i = 0; i < bits_precision; ++i) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure", evaluation_qubits[i]));
//   }
//   std::cout << "HOWDY: Canonical amplitude estimation circuit:\n";
//   std::cout << circuit->toString() << '\n';
//   auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
//   auto buffer = xacc::qalloc(bits_precision + 1);
//   acc->execute(buffer, circuit);
//   buffer->print();
// }

// TEST(AmplitudeEstimationCircuitTester, checkfordecoder) {
//   auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
//   auto circuit = gateRegistry->createComposite("sim");
//   auto state_prep = gateRegistry->createComposite("state_prep");
//   std::vector<int> metric_qubits = {0,1,2};
//   std::vector<int> evaluation_qubits = {3,4,5,6,7,8};
//   state_prep->addInstruction(gateRegistry->createInstruction("H", metric_qubits[2]));
//   state_prep->addInstruction(gateRegistry->createInstruction("CX", {static_cast<unsigned long>(metric_qubits[2]), static_cast<unsigned long>(metric_qubits[1])}));
//   state_prep->addInstruction(gateRegistry->createInstruction("CX", {static_cast<unsigned long>(metric_qubits[1]), static_cast<unsigned long>(metric_qubits[0])}));
//   circuit->addInstructions(state_prep->getInstructions());

//   for (int i = 0; i < 3; i++) {
//   auto oracle = gateRegistry->createComposite("oracle");
//   oracle->addInstruction(gateRegistry->createInstruction("Z", metric_qubits[i]));

//   std::shared_ptr<xacc::CompositeInstruction> state_prep_clone =
//       xacc::ir::asComposite(state_prep->clone());

//   std::vector<int> current_evaluation_qubits;
//   if (i == 0) {
//       current_evaluation_qubits.push_back(evaluation_qubits[0]);
//   }
//   if (i == 1) {
//       current_evaluation_qubits.push_back(evaluation_qubits[1]);
//       current_evaluation_qubits.push_back(evaluation_qubits[2]);
//   }
//   if (i == 2) {
//       current_evaluation_qubits.push_back(evaluation_qubits[3]);
//       current_evaluation_qubits.push_back(evaluation_qubits[4]);
//       current_evaluation_qubits.push_back(evaluation_qubits[5]);
//   }
//   auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
//   const bool expand_ok =
//       ae->expand({{"state_preparation_circuit", state_prep_clone},
//                   {"oracle", oracle},
//                   {"num_evaluation_qubits", i+1},
//                   {"num_trial_qubits", 3},
//                   {"trial_qubits", metric_qubits},
//                   {"evaluation_qubits", current_evaluation_qubits},
//                   {"no_state_prep", true}});
//   EXPECT_TRUE(expand_ok);
//   // Add amplitude estimation:
//   circuit->addInstructions(ae->getInstructions());
//   }
//   // Measure
//   for (int i = 0; i < metric_qubits.size(); ++i) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure", metric_qubits[i]));
//   }
//   for (int i = 0; i < evaluation_qubits.size(); ++i) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure", evaluation_qubits[i]));
//   }

//   auto acc = xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
//   auto buffer = xacc::qalloc(9);
//   acc->execute(buffer, circuit);
//   buffer->print();
// }





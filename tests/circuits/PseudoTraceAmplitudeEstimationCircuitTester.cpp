#include "qb/core/circuit_builder.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "Circuit.hpp"
#include <gtest/gtest.h>
#include <memory>
////////////////////////
// Other include statements
////////////////////////

TEST(TestCase, checksimple) {

  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  // Define the circuit we want to run
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("circuit");

  // Prepare the module

  // Registers
  std::vector<int> qubits_string = {0,1};
  std::vector<int> qubits_superfluous_flags = {2,3};
  std::vector<int> qubits_metric = {4,5};
  std::vector<int> evaluation_bits = {6,7,8};
  int qubit_flag = 9;
  std::vector<int> qubits_ancilla = {10,11,12,13,14};

  // Other inputs
  std::vector<int> num_precision_bits = {3};
  std::vector<int> ae_state_qubits;
  auto ae_state_prep_circ = gateRegistry->createComposite("ae_state_prep_circ");

  // Circuit

  // Prepare the initial state |state>|flags>|metric> = |00>|11>|00> + |10>|01>|11> + |10>|01>|10> + |11>|01>|11>
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("H", qubits_string[0]));
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("H", qubits_string[1]));

  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("X", qubits_superfluous_flags[1]));
  auto mc1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mc1->expand({{"target", qubits_superfluous_flags[0]}, {"controls_off", qubits_string}});
  ae_state_prep_circ->addInstruction(mc1);

  auto mc2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  std::vector<int> controls_on2 = {qubits_string[0]};
  std::vector<int> controls_off2 = {qubits_string[1]};
  mc2->expand({{"target", qubits_metric[0]}, {"controls_on", controls_on2}, {"controls_off", controls_off2}});
  ae_state_prep_circ->addInstruction(mc2);
  auto mc3 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mc3->expand({{"target", qubits_metric[1]}, {"controls_on", controls_on2}, {"controls_off", controls_off2}});
  ae_state_prep_circ->addInstruction(mc3);
  auto mc4 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  std::vector<int> controls_on4 = {qubits_string[1]};
  std::vector<int> controls_off4 = {qubits_string[0]};
  mc4->expand({{"target", qubits_metric[0]}, {"controls_on", controls_on4}, {"controls_off", controls_off4}});
  ae_state_prep_circ->addInstruction(mc4);
  auto mc5 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mc5->expand({{"target", qubits_metric[0]}, {"controls_on", qubits_string}});
  ae_state_prep_circ->addInstruction(mc5);
  auto mc6 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mc6->expand({{"target", qubits_metric[1]}, {"controls_on", qubits_string}});
  ae_state_prep_circ->addInstruction(mc6);

  auto swap = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("ControlledSwap"));
  std::vector<int> controls_ons = {qubits_superfluous_flags[1], qubits_metric[0]};
  std::vector<int> controls_offs = {qubits_superfluous_flags[0], qubits_metric[1]};
  swap->expand({{"qubits_a", std::vector<int> {qubits_string[0]}}, {"qubits_b", std::vector<int> {qubits_string[1]}}, {"flags_on", controls_ons}, {"flags_off", controls_offs}});
  ae_state_prep_circ->addInstruction(swap);

  circuit->addInstructions(ae_state_prep_circ->getInstructions());

  // Now perform AE on the metric register, conditional on being beam-equivalent to the string |11>.
  // This should encompass the string metrics for |10> as well.
  // So we expect AE to be performed on the metric superposition |10> + |11> + |11>.
  // Then the first qubit is always |1> so the output of AE should be |1>
  // The second qubit is |1> 2/3 of the time so the output of AE will be a distribution of states peaking near |10>

  // Check for beam equivalence
  auto beam_eq = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("CompareBeamOracle"));
  int q0 = qubits_ancilla[0];
  int q1 = qubits_ancilla[1];
  int q2 = qubits_ancilla[2];
  std::vector<int> flags_copy = {qubits_ancilla[3], qubits_ancilla[4]};
  //circuit->addInstruction(gateRegistry->createInstruction("X", flags_copy[1]));
  beam_eq->expand({{"simplified", true}, {"q0", q0}, {"q1", q1}, {"q2", q2}, {"FA", qubits_superfluous_flags}, {"FB", flags_copy}, {"SA", qubits_string}});
  //circuit->addInstructions(beam_eq->getInstructions());
  //circuit->addInstruction(gateRegistry->createInstruction("X", q0));

  //ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("CX", {static_cast<unsigned long>(q0), static_cast<unsigned long>(qubit_flag)}));

//   auto inv_beam_eq = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("InverseCircuit"));
//   inv_beam_eq->expand({{"circ", beam_eq}});
//   ae_state_prep_circ->addInstructions(inv_beam_eq->getInstructions());
//   ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("X", flags_copy[1]));

//   circuit->addInstructions(ae_state_prep_circ->getInstructions());

  auto ae_state_qubits_set = qb::uniqueBitsQD(ae_state_prep_circ);
  for (auto bit : ae_state_qubits_set) {
      ae_state_qubits.push_back(bit);
  }

  for (int q = 0; q < 1; q++) {
  std::shared_ptr<xacc::CompositeInstruction> ae_state_prep_circ_clone =
      xacc::ir::asComposite(ae_state_prep_circ->clone());

  auto oracle = gateRegistry->createComposite("oracle");
  // Use the Z gate as the oracle to detect 1's in the metric register, i.e. the marked state is |1>.
  //oracle->addInstruction(gateRegistry->createInstruction("CZ", {static_cast<unsigned long>(q0),static_cast<unsigned long>(qubits_metric[q])}));
  oracle->addInstruction(gateRegistry->createInstruction("Z", qubits_metric[q]));

  std::vector<int> current_eval_bits;
  if (q==0) {current_eval_bits = {evaluation_bits[0]};}
  if (q==1) {current_eval_bits = {evaluation_bits[1], evaluation_bits[2]};}

  auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
  xacc::HeterogeneousMap options_ae{
      {"state_preparation_circuit", ae_state_prep_circ_clone},
      {"no_state_prep", true},
      {"oracle", oracle},
      {"evaluation_qubits", current_eval_bits},
      {"num_evaluation_qubits", q+1},
      {"trial_qubits", ae_state_qubits},
      {"num_trial_qubits", (int)ae_state_qubits.size()}};
  const bool expand_ok_ae = ae->expand(options_ae);
  assert(expand_ok_ae);
  //circuit->addInstructions(ae->getInstructions());

  auto cae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("C-U"));
  const bool expand_ok = cae->expand({{"U", ae}, {"control-idx", std::vector<int> {q0}}});
  assert(expand_ok);
  //circuit->addInstruction(cae);
  }

  auto inv_beam_eq = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("InverseCircuit"));
  inv_beam_eq->expand({{"circ", beam_eq}});
  //circuit->addInstructions(inv_beam_eq->getInstructions());
  //circuit->addInstruction(gateRegistry->createInstruction("X", flags_copy[1]));

//   std::cout << "state prep\n" << ae_state_prep_circ->toString() << "\n";
//   std::cout << "circ:\n" << circuit->toString() << "\n";

  // Add measurements
  // We expect to measure |string>|metric>|evaluation_bits> =
  // |00>|00>|000>
  // |10>|10>|110>
  // |10>|11>|110>
  // |11>|11>|110>
  // +...
  for (auto bit : qubits_string) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
  }
//   for (auto bit : qubits_metric) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
//   }
//   for (auto bit : evaluation_bits) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
//   }

// for (int bit = 0; bit < 15; bit++) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure",bit));
// }


  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////

  auto accelerator = xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(15);
  accelerator->execute(buffer, circuit);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////

  // Print the buffer following the execution.
  // It provides a lot of information such as measurement results.
  buffer->print();

  // Get just the measurement counts.
  // This will return a dictionary type object {"measurement" ; counts}
  auto measurements = buffer->getMeasurementCounts();
}

// TEST(PseudoTraceAmplitudeEstimationTester, NoDesiredCorrelations) {

//   //////////////////////////////////////
//   // Define circuit
//   //////////////////////////////////////

//   // Define the circuit we want to run
//   auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
//   auto circuit = gateRegistry->createComposite("circuit");

//   // Prepare the module

//   // Registers
//   std::vector<int> qubits_metric = {0,1,2};
//   std::vector<int> qubits_to_be_traced = {3,4};
//   std::vector<int> evaluation_bits = {5,6,7,8,9,10};

//   // Other inputs
//   std::vector<int> num_precision_bits = {1,2,3};
//   std::vector<int> ae_state_qubits;
//   auto ae_state_prep_circ = gateRegistry->createComposite("ae_state_prep_circ");

//   // Circuit

//   // Prepare the initial state |superfluous>|metric> = |00000> + |01110> + |10101> + |11011>
//   for (auto bit : qubits_to_be_traced) {
//       circuit->addInstruction(gateRegistry->createInstruction("H", bit));
//   }

//   auto mcx1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("GeneralisedMCX"));
//   const bool expand_ok_mcx1 = mcx1->expand(
//       {{"target", qubits_metric[0]},
//        {"controls_off", std::vector<int>{qubits_to_be_traced[0]}},
//        {"controls_on", std::vector<int> {qubits_to_be_traced[1]}}});
//   EXPECT_TRUE(expand_ok_mcx1);
//   circuit->addInstructions(mcx1->getInstructions());

//   auto mcx2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("GeneralisedMCX"));
//   const bool expand_ok_mcx2 = mcx2->expand(
//       {{"target", qubits_metric[1]},
//        {"controls_off", std::vector<int>{qubits_to_be_traced[0]}},
//        {"controls_on", std::vector<int> {qubits_to_be_traced[1]}}});
//   EXPECT_TRUE(expand_ok_mcx2);
//   circuit->addInstructions(mcx2->getInstructions());

//   auto mcx3 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("GeneralisedMCX"));
//   const bool expand_ok_mcx3 = mcx3->expand(
//       {{"target", qubits_metric[0]},
//        {"controls_off", std::vector<int>{qubits_to_be_traced[1]}},
//        {"controls_on", std::vector<int> {qubits_to_be_traced[0]}}});
//   EXPECT_TRUE(expand_ok_mcx3);
//   circuit->addInstructions(mcx3->getInstructions());

//   auto mcx4 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("GeneralisedMCX"));
//   const bool expand_ok_mcx4 = mcx4->expand(
//       {{"target", qubits_metric[2]},
//        {"controls_off", std::vector<int>{qubits_to_be_traced[1]}},
//        {"controls_on", std::vector<int> {qubits_to_be_traced[0]}}});
//   EXPECT_TRUE(expand_ok_mcx4);
//   circuit->addInstructions(mcx4->getInstructions());

//   auto mcx5 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("GeneralisedMCX"));
//   const bool expand_ok_mcx5 = mcx5->expand(
//       {{"target", qubits_metric[1]},
//        {"controls_on", std::vector<int> {qubits_to_be_traced[0], qubits_to_be_traced[1]}}});
//   EXPECT_TRUE(expand_ok_mcx5);
//   circuit->addInstructions(mcx5->getInstructions());

//   auto mcx6 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("GeneralisedMCX"));
//   const bool expand_ok_mcx6 = mcx5->expand(
//       {{"target", qubits_metric[2]},
//        {"controls_on", std::vector<int> {qubits_to_be_traced[0], qubits_to_be_traced[1]}}});
//   EXPECT_TRUE(expand_ok_mcx6);
//   circuit->addInstructions(mcx6->getInstructions());

//   ae_state_prep_circ->addInstructions(mcx1->getInstructions());
//   ae_state_prep_circ->addInstructions(mcx2->getInstructions());
//   ae_state_prep_circ->addInstructions(mcx3->getInstructions());
//   ae_state_prep_circ->addInstructions(mcx4->getInstructions());
//   ae_state_prep_circ->addInstructions(mcx5->getInstructions());
//   ae_state_prep_circ->addInstructions(mcx6->getInstructions());

//   auto ae_state_qubits_set = qb::uniqueBitsQD(ae_state_prep_circ);
//   for (auto bit : ae_state_qubits_set) {
//       ae_state_qubits.push_back(bit);
//   }

//   auto ptae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("PseudoTraceAmplitudeEstimation"));
//   const bool expand_ok = ptae->expand({{"qubits_metric", qubits_metric},
//                                         {"qubits_to_be_traced", qubits_to_be_traced},
//                                         {"evaluation_bits", evaluation_bits},
//                                         {"num_precision_bits", num_precision_bits},
//                                         {"ae_state_qubits", ae_state_qubits},
//                                         {"ae_state_prep_circ", ae_state_prep_circ}});
//   EXPECT_TRUE(expand_ok);
//   //circuit->addInstructions(ptae->getInstructions());

//   // Add measurements
//   for (auto bit : qubits_to_be_traced) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
//   }
//   for (auto bit : qubits_metric) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
//   }
//   for (auto bit : evaluation_bits) {
//     circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
//   }

//   //////////////////////////////////////
//   // Run circuit
//   //////////////////////////////////////

//   auto accelerator = xacc::getAccelerator("qsim", {{"shots", 1024}});
//   auto buffer = xacc::qalloc(11);
//   accelerator->execute(buffer, circuit);

//   //////////////////////////////////////
//   // Check results
//   //////////////////////////////////////

//   // Print the buffer following the execution.
//   // It provides a lot of information such as measurement results.
//   buffer->print();

//   // Get just the measurement counts.
//   // This will return a dictionary type object {"measurement" ; counts}
//   auto measurements = buffer->getMeasurementCounts();
// }

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

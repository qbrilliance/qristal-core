#include "qb/core/circuit_builder.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "Circuit.hpp"
#include <gtest/gtest.h>
////////////////////////
// Other include statements
////////////////////////

TEST(ControlledQAETester, checksimple) {

  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

// Define the circuit we want to run
auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
auto circuit = gateRegistry->createComposite("circuit");

// Prepare the module
std::vector<int> qubits_string = {0, 1};
std::vector<int> qubits_superfluous_flags = {2, 3};
std::vector<int> qubits_metric = {4};
int beam_flag = 5;
std::vector<int> metric_amplitude_in_register = {6, 7, 8, 9};
std::vector<int> beam_amplitude_in_register = {10, 11, 12, 13};

// Prepare the state |string>|flags>|metric> = |00>|11>|0> + |10>|10>|1> +
// |10>|10>|0> + |11>|10>|1>
auto ae_state_prep_circ = gateRegistry->createComposite("ae_state_prep_circ");

for (auto bit : qubits_string) {
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("H", bit));
}
auto mc1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("GeneralisedMCX"));
std::vector<int> controls_off00 = qubits_string;
mc1->expand({{"target", qubits_superfluous_flags[0]},
             {"controls_off", controls_off00}});
ae_state_prep_circ->addInstruction(mc1);
auto mc2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("GeneralisedMCX"));
mc2->expand({{"target", qubits_superfluous_flags[1]},
             {"controls_off", controls_off00}});
ae_state_prep_circ->addInstruction(mc2);
auto mc3 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("GeneralisedMCX"));
std::vector<int> controls_on10 = {qubits_string[0]};
std::vector<int> controls_off10 = {qubits_string[1]};
mc3->expand({{"target", qubits_superfluous_flags[0]},
             {"controls_on", controls_on10},
             {"controls_off", controls_off10}});
ae_state_prep_circ->addInstruction(mc3);
auto mc4 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("GeneralisedMCX"));
mc4->expand({{"target", qubits_metric[0]},
             {"controls_on", controls_on10},
             {"controls_off", controls_off10}});
ae_state_prep_circ->addInstruction(mc4);
auto mc5 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("GeneralisedMCX"));
std::vector<int> controls_on01 = {qubits_string[1]};
std::vector<int> controls_off01 = {qubits_string[0]};
mc5->expand({{"target", qubits_superfluous_flags[0]},
             {"controls_on", controls_on01},
             {"controls_off", controls_off01}});
ae_state_prep_circ->addInstruction(mc5);
auto mc6 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("GeneralisedMCX"));
std::vector<int> controls_on11 = qubits_string;
mc6->expand(
    {{"target", qubits_superfluous_flags[0]}, {"controls_on", controls_on11}});
ae_state_prep_circ->addInstruction(mc6);
auto mc7 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("GeneralisedMCX"));
mc7->expand({{"target", qubits_metric[0]}, {"controls_on", controls_on11}});
ae_state_prep_circ->addInstruction(mc7);

auto swap = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("ControlledSwap"));
std::vector<int> controls_ons = {qubits_superfluous_flags[0]};
std::vector<int> controls_offs = {qubits_superfluous_flags[1],
                                  qubits_metric[0]};
swap->expand({{"qubits_a", std::vector<int>{qubits_string[0]}},
              {"qubits_b", std::vector<int>{qubits_string[1]}},
              {"flags_on", controls_ons},
              {"flags_off", controls_offs}});
ae_state_prep_circ->addInstruction(swap);

ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("CX", {static_cast<unsigned long>(qubits_string[0]), static_cast<unsigned long>(beam_flag)}));

circuit->addInstructions(ae_state_prep_circ->getInstructions());

std::vector<int> ae_state_qubits;
auto ae_state_qubits_set = qbOS::uniqueBitsQD(ae_state_prep_circ);
for (auto bit : ae_state_qubits_set) {
  ae_state_qubits.push_back(bit);
}

// Perform QAE on metric qubit
auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
std::shared_ptr<xacc::CompositeInstruction> ae_state_prep_circ_clone =
    xacc::ir::asComposite(ae_state_prep_circ->clone());
auto oracle = gateRegistry->createComposite("oracle");
oracle->addInstruction(gateRegistry->createInstruction("Z", qubits_metric[0]));
std::vector<int> current_eval_bits = metric_amplitude_in_register;
int num_evaluation_qubits = 4;
xacc::HeterogeneousMap options_ae{
    {"state_preparation_circuit", ae_state_prep_circ_clone},
    {"no_state_prep", true},
    {"oracle", oracle},
    {"evaluation_qubits", current_eval_bits},
    {"num_evaluation_qubits", num_evaluation_qubits},
    {"trial_qubits", ae_state_qubits},
    {"num_trial_qubits", (int)ae_state_qubits.size()}};
const bool expand_ok_ae = ae->expand(options_ae);
assert(expand_ok_ae);
circuit->addInstructions(ae->getInstructions());

ae_state_prep_circ->addInstructions(ae->getInstructions());
std::vector<int> ae_state_qubits2;
auto ae_state_qubits_set2 = qbOS::uniqueBitsQD(ae_state_prep_circ);
for (auto bit : ae_state_qubits_set2) {
  ae_state_qubits2.push_back(bit);
}

// Perform QAE on beam flag
auto ae2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
    xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
std::shared_ptr<xacc::CompositeInstruction> ae_state_prep_circ_clone2 =
    xacc::ir::asComposite(ae_state_prep_circ->clone());
auto oracle2 = gateRegistry->createComposite("oracle2");
oracle2->addInstruction(gateRegistry->createInstruction("Z", beam_flag));
std::vector<int> current_eval_bits2 = beam_amplitude_in_register;
int num_evaluation_qubits2 = 4;
xacc::HeterogeneousMap options_ae2{
    {"state_preparation_circuit", ae_state_prep_circ_clone2},
    {"no_state_prep", true},
    {"oracle", oracle2},
    {"evaluation_qubits", current_eval_bits2},
    {"num_evaluation_qubits", num_evaluation_qubits2},
    {"trial_qubits", ae_state_qubits2},
    {"num_trial_qubits", (int)ae_state_qubits2.size()}};
const bool expand_ok_ae2 = ae2->expand(options_ae2);
assert(expand_ok_ae2);
circuit->addInstruction(ae2);

// Add measurements
// for (auto bit : qubits_string) {
//   circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
// }
// for (auto bit : metric_amplitude_in_register) {
//   circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
// }
for (auto bit : beam_amplitude_in_register) {
  circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
}

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////

auto accelerator = xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
auto buffer = xacc::qalloc(14);
accelerator->execute(buffer, circuit);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////

// We expect that the output of ae is |0010> (or |0011>) (4 or 12)
// We expect that the output of ae2 will peak near |1010> (or |1101>) (5 or 11)
// The actual value that we want is 2/3 =approx= 4/5
buffer->print();


}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>
////////////////////////
// Other include statements
////////////////////////

TEST(QDBeamStatePrepCircuitTester, simple) {

  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  // Define the circuit we want to run
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("circuit");

  // Prepare the module

  std::vector<std::vector<float>> probability_table = {{0.5, 0.5}, {0.5, 0.5}};
  std::vector<int> qubits_string = {0, 1};
  std::vector<int> qubits_iteration = {2, 3, 4, 5};
  std::vector<int> qubits_metric = {6, 7};
  std::vector<int> qubits_next_letter = {8};
  std::vector<int> qubits_next_metric = {9};
  std::vector<int> qubits_is_occupied = {10, 11};
  int qubit_is_null = 12;
  int qubit_is_repetition = 13;
  int qubit_is_used = 14;
  std::vector<int> qubits_current_iteration = {15, 16};
  std::vector<int> qubits_ancilla_state_prep = {17, 18, 19, 20};
  std::vector<int> qubits_null = {21, 22};

  auto sp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("BeamStatePrep"));
  const bool expand_ok =
      sp->expand({{"qubits_string", qubits_string},
                  {"qubits_metric", qubits_metric},
                  {"qubits_next_letter", qubits_next_letter},
                  {"qubits_next_metric", qubits_next_metric},
                  {"probability_table", probability_table},
                  {"qubits_iteration", qubits_iteration},
                  {"qubits_is_occupied", qubits_is_occupied},
                  {"qubit_is_null", qubit_is_null},
                  {"qubit_is_repetition", qubit_is_repetition},
                  {"qubit_is_used", qubit_is_used},
                  {"qubits_null", qubits_null},
                  {"qubits_current_iteration", qubits_current_iteration},
                  {"qubits_ancilla_state_prep", qubits_ancilla_state_prep}});
  EXPECT_TRUE(expand_ok);

  // Add the module to the circuit
  circuit->addInstructions(sp->getInstructions());

  // Add measurements
  for (int q : qubits_string) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", q));
  }
  for (int q : qubits_iteration) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", q));
  }
  for (int q : qubits_metric) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", q));
  }
  for (int q : qubits_null) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", q));
  }

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////

  auto accelerator = xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(23);
  accelerator->execute(buffer, circuit);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////

  buffer->print();

  // The expected outputs are:
  // Beam --, metric 11, first iteration placed at the end second iteration at the start, both marked as null
  // --->>> |0001101111>
  // Beam a-, three components equally weighted from strings aa, a-, -a. All metrics 11. Iterations and nulls marked differently
  // --->>> |1010011100>, |1010011101>, |1001101101>
  
  int a = buffer->getMeasurementCounts()["0001101111"];
  int b = buffer->getMeasurementCounts()["1010011100"];
  int c = buffer->getMeasurementCounts()["1010011101"];
  int d = buffer->getMeasurementCounts()["1001101101"];

  EXPECT_GT(a, 0);
  EXPECT_GT(b, 0);
  EXPECT_GT(c, 0);
  EXPECT_GT(d, 0);
  EXPECT_EQ(a+b+c+d, 1024);
  
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}
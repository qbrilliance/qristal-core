// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>
#include "GateModifier.hpp"
#include "qbos_circuit_builder.hpp"

////////////////////////
// Other include statements
////////////////////////

TEST(AEtoMetricCircuitTester, checkWithAE) {
  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto test_circ = gateRegistry->createComposite("test_circ");
  std::vector<int> eval_bits = {0,1,2,3,4,5};
  std::vector<int> precision_bits = {1,2,3};
  std::vector<int> beam_metric = {6,7,8,9,10};
  int ones_idx = 2;
  std::vector<int> ancilla = {11,12,13,14,15,16,17,18,19,20}; // 3 * max(precision_bits) + 1
  
  // Prepare the evaluation bits = |1>|01>|101>
  // So that the final metric should be 1/4(1) + 1/4(2) + 25/64(4) = 2.3125 = |10010> (1/4s, 1/2s, 1s, 2s, 4s)
  test_circ->addInstruction(gateRegistry->createInstruction("X", eval_bits[0]));
  test_circ->addInstruction(gateRegistry->createInstruction("X", eval_bits[2]));
  test_circ->addInstruction(gateRegistry->createInstruction("X", eval_bits[3]));
  test_circ->addInstruction(gateRegistry->createInstruction("X", eval_bits[5]));


  // Perform AEtoMetric
  auto aetm = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("AEtoMetric"));
  const bool expand_ok =
      aetm->expand({{"evaluation_bits", eval_bits},
                    {"precision_bits", precision_bits},
                    {"qubits_ancilla", ancilla}, 
                    {"qubits_beam_metric", beam_metric},
                    {"qubits_beam_metric_ones_idx", ones_idx}});
  EXPECT_TRUE(expand_ok);
  test_circ->addInstruction(aetm);

  // Measure the final metric value
  for (int i = 0; i < eval_bits.size(); i++) {
    test_circ->addInstruction(
        gateRegistry->createInstruction("Measure", eval_bits[i]));
  }
  for (int i = 0; i < beam_metric.size(); i++) {
    test_circ->addInstruction(
        gateRegistry->createInstruction("Measure", beam_metric[i]));
  }
  for (int i = 0; i < ancilla.size(); i++) {
    test_circ->addInstruction(
        gateRegistry->createInstruction("Measure", ancilla[i]));
  }
  

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////
  //std::cout << test_circ->toString() << "\n";
  auto acc = xacc::getAccelerator("qsim", {{"shots", 1000}});
  auto buffer = xacc::qalloc(21);
  acc->execute(buffer, test_circ);

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////
  
  buffer->print();
  EXPECT_EQ(buffer->getMeasurementCounts()["101101100100000000000"], 1000);
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>
////////////////////////
// Other include statements
////////////////////////

// In this test

TEST(DecoderKernelCircuitTester, simple) {

  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto test_circ = gateRegistry->createComposite("test_circ");

  std::vector<std::vector<float>> probability_table = {{0.75, 0.25},
                                                       {0.25, 0.75}};

  int iterations = 2;
  std::vector<int> qubits_string = {0, 1};
  std::vector<int> qubits_metric = {2, 3, 4, 5};
  std::vector<int> qubits_ancilla_adder = {6};
  std::vector<int> qubits_init_null = {7,8};
  std::vector<int> qubits_init_repeat = {9,10};
  std::vector<int> qubits_superfluous_flags = {11,12};
  std::vector<int> qubits_beam_metric = {13,14,15,16,17};
  std::vector<int> qubits_ancilla_pool = {18,19,20,21,22,23,24,25,26,27};
  std::vector<int> evaluation_bits = {28,29,30,31,32,33};

  // Prepare initial state
  auto state_prep = gateRegistry->createComposite("state_prep");
  std::vector<int> qubits_next_letter = {qubits_ancilla_pool[0]};
  std::vector<int> qubits_next_metric = {qubits_ancilla_pool[1], qubits_ancilla_pool[2]};

  // Loop over rows of the probability table (i.e. over string length)
  for (int it = 0; it < iterations; it++) {
    // Initialize W prime unitary
    auto w_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("WPrime"));

    // Merge qubit register for W prime unitary into a heterogenous map
    xacc::HeterogeneousMap w_map = {{"iteration", it},
                                    {"qubits_next_letter", qubits_next_letter},
                                    {"qubits_next_metric", qubits_next_metric},
                                    {"probability_table", probability_table},
                                    {"qubits_init_null", qubits_init_null}};

    // Add qubit register to W prime
    w_prime->expand(w_map);

    // Add W prime unitary to state preparation circuit
    state_prep->addInstructions(w_prime->getInstructions());

    if (it > 0) {
      auto init_repeat = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("InitRepeatFlag"));
      xacc::HeterogeneousMap rep_map = {
          {"iteration", it},
          {"qubits_string", qubits_string},
          {"qubits_next_letter", qubits_next_letter},
          {"qubits_init_repeat", qubits_init_repeat}};
      init_repeat->expand(rep_map);
      // Add marking of repeat symbols to state preparation circuit
      state_prep->addInstructions(init_repeat->getInstructions());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////

    // Initialize U prime unitary
    auto u_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("UPrime"));

    // Merge qubit register for U prime unitary into a heterogenous map
    xacc::HeterogeneousMap u_map = {{"iteration", it},
                                    {"qubits_next_letter", qubits_next_letter},
                                    {"qubits_next_metric", qubits_next_metric},
                                    {"qubits_string", qubits_string},
                                    {"qubits_metric", qubits_metric}};

    // Add qubit register to U prime
    u_prime->expand(u_map);

    // Add U prime unitary to state preparation circuit
    state_prep->addInstructions(u_prime->getInstructions());

    /////////////////////////////////////////////////////////////////////////////////////////////

    // Initialize Q prime unitary
    auto q_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("QPrime"));

    // Merge qubit register for Q prime unitary into a heterogenous map
    xacc::HeterogeneousMap q_map = {{"iteration", it},
                                    {"qubits_next_letter", qubits_next_letter},
                                    {"qubits_next_metric", qubits_next_metric},
                                    {"qubits_string", qubits_string},
                                    {"qubits_metric", qubits_metric}};

    // Add qubit register to Q prime
    q_prime->expand(q_map);

    // Add Q prime unitary to state preparation circuit
    state_prep->addInstructions(q_prime->getInstructions());
  }

  int m = qubits_next_metric.size();
  int c_in = qubits_ancilla_pool[0];
  int k = round(0.5 + log2(iterations * (std::pow(2, m) - 1) + 1));
  std::vector<int> total_metric = {};

  for (int i = 0; i < m; i++) {
    total_metric.push_back(qubits_metric[i]);
  }

  for (int i = 0; i < qubits_ancilla_adder.size(); i++) {
    total_metric.push_back(qubits_ancilla_adder[i]);
  }

  for (int it = 1; it < iterations; it++) {
    std::vector<int> metrics = {};
    int start = it * m;
    int end = (it + 1) * m;

    for (int i = start; i < end; i++)
      metrics.push_back(qubits_metric[i]);

    for (int i = 0; i < total_metric.size() - 1 - m; i++) {
      metrics.push_back(qubits_ancilla_pool[i + 1]);
    }

    // Use ripple adder to add the qubits_metric at iteration 'it' to the total
    // metric vector
    auto adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("RippleCarryAdder"));
    bool expand_ok = adder->expand(
        {{"adder_bits", metrics}, {"sum_bits", total_metric}, {"c_in", c_in}});
    assert(expand_ok);

    // Add total metric to state preparation circuit
    state_prep->addInstructions(adder->getInstructions());
  }
  
  test_circ->addInstructions(state_prep->getInstructions());

  // Add the decoder kernel
    std::vector<int> qubits_total_metric = {qubits_metric[0], qubits_metric[1], qubits_ancilla_adder[0]};
    std::vector<int> precision_bits = {1,2,3};
    auto decoder_kernel =
    std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("DecoderKernel"));
    xacc::HeterogeneousMap options{
        {"qubits_string", qubits_string},
        {"qubits_metric", qubits_metric},
        {"qubits_ancilla_adder", qubits_ancilla_adder},
        {"qubits_init_null", qubits_init_null},
        {"qubits_init_repeat", qubits_init_repeat},
        {"qubits_superfluous_flags", qubits_superfluous_flags},
        {"qubits_beam_metric", qubits_beam_metric},
        {"total_metric", qubits_total_metric},
        {"evaluation_bits", evaluation_bits},
        {"precision_bits", precision_bits},
        {"qubits_ancilla_pool", qubits_ancilla_pool},
        {"metric_state_prep", state_prep}};
    const bool expand_ok = decoder_kernel->expand(options);
    EXPECT_TRUE(expand_ok);
    test_circ->addInstructions(decoder_kernel->getInstructions());

  // Measure the final strings and beam metrics
    for (int i = 0; i < qubits_string.size(); i++) {
        test_circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_string[i]));
    }
    for (int i = 0; i < total_metric.size(); i++) {
        test_circ->addInstruction(gateRegistry->createInstruction("Measure", total_metric[i]));
    }
    // for (int i = 0; i < qubits_superfluous_flags.size(); i++) {
    //     test_circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_superfluous_flags[i]));
    // }
    for (int i = 0; i < qubits_beam_metric.size(); i++) {
      test_circ->addInstruction(
          gateRegistry->createInstruction("Measure", qubits_beam_metric[i]));
    }
    // for (int i = 0; i < evaluation_bits.size(); i++) {
    //   test_circ->addInstruction(
    //       gateRegistry->createInstruction("Measure", evaluation_bits[i]));
    // }
    // test_circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_ancilla_pool[0]));
    

          
  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////
  std::cout << "running...\n";
  auto acc = xacc::getAccelerator("sparse-sim", {{"shots", 1000}});
  auto buffer = xacc::qalloc(37);
  xacc::ScopeTimer timer("timer");
  acc->execute(buffer, test_circ);
  std::cout << timer.getDurationMs() << " ms\n";
  std::cout << "done!\n";

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////

  buffer->print();
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}
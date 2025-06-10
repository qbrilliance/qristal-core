// Copyright (c) Quantum Brilliance Pty Ltd
#include <AllGateVisitor.hpp>
#include <CountGatesOfTypeVisitor.hpp>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <fstream>
#include <gtest/gtest.h>
#include <utility>
#include <random>
#include <qristal/core/noise_model/noise_model.hpp>
#include <qristal/core/passes/noise_aware_placement_pass.hpp>
#include <qristal/core/circuit_builder.hpp>

TEST(QBTketTester, checkSimple) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  const std::vector<std::pair<int, int>> test_connectivity(
      {{0, 1}, {1, 2}, {1, 3}, {1, 4}});
  auto acc = xacc::getAccelerator("qpp", {{"connectivity", test_connectivity}});
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      Ry(q[3], 1.234);
      CX(q[0], q[1]);
      CX(q[0], q[4]);
      Measure(q[0]);
      Measure(q[1]);
      Measure(q[2]);
      Measure(q[3]);
      Measure(q[4]);
    })",
                               acc)
                     ->getComposites()[0];

  auto tket = xacc::getIRTransformation("noise-aware");
  EXPECT_TRUE(tket != nullptr);
  tket->apply(program, acc);
  xacc::InstructionIterator it(program);
  while (it.hasNext()) {
    auto nextInst = it.next();
    if (nextInst->bits().size() > 1) {
      const std::pair<int, int> edge =
          nextInst->bits()[0] < nextInst->bits()[1]
              ? std::make_pair(nextInst->bits()[0], nextInst->bits()[1])
              : std::make_pair(nextInst->bits()[1], nextInst->bits()[0]);
      EXPECT_TRUE(xacc::container::contains(test_connectivity, edge));
    }
  }
  std::cout << "After placement:\n" << program->toString() << "\n";
}

TEST(QBTketTester, checkSwap) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  const std::vector<std::pair<int, int>> test_connectivity(
      {{0, 1}, {1, 2}, {2, 3}, {3, 4}});
  auto acc = xacc::getAccelerator("qpp", {{"connectivity", test_connectivity}});
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      CX(q[0], q[1]);
      CX(q[0], q[4]);
      CX(q[1], q[3]);
      CX(q[1], q[2]);
      CX(q[3], q[2]);
      Measure(q[0]);
      Measure(q[1]);
      Measure(q[2]);
      Measure(q[3]);
      Measure(q[4]);
    })",
                               acc)
                     ->getComposites()[0];

  auto tket = xacc::getIRTransformation("noise-aware");
  EXPECT_TRUE(tket != nullptr);
  tket->apply(program, acc);
  xacc::InstructionIterator it(program);
  while (it.hasNext()) {
    auto nextInst = it.next();
    if (nextInst->bits().size() > 1) {
      const std::pair<int, int> edge =
          nextInst->bits()[0] < nextInst->bits()[1]
              ? std::make_pair(nextInst->bits()[0], nextInst->bits()[1])
              : std::make_pair(nextInst->bits()[1], nextInst->bits()[0]);
      EXPECT_TRUE(xacc::container::contains(test_connectivity, edge));
    }
  }
  std::cout << "After placement:\n" << program->toString() << "\n";
}

TEST(QBTketTester, checkWithNoise) {
  const std::vector<std::pair<int, int>> test_connectivity({{0, 1},
                                                            {1, 2},
                                                            {2, 3},
                                                            {3, 4},
                                                            {4, 5},
                                                            {5, 6},
                                                            {6, 8},
                                                            {7, 8},
                                                            {1, 13},
                                                            {2, 12},
                                                            {3, 11},
                                                            {4, 10},
                                                            {5, 9}});
  auto accelerator =
      xacc::getAccelerator("qpp", {{"connectivity", test_connectivity}});
  // Allocate some qubits
  auto buffer = xacc::qalloc(3);
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto ir = xasmCompiler->compile(R"(__qpu__ void bell(qbit q) {
      H(q[0]);
      CX(q[0], q[1]);
      CX(q[0], q[2]);
      Measure(q[0]);
      Measure(q[1]);
      Measure(q[2]);
})",
                                  accelerator);
  auto program = ir->getComposites()[0];
  auto irt = xacc::getIRTransformation("noise-aware");
  const std::string BACKEND_JSON_FILE =
      std::string(TKET_TEST_RESOURCE_DIR) + "/backend.json";
  std::ifstream inFile;
  inFile.open(BACKEND_JSON_FILE);
  std::stringstream strStream;
  strStream << inFile.rdbuf();
  const std::string jsonStr = strStream.str();
  irt->apply(program, accelerator, {{"backend-json", jsonStr}});
  std::cout << "HOWDY: \n" << program->toString() << "\n";
  // NOTES: The noise model has been customized so that the trio {6, 7, 8}
  // qubits have good fidelity => check that the GHZ circuit is mapped to that
  // corner.
  const std::vector<size_t> GOOD_QUBITS{6, 7, 8};
  for (size_t instIdx = 0; instIdx < program->nInstructions(); ++instIdx) {
    auto instPtr = program->getInstruction(instIdx);
    // Check routing:
    if (instPtr->bits().size() == 2) {
      const std::pair<int, int> edge =
          instPtr->bits()[0] < instPtr->bits()[1]
              ? std::make_pair(instPtr->bits()[0], instPtr->bits()[1])
              : std::make_pair(instPtr->bits()[1], instPtr->bits()[0]);
      EXPECT_TRUE(xacc::container::contains(test_connectivity, edge));
    }
    for (const auto &qId : instPtr->bits()) {
      EXPECT_TRUE(xacc::container::contains(GOOD_QUBITS, qId));
    }
  }
}

TEST(QBTketTester, checkRedundancyRemoval) {
  auto tket = xacc::getIRTransformation("redundancy-removal");
  auto provider = xacc::getIRProvider("quantum");
  const auto test_cancellation = [&](auto gate1, auto gate2,
                                     bool should_cancel = true) {
    auto program = provider->createComposite("test");
    program->addInstruction(gate1);
    program->addInstruction(gate2);
    std::cout << "Before optimization:\n" << program->toString() << "\n";
    EXPECT_EQ(program->nInstructions(), 2);
    // Apply circuit optimization
    tket->apply(program, nullptr);
    std::cout << "After optimization:\n" << program->toString() << "\n";
    if (should_cancel) {
      EXPECT_EQ(program->nInstructions(), 0);
    } else {
      EXPECT_EQ(program->nInstructions(), 2);
    }
  };

  // H - H
  test_cancellation(std::make_shared<xacc::quantum::Hadamard>(0),
                    std::make_shared<xacc::quantum::Hadamard>(0));
  // X - X
  test_cancellation(std::make_shared<xacc::quantum::X>(0),
                    std::make_shared<xacc::quantum::X>(0));
  // Y - Y
  test_cancellation(std::make_shared<xacc::quantum::Y>(0),
                    std::make_shared<xacc::quantum::Y>(0));
  // Z - Z
  test_cancellation(std::make_shared<xacc::quantum::Z>(0),
                    std::make_shared<xacc::quantum::Z>(0));
  // S - S dag
  test_cancellation(std::make_shared<xacc::quantum::S>(0),
                    std::make_shared<xacc::quantum::Sdg>(0));
  // S dag - S
  test_cancellation(std::make_shared<xacc::quantum::Sdg>(0),
                    std::make_shared<xacc::quantum::S>(0));
  // T - T dag
  test_cancellation(std::make_shared<xacc::quantum::T>(0),
                    std::make_shared<xacc::quantum::Tdg>(0));
  // T dag - T
  test_cancellation(std::make_shared<xacc::quantum::Tdg>(0),
                    std::make_shared<xacc::quantum::T>(0));
  // Rx - Rx*
  test_cancellation(std::make_shared<xacc::quantum::Rx>(0, 1.234),
                    std::make_shared<xacc::quantum::Rx>(0, -1.234));
  // Ry - Ry*
  test_cancellation(std::make_shared<xacc::quantum::Ry>(0, 1.234),
                    std::make_shared<xacc::quantum::Ry>(0, -1.234));
  // Rz - Rz*
  test_cancellation(std::make_shared<xacc::quantum::Rz>(0, 1.234),
                    std::make_shared<xacc::quantum::Rz>(0, -1.234));
  // CNOT - CNOT
  test_cancellation(std::make_shared<xacc::quantum::CNOT>(0, 1),
                    std::make_shared<xacc::quantum::CNOT>(0, 1));
  // CZ - CZ
  test_cancellation(std::make_shared<xacc::quantum::CZ>(0, 1),
                    std::make_shared<xacc::quantum::CZ>(0, 1));
  // CH - CH
  test_cancellation(std::make_shared<xacc::quantum::CH>(0, 1),
                    std::make_shared<xacc::quantum::CH>(0, 1));

  // Test non cancellation
  // On different qubits
  test_cancellation(std::make_shared<xacc::quantum::Hadamard>(0),
                    std::make_shared<xacc::quantum::Hadamard>(1), false);
  // Swapping CNOT qubits (cannot combine)
  test_cancellation(std::make_shared<xacc::quantum::CNOT>(0, 1),
                    std::make_shared<xacc::quantum::CNOT>(1, 0), false);
}

TEST(QBTketTester, checkSimplifyInitial) {
  auto provider = xacc::getIRProvider("quantum");
  auto program = provider->createComposite("test");
  program->addInstruction(std::make_shared<xacc::quantum::Hadamard>(0));
  program->addInstruction(std::make_shared<xacc::quantum::X>(0));
  // Q1 is still 0 at this point -> this CNOT is effectively a noop
  program->addInstruction(std::make_shared<xacc::quantum::CNOT>(1, 2));
  program->addInstruction(std::make_shared<xacc::quantum::Y>(3));
  program->addInstruction(std::make_shared<xacc::quantum::Z>(3));
  // Similarly, Q2 is still 0 at this point -> this CNOT is effectively a noop
  program->addInstruction(std::make_shared<xacc::quantum::CNOT>(2, 3));
  // Similarly, Q1 is still 0 at this point -> this CZ is effectively a noop
  program->addInstruction(std::make_shared<xacc::quantum::CZ>(1, 2));

  std::cout << "Before optimization:\n" << program->toString() << "\n";
  {
    xacc::quantum::CountGatesOfTypeVisitor<xacc::quantum::CNOT> vis(program);
    EXPECT_EQ(vis.countGates(), 2);
  }
  {
    xacc::quantum::CountGatesOfTypeVisitor<xacc::quantum::CZ> vis(program);
    EXPECT_EQ(vis.countGates(), 1);
  }

  // Apply circuit optimization (contextual optimization)
  auto tket = xacc::getIRTransformation("simplify-initial");
  tket->apply(program, nullptr);
  std::cout << "After optimization:\n" << program->toString() << "\n";
  // CNOT and CZ are gone...
  {
    xacc::quantum::CountGatesOfTypeVisitor<xacc::quantum::CNOT> vis(program);
    EXPECT_EQ(vis.countGates(), 0);
  }
  {
    xacc::quantum::CountGatesOfTypeVisitor<xacc::quantum::CZ> vis(program);
    EXPECT_EQ(vis.countGates(), 0);
  }
}

TEST(QBTketTester, checkTwoQubitSquash) {
  // A two-qubit circuit that can be simplified (by squashing)
  auto provider = xacc::getIRProvider("quantum");
  auto program = provider->createComposite("test");
  program->addInstruction(std::make_shared<xacc::quantum::Rz>(0, -1.4));
  program->addInstruction(std::make_shared<xacc::quantum::Ry>(1, 1.0));
  program->addInstruction(std::make_shared<xacc::quantum::Rz>(0, 1.8));
  program->addInstruction(std::make_shared<xacc::quantum::CNOT>(1, 0));
  program->addInstruction(std::make_shared<xacc::quantum::Rz>(0, 0.5));
  program->addInstruction(std::make_shared<xacc::quantum::Rx>(0, 1.5));
  program->addInstruction(std::make_shared<xacc::quantum::CNOT>(0, 1));
  program->addInstruction(std::make_shared<xacc::quantum::Rz>(0, 1.2));
  program->addInstruction(std::make_shared<xacc::quantum::Rz>(0, 0.125));
  program->addInstruction(std::make_shared<xacc::quantum::Rx>(0, 1.15));
  program->addInstruction(std::make_shared<xacc::quantum::CNOT>(0, 1));
  program->addInstruction(std::make_shared<xacc::quantum::Rz>(0, 0.234));
  program->addInstruction(std::make_shared<xacc::quantum::Rz>(0, 0.55));
  program->addInstruction(std::make_shared<xacc::quantum::Rx>(0, 1.65));
  program->addInstruction(std::make_shared<xacc::quantum::CNOT>(0, 1));
  program->addInstruction(std::make_shared<xacc::quantum::Rz>(0, 0.12));
  std::cout << "Before optimization:\n" << program->toString() << "\n";
  {
    xacc::quantum::CountGatesOfTypeVisitor<xacc::quantum::CNOT> vis(program);
    // 4 CNOT gates
    EXPECT_EQ(vis.countGates(), 4);
  }
  auto tket = xacc::getIRTransformation("two-qubit-squash");
  tket->apply(program, nullptr);
  std::cout << "After optimization:\n" << program->toString() << "\n";
  {
    xacc::quantum::CountGatesOfTypeVisitor<xacc::quantum::CNOT> vis(program);
    // Only 2 CNOT gate remains 
    EXPECT_EQ(vis.countGates(), 2);
  }
}

TEST(QBTketTester, checkPeepHoleOptimise) {
  const std::string QASM_FILE =
      std::string(TKET_TEST_RESOURCE_DIR) + "/test_circuit.qasm";
  std::ifstream inFile;
  inFile.open(QASM_FILE);
  std::stringstream strStream;
  strStream << inFile.rdbuf();
  const std::string qasm = strStream.str();
  auto staq = xacc::getCompiler("staq");
  auto program = staq->compile(qasm)->getComposites()[0];
  std::cout << "Before optimization:\n" << program->toString() << "\n";
  xacc::quantum::CountGatesOfTypeVisitor<xacc::quantum::CNOT> vis_before(
      program);
  std::cout << "Number of CNOTs = " << vis_before.countGates() << "\n";
  // Apply peephole optimization
  auto tket = xacc::getIRTransformation("peephole-optimisation");
  tket->apply(program, nullptr);
  std::cout << "After optimization:\n" << program->toString() << "\n";
  xacc::quantum::CountGatesOfTypeVisitor<xacc::quantum::CNOT> vis_after(
      program);
  std::cout << "Number of CNOT = " << vis_after.countGates() << "\n";
  // Expect some reductions.
  EXPECT_LT(vis_after.countGates(), vis_before.countGates());
}

// Check gate squash + verify the correctness of XACC-TKET conversion
TEST(QBTketTester, checkGateConversion) {
  std::random_device rd;
  std::mt19937 en(rd());
  std::uniform_real_distribution<> dist(-M_PI, M_PI);
  auto provider = xacc::getIRProvider("quantum");
  auto program = provider->createComposite("test");
  // Buffers to run unoptimized/optimized circuits
  auto buffer1 = xacc::qalloc(1);
  auto buffer2 = xacc::qalloc(1);
  auto qpp = xacc::getAccelerator("qpp");
  // Test all single-qubit parameterized gates
  for (int i = 0; i < 10; ++i) {
    program->addInstruction(std::make_shared<xacc::quantum::Rx>(0, dist(en)));
    program->addInstruction(std::make_shared<xacc::quantum::Ry>(0, dist(en)));
    program->addInstruction(std::make_shared<xacc::quantum::Rz>(0, dist(en)));
    program->addInstruction(std::make_shared<xacc::quantum::U1>(0, dist(en)));
    program->addInstruction(
        std::make_shared<xacc::quantum::U>(0, dist(en), dist(en), dist(en)));
  }
  program->addInstruction(std::make_shared<xacc::quantum::Measure>(0));
  std::cout << "Before optimization:\n" << program->toString() << "\n";
  EXPECT_EQ(program->nInstructions(), 5 * 10 + 1);
  // Run simulation of the unoptimized circuit
  // Set seed for consistent sampling
  qpp->initialize({{"seed", 123}, {"shots", 1000}});
  qpp->execute(buffer1, program);
  buffer1->print();
  
  auto tket = xacc::getIRTransformation("peephole-optimisation");
  tket->apply(program, nullptr);
  std::cout << "After optimization:\n" << program->toString() << "\n";
  // Max: 3 rotations + 1 measure
  EXPECT_LE(program->nInstructions(), 4);
  // Run simulation of the optimized circuit
  // Set seed for consistent sampling
  qpp->initialize({{"seed", 123}, {"shots", 1000}});
  qpp->execute(buffer2, program);
  buffer2->print();
  // Check that the measurement distributions are exactly identical.
  EXPECT_EQ(buffer1->getMeasurementCounts(), buffer2->getMeasurementCounts());
}

TEST(QBTketTester, checkNoiseAwarePlacementFromNoiseModel) {
  // Make an empty noise model
  qristal::NoiseModel ring_noise_model;

  // Name the model whatever you like
  ring_noise_model.name = "ring_noise_model";

  // Define the gate fidelities (errors are 1 - fidelity)
  constexpr double u1_error = 1e-4;
  constexpr double u2_error = 1e-3;
  constexpr double u3_error = 1e-3;
  constexpr double cx_error = 1e-2;

  // Define the readout errors
  qristal::ReadoutError ro_error;
  ro_error.p_01 = 1e-2;
  ro_error.p_10 = 5e-3;
  constexpr int nb_qubits = 3;
  constexpr size_t bad_qubit = 0;
  // Loop over the qubits
  for (size_t qId = 0; qId < nb_qubits; ++qId) {
    // Set the readout errors
    ring_noise_model.set_qubit_readout_error(qId, ro_error);

    // Set the single-qubit gate fidelities
    if (qId == bad_qubit) {
      // Amplify the noise channels on bad qubits
      ring_noise_model.add_gate_error(
          qristal::DepolarizingChannel::Create(qId, 10 * u1_error), "u1", {qId});
      ring_noise_model.add_gate_error(
          qristal::DepolarizingChannel::Create(qId, 10 * u2_error), "u2", {qId});
      ring_noise_model.add_gate_error(
          qristal::DepolarizingChannel::Create(qId, 10 * u3_error), "u3", {qId});
    } else {
      ring_noise_model.add_gate_error(
          qristal::DepolarizingChannel::Create(qId, u1_error), "u1", {qId});
      ring_noise_model.add_gate_error(
          qristal::DepolarizingChannel::Create(qId, u2_error), "u2", {qId});
      ring_noise_model.add_gate_error(
          qristal::DepolarizingChannel::Create(qId, u3_error), "u3", {qId});
    }

    // Set the qubit connections to form a ring
    const size_t qId2 = (qId != nb_qubits - 1 ? qId + 1 : 0);
    ring_noise_model.add_qubit_connectivity(qId, qId2);

    // Set the corresponding two-qubit gate fidelities
    ring_noise_model.add_gate_error(
        qristal::DepolarizingChannel::Create(qId, qId2, cx_error), "cx",
        {qId, qId2});
    ring_noise_model.add_gate_error(
        qristal::DepolarizingChannel::Create(qId, qId2, cx_error), "cx",
        {qId2, qId});
  }

  auto noise_aware_placement =
      qristal::create_noise_aware_placement_pass(ring_noise_model);
  qristal::CircuitBuilder my_circuit;
  my_circuit.H(0);
  my_circuit.H(1);
  my_circuit.CNOT(0, 1);
  noise_aware_placement->apply(my_circuit);
  my_circuit.print();
  // Expected to map to
  // H q1
  // H q2
  // CNOT q1,q2
  // (qubit 0 has much stronger depolarizing noises => bad)
  auto xacc_ir = my_circuit.get();
  EXPECT_EQ(xacc_ir->getInstruction(0)->name(), "H");
  EXPECT_EQ(xacc_ir->getInstruction(0)->bits()[0], 1);
  EXPECT_EQ(xacc_ir->getInstruction(1)->name(), "H");
  EXPECT_EQ(xacc_ir->getInstruction(1)->bits()[0], 2);
  EXPECT_EQ(xacc_ir->getInstruction(2)->name(), "CNOT");
  EXPECT_EQ(xacc_ir->getInstruction(2)->bits()[0], 1);
  EXPECT_EQ(xacc_ir->getInstruction(2)->bits()[1], 2);
}
// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "CommonGates.hpp"
#include "Optimizer.hpp"
#include "xacc.hpp"
#include "xacc_observable.hpp"
#include "xacc_service.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <random>
TEST(QBSparseSimTester, checkSimple) {
  auto accelerator = xacc::getAccelerator("sparse-sim", {{"shots", 10000}});
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program1 = xasmCompiler
                      ->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      Measure(q[0]);
    })",
                                accelerator)
                      ->getComposites()[0];
  auto program2 = xasmCompiler
                      ->compile(R"(__qpu__ void test2(qbit q) {
      X(q[0]);
      Measure(q[0]);
    })",
                                accelerator)
                      ->getComposites()[0];

  auto buffer1 = xacc::qalloc(1);
  accelerator->execute(buffer1, program1);
  buffer1->print();
  EXPECT_EQ(buffer1->getMeasurementCounts().size(), 2);
  EXPECT_NEAR(buffer1->computeMeasurementProbability("0"), 0.5, 0.1);
  EXPECT_NEAR(buffer1->computeMeasurementProbability("1"), 0.5, 0.1);

  auto buffer2 = xacc::qalloc(1);
  accelerator->execute(buffer2, program2);
  buffer2->print();
  EXPECT_NEAR(buffer2->computeMeasurementProbability("1"), 1.0, 1e-9);
}

TEST(QBSparseSimTester, testBell) {
  auto accelerator = xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto ir = xasmCompiler->compile(R"(__qpu__ void bell(qbit q) {
      H(q[0]);
      CX(q[0], q[1]);
      CX(q[1], q[2]);
      Measure(q[0]);
      Measure(q[1]);
      Measure(q[2]);
    })",
                                  accelerator);

  auto program = ir->getComposite("bell");
  auto buffer = xacc::qalloc(3);
  accelerator->execute(buffer, program);
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 2);
  auto prob0 = buffer->computeMeasurementProbability("000");
  auto prob1 = buffer->computeMeasurementProbability("111");
  buffer->print();
  EXPECT_NEAR(prob0 + prob1, 1.0, 1e-9);
  EXPECT_NEAR(prob0, 0.5, 0.2);
  EXPECT_NEAR(prob1, 0.5, 0.2);
}

TEST(QBSparseSimTester, testDeuteron) {
  auto accelerator = xacc::getAccelerator("sparse-sim", {{"shots", 100000}});
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto ir = xasmCompiler->compile(R"(__qpu__ void ansatz(qbit q, double t) {
      X(q[0]);
      Ry(q[1], t);
      CX(q[1], q[0]);
      H(q[0]);
      H(q[1]);
      Measure(q[0]);
      Measure(q[1]);
    })",
                                  accelerator);

  auto program = ir->getComposite("ansatz");
  // Expected results from deuteron_2qbit_xasm_X0X1
  const std::vector<double> expectedResults{
      0.0,       -0.324699, -0.614213, -0.837166, -0.9694,
      -0.996584, -0.915773, -0.735724, -0.475947, -0.164595,
      0.164595,  0.475947,  0.735724,  0.915773,  0.996584,
      0.9694,    0.837166,  0.614213,  0.324699,  0.0};

  const auto angles =
      xacc::linspace(-xacc::constants::pi, xacc::constants::pi, 20);
  for (size_t i = 0; i < angles.size(); ++i) {
    auto buffer = xacc::qalloc(2);
    auto evaled = program->operator()({angles[i]});
    accelerator->execute(buffer, evaled);
    std::cout << "Angle = " << angles[i]
              << "; result = " << buffer->getExpectationValueZ() << " vs. "
              << expectedResults[i] << "\n";
    EXPECT_NEAR(buffer->getExpectationValueZ(), expectedResults[i], 0.1);
  }
}

TEST(QBSparseSimTester, testMultiControlledGateNativeSim) {
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto x = std::make_shared<xacc::quantum::X>(0);
  std::shared_ptr<xacc::CompositeInstruction> comp =
      gateRegistry->createComposite("__COMPOSITE__X");
  comp->addInstruction(x);
  auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("C-U"));
  // Testing many controls, only possible (complete in reasonable time) with
  // custom C-U handler
  const std::vector<int> ctrl_idxs{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  const auto nQubits = ctrl_idxs.size() + 1;
  mcx->expand({{"U", comp}, {"control-idx", ctrl_idxs}});
  std::cout << "HOWDY: Gate count: " << mcx->nInstructions() << "\n";
  // Test truth table
  auto acc = xacc::getAccelerator("sparse-sim", {{"shots", 100}});
  std::vector<std::shared_ptr<xacc::Instruction>> xGateVec;
  std::vector<std::shared_ptr<xacc::Instruction>> measGateVec;
  for (size_t i = 0; i < nQubits; ++i) {
    xGateVec.emplace_back(gateRegistry->createInstruction("X", {i}));
    measGateVec.emplace_back(gateRegistry->createInstruction("Measure", {i}));
  }

  const auto runTestCase = [&](const std::vector<bool> &bitVals) {
    static int counter = 0;
    auto composite = gateRegistry->createComposite("__TEMP_COMPOSITE__" +
                                                   std::to_string(counter));
    counter++;
    // State prep:
    assert(bitVals.size() == nQubits);
    std::string inputBitString;
    for (int i = 0; i < bitVals.size(); ++i) {
      if (bitVals[i]) {
        composite->addInstruction(xGateVec[i]);
      }
      inputBitString.append((bitVals[i] ? "1" : "0"));
    }

    // Add mcx
    composite->addInstruction(mcx);
    // Mesurement:
    composite->addInstructions(measGateVec);
    auto buffer = xacc::qalloc(nQubits);
    acc->execute(buffer, composite);
    // std::cout << "Input bitstring: " << inputBitString << "\n";
    // buffer->print();
    // MCX gate:
    const auto expectedBitString = [&inputBitString]() -> std::string {
      // If all control bits are 1's
      // q0q1q2q3q4
      const std::string pattern1(inputBitString.size(), '1');
      const std::string pattern0 = [&]() {
        std::string tmp(inputBitString.size(), '1');
        tmp[0] = '0';
        return tmp;
      }();
      if (inputBitString == pattern0) {
        return pattern1;
      }
      if (inputBitString == pattern1) {
        return pattern0;
      }
      // Otherwise, no changes
      return inputBitString;
    }();
    // Check bit string
    EXPECT_NEAR(buffer->computeMeasurementProbability(expectedBitString), 1.0,
                0.1);
    // std::cout << "Circuit: \n" << composite->toString() << "\n";
  };

  for (int i = 0; i < (1 << nQubits); ++i) {
    std::vector<bool> bits;
    for (int q = 0; q < nQubits; ++q) {
      bits.emplace_back((i & (1 << q)) == (1 << q));
    }
    runTestCase(bits);
  }
}

TEST(QBSparseSimTester, testDeuteronVqeH3Shots) {
  const int nbShots = 100000;
  auto accelerator = xacc::getAccelerator("sparse-sim", {{"shots", nbShots}});

  // Create the N=3 deuteron Hamiltonian
  auto H_N_3 = xacc::quantum::getObservable(
      "pauli",
      std::string("5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1 + "
                  "9.625 - 9.625 Z2 - 3.91 X1 X2 - 3.91 Y1 Y2"));

  auto optimizer = xacc::getOptimizer("nlopt", {{"nlopt-maxeval", 50}});

  // JIT map Quil QASM Ansatz to IR
  xacc::qasm(R"(
        .compiler xasm
        .circuit deuteron_ansatz_h3_2
        .parameters t0, t1
        .qbit q
        X(q[0]);
        exp_i_theta(q, t0, {{"pauli", "X0 Y1 - Y0 X1"}});
        exp_i_theta(q, t1, {{"pauli", "X0 Z1 Y2 - X2 Z1 Y0"}});
    )");
  auto ansatz = xacc::getCompiled("deuteron_ansatz_h3_2");

  // Get the VQE Algorithm and initialize it
  auto vqe = xacc::getAlgorithm("vqe");
  vqe->initialize({{"ansatz", ansatz},
                   {"observable", H_N_3},
                   {"accelerator", accelerator},
                   {"optimizer", optimizer}});

  xacc::set_verbose(true);
  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(3);
  vqe->execute(buffer);

  // Expected result: -2.04482
  // Tol: 0.25 (~10% of the true value)
  // (since we are using shots, hence will introduce randomness to the
  // optimizer)
  std::cout << "Energy = " << (*buffer)["opt-val"].as<double>() << "\n";
  // EXPECT_NEAR((*buffer)["opt-val"].as<double>(), -2.04482, 0.25);
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

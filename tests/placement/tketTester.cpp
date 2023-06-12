// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "AllGateVisitor.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <utility>

TEST(QBTketPlacementTester, checkSimple) {
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

TEST(QBTketPlacementTester, checkSwap) {
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

TEST(QBTketPlacementTester, checkWithNoise) {
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
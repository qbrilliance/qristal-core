#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>

TEST(QBLambdaTester, checkSimple) {
  auto accelerator = xacc::getAccelerator("qb-lambda", {{"device", "CPU"}});
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      Measure(q[0]);
    })",
                               accelerator)
                     ->getComposites()[0];

  auto buffer = xacc::qalloc(1);
  accelerator->execute(buffer, program);
  buffer->print();
}

TEST(QBLambdaTester, checkShots) {
  const int NB_SHOTS = 100;
  auto accelerator = xacc::getAccelerator(
      "qb-lambda", {{"device", "CPU"}, {"shots", NB_SHOTS}});
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      X(q[0]);
      Measure(q[0]);
    })",
                               accelerator)
                     ->getComposites()[0];

  auto buffer = xacc::qalloc(1);
  accelerator->execute(buffer, program);
  buffer->print();
  EXPECT_EQ(buffer->getMeasurementCounts()["1"], NB_SHOTS);
}

TEST(QBLambdaTester, checkMPS) {
  const int NB_SHOTS = 100;
  auto accelerator = xacc::getAccelerator(
      "qb-lambda", {{"device", "CPU"}, {"shots", NB_SHOTS}});
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test1(qbit q) {
      H(q[0]);
      for (int i = 0; i < 49; i ++) {
        CNOT(q[i], q[i+1]);
      }
      for (int i = 0; i < 50; i ++) {
        Measure(q[i]);
      }
  })",
                               accelerator)
                     ->getComposites()[0];
  const int nbQubits = 50;
  auto buffer = xacc::qalloc(nbQubits);
  accelerator->execute(buffer, program);
  buffer->print();
  const std::string allOnes(nbQubits, '1');
  const std::string allZeros(nbQubits, '0');
  const int nbOnes = buffer->getMeasurementCounts()[allOnes];
  const int nbZeros = buffer->getMeasurementCounts()[allZeros];
  EXPECT_EQ(nbOnes + nbZeros, NB_SHOTS);
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

#include "Algorithm.hpp"
#include "Optimizer.hpp"
#include "xacc.hpp"
#include "xacc_observable.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>

namespace {
template <typename T> std::vector<T> linspace(T a, T b, size_t N) {
  T h = (b - a) / static_cast<T>(N - 1);
  std::vector<T> xs(N);
  typename std::vector<T>::iterator x;
  T val;
  for (x = xs.begin(), val = a; x != xs.end(); ++x, val += h) {
    *x = val;
  }
  return xs;
}
} // namespace

TEST(AWSAcceleratorTester, testExpVal) {
  auto accelerator = xacc::getAccelerator("aws_acc");
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto openqasmCompiler = xacc::getCompiler("staq");
  auto program1 = xasmCompiler
                      ->compile(R"(__qpu__ void test1(qbit q) {
      H(q[1]);
      CNOT(q[1], q[2]);
      CNOT(q[0], q[1]);
      H(q[0]);
    })",
                                accelerator)
                      ->getComposites()[0];

  auto program2 = xasmCompiler
                      ->compile(R"(__qpu__ void test1(qbit q) {
   X(q[3]);
    // Hadamard on all qubits
    H(q[0]);
    H(q[1]);
    H(q[2]);
    H(q[3]);
    // Balanced Oracle
    X(q[0]);
    X(q[2]);
    CX(q[0],q[3]);
    CX(q[1],q[3]);
    CX(q[2],q[3]);
    X(q[0]);
    X(q[2]);   
    // Hadamard on q[0-2]
    H(q[0]);
    H(q[1]);
    H(q[2]);
    })",
                                accelerator)
                      ->getComposites()[0];

  auto buffer1 = xacc::qalloc(1);
  accelerator->execute(buffer1, {program1, program2});
  buffer1->print();
  //  auto buffer2 = xacc::qalloc(1);
  //  accelerator->execute(buffer2, program2);
}

int main(int argc, char **argv) {
  xacc::Initialize();
  ::testing::InitGoogleTest(&argc, argv);
  const auto result = RUN_ALL_TESTS();
  xacc::Finalize();
  return result;
}

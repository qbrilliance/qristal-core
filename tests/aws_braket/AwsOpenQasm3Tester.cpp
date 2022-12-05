#include "qb/core/aws_braket/AWSOpenQasm3Visitor.hpp"
#include "Algorithm.hpp"
#include "InstructionIterator.hpp"
#include "Optimizer.hpp"
#include "xacc.hpp"
#include "xacc_observable.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>

TEST(AwsOpenQasm3Tester, checkSimple) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void bell(qbit q) {
      H(q[0]);
      CX(q[0], q[1]);
      Measure(q[0]);
      Measure(q[1]);
    })",
                               nullptr)
                     ->getComposites()[0];

  qb::AWSOpenQASM3Visitor visitor(program->nPhysicalBits());
  xacc::InstructionIterator it(program);
  while (it.hasNext()) {
    auto nextInst = it.next();
    if (nextInst->isEnabled()) {
      nextInst->accept(&visitor);
    }
  }

  const std::string openqasm_str = visitor.getOpenQasm();
  std::cout << "HOWDY:\n" << openqasm_str << "\n";
}

TEST(AwsOpenQasm3Tester, checkT1) {
  auto xasmCompiler = xacc::getCompiler("xasm");
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void bell(qbit q) {
      X(q[0]);
      for (int i = 0; i < 10000; i++) {
        I(q[0]);
      }
      Measure(q[0]);
    })",
                               nullptr)
                     ->getComposites()[0];

  auto accelerator =
      xacc::getAccelerator("aws_acc", {{"format", "OPENQASM 3"}});
  auto buffer = xacc::qalloc(1);
  accelerator->execute(buffer, program);
  buffer->print();
}

TEST(AwsOpenQasm3Tester, checkMeasureSubset) {
  xacc::set_verbose(true);
  auto xasmCompiler = xacc::getCompiler("xasm");
  // Note: we double up the CX just to add more noise.
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void GHZ(qbit q) {
      H(q[0]);
      for (int i = 0; i < 10; i++) {
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
        CX(q[i], q[i + 1]);
      }
      Measure(q[0]);
      Measure(q[7]);
    })",
                               nullptr)
                     ->getComposites()[0];

  auto accelerator =
      xacc::getAccelerator("aws_acc", {{"format", "OPENQASM 3"}});
  auto buffer = xacc::qalloc(11);
  accelerator->execute(buffer, program);
  buffer->print();
}


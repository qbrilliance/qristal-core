// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/session.hpp>
#include <gtest/gtest.h>
#include <qristal/core/backend.hpp>
#include <qristal/core/session.hpp>
#include <xacc.hpp>

TEST(backendTester, checkOutputQASM) {

  qristal::backend acc;

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
  auto buffer = xacc::qalloc(2);
  acc.execute(buffer, program);
  auto transpiled_qasm = acc.getTranspiledResult();
  // Try recompile it with staq for testing
  auto staq = xacc::getCompiler("staq");
  auto reconstructed = staq->compile(transpiled_qasm, nullptr)->getComposites()[0];
  // H -> Ry--Rx
  EXPECT_EQ(reconstructed->getInstruction(0)->name(), "Ry");
  EXPECT_EQ(reconstructed->getInstruction(1)->name(), "Rx");
  // CX -> H - CZ - H
  //H
  EXPECT_EQ(reconstructed->getInstruction(2)->name(), "Ry");
  EXPECT_EQ(reconstructed->getInstruction(3)->name(), "Rx");
  // CZ
  EXPECT_EQ(reconstructed->getInstruction(4)->name(), "CZ");
  // H
  EXPECT_EQ(reconstructed->getInstruction(5)->name(), "Ry");
  EXPECT_EQ(reconstructed->getInstruction(6)->name(), "Rx");
  // 2 measures
  EXPECT_EQ(reconstructed->getInstruction(7)->name(), "Measure");
  EXPECT_EQ(reconstructed->getInstruction(8)->name(), "Measure");
}

TEST(backendTester, checkSessionIntegration1) {
  // Make a Qristal session
  qristal::session my_sim;
  // Set up sensible default parameters
  my_sim.qn = 2;
  my_sim.instring = R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[2];
    h q[0];
    CX q[0], q[1];
    )";
  my_sim.acc = "aer";
  my_sim.nooptimise = false;
  my_sim.noplacement = false;
  my_sim.noise = true;
  // Don't run sim, just checking transpilation
  my_sim.execute_circuit = false;
  my_sim.run();

  std::cout << "Transpiled circuit: \n" << my_sim.transpiled_circuit() << "\n";
  // Check profiling: expected native gate transpilation
  // ry(1.5708000000000000) q[0];
  // rx(3.1415899999999999) q[0];
  // ry(1.5708000000000000) q[1];
  // rx(3.1415899999999999) q[1];
  // cz q[0], q[1];
  // ry(1.5708000000000000) q[1];
  // rx(3.1415899999999999) q[1];

  // Check single qubit gate counts
  EXPECT_EQ(my_sim.one_qubit_gate_depths().size(), 2);
  // 2 single qubit gates on Q0
  EXPECT_EQ(my_sim.one_qubit_gate_depths().at(0), 2);
  // 4 single qubit gates on Q1
  EXPECT_EQ(my_sim.one_qubit_gate_depths().at(1), 4);

  // Check two qubit gate counts
  EXPECT_EQ(my_sim.two_qubit_gate_depths().size(), 2);
  // Single CZ
  EXPECT_EQ(my_sim.two_qubit_gate_depths().at(0), 1);
  EXPECT_EQ(my_sim.two_qubit_gate_depths().at(1), 1);
}

TEST(backendTester, checkSessionIntegration2) {
  // Make a Qristal session
  qristal::session my_sim;
  my_sim.qn = 2;
  my_sim.acc = "aer";
  // More complicated gate: swap -> CX -> CZ transpilation
  my_sim.instring = R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[2];
    swap q[0], q[1];
    )";
  my_sim.nooptimise = false;
  my_sim.noplacement = false;
  my_sim.noise = true;
  // Don't run sim, just checking transpilation
  my_sim.execute_circuit = false;
  my_sim.run();

  std::cout << "Transpiled circuit: \n" << my_sim.transpiled_circuit() << "\n";

  // Check single qubit gate counts
  EXPECT_EQ(my_sim.one_qubit_gate_depths().size(), 2);
  // 3 single qubit gates on Q0
  EXPECT_EQ(my_sim.one_qubit_gate_depths().at(0), 3);
  // 3 single qubit gates on Q1
  EXPECT_EQ(my_sim.one_qubit_gate_depths().at(1), 3);

  // Check two qubit gate counts
  EXPECT_EQ(my_sim.two_qubit_gate_depths().size(), 2);
  // 3 CZ's
  EXPECT_EQ(my_sim.two_qubit_gate_depths().at(0), 3);
  EXPECT_EQ(my_sim.two_qubit_gate_depths().at(1), 3);
}

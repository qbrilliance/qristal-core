// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#include "qb/core/session.hpp"
#include <gtest/gtest.h>
#include "qb/core/QuantumBrillianceAccelerator.hpp"
#include "qb/core/session.hpp"

TEST(QuantumBrillianceAcceleratorTester, checkOutputQASM) {
  qb::QuantumBrillianceAccelerator acc;

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
  std::cout << "HOWDY:\n" << transpiled_qasm << "\n";
  // Try recompile it with staq for testing
  auto staq = xacc::getCompiler("staq");
  auto reconstructed = staq->compile(transpiled_qasm, nullptr)->getComposites()[0];
  std::cout << "HELLO:\n" << reconstructed->toString() << "\n";
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

TEST(QuantumBrillianceAcceleratorTester, checkSessionIntegration1) {
    // Make a QB SDK session
  auto my_sim = qb::session(false);
  // Set up sensible default parameters
  my_sim.qb12();
  my_sim.set_qn(2);
  my_sim.set_instring(R"(
OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
h q[0];
CX q[0], q[1];
)");
  my_sim.set_nooptimise(false);
  my_sim.set_noplacement(false);
  // Don't run sim, just checking transpilation
  my_sim.set_nosim(true);
  my_sim.run();

  const std::string transpiled_circuit_qasm =
      my_sim.get_out_transpiled_circuits()[0][0];
  std::cout << "Transpiled circuit: \n" << transpiled_circuit_qasm << "\n";
  // Check profiling: expected native gate transpilation
  // ry(1.5708000000000000) q[0];
  // rx(3.1415899999999999) q[0];
  // ry(1.5708000000000000) q[1];
  // rx(3.1415899999999999) q[1];
  // cz q[0], q[1];
  // ry(1.5708000000000000) q[1];
  // rx(3.1415899999999999) q[1];
  
  // Check single qubit gate counts
  EXPECT_EQ(my_sim.get_out_single_qubit_gate_qtys()[0][0].size(), 2);
  // 2 single qubit gates on Q0
  EXPECT_EQ(my_sim.get_out_single_qubit_gate_qtys()[0][0].at(0), 2);
  // 4 single qubit gates on Q1
  EXPECT_EQ(my_sim.get_out_single_qubit_gate_qtys()[0][0].at(1), 4);

  // Check two qubit gate counts
  EXPECT_EQ(my_sim.get_out_double_qubit_gate_qtys()[0][0].size(), 2);
  // Single CZ
  EXPECT_EQ(my_sim.get_out_double_qubit_gate_qtys()[0][0].at(0), 1);
  EXPECT_EQ(my_sim.get_out_double_qubit_gate_qtys()[0][0].at(1), 1);
}

TEST(QuantumBrillianceAcceleratorTester, checkSessionIntegration2) {
    // Make a QB SDK session
  auto my_sim = qb::session(false);
  // Set up sensible default parameters
  my_sim.qb12();
  my_sim.set_qn(2);
  // More complicated gate: swap -> CX -> CZ transpilation
  my_sim.set_instring(R"(
OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
swap q[0], q[1];
)");
  my_sim.set_nooptimise(false);
  my_sim.set_noplacement(false);
  // Don't run sim, just checking transpilation
  my_sim.set_nosim(true);
  my_sim.run();

  const std::string transpiled_circuit_qasm =
      my_sim.get_out_transpiled_circuits()[0][0];
  std::cout << "Transpiled circuit: \n" << transpiled_circuit_qasm << "\n";
  // Check profiling: expected native gate transpilation
  // ry(1.5708000000000000) q[1];
  // rx(3.1415899999999999) q[1];
  // cz q[0], q[1];
  // ry(1.5708000000000000) q[1];
  // rx(3.1415899999999999) q[1];
  // ry(1.5708000000000000) q[0];
  // rx(3.1415899999999999) q[0];
  // cz q[1], q[0];
  // ry(1.5708000000000000) q[0];
  // rx(3.1415899999999999) q[0];
  // ry(1.5708000000000000) q[1];
  // rx(3.1415899999999999) q[1];
  // cz q[0], q[1];
  // ry(1.5708000000000000) q[1];
  // rx(3.1415899999999999) q[1];
  
  // Check single qubit gate counts
  EXPECT_EQ(my_sim.get_out_single_qubit_gate_qtys()[0][0].size(), 2);
  // 4 single qubit gates on Q0
  EXPECT_EQ(my_sim.get_out_single_qubit_gate_qtys()[0][0].at(0), 4);
  // 8 single qubit gates on Q1
  EXPECT_EQ(my_sim.get_out_single_qubit_gate_qtys()[0][0].at(1), 8);

  // Check two qubit gate counts
  EXPECT_EQ(my_sim.get_out_double_qubit_gate_qtys()[0][0].size(), 2);
  // 3 CZ's
  EXPECT_EQ(my_sim.get_out_double_qubit_gate_qtys()[0][0].at(0), 3);
  EXPECT_EQ(my_sim.get_out_double_qubit_gate_qtys()[0][0].at(1), 3);
}

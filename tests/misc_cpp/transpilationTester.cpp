// Copyright (c) Quantum Brilliance Pty Ltd

// Gtest
#include <gtest/gtest.h>

// Qristal
#include <qristal/core/backends/qb_hardware/qb_visitor.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/passes/circuit_opt_passes.hpp>
#include <qristal/core/session.hpp>

// Xacc
#include <xacc.hpp>
#include <xacc_service.hpp>

TEST(transpilationTester, checkCZOptimization) {
  // Make a Qristal session
  auto my_sim = qristal::session();
  my_sim.qn = 2;
  my_sim.instring = R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[2];
    h q[1];
    CX q[0], q[1];
    h q[1];
    )";
  // Only optimization is enabled: check that it can optimize this circuit to a single CZ.
  my_sim.nooptimise = false;
  my_sim.noplacement = true;
  my_sim.execute_circuit = false;
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  const std::string transpiled_circuit_qasm = my_sim.transpiled_circuit();
  std::cout << "Transpiled circuit: \n" << transpiled_circuit_qasm << "\n";
  // recompile the qasm string for validation
  auto compiler = xacc::getCompiler("staq");
  auto program = compiler->compile(transpiled_circuit_qasm, nullptr)->getComposites()[0];
  // Expect a single "CZ" gate
  EXPECT_EQ(program->nInstructions(), 1);
  EXPECT_EQ(program->getInstruction(0)->name(), "CZ");
}

TEST(transpilationTester, checkCZPlacement) {
  // Make a Qristal session
  auto my_sim = qristal::session();
  my_sim.qn = 2;
  my_sim.acc = "aer";
  my_sim.instring = R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[2];
    cz q[0], q[1];
    )";
  // Enable only placement
  my_sim.nooptimise = true;
  my_sim.noise = true;
  my_sim.noplacement = false;
  my_sim.execute_circuit = false;
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  const std::string transpiled_circuit_qasm = my_sim.transpiled_circuit();
  std::cout << "Transpiled circuit: \n" << transpiled_circuit_qasm << "\n";
  // recompile the qasm string for validation
  auto compiler = xacc::getCompiler("staq");
  auto program = compiler->compile(transpiled_circuit_qasm, nullptr)->getComposites()[0];
  // Expect it is still a single "CZ" gate
  EXPECT_EQ(program->nInstructions(), 1);
  EXPECT_EQ(program->getInstruction(0)->name(), "CZ");
}

TEST(transpilationTester, checkAngleNorm) {
  auto vis = xacc::quantum::qb_visitor(0);
  const double pi = xacc::constants::pi;
  EXPECT_DOUBLE_EQ(vis.norm(0.), 0.);
  EXPECT_DOUBLE_EQ(vis.norm(pi/3.), pi/3.);
  EXPECT_DOUBLE_EQ(vis.norm(-pi/6.), -pi/6.);
  EXPECT_DOUBLE_EQ(std::abs(vis.norm(pi)), std::abs(pi));
  EXPECT_DOUBLE_EQ(std::abs(vis.norm(-pi)), std::abs(-pi));
  EXPECT_DOUBLE_EQ(vis.norm(2.*pi), 0.);
  EXPECT_DOUBLE_EQ(vis.norm(-2.*pi), 0.);
  EXPECT_DOUBLE_EQ(vis.norm(-3.*pi/2.), pi/2.);
  EXPECT_DOUBLE_EQ(vis.norm(3.*pi/2.), -pi/2.);
  EXPECT_DOUBLE_EQ(vis.norm(5.*pi+0.01), -pi+0.01);
  EXPECT_DOUBLE_EQ(vis.norm(-5.), 2*pi-5.);
  EXPECT_DOUBLE_EQ(vis.norm(5.), 5.-2.*pi);
}

TEST(transpilationTester, checkCircuitOptimisationFailure) {
  qristal::CircuitBuilder circuit;
  circuit.CNOT(0, 1);
  circuit.CZ(0, 1);
  circuit.CNOT(0, 1);

  std::shared_ptr<qristal::CircuitPass> opt_pass = qristal::create_circuit_optimizer_pass();
  ASSERT_THROW(opt_pass->apply(circuit), std::runtime_error);
}
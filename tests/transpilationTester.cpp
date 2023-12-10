// Copyright (c) Quantum Brilliance Pty Ltd

#include "qb/core/session.hpp"
#include <gtest/gtest.h>
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "qb/core/noise_model/noise_model.hpp"
#include "qb/core/backends/qb_hardware/qb_visitor.hpp"

TEST(transpilationTester, checkCZOptimization) {
  // Make a QB SDK session
  auto my_sim = qb::session(false);
  // Set up sensible default parameters
  my_sim.qb12();
  my_sim.set_qn(2);
  my_sim.set_instring(R"(
OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
h q[1];
CX q[0], q[1];
h q[1];
)");
  // Only optimization is enabled: check that it can optimize this circuit to a
  // single CZ.
  my_sim.set_nooptimise(false);
  my_sim.set_noplacement(true);
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  const std::string transpiled_circuit_qasm =
      my_sim.get_out_transpiled_circuits()[0][0];
  std::cout << "Transpiled circuit: \n" << transpiled_circuit_qasm << "\n";
  // recompile the qasm string for validation
  auto compiler = xacc::getCompiler("staq");
  auto program =
      compiler->compile(transpiled_circuit_qasm, nullptr)->getComposites()[0];
  // Expect a single "CZ" gate
  EXPECT_EQ(program->nInstructions(), 1);
  EXPECT_EQ(program->getInstruction(0)->name(), "CZ");
}

TEST(transpilationTester, checkCZPlacement) {
  // Make a QB SDK session
  auto my_sim = qb::session(false);
  // Set up sensible default parameters
  my_sim.qb12();
  my_sim.set_qn(2);
  my_sim.set_instring(R"(
OPENQASM 2.0;
include "qelib1.inc";
qreg q[2];
cz q[0], q[1];
)");
  // Enable only placement 
  my_sim.set_nooptimise(true);
  my_sim.set_noplacement(false);
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  const std::string transpiled_circuit_qasm =
      my_sim.get_out_transpiled_circuits()[0][0];
  std::cout << "Transpiled circuit: \n" << transpiled_circuit_qasm << "\n";
  // recompile the qasm string for validation
  auto compiler = xacc::getCompiler("staq");
  auto program =
      compiler->compile(transpiled_circuit_qasm, nullptr)->getComposites()[0];
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

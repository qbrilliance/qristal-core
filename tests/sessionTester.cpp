// Copyright (c) Quantum Brilliance Pty Ltd
#include "qb/core/session.hpp"
#include <gtest/gtest.h>
#include "qb/core/circuit_builder.hpp"
#include <random>

TEST(sessionTester, test_small_angles_xasm_compilation) {
  auto my_sim = qb::session(false);
  // Set up sensible default parameters
  my_sim.qb12();
  my_sim.set_qn(1);
  my_sim.set_acc("aer");
  qb::CircuitBuilder my_circuit;
  std::random_device rd;
  std::mt19937 gen(rd());
  // Small angles only
  std::uniform_real_distribution<float> dist(0.0, 0.01);
  // Lots of gates (with small angles)
  constexpr int num_loops = 1000;
  for (int i = 0; i < num_loops; ++i) {
    my_circuit.RX(0, dist(gen));
    my_circuit.RY(0, dist(gen));
    my_circuit.RZ(0, dist(gen));
  }
  my_circuit.Measure(0);
  // Set the input circuit
  my_sim.set_irtarget_m(my_circuit.get());
  my_sim.set_nooptimise(true);
  my_sim.set_noplacement(true);
  EXPECT_NO_THROW(my_sim.run());
}
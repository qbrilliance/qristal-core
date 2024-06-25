// Copyright (c) 2024 Quantum Brilliance Pty Ltd
#include "qb/core/circuit_builder.hpp"
#include <gtest/gtest.h>

TEST(ParametrizedCircuitTester, test_builder_api) {
  /***
  Tests the circuit builder CircuitBuilder
  class.
  ***/
  size_t num_qubits = 2;
  qb::CircuitBuilder circuit = qb::CircuitBuilder();
  circuit.RX(0, "alpha");
  circuit.RY(0, "beta");
  circuit.RZ(1, "gamma");
  circuit.U1(1, "delta");
  circuit.CPhase(0, 1, "epsilon");
  circuit.U3(0, "theta_1", "theta_2", "theta_3");
  circuit.MeasureAll(num_qubits);
  std::shared_ptr<xacc::CompositeInstruction> instructions = circuit.get();
  std::string expected_circ =
      "Rx(alpha) q0\nRy(beta) q0\nRz(gamma) q1\n"
      "U1(delta) q1\nCPhase(epsilon) q0,q1\n"
      "U(theta_1,theta_2,theta_3) q0\n"
      "Measure q0\nMeasure q1\n";
  EXPECT_TRUE(circuit.is_parametrized());
  EXPECT_EQ(circuit.num_free_params(), 8);
  EXPECT_EQ(instructions->toString(), expected_circ);
}

TEST(ParametrizedCircuitTester, test_param_map_to_vec) {
    size_t num_qubits = 2;
    qb::CircuitBuilder circuit = qb::CircuitBuilder();
    circuit.RX(0, "alpha");
    circuit.RY(0, "beta");
    circuit.RZ(1, "alpha");
    circuit.U1(1, "beta");
    circuit.CPhase(0, 1, "gamma");
    circuit.MeasureAll(num_qubits);
    std::map<std::string, double> param_map;
    param_map["alpha"] = 0.1;
    param_map["beta"] = 0.2;
    param_map["gamma"] = 0.3;
    std::vector<double> expected_param_vec = {0.1, 0.2, 0.3};
    std::vector<double> actual_param_vec = circuit.param_map_to_vec(param_map);
    EXPECT_EQ(actual_param_vec.size(), 3);
    for (size_t i = 0; i < expected_param_vec.size(); i++)
        EXPECT_NEAR(expected_param_vec[i], actual_param_vec[i], 1e-7);
}

TEST(ParametrizedCircuitTester, test_circuit_append_to_repeated_param) {
  /***
  Tests the append method with parametrized circuits in the CircuitBuilder class
  Also tests the get_free_params and num_free_params getter functions

  Expected final params: {"alpha", "beta"}
  ***/
  qb::CircuitBuilder circ1;
  circ1.RX(0, "alpha");
  circ1.RX(0, "beta");
  circ1.Measure(0);

  auto circ2 = qb::CircuitBuilder(); 
  circ2.RY(0, "alpha");
  EXPECT_NO_THROW(circ2.append(circ1));
  std::vector<std::string> expected_free_params = {(std::string) "alpha", (std::string) "beta"};
  std::vector<std::string> actual_free_params = circ2.get_free_params();
  EXPECT_EQ(circ2.num_free_params(), 2);
  for (size_t i = 0; i < circ2.num_free_params(); i++) {
    EXPECT_TRUE(expected_free_params[i] == actual_free_params[i]);
  }
}

TEST(ParametrizedCircuitTester, test_circuit_append_to_new_param) {
  /***
  Tests the append method with parametrized circuits in the CircuitBuilder class
  Also tests the get_free_params and num_free_params getter functions

  Expected final params: {"alpha", "beta"}
  ***/
  qb::CircuitBuilder circ1;
  circ1.RX(0, "alpha");
  circ1.RX(0, "beta");
  circ1.Measure(0);

  auto circ2 = qb::CircuitBuilder(); 
  circ2.RY(0, "gamma");
  EXPECT_NO_THROW(circ2.append(circ1));
  std::vector<std::string> expected_free_params = {(std::string) "gamma", (std::string) "alpha", (std::string) "beta"};
  std::vector<std::string> actual_free_params = circ2.get_free_params();
  EXPECT_EQ(circ2.num_free_params(), 3);
  for (size_t i = 0; i < circ2.num_free_params(); i++) {
    EXPECT_TRUE(expected_free_params[i] == actual_free_params[i]);
  }
}

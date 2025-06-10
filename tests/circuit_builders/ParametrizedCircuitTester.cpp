// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/noise_model/noise_channel.hpp>
#include <qristal/core/session.hpp>

#include <gtest/gtest.h>
#include <unsupported/Eigen/KroneckerProduct>
#include <Eigen/Dense>
#include <random>

//helper function returning the U3 gate unitary
Eigen::MatrixXcd U3(double theta, double phi, double lambda) {
  Eigen::MatrixXcd result(2, 2);
  double c = cos(theta / 2.0);
  double s = sin(theta / 2.0);
  std::complex<double> pphi = std::complex<double>(cos(phi), sin(phi));
  std::complex<double> plambda = std::complex<double>(cos(lambda), sin(lambda));
  result << c, -1.0*plambda*s,
            pphi*s, pphi*plambda*c;
  return result;
}

//helper function returning the CRX gate unitary
Eigen::MatrixXcd CRX(double angle) {
  Eigen::MatrixXcd result(4, 4);
  double c = cos(angle / 2.0);
  double s = sin(angle / 2.0);
  result << 1, 0, 0, 0,
            0, c, 0, std::complex<double>(0, -1.0*s),
            0, 0, 1, 0,
            0, std::complex<double>(0, -1.0*s), 0, c;
  return result;
}

//helper function returning the CRY gate unitary
Eigen::MatrixXcd CRY(double angle) {
  Eigen::MatrixXcd result(4, 4);
  double c = cos(angle / 2.0);
  double s = sin(angle / 2.0);
  result << 1, 0, 0, 0,
            0, c, 0, -1.0*s,
            0, 0, 1, 0,
            0, s, 0, c;
  return result;
}

TEST(ParametrizedCircuitTester, test_crx) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(-2*M_PI, 2*M_PI);

  //(1) Generate random U3 and CRX angles and calculate ideal state vector
  std::vector<double> U3_0, U3_1;
  for (size_t i = 0; i < 3; ++i) {
    U3_0.push_back(dis(gen));
    U3_1.push_back(dis(gen));
  }
  double crx_angle = dis(gen);
  Eigen::VectorXcd ideal_state = Eigen::VectorXcd::Zero(4);
  ideal_state(0) = 1.0;
  ideal_state = Eigen::kroneckerProduct(
    U3(U3_1[0], U3_1[1], U3_1[2]), U3(U3_0[0], U3_0[1], U3_0[2])
  ).eval() * ideal_state;
  ideal_state = CRX(crx_angle) * ideal_state;

  //(2) Construct fixed and parameterized circuit
  auto circuit_1 = qristal::CircuitBuilder();
  circuit_1.U3(0, U3_0[0], U3_0[1], U3_0[2]);
  circuit_1.U3(1, U3_1[0], U3_1[1], U3_1[2]);
  circuit_1.CRX(0, 1, crx_angle);
  circuit_1.MeasureAll(2);
  auto circuit_2 = qristal::CircuitBuilder();
  circuit_2.U3(0, U3_0[0], U3_0[1], U3_0[2]);
  circuit_2.U3(1, U3_1[0], U3_1[1], U3_1[2]);
  circuit_2.CRX(0, 1, "theta");
  circuit_2.MeasureAll(2);

  //(3) Obtain simulated state vector from both circuits
  qristal::session my_sim;
  my_sim.acc = "qpp";
  my_sim.qn = 2;
  my_sim.sn = 1;
  my_sim.calc_state_vec = true;
  //non-parameterized circuit
  my_sim.irtarget = circuit_1.get();
  my_sim.run();
  auto statevec_1 = my_sim.state_vec();
  //parameterized circuit
  my_sim.irtarget = circuit_2.get();
  my_sim.circuit_parameters = std::vector<double>{crx_angle};
  my_sim.run();
  auto statevec_2 = my_sim.state_vec();

  //(4) Compare both state vectors to the ideal (correct) one
  for (size_t i = 0; i < statevec_1.size(); ++i) {
    EXPECT_NEAR(ideal_state(i).real(), statevec_1.at(i).real(), 1e-5);
    EXPECT_NEAR(ideal_state(i).imag(), statevec_1.at(i).imag(), 1e-5);
    EXPECT_NEAR(ideal_state(i).real(), statevec_2.at(i).real(), 1e-5);
    EXPECT_NEAR(ideal_state(i).imag(), statevec_2.at(i).imag(), 1e-5);
  }
}

TEST(ParametrizedCircuitTester, test_cry) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(-2*M_PI, 2*M_PI);

  //(1) Generate random U3 and CRX angles and calculate ideal state vector
  std::vector<double> U3_0, U3_1;
  for (size_t i = 0; i < 3; ++i) {
    U3_0.push_back(dis(gen));
    U3_1.push_back(dis(gen));
  }
  double cry_angle = dis(gen);
  Eigen::VectorXcd ideal_state = Eigen::VectorXcd::Zero(4);
  ideal_state(0) = 1.0;
  ideal_state = Eigen::kroneckerProduct(
    U3(U3_1[0], U3_1[1], U3_1[2]), U3(U3_0[0], U3_0[1], U3_0[2])
  ).eval() * ideal_state;
  ideal_state = CRY(cry_angle) * ideal_state;

  //(2) Construct fixed and parameterized circuit
  auto circuit_1 = qristal::CircuitBuilder();
  circuit_1.U3(0, U3_0[0], U3_0[1], U3_0[2]);
  circuit_1.U3(1, U3_1[0], U3_1[1], U3_1[2]);
  circuit_1.CRY(0, 1, cry_angle);
  circuit_1.MeasureAll(2);
  auto circuit_2 = qristal::CircuitBuilder();
  circuit_2.U3(0, U3_0[0], U3_0[1], U3_0[2]);
  circuit_2.U3(1, U3_1[0], U3_1[1], U3_1[2]);
  circuit_2.CRY(0, 1, "theta");
  circuit_2.MeasureAll(2);

  //(3) Obtain simulated state vector from both circuits
  qristal::session my_sim;
  my_sim.acc = "qpp";
  my_sim.qn = 2;
  my_sim.sn = 1;
  my_sim.calc_state_vec = true;
  //non-parameterized circuit
  my_sim.irtarget = circuit_1.get();
  my_sim.run();
  auto statevec_1 = my_sim.state_vec();
  //parameterized circuit
  my_sim.irtarget = circuit_2.get();
  my_sim.circuit_parameters = std::vector<double>{cry_angle};
  my_sim.run();
  auto statevec_2 = my_sim.state_vec();

  //(4) Compare both state vectors to the ideal (correct) one
  for (size_t i = 0; i < statevec_1.size(); ++i) {
    EXPECT_NEAR(ideal_state(i).real(), statevec_1.at(i).real(), 1e-5);
    EXPECT_NEAR(ideal_state(i).imag(), statevec_1.at(i).imag(), 1e-5);
    EXPECT_NEAR(ideal_state(i).real(), statevec_2.at(i).real(), 1e-5);
    EXPECT_NEAR(ideal_state(i).imag(), statevec_2.at(i).imag(), 1e-5);
  }
}

TEST(ParametrizedCircuitTester, test_builder_api) {
  /***
  Tests the circuit builder CircuitBuilder
  class.
  ***/
  size_t num_qubits = 2;
  qristal::CircuitBuilder circuit = qristal::CircuitBuilder();
  circuit.RX(0, "alpha");
  circuit.RY(0, "beta");
  circuit.RZ(1, "gamma");
  circuit.U1(1, "delta");
  circuit.CPhase(0, 1, "epsilon");
  circuit.U3(0, "theta_1", "theta_2", "theta_3");
  circuit.CRZ(0, 1, "phi");
  circuit.MeasureAll(num_qubits);
  circuit.print();
  std::shared_ptr<xacc::CompositeInstruction> instructions = circuit.get();
  std::string expected_circ =
      "Rx(alpha) q0\nRy(beta) q0\nRz(gamma) q1\n"
      "U1(delta) q1\nCPhase(epsilon) q0,q1\n"
      "U(theta_1,theta_2,theta_3) q0\n"
      "CRZ(phi) q0,q1\n"
      "Measure q0\nMeasure q1\n";
  EXPECT_TRUE(circuit.is_parametrized());
  EXPECT_EQ(circuit.num_free_params(), 9);
  EXPECT_EQ(instructions->toString(), expected_circ);
}

TEST(ParametrizedCircuitTester, test_param_map_to_vec) {
    size_t num_qubits = 2;
    qristal::CircuitBuilder circuit = qristal::CircuitBuilder();
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
  qristal::CircuitBuilder circ1;
  circ1.RX(0, "alpha");
  circ1.RX(0, "beta");
  circ1.Measure(0);

  auto circ2 = qristal::CircuitBuilder();
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
  qristal::CircuitBuilder circ1;
  circ1.RX(0, "alpha");
  circ1.RX(0, "beta");
  circ1.Measure(0);

  auto circ2 = qristal::CircuitBuilder();
  circ2.RY(0, "gamma");
  EXPECT_NO_THROW(circ2.append(circ1));
  std::vector<std::string> expected_free_params = {(std::string) "gamma", (std::string) "alpha", (std::string) "beta"};
  std::vector<std::string> actual_free_params = circ2.get_free_params();
  EXPECT_EQ(circ2.num_free_params(), 3);
  for (size_t i = 0; i < circ2.num_free_params(); i++) {
    EXPECT_TRUE(expected_free_params[i] == actual_free_params[i]);
  }
}


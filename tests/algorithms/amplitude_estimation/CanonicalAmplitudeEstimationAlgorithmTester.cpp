// Copyright (c) Quantum Brilliance Pty Ltd
#include <Circuit.hpp>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <gtest/gtest.h>
#include <cmath>

TEST(CanonicalAmplitudeEstimationAlgorithmTester, checkSimple) {
  // Test the example here:
  // https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
  // i.e., estimate the amplitude of the state:
  // sqrt(1-p)|0> + sqrt(p)|1>
  constexpr double p = 0.2;
  const double theta_p = 2 * std::asin(std::sqrt(p));
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  // A circuit
  auto state_prep = gateRegistry->createComposite("A");
  state_prep->addInstruction(
      gateRegistry->createInstruction("Ry", {0}, {theta_p}));
  // Q circuit
  auto grover_op = gateRegistry->createComposite("Q");
  grover_op->addInstruction(
      gateRegistry->createInstruction("Ry", {0}, {2.0 * theta_p}));
  const int bits_precision = 3;
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  int num_state_qubits = 1;
  auto buffer = xacc::qalloc(bits_precision+1);
  std::vector<int> evaluation_qubits = {1,2,3};
  std::vector<int> trial_qubits = {0};
  auto ae_algo =
      xacc::getAlgorithm("canonical-ae", {{"state_preparation_circuit", state_prep},
                                {"grover_op_circuit", grover_op}, {"trial_qubits", trial_qubits}, {"evaluation_qubits", evaluation_qubits}, {"num_evaluation_qubits", bits_precision},{"num_state_qubits",num_state_qubits}, {"num_trial_qubits", num_state_qubits},
                                {"qpu", acc}});

  ae_algo->execute(buffer);
  buffer->print();
  EXPECT_NEAR(buffer["amplitude-estimation"].as<double>(), 0.38268, 0.1);
}

TEST(CanonicalAmplitudeEstimationAlgorithmTester, checkInputOracle) {
  // Test the example here:
  // https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
  // i.e., estimate the amplitude of the state:
  // sqrt(1-p)|0> + sqrt(p)|1>
  constexpr double p = 0.2;
  const double theta_p = 2 * std::asin(std::sqrt(p));
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  // A circuit
  auto state_prep = gateRegistry->createComposite("A");
  state_prep->addInstruction(
      gateRegistry->createInstruction("Ry", {3}, {theta_p}));
  // Oracle circuit
  auto oracle = gateRegistry->createComposite("oracle");
  oracle->addInstruction(gateRegistry->createInstruction("Z", {3}));
  const int bits_precision = 3;
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  int num_state_qubits = 1;
  auto buffer = xacc::qalloc(bits_precision+1);
  std::vector<int> evaluation_qubits = {0,1,2};
  std::vector<int> trial_qubits = {3};
  auto ae_algo =
      xacc::getAlgorithm("canonical-ae", {{"state_preparation_circuit", state_prep},
                                {"oracle", oracle}, {"num_evaluation_qubits", bits_precision},{"num_state_qubits", num_state_qubits},{"trial_qubits", trial_qubits}, {"evaluation_qubits", evaluation_qubits}, {"num_trial_qubits", num_state_qubits},
                                {"qpu", acc}});

  ae_algo->execute(buffer);
  buffer->print();
  EXPECT_NEAR(buffer["amplitude-estimation"].as<double>(), 0.38268, 0.1);
}

TEST(CanonicalAmplitudeEstimationAlgorithmTester, checkHigherPrecision) {
  // Test the example here, with more qubits => expect better precision
  // https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
  // i.e., estimate the amplitude of the state:
  // sqrt(1-p)|0> + sqrt(p)|1>
  constexpr double p = 0.2;
  const double theta_p = 2 * std::asin(std::sqrt(p));
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  // A circuit
  auto state_prep = gateRegistry->createComposite("A");
  state_prep->addInstruction(
      gateRegistry->createInstruction("Ry", {10}, {theta_p}));
  // Q circuit
  auto grover_op = gateRegistry->createComposite("Q");
  grover_op->addInstruction(
      gateRegistry->createInstruction("Ry", {10}, {2.0 * theta_p}));
  // Use high precision
  const int bits_precision = 10;
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  const int num_state_qubits = 1;
  auto buffer = xacc::qalloc(bits_precision+1);
  auto ae_algo =
      xacc::getAlgorithm("canonical-ae", {{"state_preparation_circuit", state_prep},
                                {"grover_op_circuit", grover_op}, {"num_evaluation_qubits", bits_precision}, {"num_state_qubits", num_state_qubits},{"num_trial_qubits", num_state_qubits},
                                {"qpu", acc}});

  ae_algo->execute(buffer);
  buffer->print();
  // Get very close to the amplitude (sqrt(p) = 0.4472)
  std::cout << buffer["amplitude-estimation"].as<double>();
  EXPECT_NEAR(buffer["amplitude-estimation"].as<double>(), std::sqrt(p), 0.01);
}


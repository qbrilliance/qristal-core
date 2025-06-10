// Copyright (c) Quantum Brilliance Pty Ltd
#include <Circuit.hpp>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <gtest/gtest.h>

TEST(MLAmplitudeEstimationAlgorithmTester, checkInputOracle) {
  // Test the example here:
  // https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
  // i.e., estimate the amplitude of the state:
  // sqrt(1-p)|0> + sqrt(p)|1>
  constexpr double p = 0.2;
  const double theta_p = 2 * std::asin(std::sqrt(p));
  int num_state_qubits = 1;
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  // A circuit
  auto state_prep = gateRegistry->createComposite("A");
  state_prep->addInstruction(
      gateRegistry->createInstruction("Ry", {0}, {theta_p}));
  // Oracle circuit
  auto oracle = gateRegistry->createComposite("oracle");
  oracle->addInstruction(gateRegistry->createInstruction("Z", {0}));
  // Indicator function
  std::function<int(std::string)> indicator_func = [&](std::string state) {
    auto bitString = std::string(state.rbegin(), state.rend());
    auto y = std::stoi(bitString, 0, 2);
    if (y == 1) {
      return 1;
    } else {
      return 0;
    }
  };
  auto acc = xacc::getAccelerator("qpp");
  auto buffer = xacc::qalloc(num_state_qubits);
  std::vector<int> indicator_qubits = {0};
  auto ae_algo =
      xacc::getAlgorithm("ML-ae", {{"state_preparation_circuit", state_prep},
                                   {"oracle_circuit", oracle},
                                   {"is_in_good_subspace", indicator_func},
                                   {"score_qubits", indicator_qubits},
                                   {"qpu", acc}});

  ae_algo->execute(buffer);
  buffer->print();
  EXPECT_NEAR(buffer["amplitude-estimation"].as<double>(), 0.44721, 0.1);
}


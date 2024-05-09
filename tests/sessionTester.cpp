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

struct AddMapVals
{
  template<class Value, class Pair> 
  Value operator()(Value value, const Pair& pair) const
  {
    return value + pair.second;
  }
};

TEST(sessionTester, test_parametrized_run_1) {
  /***
  Tests the run method with parametrized circuits in the session class
  Also tests the get_out_probs getter function
  Input state: |00>
  All parameters set to 0
  Circuit: "yz" ansatz with 2 reps

  Expected output distribution: {00 : 100%}
  ***/

  size_t num_qubits = 2;
  qb::CircuitBuilder circuit;

  for (size_t i = 0; i < num_qubits; i++) {
    circuit.RX(i, "theta_" + std::to_string(i));
  }
  circuit.MeasureAll(num_qubits);

  std::vector<double> param_vec(circuit.num_free_params());

  qb::session my_sim;
  my_sim.qb12();
  my_sim.set_qn(num_qubits);
  my_sim.set_sn(1000);
  my_sim.set_acc("qpp");
  my_sim.set_seed(1000);
  my_sim.set_irtarget_m(circuit.get());
  my_sim.set_calc_jacobian(true);
  my_sim.set_parameter_vector(param_vec);
  my_sim.run();
  std::vector<double> stats =  my_sim.get_out_probs()[0][0];
  EXPECT_DOUBLE_EQ(std::accumulate(stats.begin(), stats.end(), 0.0),
                   1.0);  // probs sum to 1

  // Verify run
  EXPECT_DOUBLE_EQ(stats[0], 1.0);
}

TEST(sessionTester, test_parametrized_run_2) {
  /***./CI
  Tests the run method and the get_out_counts methods in the session class with pre-determined parameters

  Input state: |++>
  RX Parameter Values: pi/2
  RY Parameter Values: pi/4
  Expected output distribution: {00 : 10.9% , 01 : 47.5% , 10 : 20% , 11
  : 21.6%}
  ***/

  size_t num_qubits = 2;
  size_t num_repetitions = 2;
  qb::CircuitBuilder circuit;
  for (size_t i = 0; i < num_qubits; i++) {
    circuit.RX(i, "alpha_" + std::to_string(i));
    circuit.RY(i, "beta_" + std::to_string(i));
  }
  circuit.CNOT(0, 1);
  circuit.MeasureAll(num_qubits);

  size_t num_free_params = circuit.num_free_params();
  std::vector<double> param_vec(num_free_params, 2 * M_PI / 3);

  for (size_t i = 1; i < num_free_params; i += 2) {
    param_vec[i] = M_PI_4;
  }

  qb::session my_sim;
  my_sim.qb12();
  my_sim.set_qn(num_qubits);
  my_sim.set_sn(1000);
  my_sim.set_acc("qpp");
  my_sim.set_seed(1000);
  my_sim.set_irtarget_m(circuit.get());
  my_sim.set_parameter_vector(param_vec);
  my_sim.run();
  std::vector<int> counts = my_sim.get_out_counts()[0][0];

  // Verify get_counts
  EXPECT_EQ(counts.size(), std::pow(2, num_qubits));  // 2^n outcomes
  EXPECT_DOUBLE_EQ(std::accumulate(counts.begin(), counts.end(), 0.0),
                   1000);  // counts sum to 1000

  // Verify that the counts_map_to_vec function gives the correct probability vector
  std::vector<double> expected_counts = {109, 475, 200, 216};
  for (size_t i = 0; i < counts.size(); i++) {
    EXPECT_NEAR(counts[i], expected_counts[i], 1e-6);
  }
}

TEST(sessionTester, test_gradients) {
  /***
  Tests running gradient calculations and the get_out_prob_jacobians methods in the session
  class

  Input state is |00>
  Parameters set to {2*pi/3 + 0.1*n}
  Circuit will be run for <param> - pi/2 and <param> + pi/2 for all parameters
  and difference calculated

  Expected output shown below
  ***/
  size_t num_qubits = 2;
  qb::CircuitBuilder circuit;

  for (size_t i = 0; i < num_qubits; i++) {
    circuit.RX(i, "alpha_" + std::to_string(i));
    circuit.RY(i, "beta_" + std::to_string(i));
  }
  circuit.CNOT(0, 1);
  circuit.MeasureAll(num_qubits);

  size_t num_free_params = circuit.num_free_params();
  std::vector<double> param_vec(num_free_params, 2 * M_PI / 3);

  for (size_t i = 1; i < num_free_params; i++) {
    param_vec[i] += 0.1 * i;
  }

  qb::session my_sim;
  my_sim.qb12();
  my_sim.set_qn(num_qubits);
  my_sim.set_sn(1000);
  my_sim.set_acc("qpp");
  my_sim.set_calc_jacobian(true);
  my_sim.set_seed(1000);
  my_sim.set_irtarget_m(circuit.get());
  my_sim.set_parameter_vector(param_vec);
  
  my_sim.run();
  qb::Table2d<double> gradients = my_sim.get_out_prob_jacobians()[0][0];

  // Verify get_out_prob_jacobians
  size_t num_outputs = std::pow(2, num_qubits);
  EXPECT_EQ(gradients.size(), num_free_params);
  for (auto row: gradients) {
    EXPECT_EQ(row.size(), num_outputs);
  }
  // Verify output
  std::vector<std::vector<double>> expected_grad = {
         {0.196 , -0.0695,  0.0465, -0.173 },
         {0.159 , -0.0575,  0.043 , -0.1445},
         {0.1825, -0.1025, -0.1665,  0.0865},
         {0.1505, -0.0875, -0.1355,  0.0725}};
  for (size_t i = 0; i < num_free_params; i++) {
    for (size_t j = 0; j < num_outputs; j++) {
      EXPECT_NEAR(gradients[i][j], expected_grad[i][j], 1e-5);
    }
  }
}
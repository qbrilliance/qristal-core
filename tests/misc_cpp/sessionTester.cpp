// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/session.hpp"
#include <gtest/gtest.h>
#include "qristal/core/circuit_builder.hpp"
#include <random>

TEST(sessionTester, test_small_angles_xasm_compilation) {
  auto my_sim = qristal::session(false);
  // Set up sensible default parameters
  my_sim.init();
  my_sim.set_qn(1);
  my_sim.set_acc("aer");
  qristal::CircuitBuilder my_circuit;
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

TEST(sessionTester, test_qft4) {
  std::cout << "* qft4: Execute 4-qubit Quantum Fourier Transform, noiseless, "
               "ExaTN-MPS"
            << std::endl;

  // Start a Qristal session.
  auto s = qristal::session(false);

  s.init(); // setup defaults = 12 qubits, 1024 shots, tnqvm-exatn-mps
            // back-end

  // Override defaults
  s.set_qn(4);
  s.set_sn(1024);
  s.set_xasm(true);  // Use XASM circuit format to access XACC's qft()
  s.set_seed(23);
  // targetCircuit: contains the quantum circuit that will be processed/executed
  auto targetCircuit = R"(
    __qpu__ void qristal_circuit(qbit q) {
          qft(q, {{"nq",4}});
          Measure(q[3]);
          Measure(q[2]);
          Measure(q[1]);
          Measure(q[0]);
    }
  )";
  s.set_instring(targetCircuit);
  // Run the circuit on the back-end
  s.run();

  // Get the Z-operator expectation value
  auto iter = s.get_out_z_op_expects()[0][0].find(0);
  EXPECT_TRUE(iter != s.get_out_z_op_expects()[0][0].end());
  double exp_val = iter->second;
  // Test the value against assertions
  std::cout << "4-qubit noiseless QFT Z-operator expectation value: " << exp_val
            << std::endl;
  assert(std::abs(-0.0390625 - exp_val) < 1e-9);

}

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
  qristal::CircuitBuilder circuit;

  for (size_t i = 0; i < num_qubits; i++) {
    circuit.RX(i, "theta_" + std::to_string(i));
  }
  circuit.MeasureAll(num_qubits);

  std::vector<double> param_vec(circuit.num_free_params());

  qristal::session my_sim;
  my_sim.init();
  my_sim.set_qn(num_qubits);
  my_sim.set_sn(1000);
  my_sim.set_acc("qpp");
  my_sim.set_seed(1000);
  my_sim.set_irtarget_m(circuit.get());
  my_sim.set_calc_jacobian(true);
  my_sim.set_parameter_vector(param_vec);
  my_sim.run();
  std::vector<double> stats =  my_sim.get_out_probs()[0][0];

  EXPECT_NEAR(std::accumulate(stats.begin(), stats.end(), 0.0),
                   1.0, 1e-6);  // probs sum to 1

  // Verify run
  EXPECT_NEAR(stats[0], 1.0, 1e-6);
}

TEST(sessionTester, test_parametrized_run_2) {
  /***./CI
  Tests the run method and the get_out_counts methods in the session class with pre-determined parameters

  Input state: |++>
  RX Parameter Values: pi/2
  RY Parameter Values: pi/4
  ***/

  // Expected output distribution: {0,0 : 10.9% , 0,1 : 20% , 1,0 : 47.5% , 1,1 : 21.6%}
  const std::map<std::vector<bool>,int> expected = { {{0,0}, 109},
                                                     {{0,1}, 200},
                                                     {{1,0}, 475},
                                                     {{1,1}, 216} };

  const size_t num_qubits = 2;
  const size_t num_repetitions = 2;
  const uint shots = 1000;

  qristal::CircuitBuilder circuit;
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

  // Repeat all tests with out_counts et al indexed by both MSB and LSB, to show that it has no effect.
  for (bool MSB : {true, false}) {
    qristal::session my_sim(false, MSB);
    my_sim.init();
    my_sim.set_qn(num_qubits);
    my_sim.set_sn(shots);
    my_sim.set_acc("qpp");
    my_sim.set_seed(1000);
    my_sim.set_irtarget_m(circuit.get());
    my_sim.set_parameter_vector(param_vec);
    my_sim.set_calc_out_counts(true);
    my_sim.run();
    std::vector<int> counts = my_sim.get_out_counts()[0][0];
    std::map<std::vector<bool>,int> results = my_sim.results()[0][0];

    // Verify that get_counts has an entry for every possible bitstring
    EXPECT_EQ(counts.size(), std::pow(2, num_qubits));  // 2^n outcomes

    // Verify that both get_counts and results sum to requested number of shots
    auto summer = [](auto prev_sum, auto& entry) { return prev_sum + entry.second; };
    EXPECT_EQ(std::accumulate(counts.begin(), counts.end(), 0), shots);
    EXPECT_EQ(std::accumulate(results.begin(), results.end(), 0, summer), shots);

    // Verify that the counts_map_to_vec function gives the correct probability vector
    std::vector<double> expected_counts(counts.size());
    for (const auto& [bits, count] : expected) expected_counts[my_sim.bitstring_index(bits)] = count;
    for (size_t i = 0; i < counts.size(); i++) EXPECT_NEAR(counts[i], expected_counts[i], 1e-6);

    // Verify that the individual bitstring result counts are as expected
    for (const auto& [bits, count] : results) EXPECT_NEAR(count, expected.at(bits), 1e-6);
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
  qristal::CircuitBuilder circuit;

  for (size_t i = 0; i < num_qubits; i++) {
    circuit.RX(i, "alpha_" + std::to_string(i));
    circuit.RY(i, "beta_" + std::to_string(i));
  }
  circuit.RX(0, "gamma");
  circuit.RX(1, "delta");
  circuit.CNOT(0, 1);
  circuit.MeasureAll(num_qubits);

  size_t num_free_params = circuit.num_free_params();
  std::vector<double> param_vec(num_free_params, 2 * M_PI / 3);

  for (size_t i = 1; i < num_free_params; i++) {
    param_vec[i] += 0.1 * i;
  }

  qristal::session my_sim;
  my_sim.init();
  my_sim.set_qn(num_qubits);
  my_sim.set_sn(1000);
  my_sim.set_acc("qpp");
  my_sim.set_calc_jacobian(true);
  my_sim.set_seed(1000);
  my_sim.set_irtarget_m(circuit.get());
  my_sim.set_parameter_vector(param_vec);

  my_sim.run();
  qristal::Table2d<double> gradients = my_sim.get_out_prob_jacobians()[0][0];

  // Verify get_out_prob_jacobians
  size_t num_outputs = std::pow(2, num_qubits);
  EXPECT_EQ(gradients.size(), num_free_params);
  for (auto row: gradients) {
    EXPECT_EQ(row.size(), num_outputs);
  }
  // Verify output
  std::vector<std::vector<double>> expected_grad = {
            {-0.005, 0.044, -0.0435, 0.0045, },
            {-0.017, 0.138, -0.1375, 0.0165, },
            {-0.008, 0.0515, 0.0105, -0.054, },
            {-0.0265, 0.165, 0.012, -0.1505, },
            {0.028, -0.236, 0.2375, -0.0295, },
            {0.0245, -0.177, -0.011, 0.1635, }};
  for (size_t i = 0; i < num_free_params; i++) {
    for (size_t j = 0; j < num_outputs; j++) {
      EXPECT_NEAR(gradients[i][j], expected_grad[i][j], 1e-5);
    }
  }
}

TEST(sessionTester, test_draw_shot) {
  /***
  Tests single-shot draw.  Runs a circuit, draws each shot one-by-one,
  puts them into a map, and compares that map to the actual results map
  reported by Qristal.  Then tries to draw another shot in order to
  test the exception expected to be generated by doing so.
  ***/

  using qristal::operator <<;

  qristal::session my_sim;
  my_sim.init();
  my_sim.set_acc("qpp");
  my_sim.set_qn(4);
  my_sim.set_sn(1000);

  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[4];
      x q[0];
      h q[1];
      h q[2];
      h q[3];
      measure q[0] -> c[0];
      measure q[1] -> c[1];
      measure q[2] -> c[2];
      measure q[3] -> c[3];
    }
    )";
  my_sim.set_instring(targetCircuit);

  my_sim.run();
  std::map<std::vector<bool>, int> qristal_results = my_sim.results()[0][0];
  std::map<std::vector<bool>, int> my_results;
  for (int i = 0; i < my_sim.get_sns()[0][0]; i++) my_results[my_sim.draw_shot(0,0)]++;
  EXPECT_EQ(qristal_results, my_results);
  EXPECT_THROW(my_sim.draw_shot(0,0), std::out_of_range);
}

TEST(sessionTester, test_state_vec_order) {
  const size_t num_qubits = 2;
  const uint shots = 1000;

  qristal::CircuitBuilder circuit;
  circuit.X(0);
  circuit.MeasureAll(num_qubits);

  // Repeat all tests with out_counts and state vector indexed by both MSB and LSB, to show that it has no effect.
  for (bool MSB : {true, false}) {
    qristal::session my_sim(false, MSB);
    my_sim.init();
    my_sim.set_qn(num_qubits);
    my_sim.set_sn(shots);
    my_sim.set_acc("qpp");
    my_sim.set_seed(1000);
    my_sim.set_irtarget_m(circuit.get());
    my_sim.set_calc_out_counts(true);
    my_sim.get_state_vec(true);
    my_sim.run();

    // Check that the state vector and counts for MSB and LSB have the same non-zero valued index
    std::shared_ptr<std::vector<std::complex<double>>> stateVec = my_sim.get_state_vec_raw();
    std::vector<int> counts = my_sim.get_out_counts()[0][0];
    for (size_t i = 0; i < counts.size(); i++) {
      EXPECT_EQ(shots * stateVec->at(i).real(), counts[i]);
    }
  }
}

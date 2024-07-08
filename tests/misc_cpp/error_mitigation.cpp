// (c) 2023 Quantum Brilliance Pty Ltd
#include <ostream>
#include <gtest/gtest.h>
#include "qb/core/session.hpp"

TEST(TestErrorMitigation, test_readout_error_mitigation) {
  std::cout << "* Test readout error mitigation *" << std::endl;

  // Start a session.
  auto s = qb::session(false);
  // Default parameters
  s.init();

  // Override defaults
  int n_shots = 1024;
  s.set_qn(1);
  s.set_sn(n_shots);
  s.set_xasm(true);
  s.set_noise(true);
  s.set_nooptimise(true);
  s.set_noplacement(true);
  s.set_noise_mitigation("ro-error");
  s.set_acc("aer");
  auto targetCircuit = R"(
    __qpu__ void QBCIRCUIT(qbit q) {
        X(q[0]);
        Measure(q[0]);
    }
  )";
  s.set_instring(targetCircuit);
  // Run the circuit on the back-end
  s.run();
  // Get Z expectation
  auto iter = s.get_out_z_op_expects()[0][0].find(0);
  EXPECT_TRUE(iter != s.get_out_z_op_expects()[0][0].end());
  const double exp_val = iter->second;
  std::vector<int> out_counts = s.get_out_counts()[0][0];
  double raw_exp_val = (out_counts[0] - out_counts[1]) / (1.0 * n_shots);
  std::cout << "Error mitigated exp-val = " << exp_val
            << " vs. raw exp-val = " << raw_exp_val << "\n";
  // Ideal result is -1.0 (|1> state)
  double delta_mitigated = -1.0 - exp_val;
  double delta_raw = -1.0 - raw_exp_val;
  // Check if readout error mitigation improved accuracy
  EXPECT_TRUE(std::abs(delta_mitigated) <= std::abs(delta_raw));
}

TEST(TestErrorMitigation, test_richardson_error_mitigation) {
  std::cout << "* Test Richardson error mitigation *" << std::endl;
  // Start a QB SDK session.
  auto s = qb::session(false);
  s.init();
  // Fix random seed
  s.set_seed(1);
  // Override defaults
  s.set_qn(2);
  s.set_sn(1024); 
  s.set_xasm(true);
  s.set_noise(true);
  s.set_nooptimise(true);
  s.set_noplacement(true);
  s.set_acc("aer");
  auto targetCircuit = R"(
    __qpu__ void QBCIRCUIT(qbit q) {
        H(q[0]);
        CNOT(q[0],q[1]);
        Measure(q[0]);
        Measure(q[1]);
    }
  )";
  s.set_instring(targetCircuit);
  // Run the circuit on the back-end
  s.run();
  auto raw_iter = s.get_out_z_op_expects()[0][0].find(0);
  EXPECT_TRUE(raw_iter != s.get_out_z_op_expects()[0][0].end());
  double raw_exp_val = raw_iter->second;
  std::cout << "Raw exp-val-z = " << raw_exp_val << "\n";
  // Set noise mitigation and re-run simulation
  s.set_noise_mitigation("rich-extrap");
  s.run();
  auto iter = s.get_out_z_op_expects()[0][0].find(0);
  EXPECT_TRUE(iter != s.get_out_z_op_expects()[0][0].end());
  double exp_val = iter->second;
  std::cout << "Richardson extrapolation error mitigated exp-val-z = " << exp_val << "\n";

  // Ideal result is 1.0 
  double delta_mitigated = 1.0 - exp_val;
  double delta_raw = 1.0 - raw_exp_val;
  // Check if readout error mitigation improved accuracy
  EXPECT_TRUE(std::abs(delta_mitigated) <= std::abs(delta_raw));
}


TEST(TestErrorMitigation, test_assignment_kernel_error_mitigation) {
  std::cout << "* Test readout assignment kernel error mitigation *"
            << std::endl;

  // Start a QB SDK session.
  auto s = qb::session(false);
  s.init();
  // Fix random seed
  s.set_seed(1);

  // Override defaults
  s.set_qn(1);
  s.set_sn(1024);
  s.set_xasm(true);
  s.set_noise(true);
  s.set_nooptimise(true);
  s.set_noplacement(true);
  s.set_acc("aer");
  auto targetCircuit = R"(
    __qpu__ void QBCIRCUIT(qbit q) {
        X(q[0]);
        Measure(q[0]);
    }
  )";
  s.set_instring(targetCircuit);
  // Run the circuit on the back-end
  s.run();
  auto raw_iter = s.get_out_z_op_expects()[0][0].find(0);
  EXPECT_TRUE(raw_iter != s.get_out_z_op_expects()[0][0].end());
  double raw_exp_val = raw_iter->second;
  std::cout << "Raw exp-val-z = " << raw_exp_val << "\n";

  // Use error mitigation and re-run simulation
  s.set_noise_mitigation("assignment-error-kernel");
  s.run();
  auto iter = s.get_out_z_op_expects()[0][0].find(0);
  EXPECT_TRUE(iter != s.get_out_z_op_expects()[0][0].end());
  double exp_val = iter->second;
  std::cout << "Assignment-error-kernel mitigation exp-val-z = " << exp_val << "\n";

  // Ideal result is -1.0 
  double delta_mitigated = -1.0 - exp_val;
  double delta_raw = -1.0 - raw_exp_val;
  // Check if readout error mitigation improved accuracy
  EXPECT_TRUE(std::abs(delta_mitigated) <= std::abs(delta_raw));
}

// (c) 2023 Quantum Brilliance Pty Ltd
#include <ostream>
#include <random>
#include <gtest/gtest.h>
#include <unsupported/Eigen/KroneckerProduct>
#include "qristal/core/session.hpp"
#include "qristal/core/circuit_builder.hpp"

TEST(TestErrorMitigation, test_SPAM_correction_fixed) {
  //using fixed counts map from the QDK 1.1 measurements 
  std::vector<std::map<std::vector<bool>, int>> list_measured_counts{
    {{{0, 0}, 317}, {{0, 1}, 270}, {{1, 0}, 223}, {{1, 1}, 246}},
    {{{0, 0}, 560}, {{0, 1}, 555}, {{1, 0}, 40}, {{1, 1}, 38}}, 
    {{{0, 0}, 512}, {{0, 1}, 50}, {{1, 0}, 414}, {{1, 1}, 28}},
    {{{0, 0}, 1008}, {{0, 1}, 69}, {{1, 0}, 69}, {{1, 1}, 6}}, 
    {{{0, 0}, 51}, {{0, 1}, 43}, {{1, 0}, 426}, {{1, 1}, 403}}
  }; 
  //and the (by hand) SPAM-corrected results to check
  std::vector<std::map<std::vector<bool>, int>> list_corrected_counts{
    {{{0, 0}, 328}, {{0, 1}, 271}, {{1, 0}, 211}, {{1, 1}, 246}},
    {{{0, 0}, 596}, {{0, 1}, 597}},
    {{{0, 0}, 542}, {{0, 1}, 31}, {{1, 0}, 426}, {{1, 1}, 4}},
    {{{0, 0}, 1119}, {{0, 1}, 33}},
    {{{0, 0}, 24}, {{0, 1}, 12}, {{1, 0}, 466}, {{1, 1}, 421}}
  };
  //measured confusion matrix on the QDK 1.1
  Eigen::MatrixXd confusion(4, 4);
  confusion << 0.88779689, 0.05241605, 0.05487305, 0.004914,
               0.0349345 , 0.89344978, 0.00524017, 0.06637555,
               0.09011628, 0.02713178, 0.84496124, 0.0377907,
               0.01373896, 0.06673209, 0.03336605, 0.8861629;
  Eigen::MatrixXd correction = confusion.inverse();

  for (size_t i = 0; i < list_measured_counts.size(); ++i) {
    auto corrected = qristal::apply_SPAM_correction(list_measured_counts[i], correction);
    EXPECT_EQ(corrected, list_corrected_counts[i]);
  }
}

TEST(TestErrorMitigation, test_SPAM_correction_random) {
  std::cout << "* Test SPAM correction *" << std::endl;
  size_t n_qubits = 2; 
  size_t n_shots = 1024;

  //(1) Build random noise model using readout errors only
  std::random_device rd; 
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 1.0/n_qubits);
  std::vector<std::pair<double, double>> ro_errors_per_qubit;
  qristal::NoiseModel SPAM_error;
  for (size_t q = 0; q < n_qubits; ++q) {
    double p_01 = dis(gen); 
    double p_10 = dis(gen);
    SPAM_error.set_qubit_readout_error(q, qristal::ReadoutError(p_01, p_10));
    for (size_t qq = q+1; qq < n_qubits; ++qq) {
      SPAM_error.add_qubit_connectivity(q, qq);
    }
    ro_errors_per_qubit.push_back(std::make_pair(p_01, p_10));
  }

  //(2) Construct the corresponding SPAM confusion matrix
  Eigen::MatrixXd confusion_mat = Eigen::MatrixXd::Ones(1,1);
  for (auto const & [p_01, p_10] : ro_errors_per_qubit) {
    Eigen::MatrixXd temp(2, 2);
    temp << 1.0 - p_01, p_01, 
            p_10, 1.0-p_10;
    confusion_mat = Eigen::kroneckerProduct(confusion_mat, temp).eval();
  }

  //(3) Construct session
  auto s = qristal::session(false);
  s.init();
  s.set_qn(n_qubits);
  s.set_sn(n_shots);
  s.set_acc("aer");
  qristal::CircuitBuilder circuit; 
  circuit.H(0);
  circuit.CNOT(0, 1);
  circuit.MeasureAll(2);
  s.set_irtarget_m(circuit.get());
  s.set_noise(true);
  s.set_noise_model(SPAM_error);

  //(4) Set SPAM confusion matrix and run 
  s.set_SPAM_confusion_matrix(confusion_mat);
  s.run();

  //(5) Obtain SPAM-corrected and native <ZZ>
  double native_ZZ = 0.0;
  for (const auto& [bitstring, counts] : s.results_native()[0][0]) {
    native_ZZ += (bitstring[0] == bitstring[1] ? +1.0 : -1.0) * static_cast<double>(counts) / static_cast<double>(n_shots);
  }
  double corrected_ZZ = 0.0;
  for (const auto& [bitstring, counts] : s.results()[0][0]) {
    corrected_ZZ += (bitstring[0] == bitstring[1] ? +1.0 : -1.0) * static_cast<double>(counts) / static_cast<double>(n_shots);
  }
  std::cout << "Error mitigated exp-val = " << corrected_ZZ
            << " vs. raw exp-val = " <<  native_ZZ << "\n";

  EXPECT_TRUE(corrected_ZZ >= native_ZZ);
}

TEST(TestErrorMitigation, test_readout_error_mitigation) {
  std::cout << "* Test readout error mitigation *" << std::endl;

  // Start a session.
  auto s = qristal::session(false);
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
    __qpu__ void qristal_circuit(qbit q) {
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
  const auto& res = s.results()[0][0];
  double raw_exp_val = (res.at({0}) - res.at({1})) / (1.0 * n_shots);
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
  // Start a Qristal session.
  auto s = qristal::session(false);
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
    __qpu__ void qristal_circuit(qbit q) {
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

  // Start a Qristal session.
  auto s = qristal::session(false);
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
    __qpu__ void qristal_circuit(qbit q) {
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

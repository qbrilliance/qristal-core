// Copyright (c) 2022 Quantum Brilliance Pty Ltd
/**
 * This example shows how to use your own Kraus operators.
 */

#include "qristal/core/session.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include <unsupported/Eigen/KroneckerProduct>

// Build and return a noise model for an n-qubit ring
qristal::NoiseModel ring_noise_model(size_t nb_qubits) {
  // Make an empty noise model
  qristal::NoiseModel noise_model;

  // Name the model whatever you like
  noise_model.name = "ring_noise_model";

  // Define the readout errors
  qristal::ReadoutError ro_error;
  ro_error.p_01 = 1e-2;
  ro_error.p_10 = 5e-3;

  // Define the gate fidelities (errors are 1 - fidelity)
  constexpr double u1_error = 1e-3;
  constexpr double u2_error = 1e-3;
  constexpr double u3_error = 1e-3;
  constexpr double cx_error = 1e-2;

  // Create depolarizing channel Kraus operators for u1 gate
  const double p_u1_1 = std::sqrt(1 - u1_error);
  const double p_u1_2 = std::sqrt(u1_error / 3);
  Eigen::MatrixXcd mat_Id_u1{{p_u1_1, 0.0}, {0.0, p_u1_1}};
  Eigen::MatrixXcd mat_X_u1{{0, p_u1_2}, {p_u1_2, 0}};
  Eigen::MatrixXcd mat_Y_u1{{0, -p_u1_2 * std::complex<double>(0, 1)}, {p_u1_2 * std::complex<double>(0, 1), 0}};
  Eigen::MatrixXcd mat_Z_u1{{p_u1_2, 0}, {0, -p_u1_2}};
  std::vector<Eigen::MatrixXcd> single_qubit_kraus_u1 = {mat_Id_u1, mat_X_u1, mat_Y_u1, mat_Z_u1};

  // Create depolarizing channel Kraus operators for u2 gate
  const double p_u2_1 = std::sqrt(1 - u2_error);
  const double p_u2_2 = std::sqrt(u2_error / 3);
  Eigen::MatrixXcd mat_Id_u2{{p_u2_1, 0.0}, {0.0, p_u2_1}};
  Eigen::MatrixXcd mat_X_u2{{0, p_u2_2}, {p_u2_2, 0}};
  Eigen::MatrixXcd mat_Y_u2{{0, -p_u2_2 * std::complex<double>(0, 1)}, {p_u2_2 * std::complex<double>(0, 1), 0}};
  Eigen::MatrixXcd mat_Z_u2{{p_u2_2, 0}, {0, -p_u2_2}};
  std::vector<Eigen::MatrixXcd> single_qubit_kraus_u2 = {mat_Id_u2, mat_X_u2, mat_Y_u2, mat_Z_u2};

  // Create depolarizing channel Kraus operators for u3 gate
  const double p_u3_1 = std::sqrt(1 - u3_error);
  const double p_u3_2 = std::sqrt(u3_error / 3);
  Eigen::MatrixXcd mat_Id_u3{{p_u3_1, 0.0}, {0.0, p_u3_1}};
  Eigen::MatrixXcd mat_X_u3{{0, p_u3_2}, {p_u3_2, 0}};
  Eigen::MatrixXcd mat_Y_u3{{0, -p_u3_2 * std::complex<double>(0, 1)}, {p_u3_2 * std::complex<double>(0, 1), 0}};
  Eigen::MatrixXcd mat_Z_u3{{p_u3_2, 0}, {0, -p_u3_2}};
  std::vector<Eigen::MatrixXcd> single_qubit_kraus_u3 = {mat_Id_u3, mat_X_u3, mat_Y_u3, mat_Z_u3};

  // Create depolarizing channel Kraus operators for cz gate
  constexpr size_t num_terms = 16;
  constexpr double max_param = num_terms / (num_terms - 1.0);
  const double coeff_iden = std::sqrt(1 - cx_error / max_param);
  const double coeff_pauli = std::sqrt(cx_error / num_terms);

  static const std::unordered_map<char, Eigen::MatrixXcd> pauli_op_map = []() {
    Eigen::MatrixXcd Id{Eigen::MatrixXcd::Identity(2, 2)};
    Eigen::MatrixXcd X{Eigen::MatrixXcd::Zero(2, 2)}; ///< Pauli Sigma-X gate
    Eigen::MatrixXcd Y{Eigen::MatrixXcd::Zero(2, 2)}; ///< Pauli Sigma-Y gate
    Eigen::MatrixXcd Z{Eigen::MatrixXcd::Zero(2, 2)}; ///< Pauli Sigma-Z gate
    X << 0, 1, 1, 0;
    Y << 0, -std::complex<double>(0, 1), std::complex<double>(0, 1), 0;
    Z << 1, 0, 0, -1;
    return std::unordered_map<char, Eigen::MatrixXcd>{{'I', Id}, {'X', X}, {'Y', Y}, {'Z', Z}};
  }();

  const auto build_kraus_op = [](const std::string &pauli_str, double coeff) {
    assert(pauli_str.size() == 2);
    const auto first_mat = pauli_op_map.find(pauli_str[0])->second;
    const auto second_mat = pauli_op_map.find(pauli_str[1])->second;
    Eigen::MatrixXcd kron_mat = coeff * Eigen::kroneckerProduct(first_mat, second_mat);
    assert(kron_mat.rows() == 4);
    assert(kron_mat.cols() == 4);
    return kron_mat;
  };

  std::vector<Eigen::MatrixXcd> two_qubit_kraus;
  const std::vector<std::pair<std::string, double>> pauli_kraus_ops {
    {"II", coeff_iden}, {"IX", coeff_pauli}, {"IY", coeff_pauli}, {"IZ", coeff_pauli},
    {"XI", coeff_pauli}, {"XX", coeff_pauli}, {"XY", coeff_pauli}, {"XZ", coeff_pauli},
    {"YI", coeff_pauli}, {"YX", coeff_pauli}, {"YY", coeff_pauli}, {"YZ", coeff_pauli},
    {"ZI", coeff_pauli}, {"ZX", coeff_pauli}, {"ZY", coeff_pauli}, {"ZZ", coeff_pauli}};
  for (const auto &[op_label, coeff] : pauli_kraus_ops) {
    two_qubit_kraus.emplace_back(build_kraus_op(op_label, coeff));
  }

  // Loop over the qubits
  for (size_t qId = 0; qId < nb_qubits; ++qId) {
    // Set the readout errors
    noise_model.set_qubit_readout_error(qId, ro_error);

    // Note: To use the emulator backends, Kraus operators for native gate set {rx, ry, cz} must be supplied.
    // Set the single-qubit gate fidelities.
    // Kraus operators for 1-qubit gate depolarizing channel
    noise_model.add_gate_error(qristal::krausOpToChannel::Create({qId}, single_qubit_kraus_u1), "u1", {qId});
    noise_model.add_gate_error(qristal::krausOpToChannel::Create({qId}, single_qubit_kraus_u2), "u2", {qId});
    noise_model.add_gate_error(qristal::krausOpToChannel::Create({qId}, single_qubit_kraus_u3), "u3", {qId});

    // Set the qubit connections to form a ring
    const size_t qId2 = (qId != nb_qubits - 1 ? qId + 1 : 0);
    noise_model.add_qubit_connectivity(qId, qId2);

    // Set the corresponding two-qubit gate fidelities.
    noise_model.add_gate_error(qristal::krausOpToChannel::Create({qId, qId2}, two_qubit_kraus), "cx", {qId, qId2});
    noise_model.add_gate_error(qristal::krausOpToChannel::Create({qId, qId2}, two_qubit_kraus), "cx", {qId2, qId});
  }

  return noise_model;
}


int main(int argc, char * argv[])
{
  qristal::session my_sim;

  // 4 qubits
  const int n = 4;

  // Set up meaningful defaults
  my_sim.init();

  // Set the number of qubits
  my_sim.set_qn(n);

  // qb-mps simulator selected
  my_sim.set_acc("aer");

  // Set this to true to include noise
  my_sim.set_noise(true);

  // Hand over the noise model to the session.
  my_sim.set_noise_model(ring_noise_model(n));

  // Define the kernel
  my_sim.set_instring(R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[2];
    h q[0];
    cx q[0],q[1];
    measure q[1] -> c[1];
    measure q[0] -> c[0];
    )");

  // Execute circuit
  my_sim.run();

  // Print results
  std::cout << my_sim.results()[0][0] << std::endl;

  // Exit
  return 0;
}

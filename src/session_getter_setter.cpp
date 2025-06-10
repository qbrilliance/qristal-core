// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/session.hpp>

namespace qristal {

  /// Helper template function for returning a quantity or an error, depending on the value of a passed flag
  template<typename T>
  const T& get_conditional(const T& quantity, bool condition, std::string_view err) {
    if (not condition) throw std::logic_error(err.data());
    return quantity;
  }

  const std::vector<int>& session::all_bitstring_counts() const {
    return get_conditional(all_bitstring_counts_, calc_all_bitstring_counts, "all_bitstring_counts() requires calc_all_bitstring_counts = true.");
  }

  const std::vector<double>& session::all_bitstring_probabilities() const {
    return get_conditional(all_bitstring_probabilities_, calc_gradients, "all_bitstring_probabilities() requires calc_gradients = true.");
  }

  const std::vector<std::vector<double>>& session::all_bitstring_probability_gradients() const {
    return get_conditional(all_bitstring_probability_gradients_, calc_gradients, "all_bitstring_probability_gradients() requires calc_gradients = true.");
  }

  const std::vector<std::complex<double>>& session::state_vec() const {
    return get_conditional(*state_vec_, calc_state_vec, "state_vec() requires calc_state_vec = true.");
  }

  const std::map<std::vector<bool>,int>& session::results() const { return results_; }

  const std::map<std::vector<bool>,int>& session::results_native() const { return results_native_; }

  std::string session::transpiled_circuit() const { return transpiled_circuit_; }

  std::string session::qobj() const { return qobj_; }

  std::string session::qbjson() const { return qbjson_; }

  std::map<int,int> session::one_qubit_gate_depths() const { return one_qubit_gate_depths_; }

  std::map<int,int> session::two_qubit_gate_depths() const { return two_qubit_gate_depths_; }

  std::map<int,double> session::timing_estimates() const { return timing_estimates_; }

  double session::z_op_expectation() const { return z_op_expectation_; }

  void session::set_SPAM_confusion_matrix(Eigen::MatrixXd mat) { SPAM_correction_matrix = mat.inverse(); };

  Eigen::MatrixXd session::get_SPAM_confusion_matrix() const { return SPAM_correction_matrix.inverse(); };

}

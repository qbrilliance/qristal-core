// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/utils.hpp>
#include <ranges>
#include <boost/dynamic_bitset.hpp>
#include <boost/math/constants/constants.hpp>

std::ostream& operator<<(std::ostream& s, const std::map<std::vector<bool>, int>& m) {
  using qristal::operator<<;
  std::string newline;
  for (const auto& [key, val] : m) {
    s << newline << key << ":  " << val;
    if (newline.empty()) newline = "\n";
  }
  return s;
}

namespace qristal {

  std::map<std::vector<bool>, int> apply_SPAM_correction(
      const std::map<std::vector<bool>, int>& counts,
      const Eigen::MatrixXd& SPAM_correction_mat
  ) {
    //(1) organize counts into Eigen::Vector
    size_t n_qubits = counts.begin()->first.size();
    Eigen::VectorXd counts_vec = Eigen::VectorXd::Zero(SPAM_correction_mat.rows());
    for (auto const & [bitstring, count] : counts) {
      //convert std::vector<bool> to boost::dynamic_bitset
      boost::dynamic_bitset<> bitset(bitstring.size());
      for (size_t i = 0; i < n_qubits; ++i) {
        bitset[n_qubits - 1 - i] = bitstring[i]; //boost uses r2l ordering!
      }
      //and to integer index
      counts_vec(bitset.to_ulong()) = count;
    }
    double N = counts_vec.sum();
    //(2) apply SPAM correction
    counts_vec = SPAM_correction_mat * counts_vec;
    //(3) re-set counts
    //(3.1) replace negative values with zero
    for (Eigen::Index i = 0; i < counts_vec.rows(); ++i) {
      counts_vec(i) = std::max(0.0, counts_vec(i));
    }
    //(3.2) Rescale based on the total sum and assemble corrected counts
    std::map<std::vector<bool>, int> corrected_counts;
    double scale_factor = N / counts_vec.sum();
    for (Eigen::Index i = 0; i < counts_vec.rows(); ++i) {
      int c = std::round(counts_vec(i) * scale_factor);
      if (c > 0) {
        boost::dynamic_bitset<> bits(n_qubits, i);
        std::vector<bool> key(n_qubits);
        for (size_t j = 0; j < n_qubits; ++j) {
          key[j] = bits[n_qubits - 1 - j];
        }
        corrected_counts[key] = c;
      }
    }
    return corrected_counts;
  }

  template<>
  std::ostream& operator<<(std::ostream& s, const std::vector<bool>& v) {
    for (const auto& i : v | std::views::reverse) s << i;
    return s;
  }

  int ipow(int base, int exp) {
    int result = 1;
    for (;;) {
      if (exp & 1)
        result *= base;
      exp >>= 1;
      if (!exp)
        break;
      base *= base;
    }
    return result;
  }

  std::string aer_circuit_transpiler(std::string& circuit) {

    std::string NEWcircuit = circuit;
    std::istringstream stream{circuit};
    std::string current_line;

    // Repeat till end is reached
    while(std::getline(stream, current_line))
    {
       size_t pos = NEWcircuit.find(current_line);

       size_t pos_cy = current_line.find("cy q[");
       size_t pos_rzz = current_line.find("rzz(");

       //size_t pos_q = current_line.find("q");
       size_t open1 = current_line.find("[");
       size_t open2 = current_line.find("[",open1+1);
       size_t close1 = current_line.find("]");
       size_t close2 = current_line.find("]",close1+1);
       size_t par1 = current_line.find("(");
       size_t par2 = current_line.find(")");
       // size_t com1 = current_line.find(",");
       // size_t com2 = current_line.find(",",com1+1);
       // size_t com3 = current_line.find(",",com2+1);

       if (pos_cy != std::string::npos) {
          // find the qubit numbering
          std::string q_i = current_line.substr(open1+1,close1-open1-1);
          std::string q_f = current_line.substr(open2+1,close2-open2-1);
          // building up the replacement circuit
          std::stringstream repl_circuit;
          repl_circuit << "  sdg q[" << q_f << "];" << std::endl;
          repl_circuit << "  cx q[" << q_i << "],q[" << q_f << "];" << std::endl;
          repl_circuit << "  s q[" << q_f << "];";
          // Replace this occurrence
          NEWcircuit.replace(pos, current_line.size(), repl_circuit.str());
       }
       else if (pos_rzz != std::string::npos) {
          //std::string angle = current_line.substr(pos_rzz+4,8);
          std::string angle = current_line.substr(par1+1,par2-par1-1);
          //std::string q_i = current_line.substr(pos_q+2,1);
          std::string q_i = current_line.substr(open1+1,close1-open1-1);
          std::string q_f = current_line.substr(open2+1,close2-open2-1);
          // building up the replacement circuit
          std::stringstream repl_circuit;
          repl_circuit << "  cx q[" << q_i << "],q[" << q_f << "];" << std::endl;
          repl_circuit << "  rz(" << angle << ") q[" << q_f << "];" << std::endl;
          repl_circuit << "  cx q[" << q_i << "],q[" << q_f << "];";
          // Replace this occurrence
          NEWcircuit.replace(pos, current_line.size(), repl_circuit.str());
       }
    }

    return NEWcircuit;
  }

  nlohmann::json get_session_cfg(const std::string &config_buf) {
    nlohmann::json output_to_js;
    nlohmann::json config;

    config = nlohmann::json::parse(config_buf);
    if (config.count("n_qubits")) {
      output_to_js["n_qubits"] = config["n_qubits"];
    }
    if (config.count("shots")) {
      output_to_js["shots"] = config["shots"];
    }
    if (config.count("acc")) {
      output_to_js["acc"] = config["acc"];
    }
    if (config.count("output_oqm")) {
      output_to_js["output_oqm"] = config["output_oqm"];
    }
    if (config.count("expected_amplitudes")) {
        output_to_js["expected_amplitudes"] = config["expected_amplitudes"];
    }
    if (config.count("probabilities")) {
        output_to_js["output_probabilities"] = config["probabilities"];
    }
    if (config.count("svd-cutoff")) {
        output_to_js["svd_cutoff"] = config["svd-cutoff"];
    }
    if (config.count("rel-svd-cutoff")) {
        output_to_js["rel_svd_cutoff"] = config["rel-svd-cutoff"];
    }
    if (config.count("initial-bond-dimension")) {
        output_to_js["initial_bond_dimension"] = config["initial-bond-dimension"];
    }
    if (config.count("initial-kraus-dimension")) {
        output_to_js["initial_kraus_dimension"] = config["initial-kraus-dimension"];
    }
    if (config.count("max-bond-dimension")) {
        output_to_js["max_bond_dimension"] = config["max-bond-dimension"];
    }
    if (config.count("max-kraus-dimension")) {
        output_to_js["max_kraus_dimension"] = config["max-kraus-dimension"];
    }
    if (config.count("gpu-device-ids")) {
        output_to_js["gpu_device_ids"] = config["gpu-device-ids"];
    }
    if (config.count("svd-type")) {
        output_to_js["svd_type"] = config["svd-type"];
    }
    if (config.count("svdj-tol")) {
        output_to_js["svdj_tol"] = config["svdj-tol"];
    }
    if (config.count("svdj-max-sweeps")) {
        output_to_js["svdj_max_sweeps"] = config["svdj-max-sweeps"];
    }
    if (config.count("vqe-aswap-particles")) {
      output_to_js["vqe_aswap_particles"] = config["vqe-aswap-particles"];
    }
    if (config.count("vqe-depth")) {
      output_to_js["vqe_depth"] = config["vqe-depth"];
    }
    if (config.count("vqe-ansatz")) {
      output_to_js["vqe_ansatz"] = config["vqe-ansatz"];
    }
    if (config.count("hybrid-ham")) {
      output_to_js["hybrid_ham"] = config["hybrid-ham"];
    }
    if (config.count("hybrid-optimizer-method")) {
      output_to_js["hybrid_optim_method"] = config["hybrid-optimizer-method"];
    }
    if (config.count("hybrid-optimizer-functol")) {
      output_to_js["hybrid_optim_functol"] = config["hybrid-optimizer-functol"];
    }
    if (config.count("hybrid-optimizer-maxeval")) {
      output_to_js["hybrid_optim_maxeval"] = config["hybrid-optimizer-maxeval"];
    }
    if (config.count("hybrid-lower-bound")) {
      output_to_js["hybrid_lower_bound"] = config["hybrid-lower-bound"];
    }
    if (config.count("hybrid-abstol")) {
      output_to_js["hybrid_abstol"] = config["hybrid-abstol"];
    }
    if (config.count("optimal-states")) {
      output_to_js["optimal_states"] = config["optimal-states"];
    }

    // Add additional entries in JSON for processing here.

    return output_to_js;
  }

  std::string double_to_string(double input, int precision) {
    std::ostringstream val;
    val.precision(precision);
    val << std::fixed << input;
    std::string val_str = std::move(val).str();
    return val_str;
  }
}

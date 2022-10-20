// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#include "qb/core/method_utils.hpp"

namespace qbOS {
int binomialCoefficient(int n, int k) {
  // Base Cases
  if (k == 0 || k == n) {
    return 1;
  }
  // Recursive
  return binomialCoefficient(n - 1, k - 1) + binomialCoefficient(n - 1, k);
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

template <typename TT>
double get_jensen_shannon(std::map<std::string, int> &in_q, const TT &in_p, bool is_sim_msb) {
    double divergence = 0.0;
    int sum_in_q = 0;
    int i_iter = 0;
    std::string n_str = in_q.begin()->first; unsigned int n_q = n_str.size();
    for (auto in_q_elem = in_q.begin(); in_q_elem != in_q.end(); in_q_elem++) { sum_in_q += in_q_elem->second; }
    std::map<std::string, int>::iterator in_q_elem;
    for (auto in_p_elem = in_p.begin(); in_p_elem != in_p.end(); in_p_elem++) {
      double nipe = get_probability(*in_p_elem);

      // Search in_q for a state with the label (BCD) matching i_iter
      boost::dynamic_bitset<> statelabel_v(n_q, i_iter);
      std::string statelabel;
      to_string(statelabel_v, statelabel);

      if (is_sim_msb) {
          std::reverse(statelabel.begin(),statelabel.end());
      }

      in_q_elem = in_q.find(statelabel);
      if (in_q_elem != in_q.end()) {
            double rfq = (1.0/sum_in_q)*(in_q_elem->second);
            double m = 0.5*(rfq + nipe);
            if ((in_q_elem->second > 0) && (nipe > 0)) {
                divergence += 0.5*( nipe*(std::log(nipe) - std::log(m)) + rfq*(std::log(rfq) - std::log(m)) );
            } else if ((in_q_elem->second > 0) && (nipe == 0)) {
                divergence += 0.5*rfq*(std::log(rfq) - std::log(m));
            } else if ((in_q_elem->second == 0) && (nipe > 0)) {
                divergence += 0.5*nipe*(std::log(nipe) - std::log(m));
            }
      } else {
          // Sparse element - in_q_elem assumed to have 0 probability
          divergence += 0.5*nipe*std::log(2);
      }
      i_iter++;
    }
    return divergence;
}

double get_XEBdiff(std::vector<std::map<std::string, int>>& allresults, const int& shots, const int& n_exp) {

    double XEBdiff = 0.0;
    //std::vector<std::string> found_states;
    double PLogP = 0.0;
    std::map<std::string, int>::iterator it;
    std::map<std::string, int> in_q=allresults[0]; std::string n_str=in_q.begin()->first; int n_q=n_str.size();

    for (int i_iter = 0; i_iter < std::min(n_exp,ipow(2,n_q)); i_iter++) {
      for (std::map<std::string, int> experiment : allresults) {
        boost::dynamic_bitset<> statelabel_v(n_q, i_iter); std::string statelabel; to_string(statelabel_v, statelabel);
        it = experiment.find(statelabel);
        if  (it != experiment.end()) {
            int count = it->second;
            PLogP += ((1.0*count)/shots)*std::log((1.0*count)/shots);
        }
      }
      XEBdiff += -PLogP/n_exp;
      PLogP=0.0;
    }

    return n_q*std::log(2)+boost::math::constants::euler<double>() - XEBdiff;
}

double accumulate_counts_with_parity(const std::map<std::string, int> &in_stateVec) {
  double result = 0;
  double denom = 0;
  for (auto isv_elem = in_stateVec.begin(); isv_elem != in_stateVec.end();
       isv_elem++) {
    size_t count = 0;
    std::string statelabel = isv_elem->first;
    for (uint64_t lb_elem = 0; lb_elem < statelabel.length(); lb_elem++) {
      if (statelabel[lb_elem] == '1') {
        count++;
      }
    }
    denom += isv_elem->second;
    if ((count % 2) == 0) {
        result += isv_elem->second;
    } else {
        result -= isv_elem->second;
    }
  }
  return (result/denom);
}

int choose_random_int( const std::vector<int>& v ) {
    //int r = 1;
    //if (v.size()>1) { r = std::rand()%v.size()+1; }
    int r = std::rand()%v.size();
    //std::cout << "DEBUG: r is " << r << '\n'; // DEBUG
    return v.at(r);
}

std::string choose_random_str( const std::vector<std::string>& v ) {
    //int r = 1;
    //if (v.size()>1) { r = std::rand()%v.size()+1; }
    int r = std::rand()%v.size();
    return v.at(r);
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

json get_qbqe_cfg(const std::string &config_buf) {
  json output_to_js;
  json config;

  config = json::parse(config_buf);
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
  if (config.count("output_amplitude")) {
      output_to_js["output_amplitude"] = config["output_amplitude"];
  }
  if (config.count("probabilities")) {
      output_to_js["output_probabilities"] = config["probabilities"];
  }
  if (config.count("svd-cutoff")) {
      output_to_js["svd_cutoff"] = config["svd-cutoff"];
  }
  if (config.count("max-bond-dimension")) {
      output_to_js["max_bond_dimension"] = config["max-bond-dimension"];
  }
  if (config.count("qaoa")) {
    output_to_js["qaoa"] = config["qaoa"].get<std::vector<double>>();
  }
  if (config.count("qaoa-steps")) {
    output_to_js["qaoa_steps"] = config["qaoa-steps"];
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

template <>
template <>
bool ValidatorTwoDim<VectorMapNN, int>::is_lt_upperbound<NN>(
    const NN &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapNN, int>::is_lt_eq_upperbound<NN>(
    const NN &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapND, double>::is_lt_upperbound<ND>(
    const ND &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapND, double>::is_lt_eq_upperbound<ND>(
    const ND &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapNC, std::complex<double>>::is_lt_upperbound<NC>(
    const NC &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapNC, std::complex<double>>::is_lt_eq_upperbound<
    NC>(const NC &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapND, ND>::is_lt_upperbound<ND>(
    const ND &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapND, ND>::is_lt_eq_upperbound<ND>(
    const ND &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapNN, int>::is_gt_lowerbound<NN>(
    const NN &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapNN, int>::is_gt_eq_lowerbound<NN>(
    const NN &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapND, double>::is_gt_lowerbound<ND>(
    const ND &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapND, double>::is_gt_eq_lowerbound<ND>(
    const ND &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapNC, std::complex<double>>::is_gt_lowerbound<NC>(
    const NC &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapNC, std::complex<double>>::is_gt_eq_lowerbound<
    NC>(const NC &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapND, ND>::is_gt_lowerbound<ND>(
    const ND &subj, const std::string &in_desc) {
  return true;
}

template <>
template <>
bool ValidatorTwoDim<VectorMapND, ND>::is_gt_eq_lowerbound<ND>(
    const ND &subj, const std::string &in_desc) {
  return true;
}
} // namespace qbOS

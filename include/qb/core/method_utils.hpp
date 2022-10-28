// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#pragma once
#include "qb/core/typedefs.hpp"

#include "args.hxx"

#include <boost/dynamic_bitset.hpp>
#include <boost/math/constants/constants.hpp>

#include <stdexcept>
#include <unordered_set>

namespace qbOS {
/**
* Utility (helper) functions
*/

/// Computes the binomial coefficient for (n, k)
int binomialCoefficient(int n, int k);

// The most efficient exponentiating method by Elias Yarrkov:
// https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
int ipow(int base, int exp);

/**
 * get_jensen_shannon - calculate the divergence between two discrete probability distributions supported in the same space
 *
 * Input:
 *
 *     [std::map<std::string, int>]        in_q : the counts from a quantum simulation.  The string key is assumed to be a BCD index for in_p
 *
 *     [std::vector<std::complex<double>>] in_p : the amplitudes for the theoretical distribution of states from which in_q has been sampled.  begin() corresponds to |00...0>. end() corresponds to |111...1>.  Increments in the state label correspond to increments in the iterator.
 *
 * Output:
 *
 *     [double]  divergence : 0.5*(D_KL(in_p||m) + D_KL(in_q||m))
 *
 * where:
 *
 *   m = 0.5*(in_p + in_q)
 *
 *   D_KL(P||Q) = P' * (log(P)-log(Q))  : P,Q are column vectors.  Exclude all elements of Q that are zero
**/

/*
template <typename TT>
double get_probability(const TT &in_elem) {
    return in_elem;
}

template <>
double get_probability(const std::complex<double> &in_elem) {
    return std::norm(in_elem);
}
*/

template <typename TT>
double get_jensen_shannon(std::map<std::string, int> &in_q, const TT &in_p, bool is_sim_msb = false);

double get_XEBdiff(std::vector<std::map<std::string, int>>& allresults, const int& shots, const int& n_exp);

double accumulate_counts_with_parity(const std::map<std::string, int> &in_stateVec);

/// Chooses a random element in the input vector [v]
template<typename T>
T choose_random(const std::vector<T>& v) {
  if (v.empty()) {
    throw std::invalid_argument("Input vector cannot be empty.");
  }

  const int r = std::rand() % v.size();
  return v.at(r);
}

/**
  Desc:
  Generates random circuits of arbitrary size and form.

  Notes:
  - The user needs to study and eventually insert large enough circuit depth for proper random behaviour.
    This would allow for the appearance of all basic gates and reaching Porter-Thomas distribution
    ([Boixo2018] argues sub-linear scaling is enough to achieve this). Note some basic gates are not
    available through XACC framework but all can be implemented in terms of others gates we list here. In
    addition if the optimisations and placement features can make the actual depth larger than this fixed
    amount, however, it always atays as O(n_q).
  - We include only maximally two-operand gates.
  - Currently does not include any middle measurements and conditional operations.
  - Currently does not include any middle resets.
  - Inputting a fixed random seed is not yet implemented.
  - Currently we are not (weakly) conditioning any of the quantum wires to the classical bit values.

  Args:
  [n_q (int)] number of quantum registers -- must be equal or greater than 3
  [seed (int)] sets the random seed

  Returns:
  [RandomQcircuit (string)] generated random quantum circuit in OpenCasm format

  Raises:
  NONE
**/
std::string random_circuit(const int& n_q, const int& depth);

std::string aer_circuit_transpiler(std::string& circuit);

/**
  get_qbqe_cfg : this function processes fields that have come from a qbos configuration file
  Input - JSON string with configuration fields
  Output - JSON of qbos options
**/
json get_qbqe_cfg(const std::string &config_buf);

/**
  get_arg_or_cfg : this function accepts a variable and a default value, and returns
  1. (highest priority): the value of the variable from the relevant command line option
  2. (second priority): the value of the variable in the qbos configuration file
  3. (lowest priority): the default value (unchanged).
**/
template <typename TT2>
TT2 get_arg_or_cfg(const TT2 &in_v, args::ValueFlag<TT2> &in_arg,
                   const json &in_cfg, const char *aname) {
  TT2 retval;
  if (in_arg) {
    retval = args::get(in_arg);
  } else if (in_cfg.count(aname) > 0) {
    retval = in_cfg[aname];
  } else {
    retval = in_v;
  }
  return retval;
}

/// Helper template function that converts a vector [v] to a map [m]
template <typename M, typename V> void vec_to_map(M &m, const V &v) {
  int iix = 0;
  for (typename V::const_iterator it = v.begin(); it != v.end(); ++it) {
    m.insert({iix, v.at(iix)});
    iix++;
  }
}

/// Helper template function that converts a map [m] to a vector [v]
template <typename M, typename V> void map_to_vec(const M &m, V &v) {
  for (typename M::const_iterator it = m.begin(); it != m.end(); ++it) {
    if (v.size() < ((it->first) + 1)) {
      v.resize((it->first) + 1);
    }
    v.at(it->first) = it->second;
  }
}

// Validator class for 2-D array table: shape consistency, upper/lower bounds for numerical values, etc.
template <class TCON, class TVAL> class ValidatorTwoDim {
private:
  TCON data_;
  TVAL lowerbound_;
  TVAL upperbound_;
  std::unordered_set<std::string> validvals_;
  int ii_jj_pattern_;
  std::string description_;


  const int II_JJ_INVALID = -1;
  const int II_JJ_FULL = 0;
  const int II_VECTOR_JJ_SINGLETON = 1;
  const int II_SINGLETON_JJ_VECTOR = 2;
  const int II_SINGLETON_JJ_SINGLETON = 3;


public:
  // Non-empty test
  bool is_data_empty() const {
    if (data_.empty()) {
      return true;
    } else {
      bool is_empty = std::all_of(data_.cbegin(), data_.cend(),
                                  [](auto delem) { return delem.empty(); });
      return is_empty;
    }
  }

  void detect_ii_jj_pattern() {
    if ((data_.size() > 1) && (data_[0].size() > 1)) {
      // test for II_JJ_FULL
      // const int ii_max = data_.size();
      const int jj_max = data_[0].size();
      bool all_jj_max =
          std::all_of(data_.cbegin(), data_.cend(), [&jj_max](auto delem) {
            bool returnval = false;
            if (delem.size() == jj_max) {
              returnval = true;
            };
            return returnval;
          });
      if (all_jj_max) {
        ii_jj_pattern_ = II_JJ_FULL;
      }
    } else if ((data_.size() == 1) && (data_[0].size() > 1)) {
      // test for II_SINGLETON_JJ_VECTOR
      ii_jj_pattern_ = II_SINGLETON_JJ_VECTOR;
    } else if ((data_.size() > 1) && (data_[0].size() == 1)) {
      // test for II_VECTOR_JJ_SINGLETON
      bool all_jj_singleton =
          std::all_of(data_.cbegin(), data_.cend(), [](auto delem) {
            bool returnval = false;
            if (delem.size() == 1) {
              returnval = true;
            };
            return returnval;
          });
      if (all_jj_singleton) {
        ii_jj_pattern_ = II_VECTOR_JJ_SINGLETON;
      }
    } else if ((data_.size() == 1) && (data_[0].size() == 1)) {
      // test for II_SINGLETON_JJ_SINGLETON
      ii_jj_pattern_ = II_SINGLETON_JJ_SINGLETON;
    } else {
      ii_jj_pattern_ = II_JJ_INVALID;
      std::string errmsg = description_;
      errmsg += "- Data in object does not meet the shape requirements to be a "
                "scalar, vector or full 2-d array";
      throw std::invalid_argument(errmsg);
    }
  }

  // Constructors
  ValidatorTwoDim()
      : data_{{}}, ii_jj_pattern_(II_JJ_INVALID), description_("") {
    if (is_data_empty()) {
      std::cout << "Warning: " << description_ << "has empty data" << std::endl;
    }
  }

  template <class V>
  ValidatorTwoDim(V &in_d, const std::string &in_desc = "")
      : data_(in_d), ii_jj_pattern_(II_JJ_INVALID), description_(in_desc) {
    if (is_data_empty()) {
      std::cout << "Warning: " << description_ << "has empty data" << std::endl;
    }
    detect_ii_jj_pattern();
  }

  template <class V>
  ValidatorTwoDim(V &in_d, const std::unordered_set<std::string> &validset,
                  const std::string &in_desc = "")
      : data_(in_d), validvals_(validset), ii_jj_pattern_(II_JJ_INVALID),
        description_(in_desc) {
    if (is_data_empty()) {
      std::cout << "Warning: " << description_ << "has empty data" << std::endl;
    }
    detect_ii_jj_pattern();
    for (auto el_data : data_) {
      for (auto el : el_data) {
        if (validvals_.find(el) == validvals_.end()) {
          std::string errstr = "Value is not permitted: ";
          errstr += description_;
          errstr += "  = ";
          throw std::invalid_argument(errstr += el);
        }
      }
    }
  }

  template <class V, class TT>
  ValidatorTwoDim(V &in_d, TT ineltype_com, const std::string &in_desc = "")
      : data_(in_d), lowerbound_(ineltype_com), upperbound_(ineltype_com),
        ii_jj_pattern_(II_JJ_INVALID), description_(in_desc) {
    if (is_data_empty()) {
      std::cout << "Warning: " << description_ << "has empty data" << std::endl;
    }
    detect_ii_jj_pattern();
    for (auto el_data : data_) {
      for (auto el : el_data) {
        is_lt_eq_upperbound(el, description_);
        is_gt_eq_lowerbound(el, description_);
      }
    }
  }

  template <class V2, class TT2>
  ValidatorTwoDim(V2 &in_d, TT2 ineltype_lb, TT2 ineltype_ub,
                  const std::string &in_desc = "")
      : data_(in_d), lowerbound_(ineltype_lb), upperbound_(ineltype_ub),
        ii_jj_pattern_(II_JJ_INVALID), description_(in_desc) {
    if (is_data_empty()) {
      std::cout << "Warning: " << description_ << "has empty data" << std::endl;
    }
    detect_ii_jj_pattern();
    for (auto el_data : data_) {
      for (auto el : el_data) {
        is_lt_eq_upperbound(el, description_);
        is_gt_eq_lowerbound(el, description_);
      }
    }
  }

  // Bounds checking
  template <class TINNER>
  bool is_lt_upperbound(const TINNER &subj, const std::string &in_desc) {
    bool returnval = false;
    if (subj < upperbound_) {
      returnval = true;
    } else {
      std::stringstream errmsg;
      errmsg << "Bounds for " << in_desc << ": lt exceeded [Value: " << subj
             << " Limit: " << upperbound_ << "]" << std::endl;
      throw std::range_error(errmsg.str());
    }
    return returnval;
  }

  bool is_lt_upperbound(const size_t &ii, const size_t &jj,
                        const std::string &in_desc) {
    bool returnval = is_lt_upperbound((data_[ii])[jj], in_desc);
    return returnval;
  }

  template <class TINNER>
  bool is_lt_eq_upperbound(const TINNER &subj, const std::string &in_desc) {
    bool returnval = false;
    if (subj <= upperbound_) {
      returnval = true;
    } else {
      std::stringstream errmsg;
      errmsg << "Bounds for " << in_desc << ": lt_eq exceeded [Value: " << subj
             << " Limit: " << upperbound_ << "]" << std::endl;
      throw std::range_error(errmsg.str());
    }
    return returnval;
  }

  bool is_lt_eq_upperbound(const size_t &ii, const size_t &jj,
                           const std::string &in_desc) {
    bool returnval = is_lt_eq_upperbound((data_[ii])[jj], in_desc);
    return returnval;
  }

  template <class TINNER>
  bool is_gt_lowerbound(const TINNER &subj, const std::string &in_desc) {
    bool returnval = false;
    if (subj > lowerbound_) {
      returnval = true;
    } else {
      std::stringstream errmsg;
      errmsg << "Bounds for " << in_desc << ": gt exceeded [Value: " << subj
             << " Limit: " << lowerbound_ << "]" << std::endl;
      throw std::range_error(errmsg.str());
    }
    return returnval;
  }

  bool is_gt_lowerbound(const size_t &ii, const size_t &jj,
                        const std::string &in_desc) {
    bool returnval = is_gt_lowerbound((data_[ii])[jj], in_desc);
    return returnval;
  }

  template <class TINNER>
  bool is_gt_eq_lowerbound(const TINNER &subj, const std::string &in_desc) {
    bool returnval = false;
    if (subj >= lowerbound_) {
      returnval = true;
    } else {
      std::stringstream errmsg;
      errmsg << "Bounds for" << in_desc << ": gt_eq exceeded [Value: " << subj
             << " Limit: " << lowerbound_ << "]" << std::endl;
      throw std::range_error(errmsg.str());
    }
    return returnval;
  }

  bool is_gt_eq_lowerbound(const size_t &ii, const size_t &jj,
                           const std::string &in_desc) {
    bool returnval = is_gt_eq_lowerbound((data_[ii])[jj], in_desc);
    return returnval;
  }

  // Getters with broadcast
  const TVAL get(const size_t &ii, const size_t &jj) const {
    if (ii_jj_pattern_ == II_JJ_FULL) {
      // 2-D case where dim ii and dim jj are non-singleton
      if ((ii >= 0) && (ii < data_.size()) && (jj >= 0) &&
          (jj < data_[ii].size())) {
        return (data_[ii])[jj];
      } else {
        std::cout << "Index range ii: " << ii << ", jj: " << jj
                  << " is outside the valid range" << std::endl;
      }
    } else if (ii_jj_pattern_ == II_VECTOR_JJ_SINGLETON) {
      // Vector in dim ii, singleton in dim jj
      if ((ii >= 0) && (ii < data_.size()) && (jj >= 0)) {
        return (data_[ii])[0];
      } else {
        if (!((ii >= 0) && (ii < data_.size()))) {
          std::cout << "Index range ii: " << ii << ", jj: " << jj
                    << " is outside the valid range" << std::endl;
        }
      }
    } else if (ii_jj_pattern_ == II_SINGLETON_JJ_VECTOR) {
      // Singleton in dim ii, vector in dim jj
      if ((ii >= 0) && (jj >= 0) && (jj < data_[0].size())) {
        return (data_[0])[jj];
      } else {
        std::cout << "Index range ii: " << ii << ", jj: " << jj
                  << " is outside the valid range" << std::endl;
      }
    } else if (ii_jj_pattern_ == II_SINGLETON_JJ_SINGLETON) {
      // Scalar case, ie singleton in both dim ii and dim jj
      if ((ii >= 0) && (jj >= 0)) {
        return (data_[0])[0];
      }
    } else {
      std::cout << "The data pattern is invalid" << std::endl;
    }
    std::cout
        << "No matching pattern - should throw an error now and not return..."
        << std::endl;
    return (data_[0])[0];
  }
};

// Specialisations for ValidatorTwoDim
template <>
template <>
bool ValidatorTwoDim<VectorMapNN, int>::is_lt_upperbound<NN>(
    const NN &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapNN, int>::is_lt_eq_upperbound<NN>(
    const NN &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapND, double>::is_lt_upperbound<ND>(
    const ND &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapND, double>::is_lt_eq_upperbound<ND>(
    const ND &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapNC, std::complex<double>>::is_lt_upperbound<NC>(
    const NC &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapNC, std::complex<double>>::is_lt_eq_upperbound<
    NC>(const NC &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapND, ND>::is_lt_upperbound<ND>(
    const ND &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapND, ND>::is_lt_eq_upperbound<ND>(
    const ND &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapNN, int>::is_gt_lowerbound<NN>(
    const NN &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapNN, int>::is_gt_eq_lowerbound<NN>(
    const NN &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapND, double>::is_gt_lowerbound<ND>(
    const ND &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapND, double>::is_gt_eq_lowerbound<ND>(
    const ND &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapNC, std::complex<double>>::is_gt_lowerbound<NC>(
    const NC &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapNC, std::complex<double>>::is_gt_eq_lowerbound<
    NC>(const NC &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapND, ND>::is_gt_lowerbound<ND>(
    const ND &subj, const std::string &in_desc);

template <>
template <>
bool ValidatorTwoDim<VectorMapND, ND>::is_gt_eq_lowerbound<ND>(
    const ND &subj, const std::string &in_desc);
} // namespace qbOS
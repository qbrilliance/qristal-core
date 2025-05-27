// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include "qristal/core/typedefs.hpp"

#include <iostream>
#include <map>
#include <ranges>
#include <stdexcept>
#include <unordered_set>

#include <Eigen/Dense>
#include <args.hxx>
#include <fmt/base.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

/**
* Utility (helper) functions
*/

/// Stream overload for map from vector of bools to ints
std::ostream &operator<<(std::ostream &s,
                         const std::map<std::vector<bool>, int> &m);

// fmt formatter for std::map<std::vector<bool>, int>
template <> struct fmt::formatter<std::map<std::vector<bool>, int>> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const std::map<std::vector<bool>, int> &map,
              FormatContext &ctx) const {
    auto out = ctx.out();
    bool first_key = true;
    for (const auto &[key, val] : map) {
      fmt::format_to(out, "{}{}: {}", first_key ? "" : "\n",
                     fmt::join(key | std::views::transform([](bool b) {
                                 return static_cast<int32_t>(b);
                               }),
                               ""),
                     val);
      first_key = false;
    }
    return out;
  }
};

namespace qristal {

  /**
  * @brief A helper function applying a given SPAM correction matrix to measured bit string counts
  *
  * Arguments: 
  * @param counts : A constant reference to the native measured counts std::map. 
  * @param SPAM_correction_mat : A constant reference to the SPAM correction matrix.
  *
  * @return std::map<std::vector<bool>, int> : The SPAM-corrected counts.
  */
  std::map<std::vector<bool>, int> apply_SPAM_correction(
      const std::map<std::vector<bool>, int>& counts, 
      const Eigen::MatrixXd& SPAM_correction_mat
  );

  /// Overload to allow `std::cout << vector`
  template <typename T>
  std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec){
    os<<'[';
    for(auto& elem : vec) {
      os<<elem<< ", ";
    }
    os << "\b\b]"; // Remove ", " from last element and close vector with "]". One character at the end is still to be overwritten
    return os;
  }

  /// Stream overload for vector of bools. Prints bits in reverse (MSB) order.
  template<>
  std::ostream& operator<<(std::ostream& s, const std::vector<bool>& v);

  /// Computes the binomial coefficient for (n, k)
  int binomialCoefficient(int n, int k);

  template <typename num_t>
  size_t count_nonzero(std::vector<num_t> in_vec) {
    size_t num_nonzero = 0;
    for (auto elem: in_vec) {
      num_nonzero += (elem != 0);
    }
    return num_nonzero;
  }

  // The most efficient exponentiating method by Elias Yarrkov:
  // https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
  int ipow(int base, int exp);

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

  std::string aer_circuit_transpiler(std::string& circuit);

  /**
    get_session_cfg : this function processes fields that have come from an SDK configuration file
    Input - JSON string with configuration fields
    Output - JSON of SDK options
  **/
  nlohmann::json get_session_cfg(const std::string &config_buf);

  /**
    get_arg_or_cfg : this function accepts a variable and a default value, and returns
    1. (highest priority): the value of the variable from the relevant command line option
    2. (second priority): the value of the variable in the SDK configuration file
    3. (lowest priority): the default value (unchanged).
  **/
  template <typename TT2>
  TT2 get_arg_or_cfg(const TT2 &in_v, args::ValueFlag<TT2> &in_arg,
                     const nlohmann::json &in_cfg, const char *aname) {
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
      if (v.at(iix) != 0) m.insert({iix, v.at(iix)});
      iix++;
    }
  }

  /// Helper template function that converts a map [m] to a vector [v]
  template <typename M, typename V> void map_to_vec(const M &m, V &v) {
    for (typename M::const_iterator it = m.begin(); it != m.end(); ++it) {
      if (it->first < 0) {
        throw std::range_error("Map cannot be converted to vector as it does not have keys that are compatible with STL vector");
      }
      if (v.size() < (size_t)((it->first) + 1)) {
        v.resize((it->first) + 1);
      }
      v.at(it->first) = it->second;
    }
  }

  template <class TT>
  int singleton_or_eqlength(const TT &in_d, const size_t N_ii) {
    const int INVALID = -1;
    const int SINGLETON = 1;
    if (in_d.size() > 0) {
      if (N_ii == SINGLETON) {
        return in_d.size();
      } else {
        if ((in_d.size() == N_ii) || (in_d.size() == SINGLETON)) {
          return N_ii;
        } else {
          return INVALID;
        }
      }
    } else {
      return N_ii;
    }
  }

  template <class TT>
  int eqlength(const TT &in_d, const int N_ii) {
    const int INVALID = -1;
    if (in_d.size() == N_ii) {
      return N_ii;
    } else {
      return INVALID;
    }
  }

  // Validator class for 2-D array table: shape consistency, upper/lower bounds for numerical values, etc.
  template <class TVAL> class ValidatorTwoDim {
  private:
    Table2d<TVAL> data_;
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
        const size_t jj_max = data_[0].size();
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
  bool ValidatorTwoDim<std::map<int,double>>::is_lt_eq_upperbound<std::map<int,double>>(
      const std::map<int,double> &subj, const std::string &in_desc);

  template <>
  template <>
  bool ValidatorTwoDim<std::map<int,double>>::is_gt_eq_lowerbound<std::map<int,double>>(
      const std::map<int,double> &subj, const std::string &in_desc);

  /**
   * @brief Convert a variable of type double to a type string with N precision.
   * 
   * @param input Input of type double
   * @param precision Precision/number of decimal places to keep
   * @return Converted variable of type string
   */
  std::string double_to_string(double input, int precision);
}

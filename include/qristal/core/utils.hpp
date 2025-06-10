// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

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

  // The most efficient exponentiating method by Elias Yarrkov:
  // https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
  int ipow(int base, int exp);

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

  /**
   * @brief Convert a variable of type double to a type string with N precision.
   *
   * @param input Input of type double
   * @param precision Precision/number of decimal places to keep
   * @return Converted variable of type string
   */
  std::string double_to_string(double input, int precision);
}

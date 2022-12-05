#pragma once
#include "qb/core/session.hpp"


namespace qb {
namespace op {

// Validators for qbOS 2-D arrays
template <class TCON, class TVAL>
class ValidatorTwoDimOp {
  private:
    TCON data_;
    TVAL upperbound_;
    TVAL lowerbound_;
    std::string description_;
    std::unordered_set<std::string> validvals_;
    
    const int II_JJ_INVALID = -1;
    const int II_JJ_FULL = 0;
    const int II_VECTOR_JJ_SINGLETON = 1;
    const int II_SINGLETON_JJ_VECTOR = 2;
    const int II_SINGLETON_JJ_SINGLETON = 3;
    int ii_jj_pattern_;
  
  public:
    // Non-empty test
    bool is_data_empty() const {
      if (data_.empty()) {
        return true;
      } else {
        bool is_empty = std::all_of(
          data_.cbegin(),
          data_.cend(),
          [](auto delem) { return delem.empty(); }
        );

        return is_empty;
      }
    }
    
    void detect_ii_jj_pattern() {
      if ((data_.size() == 1) && (data_.at(0).size() > 1)) {
        // test for II_SINGLETON_JJ_VECTOR
        ii_jj_pattern_ = II_SINGLETON_JJ_VECTOR;

      } else if ((data_.size() == 1) && (data_.at(0).size() == 1)) {
        // test for II_SINGLETON_JJ_SINGLETON
        ii_jj_pattern_ = II_SINGLETON_JJ_SINGLETON;

      } else if ((data_.size() > 1) && (data_.at(0).size() == 1)) {
        // test for II_VECTOR_JJ_SINGLETON
        bool all_jj_singleton = std::all_of(
          data_.cbegin(), 
          data_.cend(), 
          [](auto delem) {
            bool returnval = false;
            if (delem.size() == 1) {
              returnval = true;
            };
            return returnval;
          }
        );

        if (all_jj_singleton) {
          ii_jj_pattern_ = II_VECTOR_JJ_SINGLETON;
        }

      } else if ((data_.size() > 1) && (data_.at(0).size() > 1)) {
        // test for II_JJ_FULL
        const int ii_max = data_.size();
        const int jj_max = data_.at(0).size();

        bool all_jj_max = std::all_of(
          data_.cbegin(), 
          data_.cend(), 
          [&jj_max](auto delem) {
            bool returnval = false;
            if (delem.size() == jj_max) {
              returnval = true;
            };
            return returnval;
          }
        );

        if (all_jj_max) {
          ii_jj_pattern_ = II_JJ_FULL;
        }

      } else {
        ii_jj_pattern_ = II_JJ_INVALID;
        std::string errmsg = description_;
        errmsg += "- Data in object does not meet the shape requirements to be a "
          "scalar, vector or full 2-d array";

        throw std::invalid_argument(errmsg);
      }
    }
    
    // Constructors
    ValidatorTwoDimOp() : 
      data_{{}}, 
      ii_jj_pattern_(II_JJ_INVALID), 
      description_("") 
    {
      if (is_data_empty()) {
        std::cout << "Warning: " << description_ << "has empty data" << std::endl;
      }
    }
    
    template <class V>
    ValidatorTwoDimOp(
      V &in_d, 
      const std::string &in_desc
    ) : 
      data_(in_d), 
      ii_jj_pattern_(II_JJ_INVALID), 
      description_(in_desc) 
    {
      if (is_data_empty()) {
        std::cout << "Warning: " << description_ << "has empty data" << std::endl;
      }

      // std::cout << "* Debug: proceeding to detect ii jj pattern of: " << in_desc << std::endl;
      detect_ii_jj_pattern();
    }
    
    template <class V>
    ValidatorTwoDimOp(
      V &in_d, 
      const std::unordered_set<std::string> &validset,
      const std::string &in_desc
    ) : 
      data_(in_d), 
      validvals_(validset), 
      ii_jj_pattern_(II_JJ_INVALID),
      description_(in_desc) 
    {
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
    ValidatorTwoDimOp(
      V &in_d, 
      TT ineltype_com, 
      const std::string &in_desc
    ) : 
      data_(in_d), 
      lowerbound_(ineltype_com), 
      upperbound_(ineltype_com),
      ii_jj_pattern_(II_JJ_INVALID), 
      description_(in_desc) 
    {
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
    ValidatorTwoDimOp(
      V2 &in_d, 
      TT2 ineltype_lb, 
      TT2 ineltype_ub,
      const std::string &in_desc
    ) : 
      data_(in_d), 
      lowerbound_(ineltype_lb), 
      upperbound_(ineltype_ub),
      ii_jj_pattern_(II_JJ_INVALID), 
      description_(in_desc) 
    {
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
    bool is_lt_upperbound(
      const TINNER &subj, 
      const std::string &in_desc
    ) {
      bool returnval = false;

      if (subj < upperbound_) {
        returnval = true;

      } else {
        std::stringstream errmsg;
        errmsg 
          << "Bounds for " << in_desc 
          << ": lt exceeded [Value: " << subj
          << " Limit: " << upperbound_ 
          << "]" 
          << std::endl;

        throw std::range_error(errmsg.str());
      }

      return returnval;
    }
    
    bool is_lt_upperbound(
      const size_t &ii, 
      const size_t &jj,
      const std::string &in_desc
    ) {
      bool returnval = is_lt_upperbound((data_.at(ii)).at(jj), in_desc);
      return returnval;
    }
    
    template <class TINNER>
    bool is_lt_eq_upperbound(
      const TINNER &subj, 
      const std::string &in_desc
    ) {
      bool returnval = false;

      if (subj <= upperbound_) {
        returnval = true;
      } else {
        std::stringstream errmsg;
        errmsg 
          << "Bounds for " << in_desc 
          << ": lt_eq exceeded [Value: " << subj
          << " Limit: " << upperbound_ 
          << "]" 
          << std::endl;

        throw std::range_error(errmsg.str());
      }

      return returnval;
    }
    
    bool is_lt_eq_upperbound(
      const size_t &ii, 
      const size_t &jj,
      const std::string &in_desc
    ) {
      bool returnval = is_lt_eq_upperbound((data_.at(ii)).at(jj), in_desc);
      return returnval;
    }
    
    template <class TINNER>
    bool is_gt_lowerbound(
      const TINNER &subj, 
      const std::string &in_desc
    ) {
      bool returnval = false;

      if (subj > lowerbound_) {
        returnval = true;
      } else {
        std::stringstream errmsg;
        errmsg 
          << "Bounds for " << in_desc 
          << ": gt exceeded [Value: " << subj
          << " Limit: " << lowerbound_ 
          << "]" 
          << std::endl;

        throw std::range_error(errmsg.str());
      }

      return returnval;
    }
    
    bool is_gt_lowerbound(
      const size_t &ii, 
      const size_t &jj,
      const std::string &in_desc
    ) {
      bool returnval = is_gt_lowerbound((data_.at(ii)).at(jj), in_desc);
      return returnval;
    }
    
    template <class TINNER>
    bool is_gt_eq_lowerbound(
      const TINNER &subj, 
      const std::string &in_desc
    ) {
      bool returnval = false;

      if (subj >= lowerbound_) {
        returnval = true;
      } else {
        std::stringstream errmsg;
        errmsg 
          << "Bounds for" << in_desc 
          << ": gt_eq exceeded [Value: " << subj
          << " Limit: " << lowerbound_ 
          << "]" 
          << std::endl;

        throw std::range_error(errmsg.str());
      }

      return returnval;
    }
    
    bool is_gt_eq_lowerbound(
      const size_t &ii, 
      const size_t &jj,
      const std::string &in_desc
    ) {
      bool returnval = is_gt_eq_lowerbound((data_.at(ii)).at(jj), in_desc);
      return returnval;
    }
    
    // Getters with broadcast
    const TVAL get(
      const size_t &ii, 
      const size_t &jj
    ) const {
      if (ii_jj_pattern_ == II_JJ_FULL) {
        // 2-D case where dim ii and dim jj are non-singleton
        if ((ii >= 0) && (ii < data_.size()) 
          && (jj >= 0) && (jj < (data_.at(ii)).size())
        ) {
          return (data_.at(ii)).at(jj);
        } else {
          std::cout 
            << "Index range ii: " << ii 
            << ", jj: " << jj
            << " is outside the valid range" 
            << std::endl;
        }
      } else if (ii_jj_pattern_ == II_VECTOR_JJ_SINGLETON) {
        // Vector in dim ii, singleton in dim jj
        if ((ii >= 0) && (ii < data_.size()) && (jj >= 0)) {
          return (data_.at(ii)).at(0);
        } else {
          if (!((ii >= 0) && (ii < data_.size()))) {
            std::cout 
              << "Index range ii: " << ii 
              << ", jj: " << jj
              << " is outside the valid range" 
              << std::endl;
          }
        }
      } else if (ii_jj_pattern_ == II_SINGLETON_JJ_VECTOR) {
        // Singleton in dim ii, vector in dim jj
        if ((ii >= 0) && (jj >= 0) && (jj < (data_.at(0)).size())) {
          return (data_.at(0)).at(jj);
        } else {
          std::cout 
            << "Index range ii: " << ii 
            << ", jj: " << jj
            << " is outside the valid range" 
            << std::endl;
        }
      } else if (ii_jj_pattern_ == II_SINGLETON_JJ_SINGLETON) {
        // Scalar case, ie singleton in both dim ii and dim jj
        if ((ii >= 0) && (jj >= 0)) {
          return (data_.at(0)).at(0);
        }
      } else {
        std::cout << "The data pattern is invalid" << std::endl;
      }

      std::cout
        << "No matching pattern - should throw an error now and not return..."
        << std::endl;

      return (data_.at(0)).at(0);
    }
};

// Specialisations - no validation is implemented for these classes
template <>
template <> 
bool ValidatorTwoDimOp<VectorMapNN, int>::is_lt_upperbound<NN>(
  const NN &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNN, int>::is_lt_eq_upperbound<NN>(
  const NN &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, double>::is_lt_upperbound<ND>(
  const ND &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, double>::is_lt_eq_upperbound<ND>(
  const ND &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNC, std::complex<double>>::is_lt_upperbound<NC>(
  const NC &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNC, std::complex<double>>::is_lt_eq_upperbound<NC>(
  const NC &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, ND>::is_lt_upperbound<ND>(
  const ND &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, ND>::is_lt_eq_upperbound<ND>(
  const ND &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNN, int>::is_gt_lowerbound<NN>(
  const NN &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNN, int>::is_gt_eq_lowerbound<NN>(
  const NN &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, double>::is_gt_lowerbound<ND>(
  const ND &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, double>::is_gt_eq_lowerbound<ND>(
  const ND &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNC, std::complex<double>>::is_gt_lowerbound<NC>(
  const NC &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapNC, std::complex<double>>::is_gt_eq_lowerbound<NC>(
  const NC &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, ND>::is_gt_lowerbound<ND>(
  const ND &subj,
  const std::string &in_desc
);

template <>
template <>
bool ValidatorTwoDimOp<VectorMapND, ND>::is_gt_eq_lowerbound<ND>(
  const ND &subj,
  const std::string &in_desc
);

}
}

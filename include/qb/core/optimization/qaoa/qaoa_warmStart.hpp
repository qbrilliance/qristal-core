// Copyright (c) 2021 Quantum Brilliance Pty Ltd
#pragma once

#include "qb/core/optimization/qaoa/qaoa_base.hpp"

namespace qb {
namespace op {
class QaoaWarmStart : public QaoaBase {
protected:
  VectorMapND thetas_;  
  VectorString good_cuts_;
  
public:
  QaoaWarmStart() : 
    thetas_{{{{0,1.0}}}},
    good_cuts_{{"0"}} { }

  // Constructors
  QaoaWarmStart(const bool debug) : QaoaBase(debug) { }
  
  // Summary printout
  const std::string get_summary() const override;

  // Setters and Getters
  void set_theta(const ND &in_theta);
  void set_thetas(const VectorMapND &in_thetas);
  const VectorMapND &get_thetas() const;
  static const char *help_thetas_;
  //
  void set_good_cut(const std::string &in_good_cut);
  void set_good_cuts(const VectorString &in_good_cuts);
  const VectorString &get_good_cuts() const;
  static const char *help_good_cuts_; 

  // Misc functions
  std::string ws_measurement_circ(
  const int & n_qubits,
    const int & qaoa_steps,
    const std::string & H_string,
    const std::string & good_cut,
    const bool & extended_param,
    const std::vector<double> & params
  );
 
  // Validation methods
  int is_ii_consistent() override;
  int is_jj_consistent() override;

  // Methods
  void run(const size_t &ii, const size_t &jj) override;
};


} // namespace op
} // namespace qb

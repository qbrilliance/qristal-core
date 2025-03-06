// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

#include "qristal/core/optimization/qaoa/qaoa_base.hpp"

namespace qristal {
namespace op {

class QaoaRecursive : public QaoaBase
{
  protected:
    Table2d<size_t> n_cs_;
    
    // Bounds
    const size_t N_CS_LOWERBOUND = 0;
    const size_t N_CS_UPPERBOUND = 100;
  
  public:
    QaoaRecursive() : n_cs_{{10}}{}
    // Constructors
    QaoaRecursive(const bool debug) : QaoaBase(debug) { }
  
    // Summary printout
    const std::string get_summary() const override;
    
    // Setters and Getters
    void set_n_c(const size_t &in_n_c);
    void set_n_cs(const Table2d<size_t> &in_n_cs);
    const Table2d<size_t> &get_n_cs() const;
    static const char *help_n_cs_;
    
    // Misc functions
    std::string measurement_circ_rqaoa(
    const int &n_qubits,
      const int & rqaoa_steps,
      const std::string &H_string,
      const bool &extended_param,
      const std::vector<double> &params,
      const std::vector<int> &array_of_indices
    );
  
    int is_ii_consistent() override;
    int is_jj_consistent() override;
  
    // Methods
    void run(const size_t &ii, const size_t &jj) override;
};


} // namespace op
}

// Copyright (c) 2021 Quantum Brilliance Pty Ltd
#pragma once

#include "qb/core/optimization/qaoa/qaoa_validators.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "PauliOperator.hpp"
#include "Autodiff.hpp"
#include "InstructionIterator.hpp"

#include <sstream>

namespace qb {
namespace op {

class QaoaBase {
  protected:
    VectorString hams_;
    VectorN qns_;
    
    VectorString accs_;
    VectorN sns_;
    VectorBool noises_;
    
    VectorN qaoa_steps_;
    VectorBool extended_params_;  
    
    VectorN rns_;
    VectorString rownames_;
    VectorString colnames_;
    
    VectorString methods_;
    VectorN maxevals_;
    VectorMapND functols_;
    VectorMapND optimum_energy_abstols_;
    VectorMapND optimum_energy_lowerbounds_;
    VectorBool grads_; 
    VectorString gradient_strategys_; 
    
    // Variables not wrapped to Python
    VectorBool acc_uses_lsbs_;
    VectorN acc_uses_n_bits_;
    
    // Storage for quantities of interest
    VectorString out_eigenstates_;
    VectorMapND out_energys_;
    VectorMapND out_jacobians_;
    VectorMapND out_thetas_;
    VectorMapND out_quantum_energy_calc_times_;
    VectorMapND out_quantum_jacobian_calc_times_;
    VectorMapND out_classical_energy_jacobian_total_calc_times_;
    
    // Debugging
    bool debug_qbos_;
    
    // Constants
    const int INVALID = -1;
    const int VALID = 0;
    const int VALID_QAOA_STEPS = 1;
    const int VALID_HAM = 2;
    const int VALID_THETA = 3;
    
    // Valid strings
    const std::unordered_set<std::string> VALID_ACCS = {
      "aer", 
      "tnqvm", 
      "qpp"
    };
    
    const std::unordered_set<std::string> VALID_OPTIMISER_METHODS = {
      "nelder-mead",  
      "cobyla",
      "l-bfgs",
      "adam",
      "sgd",
      "momentum-sgd",
      "momentum-nestorov",
      "rms-prop",
      "gd"
    };
    
    const std::unordered_set<std::string> VALID_MLPACK_OPTIMISER_METHODS = {
      "l-bfgs",
      "adam",
      "sgd",
      "momentum-sgd",
      "momentum-nestorov",
      "rms-prop",
      "gd"
    };
    
    const std::unordered_set<std::string> VALID_GRADIENT_STRATEGYS = {
      "parameter-shift", 
      "central", 
      "forward",
      "backward",
      "autodiff"
    };
    
    // Bounds
    const size_t SNS_LOWERBOUND = 0;
    const size_t SNS_UPPERBOUND = 1000000;
    const size_t QNS_LOWERBOUND = 1;
    const size_t QNS_UPPERBOUND = 10000;
    const size_t RNS_LOWERBOUND = 1;
    const size_t RNS_UPPERBOUND = 1000000;
    const size_t QAOA_STEPS_LOWERBOUND = 1;
    const size_t QAOA_STEPS_UPPERBOUND = 10000;
    const size_t MAXEVALS_LOWERBOUND = 1;
    const size_t MAXEVALS_UPPERBOUND = 1000000;
  
  public:
    QaoaBase() : 
      debug_qbos_(false),
      rownames_{{"-unnamed experiment-"}},
      colnames_{{"-unnamed condition-"}},
      hams_{{"1.0 Z0"}},
      qaoa_steps_{{1}},
      extended_params_{{false}},
      methods_{{"nelder-mead"}},
      maxevals_{{1}},
      functols_{{{{0,1.0e-6}}}},
      grads_{{false}},
      gradient_strategys_{{"parameter-shift"}},
      optimum_energy_abstols_{{}},
      optimum_energy_lowerbounds_{{}},
      accs_{{"qpp"}},
      qns_{{1}},
      rns_{{1}},
      sns_{{256}},
      noises_{{false}},
      acc_uses_lsbs_{{{}}},
      acc_uses_n_bits_{{{}}},
      out_eigenstates_{{{}}},
      out_energys_{{{}}},
      out_jacobians_{{{}}},
      out_thetas_{{{}}},
      out_quantum_energy_calc_times_{{{}}},
      out_quantum_jacobian_calc_times_{{{}}},
      out_classical_energy_jacobian_total_calc_times_{{{}}} 
    {
      xacc::Initialize();
      xacc::setIsPyApi();
      xacc::set_verbose(false);
    }

    // Constructors
    QaoaBase(const bool debug) : QaoaBase() { debug_qbos_ = debug; }
    
    // Summary printout
    virtual const std::string get_summary() const=0;
    
    // Setters and Getters
    void set_colname(const std::string & in_colname);  
    void set_colnames(const VectorString & in_colnames);
    const VectorString &get_colnames() const;
    static const char *help_colnames_;
    //
    void set_rowname(const std::string & in_rowname);  
    void set_rownames(const VectorString & in_rownames);
    const VectorString &get_rownames() const;
    static const char *help_rownames_;
    //
    void set_acc(const std::string &in_acc);
    void set_accs(const VectorString &in_accs);
    const VectorString &get_accs() const;
    static const char *help_accs_;
    void validate_acc(const std::string &acc);
    //
    void set_ham(const std::string &in_ham);
    void set_hams(const VectorString &in_hams);
    const VectorString &get_hams() const;
    static const char *help_hams_;  
    //
    void set_qaoa_step(const size_t &in_qaoa_step);
    void set_qaoa_steps(const VectorN &in_qaoa_steps);
    const VectorN &get_qaoa_steps() const;
    static const char *help_qaoa_steps_;
    //
    void set_qn(const size_t &in_qn);
    void set_qns(const VectorN &in_qns);
    const VectorN &get_qns() const;
    static const char *help_qns_;
    //
    void set_rn(const size_t &in_rn);
    void set_rns(const VectorN &in_rns);
    const VectorN &get_rns() const;
    static const char *help_rns_;
    //
    void set_sn(const size_t &in_sn);
    void set_sns(const VectorN &in_sns);
    const VectorN &get_sns() const;
    static const char *help_sns_;
    //
    void set_noise(const bool &in_noise);
    void set_noises(const VectorBool &in_noises);
    const VectorBool &get_noises() const;
    static const char *help_noises_;
    //
    void set_extended_param(const bool &in_extended_param);
    void set_extended_params(const VectorBool &in_extended_params);
    const VectorBool &get_extended_params() const;
    static const char *help_extended_params_;
    //
    void validate_method(const std::string &method);
    void set_method(const std::string &in_method);
    void set_methods(const VectorString &in_methods);
    const VectorString &get_methods() const;
    static const char *help_methods_;
    //
    void set_grad(const bool &in_grad);
    void set_grads(const VectorBool &in_grads);
    const VectorBool &get_grads() const;
    static const char *help_grads_;
    //
    void validate_gradient_strategy(const std::string &gradient_strategy);
    void set_gradient_strategy(const std::string &in_gradient_strategy);
    void set_gradient_strategys(const VectorString &in_gradient_strategys);
    const VectorString &get_gradient_strategys() const;
    static const char *help_gradient_strategys_;
    //
    void set_maxeval(const size_t &in_maxeval);
    void set_maxevals(const VectorN &in_maxevals);
    const VectorN &get_maxevals() const;
    static const char *help_maxevals_;
    //
    void set_functol(const ND & in_functol);
    void set_functols(const VectorMapND & in_functols);
    const VectorMapND & get_functols() const;
    static const char* help_functols_;
    //
    void set_optimum_energy_abstol(const ND & in_optimum_energy_abstol);
    void set_optimum_energy_abstols(const VectorMapND & in_optimum_energy_abstols);
    const VectorMapND & get_optimum_energy_abstols() const;
    static const char* help_optimum_energy_abstols_;
    //
    void set_optimum_energy_lowerbound(const ND & in_optimum_energy_lowerbound);
    void set_optimum_energy_lowerbounds(const VectorMapND & in_optimum_energy_lowerbounds);
    const VectorMapND & get_optimum_energy_lowerbounds() const;
    static const char* help_optimum_energy_lowerbounds_;
    //
    void set_out_eigenstate(const std::string & out_eigenstate);
    void set_out_eigenstates(const VectorString & out_eigenstates);
    const VectorString & get_out_eigenstates() const;
    static const char* help_out_eigenstates_;
    //
    void set_out_energy(const ND & out_energy);
    void set_out_energys(const VectorMapND & out_energys);
    const VectorMapND & get_out_energys() const;
    static const char* help_out_energys_;
    //
    void set_out_jacobian(const ND & out_jacobian);
    void set_out_jacobians(const VectorMapND & out_jacobians);
    const VectorMapND & get_out_jacobians() const;
    static const char* help_out_jacobians_;
    //
    void set_out_theta(const ND & out_theta);
    void set_out_thetas(const VectorMapND & out_thetas);
    const VectorMapND & get_out_thetas() const;
    static const char* help_out_thetas_;
    //
    void set_out_quantum_energy_calc_time(const ND & out_quantum_energy_calc_time);
    void set_out_quantum_energy_calc_times(const VectorMapND & out_quantum_energy_calc_times);
    const VectorMapND & get_out_quantum_energy_calc_times() const;
    static const char* help_out_quantum_energy_calc_times_;
    //
    void set_out_quantum_jacobian_calc_time(const ND & out_quantum_jacobian_calc_time);
    void set_out_quantum_jacobian_calc_times(const VectorMapND & out_quantum_jacobian_calc_times);
    const VectorMapND & get_out_quantum_jacobian_calc_times() const;
    static const char* help_out_quantum_jacobian_calc_times_;
    //
    void set_out_classical_energy_jacobian_total_calc_time(const ND & out_classical_energy_jacobian_total_calc_time);
    void set_out_classical_energy_jacobian_total_calc_times(const VectorMapND & out_classical_energy_jacobian_total_calc_times);
    const VectorMapND & get_out_classical_energy_jacobian_total_calc_times() const;
    static const char* help_out_classical_energy_jacobian_total_calc_times_;
    
    // Misc functions
    int binomialCoefficient(int n, int k);
    int ipow(int base, int exp);
    std::string measurement_circ(
      const int & n_qubits,
      const int & qaoa_steps,
      const std::string & H_string,
      const bool & extended_param,
      const std::vector<double> & params
    );
    
    // Validation methods
    template <class TT> int eqlength(const TT &in_d, const int N_ii);
    template <class TT> int singleton_or_eqlength(const TT &in_d, const int N_ii);
    virtual int is_ii_consistent()=0;
    virtual int is_jj_consistent()=0;
    
    // Methods
    virtual void run(const size_t &ii, const size_t &jj)=0;
    void run();
};

template <class TT> 
int QaoaBase::eqlength(const TT &in_d, const int N_ii) {
  const int INVALID = -1;
  if (in_d.size() == N_ii) {
    return N_ii;
  } else {
    return INVALID;
  }
}

template <class TT>
int QaoaBase::singleton_or_eqlength(const TT &in_d, const int N_ii) {
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

} // namespace op
} // namespace qb

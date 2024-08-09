// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#pragma once

#include "xacc.hpp"
#include "xacc_service.hpp"
#include "PauliOperator.hpp"
#include "InstructionIterator.hpp"

#include "qristal/core/typedefs.hpp"
#include "qristal/core/utils.hpp"
#include "qristal/core/optimization/qaoa/autodiff.hpp"

#include <sstream>

namespace qristal {
namespace op {

class QaoaBase {
  protected:
    Table2d<std::string> hams_;
    Table2d<size_t> qns_;

    Table2d<std::string> accs_;
    Table2d<size_t> sns_;
    Table2d<bool> noises_;

    Table2d<size_t> qaoa_steps_;
    Table2d<bool> extended_params_;

    Table2d<std::string> rownames_;
    Table2d<std::string> colnames_;

    Table2d<std::string> methods_;
    Table2d<size_t> maxevals_;
    Table2d<std::map<int,double>> functols_;
    Table2d<std::map<int,double>> optimum_energy_abstols_;
    Table2d<std::map<int,double>> optimum_energy_lowerbounds_;
    Table2d<bool> grads_;
    Table2d<std::string> gradient_strategys_;

    // Variables not wrapped to Python
    Table2d<bool> acc_outputs_qbit0_left_;
    Table2d<size_t> acc_uses_n_bits_;

    // Storage for quantities of interest
    Table2d<std::string> out_eigenstates_;
    Table2d<std::map<int,double>> out_energys_;
    Table2d<std::map<int,double>> out_jacobians_;
    Table2d<std::map<int,double>> out_thetas_;
    Table2d<std::map<int,double>> out_quantum_energy_calc_times_;
    Table2d<std::map<int,double>> out_quantum_jacobian_calc_times_;
    Table2d<std::map<int,double>> out_classical_energy_jacobian_total_calc_times_;

    // Debugging
    bool debug_qristal_;

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
    const size_t QAOA_STEPS_LOWERBOUND = 1;
    const size_t QAOA_STEPS_UPPERBOUND = 10000;
    const size_t MAXEVALS_LOWERBOUND = 1;
    const size_t MAXEVALS_UPPERBOUND = 1000000;

  public:
    QaoaBase() :
      debug_qristal_(false),
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
      sns_{{256}},
      noises_{{false}},
      acc_outputs_qbit0_left_{{{}}},
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
    QaoaBase(const bool debug) : QaoaBase() { debug_qristal_ = debug; }

    // Summary printout
    virtual const std::string get_summary() const=0;

    // Setters and Getters
    void set_colname(const std::string & in_colname);
    void set_colnames(const Table2d<std::string> & in_colnames);
    const Table2d<std::string> &get_colnames() const;
    static const char *help_colnames_;
    //
    void set_rowname(const std::string & in_rowname);
    void set_rownames(const Table2d<std::string> & in_rownames);
    const Table2d<std::string> &get_rownames() const;
    static const char *help_rownames_;
    //
    void set_acc(const std::string &in_acc);
    void set_accs(const Table2d<std::string> &in_accs);
    const Table2d<std::string> &get_accs() const;
    static const char *help_accs_;
    void validate_acc(const std::string &acc);
    //
    void set_ham(const std::string &in_ham);
    void set_hams(const Table2d<std::string> &in_hams);
    const Table2d<std::string> &get_hams() const;
    static const char *help_hams_;
    //
    void set_qaoa_step(const size_t &in_qaoa_step);
    void set_qaoa_steps(const Table2d<size_t> &in_qaoa_steps);
    const Table2d<size_t> &get_qaoa_steps() const;
    static const char *help_qaoa_steps_;
    //
    void set_qn(const size_t &in_qn);
    void set_qns(const Table2d<size_t> &in_qns);
    const Table2d<size_t> &get_qns() const;
    static const char *help_qns_;
    //
    void set_sn(const size_t &in_sn);
    void set_sns(const Table2d<size_t> &in_sns);
    const Table2d<size_t> &get_sns() const;
    static const char *help_sns_;
    //
    void set_noise(const bool &in_noise);
    void set_noises(const Table2d<bool> &in_noises);
    const Table2d<bool> &get_noises() const;
    static const char *help_noises_;
    //
    void set_extended_param(const bool &in_extended_param);
    void set_extended_params(const Table2d<bool> &in_extended_params);
    const Table2d<bool> &get_extended_params() const;
    static const char *help_extended_params_;
    //
    void validate_method(const std::string &method);
    void set_method(const std::string &in_method);
    void set_methods(const Table2d<std::string> &in_methods);
    const Table2d<std::string> &get_methods() const;
    static const char *help_methods_;
    //
    void set_grad(const bool &in_grad);
    void set_grads(const Table2d<bool> &in_grads);
    const Table2d<bool> &get_grads() const;
    static const char *help_grads_;
    //
    void validate_gradient_strategy(const std::string &gradient_strategy);
    void set_gradient_strategy(const std::string &in_gradient_strategy);
    void set_gradient_strategys(const Table2d<std::string> &in_gradient_strategys);
    const Table2d<std::string> &get_gradient_strategys() const;
    static const char *help_gradient_strategys_;
    //
    void set_maxeval(const size_t &in_maxeval);
    void set_maxevals(const Table2d<size_t> &in_maxevals);
    const Table2d<size_t> &get_maxevals() const;
    static const char *help_maxevals_;
    //
    void set_functol(const std::map<int,double> & in_functol);
    void set_functols(const Table2d<std::map<int,double>> & in_functols);
    const Table2d<std::map<int,double>> & get_functols() const;
    static const char* help_functols_;
    //
    void set_optimum_energy_abstol(const std::map<int,double> & in_optimum_energy_abstol);
    void set_optimum_energy_abstols(const Table2d<std::map<int,double>> & in_optimum_energy_abstols);
    const Table2d<std::map<int,double>> & get_optimum_energy_abstols() const;
    static const char* help_optimum_energy_abstols_;
    //
    void set_optimum_energy_lowerbound(const std::map<int,double> & in_optimum_energy_lowerbound);
    void set_optimum_energy_lowerbounds(const Table2d<std::map<int,double>> & in_optimum_energy_lowerbounds);
    const Table2d<std::map<int,double>> & get_optimum_energy_lowerbounds() const;
    static const char* help_optimum_energy_lowerbounds_;
    //
    void set_out_eigenstate(const std::string & out_eigenstate);
    void set_out_eigenstates(const Table2d<std::string> & out_eigenstates);
    const Table2d<std::string> & get_out_eigenstates() const;
    static const char* help_out_eigenstates_;
    //
    void set_out_energy(const std::map<int,double> & out_energy);
    void set_out_energys(const Table2d<std::map<int,double>> & out_energys);
    const Table2d<std::map<int,double>> & get_out_energys() const;
    static const char* help_out_energys_;
    //
    void set_out_jacobian(const std::map<int,double> & out_jacobian);
    void set_out_jacobians(const Table2d<std::map<int,double>> & out_jacobians);
    const Table2d<std::map<int,double>> & get_out_jacobians() const;
    static const char* help_out_jacobians_;
    //
    void set_out_theta(const std::map<int,double> & out_theta);
    void set_out_thetas(const Table2d<std::map<int,double>> & out_thetas);
    const Table2d<std::map<int,double>> & get_out_thetas() const;
    static const char* help_out_thetas_;
    //
    void set_out_quantum_energy_calc_time(const std::map<int,double> & out_quantum_energy_calc_time);
    void set_out_quantum_energy_calc_times(const Table2d<std::map<int,double>> & out_quantum_energy_calc_times);
    const Table2d<std::map<int,double>> & get_out_quantum_energy_calc_times() const;
    static const char* help_out_quantum_energy_calc_times_;
    //
    void set_out_quantum_jacobian_calc_time(const std::map<int,double> & out_quantum_jacobian_calc_time);
    void set_out_quantum_jacobian_calc_times(const Table2d<std::map<int,double>> & out_quantum_jacobian_calc_times);
    const Table2d<std::map<int,double>> & get_out_quantum_jacobian_calc_times() const;
    static const char* help_out_quantum_jacobian_calc_times_;
    //
    void set_out_classical_energy_jacobian_total_calc_time(const std::map<int,double> & out_classical_energy_jacobian_total_calc_time);
    void set_out_classical_energy_jacobian_total_calc_times(const Table2d<std::map<int,double>> & out_classical_energy_jacobian_total_calc_times);
    const Table2d<std::map<int,double>> & get_out_classical_energy_jacobian_total_calc_times() const;
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
}

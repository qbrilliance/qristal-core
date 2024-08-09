// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/optimization/qaoa/qaoa_base.hpp"

namespace qristal::op {

  // colnames
  void QaoaBase::set_colname(const std::string &colname) {
    QaoaBase::colnames_.clear();
    QaoaBase::colnames_.push_back({colname});
  }
  void QaoaBase::set_colnames(const Table2d<std::string> &colnames) {
    QaoaBase::colnames_ = colnames;
  }
  const Table2d<std::string> & QaoaBase::get_colnames() const { return QaoaBase::colnames_; }

  // rownames
  void QaoaBase::set_rowname(const std::string &rowname) {
    QaoaBase::rownames_.clear();
    QaoaBase::rownames_.push_back({rowname});
  }
  void QaoaBase::set_rownames(const Table2d<std::string> &rownames) {
    QaoaBase::rownames_ = rownames;
  }
  const Table2d<std::string> & QaoaBase::get_rownames() const { return QaoaBase::rownames_; }

  // accs
  void QaoaBase::validate_acc(const std::string &acc) {

    if (VALID_ACCS.find(acc) == VALID_ACCS.end()) {
      std::stringstream sstr;
      sstr << "Invalid value: " << acc <<" - valid values are:" << std::endl << std::endl;
      for (auto & el : VALID_ACCS) {
        sstr << "  " << el << std::endl;
      }
      throw std::range_error(sstr.str());
    }
  }
  void QaoaBase::set_acc(const std::string &acc) {
    QaoaBase::validate_acc(acc);
    QaoaBase::accs_.clear();
    QaoaBase::accs_.push_back({acc});
  }
  void QaoaBase::set_accs(const Table2d<std::string> &accs) {
    for (auto item : accs) {
      for (auto im : item) {
        QaoaBase::validate_acc(im);
      }
    }
    QaoaBase::accs_ = accs;
  }
  const Table2d<std::string> & QaoaBase::get_accs() const { return QaoaBase::accs_; }

  // hams (Hamiltonians as a string consisting of the sum of Pauli terms)
  void QaoaBase::set_ham(const std::string &ham) {
    QaoaBase::hams_.clear();
    QaoaBase::hams_.push_back({ham});
  }
  void QaoaBase::set_hams(const Table2d<std::string> &hams) {
    QaoaBase::hams_ = hams;
  }
  const Table2d<std::string> & QaoaBase::get_hams() const { return QaoaBase::hams_; }

  // qaoa_steps (parameter setting the number of layers in QAOA)
  void QaoaBase::set_qaoa_step(const std::size_t &in_qaoa_step) {
    QaoaBase::qaoa_steps_.clear();
    QaoaBase::qaoa_steps_.push_back({in_qaoa_step});
  }
  void QaoaBase::set_qaoa_steps(const Table2d<size_t> &in_qaoa_steps) {
    QaoaBase::qaoa_steps_ = in_qaoa_steps;
  }
  const Table2d<size_t> & QaoaBase::get_qaoa_steps() const { return QaoaBase::qaoa_steps_; }

  // qns (number of physical qubits)
  void QaoaBase::set_qn(const std::size_t &in_qn) {
    QaoaBase::qns_.clear();
    QaoaBase::qns_.push_back({in_qn});
  }
  void QaoaBase::set_qns(const Table2d<size_t> &in_qns) {
    QaoaBase::qns_ = in_qns;
  }
  const Table2d<size_t> & QaoaBase::get_qns() const { return QaoaBase::qns_; }

  // sns (number of shots)
  void QaoaBase::set_sn(const std::size_t &in_sn) {
    QaoaBase::sns_.clear();
    QaoaBase::sns_.push_back({in_sn});
  }
  void QaoaBase::set_sns(const Table2d<size_t> &in_sns) {
    QaoaBase::sns_ = in_sns;
  }
  const Table2d<size_t> & QaoaBase::get_sns() const { return QaoaBase::sns_; }

  // noises
  void QaoaBase::set_noise(const bool &in_noise) {
    QaoaBase::noises_.clear();
    QaoaBase::noises_.push_back({in_noise});
  }
  void QaoaBase::set_noises(const Table2d<bool> &in_noises) {
    QaoaBase::noises_ = in_noises;
  }
  const Table2d<bool> & QaoaBase::get_noises() const { return QaoaBase::noises_; }

  // extended_params
  void QaoaBase::set_extended_param(const bool &in_extended_param) {
    QaoaBase::extended_params_.clear();
    QaoaBase::extended_params_.push_back({in_extended_param});
  }
  void QaoaBase::set_extended_params(const Table2d<bool> &in_extended_params) {
    QaoaBase::extended_params_ = in_extended_params;
  }
  const Table2d<bool> & QaoaBase::get_extended_params() const { return QaoaBase::extended_params_; }

  // methods (optimiser algorithm)
  void QaoaBase::validate_method(const std::string &method) {
    if (VALID_OPTIMISER_METHODS.find(method) == VALID_OPTIMISER_METHODS.end()) {
      std::stringstream sstr;
      sstr << "Invalid value: " << method <<" - valid values are:" << std::endl << std::endl;
      for (auto & el : VALID_OPTIMISER_METHODS) {
        sstr << "  " << el << std::endl;
      }
      throw std::range_error(sstr.str());
    }
  }
  void QaoaBase::set_method(const std::string &method) {
    QaoaBase::validate_method(method);
    QaoaBase::methods_.clear();
    QaoaBase::methods_.push_back({method});
  }
  void QaoaBase::set_methods(const Table2d<std::string> &methods) {
    for (auto item : methods) {
      for (auto im : item) {
        QaoaBase::validate_method(im);
      }
    }
    QaoaBase::methods_ = methods;
  }
  const Table2d<std::string> & QaoaBase::get_methods() const { return QaoaBase::methods_; }

  // grads (enable to return the gradient at the optimum solution)
  void QaoaBase::set_grad(const bool &in_grad) {
    QaoaBase::grads_.clear();
    QaoaBase::grads_.push_back({in_grad});
  }
  void QaoaBase::set_grads(const Table2d<bool> &in_grads) {
    QaoaBase::grads_ = in_grads;
  }
  const Table2d<bool> & QaoaBase::get_grads() const { return QaoaBase::grads_; }

  // gradient_strategys (method used for gradient calculations)
  void QaoaBase::validate_gradient_strategy(const std::string &gradient_strategy) {
    if (VALID_GRADIENT_STRATEGYS.find(gradient_strategy) == VALID_GRADIENT_STRATEGYS.end()) {
      std::stringstream sstr;
      sstr << "Invalid value: " << gradient_strategy <<" - valid values are:" << std::endl << std::endl;
      for (auto & el : VALID_GRADIENT_STRATEGYS) {
        sstr << "  " << el << std::endl;
      }
      throw std::range_error(sstr.str());
    }
  }
  void QaoaBase::set_gradient_strategy(const std::string &gradient_strategy) {
    QaoaBase::validate_gradient_strategy(gradient_strategy);
    QaoaBase::gradient_strategys_.clear();
    QaoaBase::gradient_strategys_.push_back({gradient_strategy});
  }
  void QaoaBase::set_gradient_strategys(const Table2d<std::string> &gradient_strategys) {
    for (auto item : gradient_strategys) {
      for (auto im : item) {
        QaoaBase::validate_gradient_strategy(im);
      }
    }
    QaoaBase::gradient_strategys_ = gradient_strategys;
  }
  const Table2d<std::string> & QaoaBase::get_gradient_strategys() const { return QaoaBase::gradient_strategys_; }

  // maxeval (number of iterations for the optimiser)
  void QaoaBase::set_maxeval(const std::size_t &in_maxeval) {
    QaoaBase::maxevals_.clear();
    QaoaBase::maxevals_.push_back({in_maxeval});
  }
  void QaoaBase::set_maxevals(const Table2d<size_t> &in_maxevals) {
    QaoaBase::maxevals_ = in_maxevals;
  }
  const Table2d<size_t> & QaoaBase::get_maxevals() const { return QaoaBase::maxevals_; }

  // functol
  void QaoaBase::set_functol(const std::map<int,double> &in_functol) {
    QaoaBase::functols_.clear();
    QaoaBase::functols_.push_back({in_functol});
  }
  void QaoaBase::set_functols(const Table2d<std::map<int,double>> &in_functols) { QaoaBase::functols_ = in_functols; }
  const Table2d<std::map<int,double>> & QaoaBase::get_functols() const { return QaoaBase::functols_; }

  // optimum_energy_abstol
  void QaoaBase::set_optimum_energy_abstol(const std::map<int,double> &in_optimum_energy_abstol) {
    QaoaBase::optimum_energy_abstols_.clear();
    QaoaBase::optimum_energy_abstols_.push_back({in_optimum_energy_abstol});
  }
  void QaoaBase::set_optimum_energy_abstols(const Table2d<std::map<int,double>> &in_optimum_energy_abstols) {
    QaoaBase::optimum_energy_abstols_ = in_optimum_energy_abstols;
  }
  const Table2d<std::map<int,double>> & QaoaBase::get_optimum_energy_abstols() const {
    return QaoaBase::optimum_energy_abstols_;
  }

  // optimum_energy_lowerbound
  void QaoaBase::set_optimum_energy_lowerbound(const std::map<int,double> &in_optimum_energy_lowerbound) {
    QaoaBase::optimum_energy_lowerbounds_.clear();
    QaoaBase::optimum_energy_lowerbounds_.push_back({in_optimum_energy_lowerbound});
  }
  void QaoaBase::set_optimum_energy_lowerbounds(const Table2d<std::map<int,double>> &in_optimum_energy_lowerbounds) {
    QaoaBase::optimum_energy_lowerbounds_ = in_optimum_energy_lowerbounds;
  }
  const Table2d<std::map<int,double>> & QaoaBase::get_optimum_energy_lowerbounds() const {
    return QaoaBase::optimum_energy_lowerbounds_;
  }

  // out_eigenstate
  void QaoaBase::set_out_eigenstate(const std::string &out_eigenstate) {
    QaoaBase::out_eigenstates_.clear();
    QaoaBase::out_eigenstates_.push_back({out_eigenstate});
  }
  void QaoaBase::set_out_eigenstates(const Table2d<std::string> &out_eigenstates) { QaoaBase::out_eigenstates_ = out_eigenstates; }
  const Table2d<std::string> & QaoaBase::get_out_eigenstates() const { return QaoaBase::out_eigenstates_; }

  // out_energy
  void QaoaBase::set_out_energy(const std::map<int,double> &out_energy) {
    QaoaBase::out_energys_.clear();
    QaoaBase::out_energys_.push_back({out_energy});
  }
  void QaoaBase::set_out_energys(const Table2d<std::map<int,double>> &out_energys) { QaoaBase::out_energys_ = out_energys; }
  const Table2d<std::map<int,double>> & QaoaBase::get_out_energys() const { return QaoaBase::out_energys_; }


  // out_jacobian
  void QaoaBase::set_out_jacobian(const std::map<int,double> &out_jacobian) {
    QaoaBase::out_jacobians_.clear();
    QaoaBase::out_jacobians_.push_back({out_jacobian});
  }
  void QaoaBase::set_out_jacobians(const Table2d<std::map<int,double>> &out_jacobians) { QaoaBase::out_jacobians_ = out_jacobians; }
  const Table2d<std::map<int,double>> & QaoaBase::get_out_jacobians() const { return QaoaBase::out_jacobians_; }

  // out_theta
  void QaoaBase::set_out_theta(const std::map<int,double> &out_theta) {
    QaoaBase::out_thetas_.clear();
    QaoaBase::out_thetas_.push_back({out_theta});
  }
  void QaoaBase::set_out_thetas(const Table2d<std::map<int,double>> &out_thetas) { QaoaBase::out_thetas_ = out_thetas; }
  const Table2d<std::map<int,double>> & QaoaBase::get_out_thetas() const { return QaoaBase::out_thetas_; }

  // out_quantum_energy_calc_time
  void QaoaBase::set_out_quantum_energy_calc_time(const std::map<int,double> &out_quantum_energy_calc_time) {
    QaoaBase::out_quantum_energy_calc_times_.clear();
    QaoaBase::out_quantum_energy_calc_times_.push_back({out_quantum_energy_calc_time});
  }
  void QaoaBase::set_out_quantum_energy_calc_times(const Table2d<std::map<int,double>> &out_quantum_energy_calc_times) {
    QaoaBase::out_quantum_energy_calc_times_ = out_quantum_energy_calc_times;
  }
  const Table2d<std::map<int,double>> & QaoaBase::get_out_quantum_energy_calc_times() const {
    return QaoaBase::out_quantum_energy_calc_times_;
  }

  // out_quantum_jacobian_calc_time
  void QaoaBase::set_out_quantum_jacobian_calc_time(const std::map<int,double> &out_quantum_jacobian_calc_time) {
    QaoaBase::out_quantum_jacobian_calc_times_.clear();
    QaoaBase::out_quantum_jacobian_calc_times_.push_back({out_quantum_jacobian_calc_time});
  }
  void QaoaBase::set_out_quantum_jacobian_calc_times(const Table2d<std::map<int,double>> &out_quantum_jacobian_calc_times) {
    QaoaBase::out_quantum_jacobian_calc_times_ = out_quantum_jacobian_calc_times;
  }
  const Table2d<std::map<int,double>> & QaoaBase::get_out_quantum_jacobian_calc_times() const {
    return QaoaBase::out_quantum_jacobian_calc_times_;
  }

  // out_classical_energy_jacobian_total_calc_time
  void QaoaBase::set_out_classical_energy_jacobian_total_calc_time(const std::map<int,double> &out_classical_energy_jacobian_total_calc_time) {
    QaoaBase::out_classical_energy_jacobian_total_calc_times_.clear();
    QaoaBase::out_classical_energy_jacobian_total_calc_times_.push_back({out_classical_energy_jacobian_total_calc_time});
  }
  void QaoaBase::set_out_classical_energy_jacobian_total_calc_times(const Table2d<std::map<int,double>> &out_classical_energy_jacobian_total_calc_times) {
    QaoaBase::out_classical_energy_jacobian_total_calc_times_ = out_classical_energy_jacobian_total_calc_times;
  }
  const Table2d<std::map<int,double>> & QaoaBase::get_out_classical_energy_jacobian_total_calc_times() const {
    return QaoaBase::out_classical_energy_jacobian_total_calc_times_;
  }

  // Start of help text
  const char* QaoaBase::help_colnames_ = R"(
          colname:

          String array for storing the name of each condition (column).

          colnames:

          A 2d-array version of colname.

  )";

  const char* QaoaBase::help_rownames_ = R"(
          colname:

          String array for storing the name of each experiment (row).

          rownames:

          A 2d-array version of rowname.

  )";

  const char* QaoaBase::help_hams_ = R"(
          ham:

          String of a Hamiltonian expressed as a sum of Pauli terms (note: X, Y and Z terms only, I is implied).

          hams:

          A 2d-array version of ham.

  )";

  /*
  const char* QaoaBase::help_ansatz_depths_ = R"(
          ansatz_depth:

          Depth for built-in default ansatz.

          ansatz_depths:

          A 2d-array version of ansatz_depth.

  )";

  const char* QaoaBase::help_aswapns_ = R"(
          aswapn:

          Number of particles when using the ASWAP built-in ansatz.

          aswapns:

          A 2d-array version of ansatz_depth.

  )";
  */

  const char* QaoaBase::help_qaoa_steps_ = R"(
          qaoa_step:

          Number of layers in the QAOA ansatz.

          qaos_depths:

          A 2d-array version of qaoa_step.

  )";

  const char* QaoaBase::help_accs_ = R"(
          acc:

          Valid settings: "aer" | "tnqvm" | "qpp"

          Select a back-end simulator.

          accs:

          A 2d-array version of acc.
  )";

  const char* QaoaBase::help_qns_ = R"(
          qn:

          Number of physical qubits.

          qns:

          A 2d-array version of qn.

  )";

  const char* QaoaBase::help_sns_ = R"(
          sn:

          Number of shots.  If set to 0, QAOA will use non-stochastic calculations.

          sns:

          A 2d-array version of sn.

  )";

  const char* QaoaBase::help_noises_ = R"(
          noise:

          Set to True to enable noise modelling (requires .acc='aer').

          noises:

          A 2d-array version of noise.

  )";

  const char* QaoaBase::help_extended_params_ = R"(
          extended_param:

          Set to True to enable QAOA with extended parameters.

          extended_params:

          A 2d-array version of extended_param.

  )";

  const char* QaoaBase::help_methods_ = R"(
          method:

          Valid settings: "nelder-mead" | "cobyla" | "l-bfgs"

          Algorithm used by the optimiser.

          methods:

          A 2d-array version of method.

  )";

  const char* QaoaBase::help_grads_ = R"(
          grad:

          Set to True to enable gradient calculations.

          grads:

          A 2d-array version of grad.

  )";

  const char* QaoaBase::help_gradient_strategys_ = R"(
          method:

          Valid settings: "parameter-shift" | "central" | "forward" | "backward" | "autodiff"

          Algorithm used to calculate gradients.

          methods:

          A 2d-array version of gradient_strategy.

  )";

  const char* QaoaBase::help_maxevals_ = R"(
          maxeval:

          Number of iterations (stopping criterion of the optimiser).

          maxevals:

          A 2d-array version of maxeval.

  )";

  const char* QaoaBase::help_functols_ = R"(
          functol:

          Tolerance (stopping criterion of the optimiser).

          functols:

          A 2d-array version of functol.

  )";

  const char* QaoaBase::help_optimum_energy_abstols_ = R"(
          optimum_energy_abstol:

          Interval [optimum_energy_lowerbound, optimum_energy_lower_bound + optimum_energy_abstol] that is
          expected to contain the minimum eigenvalue returned by QAOA.

          optimum_energy_abstols:

          A 2d-array version of optimum_energy_abstol.

  )";

  const char* QaoaBase::help_optimum_energy_lowerbounds_ = R"(
          optimum_energy_lowerbound:

          Interval [optimum_energy_lowerbound, optimum_energy_lowerbound + optimum_energy_abstol] that is
          expected to contain the minimum eigenvalue returned by QAOA.

          optimum_energy_lowerbounds:

          A 2d-array version of optimum_energy_lowerbound.

  )";

  const char* QaoaBase::help_out_eigenstates_ = R"(
          out_eigenstate:

          For a classical Hamiltonian, at the optimum solution returned by QAOA,
          this string is q[n-1]q[n-2]..q[1]q[0] (for "qpp", reverse for "aer")
          == the bit string for the optimum state.

          out_eigenstates:

          A 2d-array version of out_eigenstate.

  )";

  const char* QaoaBase::help_out_energys_ = R"(
          out_energy:

          Energy returned by QAOA over maxeval iterations.

          out_energys:

          A 2d-array version of out_energy.

  )";

  const char* QaoaBase::help_out_jacobians_ = R"(
          out_jacobian:

          Jacobian returned by QAOA over maxeval iterations.

          out_jacobians:

          A 2d-array version of out_jacobian.

  )";

  const char* QaoaBase::help_out_thetas_ = R"(
          out_theta:

          theta (optimed solution) returned by QAOA over maxeval iterations.

          out_thetas:

          A 2d-array version of out_theta.

  )";

  const char* QaoaBase::help_out_quantum_energy_calc_times_ = R"(
          out_quantum_energy_calc_time:

          Quantum estimated time needed to calculate the energy per iteration, over maxeval iterations.

          out_quantum_energy_calc_times:

          A 2d-array version of out_quantum_energy_calc_time.

  )";

  const char* QaoaBase::help_out_quantum_jacobian_calc_times_ = R"(
          out_quantum_jacobian_calc_time:

          Quantum estimated time needed to calculate the Jacobian per iteration, over maxeval iterations.

          out_quantum_jacobian_calc_times:

          A 2d-array version of out_quantum_jacobian_calc_time.

  )";

  const char* QaoaBase::help_out_classical_energy_jacobian_total_calc_times_ = R"(
          out_classical_energy_jacobian_total_calc_time:

          Classical wall-time needed to calculate the energy and Jacobian (combined) per iteration, over maxeval iterations.

          out_classical_energy_jacobian_total_calc_times:

          A 2d-array version of out_classical_energy_jacobian_total_calc_time.

  )";

  int QaoaBase::binomialCoefficient(int n, int k) {
    // Base Cases
    if (k == 0 || k == n) {
       return 1;
    }
    // Recursive NB doubly recursive so potwentially prone to stack overflow
    return binomialCoefficient(n - 1, k - 1) + binomialCoefficient(n - 1, k);
  }

  int QaoaBase::ipow(int base, int exp) {
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

  std::string QaoaBase::measurement_circ(
    const int &n_qubits,
    const int &qaoa_steps,
    const std::string &H_string,
    const bool &extended_param,
    const std::vector<double> &params
  ) {
    //auto H = xacc::quantum::getObservable("pauli", H_string);
    std::shared_ptr<xacc::Observable> H = std::make_shared<xacc::quantum::PauliOperator>();
    H->fromString(H_string);


    auto qaoa_ansatz = xacc::createComposite(
      "qaoa",
      {
        {"nbQubits", n_qubits},
        {"nbSteps", qaoa_steps},
        {"cost-ham", H},
        {"parameter-scheme", "Standard"}
      }
    );

    if (extended_param) {
      qaoa_ansatz = xacc::createComposite(
        "qaoa",
        {
          {"nbQubits", n_qubits},
          {"nbSteps", qaoa_steps},
          {"cost-ham", H},
          {"parameter-scheme", "Extended"}
        }
      );
    }

    std::vector<double> param_rv = params;
    std::reverse(param_rv.begin(), param_rv.end());

    qaoa_ansatz = qaoa_ansatz->operator()(param_rv);

    // building the circuit:
    std::stringstream circuit;

    // qaoa_ansatz = qaoa_ansatz->operator()(params);
    auto xasmCompiler = xacc::getCompiler("xasm");
    auto xasm_str = xasmCompiler->translate(qaoa_ansatz);

    // Removes '}' at the end of the ansatz
    circuit << xasm_str.substr(0, xasm_str.length() - 2) << std::endl;

    // Add measurement lines to ansatz circuit
    for (int i = (n_qubits - 1); i >= 0; i--) {
      circuit << "Measure(q[" << i << "]);" << std::endl;
    }
    circuit << "}" << std::endl;
    return circuit.str();
  }

  void QaoaBase::run() {
    if (debug_qristal_) std::cout << "Invoked run()" << std::endl;

    // Shape consistency
    const int N_ii = is_ii_consistent();
    const int N_jj = is_jj_consistent();
    if (N_ii < 0) {
      throw std::range_error("Leading dimension has inconsistent length");
    }

    if (N_jj < 0) {
      throw std::range_error("Second dimension has inconsistent length");
    }
    if (debug_qristal_) {
      std::cout << "N_ii: " << N_ii << std::endl;
    }
    if (debug_qristal_) {
      std::cout << "N_jj: " << N_jj << std::endl;
    }

    // Clear all stored results:
    out_eigenstates_.clear();
    out_eigenstates_.resize(N_ii);
    for (auto el : out_eigenstates_) {
      el.resize(N_jj);
    }
    out_energys_.clear();
    out_energys_.resize(N_ii);
    for (auto el : out_energys_) {
      el.resize(N_jj);
    }
    out_jacobians_.clear();
    out_jacobians_.resize(N_ii);
    for (auto el : out_jacobians_) {
      el.resize(N_jj);
    }
    out_thetas_.clear();
    out_thetas_.resize(N_ii);
    for (auto el : out_thetas_) {
      el.resize(N_jj);
    }
    out_quantum_energy_calc_times_.clear();
    out_quantum_energy_calc_times_.resize(N_ii);
    for (auto el : out_quantum_energy_calc_times_) {
      el.resize(N_jj);
    }
    out_quantum_jacobian_calc_times_.clear();
    out_quantum_jacobian_calc_times_.resize(N_ii);
    for (auto el : out_quantum_jacobian_calc_times_) {
      el.resize(N_jj);
    }
    out_quantum_jacobian_calc_times_.clear();
    out_quantum_jacobian_calc_times_.resize(N_ii);
    for (auto el : out_quantum_jacobian_calc_times_) {
      el.resize(N_jj);
    }
    out_classical_energy_jacobian_total_calc_times_.clear();
    out_classical_energy_jacobian_total_calc_times_.resize(N_ii);
    for (auto el : out_classical_energy_jacobian_total_calc_times_) {
      el.resize(N_jj);
    }

    acc_outputs_qbit0_left_.clear();
    acc_outputs_qbit0_left_.resize(N_ii);
    for (auto el : acc_outputs_qbit0_left_) {
      el.resize(N_jj);
    }

    acc_uses_n_bits_.clear();
    acc_uses_n_bits_.resize(N_ii);
    for (auto el : acc_uses_n_bits_) {
      el.resize(N_jj);
    }

    for (int ii = 0; ii < N_ii; ii++) {
      for (int jj = 0; jj < N_jj; jj++) {
        run(ii, jj);
      }
    }
  }

}

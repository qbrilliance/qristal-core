// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/optimization/qaoa/qaoa_simple.hpp"
#include "qristal/core/profiler.hpp"
#include "qristal/core/pretranspiler.hpp"

namespace qristal::op {

  // theta
  void QaoaSimple::set_theta(const std::map<int,double> &in_theta) {
    QaoaSimple::thetas_.clear();
    QaoaSimple::thetas_.push_back({in_theta});
  }
  void QaoaSimple::set_thetas(const Table2d<std::map<int,double>> &in_thetas) { QaoaSimple::thetas_ = in_thetas; }
  const Table2d<std::map<int,double>> & QaoaSimple::get_thetas() const { return QaoaSimple::thetas_; }

  // Start of help text
  const char* QaoaSimple::help_thetas_ = R"(
          theta:

          Parameters for ansatz, in the form integer->double map.

          thetas:

          A 2d-array version of theta.

  )";

  const std::string QaoaSimple::get_summary() const {
    using namespace xacc;
    std::ostringstream out;

    out << "* theta, initial:" << std::endl <<
    "    Initial values of theta parameter" << std::endl <<
    "  = ";
    for (auto item : get_thetas()) {
      out << std::endl << " ";
      for (auto itel : item) {
        for (auto it : itel) {
          out << " | " << it.first << ": " << it.second;
        }
        if (itel.size() > 0) {
          out << " | ";
        } else {
          out << " NA ";
        }
      }
    }
    out << std::endl << std::endl;
    //

    out << "* qaoa_step:" << std::endl <<
    "    Number of QAOA layers" << std::endl <<
    "  = ";
    for (auto item : get_qaoa_steps()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* ham:" << std::endl <<
    "    Hamiltonian as sum of Pauli terms" << std::endl <<
    "  = ";
    for (auto item : get_hams()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* sn:" << std::endl <<
    "    Number of shots" << std::endl <<
    "  = ";
    for (auto item : get_sns()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* qn:" << std::endl <<
    "    Number of qubits" << std::endl <<
    "  = ";
    for (auto item : get_qns()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* acc:" << std::endl <<
    "    Back-end simulator" << std::endl <<
    "  = ";
    for (auto item : get_accs()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* noise:" << std::endl <<
    "    Enable noise modelling" << std::endl <<
    "  = ";
    for (auto item : get_noises()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* extended_param:" << std::endl <<
    "    Enable extended QAOA parameters" << std::endl <<
    "  = ";
    for (auto item : get_extended_params()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* method:" << std::endl <<
    "    Method used by optimiser" << std::endl <<
    "  = ";
    for (auto item : get_methods()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* grad:" << std::endl <<
    "    Enable gradient/Jacobian calculation" << std::endl <<
    "  = ";
    for (auto item : get_grads()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* gradient_strategy:" << std::endl <<
    "    Gradient strategy used by optimiser" << std::endl <<
    "  = ";
    for (auto item : get_gradient_strategys()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* maxeval:" << std::endl <<
    "    Maximum number of evaluations by the optimiser" << std::endl <<
    "  = ";
    for (auto item : get_maxevals()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* functol:" << std::endl <<
    "    Tolerance used by the optimiser" << std::endl <<
    "  = ";
    for (auto item : get_functols()) {
      for (auto itel : item) {
        for (auto [key, val] : itel) {
          out << " " << key << ":" << val;
        }
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* optimum_energy_abstol:" << std::endl <<
    "    Expect optimum energy in the interval [optimum_energy_lowerbound, optimum_energy_lowerbound + optimum_energy_abstol]" << std::endl <<
    "  = ";
    for (auto item : get_optimum_energy_abstols()) {
      for (auto itel : item) {
        for (auto [key, val] : itel) {
          out << " " << key << ":" << val;
        }
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* optimum_energy_lowerbound:" << std::endl <<
    "    Expect optimum energy in the interval [optimum_energy_lowerbound, optimum_energy_lowerbound + optimum_energy_abstol]" << std::endl <<
    "  = ";
    for (auto item : get_optimum_energy_lowerbounds()) {
      for (auto itel : item) {
        for (auto [key, val] : itel) {
          out << " " << key << ":" << val;
        }
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* colname:" << std::endl <<
    "    Name for condition/column" << std::endl <<
    "  = ";
    for (auto item : get_colnames()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* rowname:" << std::endl <<
    "    Name for experiment/row" << std::endl <<
    "  = ";
    for (auto item : get_rownames()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* eigenstate:" << std::endl <<
    "    Bitstring of QAOA optimum state (for classical Hamiltonian)" << std::endl <<
    " = ";
    for (auto item : get_out_eigenstates()) {
      for (auto itel : item) {
        out << " " << itel;
      }
      out << std::endl;
    }
    out << std::endl << std::endl;
    //

    out << "* energy:" << std::endl <<
    "    Keys:" << std::endl <<
    "                     0: energy at optimum theta" << std::endl <<
    "     1,2,...,[maxeval]: energy trace over [maxeval] iterations" << std::endl <<
    "  = ";
    for (auto item : get_out_energys()) {
      out << std::endl << " ";
      for (auto itel : item) {
        for (auto it : itel) {
          out << " | " << it.first << ": " << it.second;
        }
        if (itel.size() > 0) {
          out << " | ";
        } else {
          out << " NA ";
        }
      }
    }
    out << std::endl << std::endl;
    //

    out << "* theta, optimum:" << std::endl <<
    "    Keys:" << std::endl <<
    "                     0: optimum theta" << std::endl <<
    "     1,2,...,[maxeval]: theta trace over [maxeval] iterations" << std::endl <<
    "  = ";
    for (auto item : get_out_thetas()) {
      out << std::endl << " ";
      for (auto itel : item) {
        for (auto it : itel) {
          out << " | " << it.first << ": " << it.second;
        }
        if (itel.size() > 0) {
          out << " | ";
        } else {
          out << " NA ";
        }
      }
    }
    out << std::endl << std::endl;
    //

    out << "* quantum_energy_calc_time:" << std::endl <<
    "    Keys:" << std::endl <<
    "                     0: estimated time, in ms, for calculating energy" << std::endl <<
    "                        on QB hardware, covering [maxeval] evaluations" << std::endl <<
    "  = ";
    for (auto item : get_out_quantum_energy_calc_times()) {
      out << std::endl << " ";
      for (auto itel : item) {
        for (auto it : itel) {
          out << " | " << it.first << ": " << it.second;
        }
        if (itel.size() > 0) {
          out << " | ";
        } else {
          out << " NA ";
        }
      }
    }
    out << std::endl << std::endl;
    //

    out << "* jacobian:" << std::endl <<
    "    Keys:" << std::endl <<
    "       0,...,(size([theta]) - 1): Jacobian at optimum theta (d_energy/d_theta)" << std::endl <<
    "  = ";
    for (auto item : get_out_jacobians()) {
      out << std::endl << " ";
      for (auto itel : item) {
        for (auto it : itel) {
          out << " | " << it.first << ": " << it.second;
        }
        if (itel.size() > 0) {
          out << " | ";
        } else {
          out << " NA ";
        }
      }
    }
    out << std::endl << std::endl;
    //

    out << "* quantum_jacobian_calc_time:" << std::endl <<
    "    Keys:" << std::endl <<
    "                     0: estimated time, in ms, for calculating the Jacobian" << std::endl <<
    "                        on QB hardware, covering [maxeval] evaluations" << std::endl <<
    "  = ";
    for (auto item : get_out_quantum_jacobian_calc_times()) {
      out << std::endl << " ";
      for (auto itel : item) {
        for (auto it : itel) {
          out << " | " << it.first << ": " << it.second;
        }
        if (itel.size() > 0) {
          out << " | ";
        } else {
          out << " NA ";
        }
      }
    }
    out << std::endl << std::endl;
    //

    out << "* classical_energy_jacobian_total_calc_time:" << std::endl <<
    "    Keys:" << std::endl <<
    "                     0: estimated time, in ms, for calculating the energy" << std::endl <<
    "                        as well as the Jacobian classically, covering 1 evaluation only" << std::endl <<
    "  = ";
    for (auto item : get_out_classical_energy_jacobian_total_calc_times()) {
      out << std::endl << " ";
      for (auto itel : item) {
        for (auto it : itel) {
          out << " | " << it.first << ": " << it.second;
        }
        if (itel.size() > 0) {
          out << " | ";
        } else {
          out << " NA ";
        }
      }
    }
    out << std::endl << std::endl;
    //

    return out.str();
  }

  int QaoaSimple::is_ii_consistent() {
    const int INVALID = -1;
    const int SINGLETON = 1;
    int N_ii = 0;

    // Use the size of hams_ to get an idea of N_ii
    if (hams_.size() >= 1) {
      if (hams_.size() > N_ii) {
        N_ii = hams_.size();
      }
    }

    // Now check consistency of the number of rows in other input arrays
    if ((N_ii = singleton_or_eqlength(thetas_, N_ii)) == INVALID) {
      std::cout << "[theta] shape is invalid" << std::endl;
      return INVALID;
    }
      if ((N_ii = singleton_or_eqlength(hams_, N_ii)) == INVALID) {
      std::cout << "[ham] shape is invalid" << std::endl;
      return INVALID;
    }
      if ((N_ii = singleton_or_eqlength(qns_, N_ii)) == INVALID) {
      std::cout << "[qn] shape is invalid" << std::endl;
      return INVALID;
    }
    if ((N_ii = singleton_or_eqlength(sns_, N_ii)) == INVALID) {
      std::cout << "[sn] shape is invalid" << std::endl;
      return INVALID;
    }
    if ((N_ii = singleton_or_eqlength(accs_, N_ii)) == INVALID) {
      std::cout << "[acc] shape is invalid" << std::endl;
      return INVALID;
    }
    if ((N_ii = singleton_or_eqlength(noises_, N_ii)) == INVALID) {
      std::cout << "[noise] shape is invalid" << std::endl;
      return INVALID;
    }
    if ((N_ii = singleton_or_eqlength(colnames_, N_ii)) == INVALID) {
      std::cout << "[colname] shape is invalid" << std::endl;
      return INVALID;
    }
    if ((N_ii = singleton_or_eqlength(rownames_, N_ii)) == INVALID) {
      std::cout << "[rowname] shape is invalid" << std::endl;
      return INVALID;
    }
    return N_ii;
  }

  int QaoaSimple::is_jj_consistent() {
    const int INVALID = -1;
    const int SINGLETON = 1;
    int N_jj = SINGLETON;

    for (auto el : qns_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[qn] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    // The remaining shapes need not be singleton
    for (auto el : thetas_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[theta] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : hams_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[theta] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : accs_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[acc] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : noises_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[noise] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : sns_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[sn] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : colnames_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[colname] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    for (auto el : rownames_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        std::cout << "[rowname] shape is invalid" << std::endl;
        return INVALID;
      }
    }
    return N_jj;
  }

  void QaoaSimple::run(const size_t &ii, const size_t &jj) {
    using namespace xacc;

    // Construct validations
    ValidatorTwoDim<std::string> colnames_valid(colnames_);

    ValidatorTwoDim<std::string> rownames_valid(rownames_);

    ValidatorTwoDim<size_t> qaoa_steps_valid(
      qaoa_steps_,
      QaoaBase::QAOA_STEPS_LOWERBOUND,
      QaoaBase::QAOA_STEPS_UPPERBOUND,
      " number of QAOA layers [qaoa_step] "
    );

    if (qaoa_steps_valid.is_data_empty()) {
      throw std::range_error("Number of QAOA layers [qaoa_step] cannot be empty");
    }

    ValidatorTwoDim<bool> extended_params_valid(
      extended_params_,
      false,
      true,
      " enable QAOA with extended parameters [extended_param] "
    );

    if (extended_params_valid.is_data_empty()) {
      throw std::range_error("Enable QAOA extended parameters [extended_param] cannot be empty");
    }

    ValidatorTwoDim<std::string> hams_valid(hams_);

    if (hams_valid.is_data_empty()) {
      throw std::range_error("A Hamiltonian [ham] must be specified");
    }

    std::map<int,double> thetas_lowerbound{{0, -1.0e9}};
    std::map<int,double> thetas_upperbound{{0, 1.0e9}};
    ValidatorTwoDim<std::map<int,double>> thetas_valid(
      thetas_,
      thetas_lowerbound,
      thetas_upperbound,
      " parameter values in ansatz [theta] "
    );

    if (thetas_valid.is_data_empty()) {
      throw std::range_error("Initial values for ansatz parameters [theta] must be specified");
    }

    ValidatorTwoDim<size_t> sns_valid(
      sns_,
      QaoaBase::SNS_LOWERBOUND,
      QaoaBase::SNS_UPPERBOUND,
      " number of shots [sn] "
    );

    if (sns_valid.is_data_empty()) {
      throw std::range_error("Number of shots [sn] cannot be empty");
    }

    ValidatorTwoDim<size_t> qns_valid(
      qns_,
      QaoaBase::QNS_LOWERBOUND,
      QaoaBase::QNS_UPPERBOUND,
      " number of qubits [qn] "
    );

    if (qns_valid.is_data_empty()) {
      throw std::range_error("Number of qubits [qn] cannot be empty");
    }

    ValidatorTwoDim<size_t> maxevals_valid(
      maxevals_,
      QaoaBase::MAXEVALS_LOWERBOUND,
      QaoaBase::MAXEVALS_UPPERBOUND,
      " number of optimiser evaluations [maxeval] "
    );

    if (maxevals_valid.is_data_empty()) {
      throw std::range_error("Number of optimiser evaluation [maxeval] cannot be empty");
    }

    ValidatorTwoDim<std::string> accs_valid(
      accs_,
      VALID_ACCS,
      " name of back-end simulator [acc] "
    );

    if (accs_valid.is_data_empty()) {
      throw std::range_error("A back-end simulator [acc] must be specified");
    }

    ValidatorTwoDim<std::string> methods_valid(
      methods_,
      VALID_OPTIMISER_METHODS,
      " optimiser algorithm [method] "
    );

    if (methods_valid.is_data_empty()) {
      throw std::range_error("An optmiser method [method] must be specified");
    }

    ValidatorTwoDim<bool> grads_valid(
      grads_,
      false,
      true,
      " enable return of gradient calculation at the optimum [grad] "
    );

    if (grads_valid.is_data_empty()) {
      throw std::range_error("Enable gradient calculation at the optimum [grad] cannot be empty");
    }

    ValidatorTwoDim<std::string> gradient_strategys_valid(
      gradient_strategys_,
      VALID_GRADIENT_STRATEGYS,
      " gradient calculation method [gradient_strategy] "
    );

    if (gradient_strategys_valid.is_data_empty()) {
      throw std::range_error("A gradient strategy [gradient_strategy] must be specified");
    }

    ValidatorTwoDim<bool> noises_valid(
      noises_,
      false,
      true,
      " enable noise modelling [noise] "
    );

    if (noises_valid.is_data_empty()) {
      throw std::range_error("Enable noise modelling [noise] cannot be empty");
    }

    // Shape consistency
    int N_ii = is_ii_consistent();
    int N_jj = is_jj_consistent();
    if (N_ii < 0) {
      throw std::range_error("Leading dimension has inconsistent length");
    }

    if (debug_qristal_) {
      std::cout
        << "Invoked run on Experiment " << ii
        << ", name: " << rownames_valid.get(ii,jj)
        << " ; Condition " << jj
        << ", name: " << colnames_valid.get(ii,jj)
        << std::endl;
    }

    int sn = sns_valid.get(ii,jj);
    if (debug_qristal_) {
      std::cout << "Number of shots [sn]: " << sn << std::endl;
    }

    int qn = qns_valid.get(ii,jj);
    if (debug_qristal_) {
      std::cout << "Number of qubits [qn]: " << qn << std::endl;
    }

    int maxeval = maxevals_valid.get(ii,jj);
    if (debug_qristal_) {
      std::cout << "Number of optimiser evaluations [maxeval]: " << maxeval << std::endl;
    }

    bool is_deterministic = false;
    if (sn == 0) {
      is_deterministic = true;
      sn = 1;
    }
    if (debug_qristal_) {
      if (is_deterministic) {
        std::cout << "Using deterministic QAOA..." << std::endl;
      }
      else {
        std::cout << "Using stochastic QAOA..." << std::endl;
      }
    }

    // theta
    std::vector<double> theta;
    qristal::map_to_vec(thetas_valid.get(ii,jj), theta);
    std::reverse(theta.begin(), theta.end());

    if (debug_qristal_) {
      std::cout << "Parameters to optimise [theta]: " << theta << std::endl;
    }

    // qaoa_step
    int qaoa_step = qaoa_steps_valid.get(ii,jj);
    if (debug_qristal_) {
      std::cout << "Number of QAOA layers [qaoa_step]: " << qaoa_step << std::endl;
    }

    // use extended parameters in QAOA
    bool extended_param = extended_params_valid.get(ii,jj);
    if (debug_qristal_) {
      std::cout << "Use extended parameters for QAOA [extended_param]: " << extended_param << std::endl;
    }

    // Hamiltonian
    std::string ham = hams_valid.get(ii,jj);
    if (debug_qristal_) {
      std::cout << "Hamiltonian [ham]: " << ham << std::endl;
    }

    // Observable from Hamiltonian string
    std::shared_ptr<Observable> observable = std::make_shared<xacc::quantum::PauliOperator>();
    observable->fromString(ham);

    // Noise
    bool noise = noises_valid.get(ii, jj);

    //auto noiseModel = std::make_shared<xacc::quantum::QuantumBrillianceNoiseModel>();
  /*
    auto noiseModel = std::make_shared<qristal::NoiseModel>();
    noiseModel->setup_48_qubits();
    noiseModel->set_m_nbQubits(qn);
    noiseModel->set_qb_connectivity_to_limit(qn);
  */

    // Accelerator : "vqe-mode"=true is non-stochastic for qpp back-end
    std::string tacc = accs_valid.get(ii, jj);
    auto acc = xacc::getAccelerator(
      tacc,
      {{"vqe-mode", is_deterministic}, {"shots", sn}}
    );

    // If noise is enabled, we force the use of aer:
    if (noise) {
      tacc = "aer";
  //    acc = xacc::getAccelerator(tacc, {{"shots", sn},
  //                                      {"noise-model", noiseModel->to_json()}});
      acc = xacc::getAccelerator(tacc, {{"shots", sn}});

      if (debug_qristal_)
        std::cout << "# Noise model: enabled - 48 qubit" << std::endl;
    } else {
      if (debug_qristal_)
        std::cout << "# Noise model: disabled" << std::endl;
    }
    if (debug_qristal_) {
      std::cout << "Accelerator [acc]: " << tacc << std::endl;
    }

    // Optimiser
    std::string tmethod = methods_valid.get(ii,jj);
    std::string nlopt_mlpack_select = "nlopt";

    // Use MLPACK if available
    if (VALID_MLPACK_OPTIMISER_METHODS.find(tmethod) == VALID_MLPACK_OPTIMISER_METHODS.end()) {
      if (debug_qristal_) {
        std::cout << "Assuming an optimiser: " << tmethod << " supported by the nlopt library..." << std::endl;
      }
    } else {
      nlopt_mlpack_select = "mlpack";
      if (debug_qristal_) {
        std::cout << "Assuming an optimiser: " << tmethod << " supported by the MLPACK library..." << std::endl;
      }
    }

    auto optim = xacc::getOptimizer(nlopt_mlpack_select);
    optim->setOptions(
      xacc::HeterogeneousMap{
        std::make_pair("initial-parameters", theta),
        std::make_pair("nlopt-maxeval", maxeval),
        std::make_pair("nlopt-optimizer", tmethod),
        std::make_pair("mlpack-max-iter", maxeval),
        std::make_pair("mlpack-optimizer", tmethod)
      }
    );

    // instantiate XACC QAOA
    std::string tgradient_strategy = gradient_strategys_valid.get(ii,jj);
    if (debug_qristal_) {
      std::cout << "Gradient strategy [gradient_strategy]: " << tgradient_strategy << std::endl;
    }

    auto qaoa = xacc::getService<Algorithm>("QAOA");
    if (extended_param) {
      qaoa->initialize({
        {"steps", qaoa_step},
        {"parameter-scheme", "Extended"},
        {"accelerator", acc},
        {"observable", observable},
        {"gradient_strategy", tgradient_strategy}, // Order of valid settings, starting from the most preferred: parameter-shift, central, forward, backward, [autodiff - does not allow u3 in ansatz]
        {"optimizer", optim}
       });
    } else { // standard QAOA
      qaoa->initialize({
        {"steps", qaoa_step},
        {"parameter-scheme", "Standard"},
        {"accelerator", acc},
        {"observable", observable},
        {"gradient_strategy", tgradient_strategy}, // Order of valid settings, starting from the most preferred: parameter-shift, central, forward, backward, [autodiff - does not allow u3 in ansatz]
        {"optimizer", optim}
      });
    }

    // Allocate some qubits and execute
    auto buffer = xacc::qalloc(qn);

    // Last validations prior to execution go here...

    // All validations passed... proceed to execute QAOA
    qaoa->execute(buffer);

    // Energy - store output
    const double opt_val = (*buffer)["opt-val"].as<double>();

    if (debug_qristal_) {
      std::cout
        << "* Trace the number of iterations which should equal nChildren():"
        << std::endl;
      std::cout
        << "* nChildren = " << buffer->nChildren()
        << std::endl;
    }

    const auto nIters = buffer->nChildren();
    int stepidx = 0;
    std::vector<double> energies;

    if (debug_qristal_) {
      std::cout << "optimal energy value: " << opt_val << std::endl;
    }

    // Element 0 of the vector is the optimum value
    energies.insert(energies.begin(), opt_val);

    for (auto &childBuff : buffer->getChildren()) {
      if (childBuff->hasExtraInfoKey("energy")) {
        double energy = (*childBuff)["energy"].as<double>();
        energies.insert(energies.end(), energy);
      }
      stepidx++;
    }

    // Save energy trace to: out_energys_ [Table2d<std::map<int,double>>]
    if (out_energys_.size() < (ii + 1)) {
      if (debug_qristal_) {
        std::cout
          << "Resizing ii: " << (ii + 1)
          << std::endl;
      }
      out_energys_.resize(ii + 1);
    }
    if ((out_energys_.at(ii)).size() < (jj + 1)) {
      if (debug_qristal_) {
        std::cout
          << "ii: " << ii
          << ", resizing jj: " << (jj + 1)
          << std::endl;
      }
      (out_energys_.at(ii)).resize(jj + 1);
    }

    std::map<int,double> res_energy;
    vec_to_map(res_energy, energies);
    (out_energys_.at(ii)).at(jj) = res_energy;

    // Element 0 is where theta at the optimum is saved to
    auto opt_params = (*buffer)["opt-params"].as<std::vector<double>>();
    std::reverse(opt_params.begin(), opt_params.end());

    if (debug_qristal_){
      std::cout << "optimal params " << opt_params << std::endl;
    }

    std::vector<double> alliters_theta;
    alliters_theta.insert(
      alliters_theta.end(),
      opt_params.begin(),
      opt_params.end()
    );

    // theta parameters - store output
    // Save theta trace to: out_thetas_ [Table2d<std::map<int,double>>]
    int step = (buffer->nChildren()) / nIters;
    stepidx = 0;
    for (auto &childBuff : buffer->getChildren()) {
      if (stepidx % step == 0) {
        if (childBuff->hasExtraInfoKey("parameters")) {
          auto param = (*childBuff)["parameters"].as<std::vector<double>>();
          std::reverse(param.begin(), param.end());
          alliters_theta.insert(alliters_theta.end(), param.begin(), param.end());
        }
      }
      stepidx++;
    }

    std::map<int,double> res_theta;
    vec_to_map(res_theta, alliters_theta);
    if (out_thetas_.size() < (ii + 1)) {
      if (debug_qristal_) {
        std::cout << "Resizing ii: " << (ii + 1) << std::endl;
      }
      out_thetas_.resize(ii + 1);
    }
    if ((out_thetas_.at(ii)).size() < (jj + 1)) {
      if (debug_qristal_) {
        std::cout
          << "ii: " << ii
          << ", resizing jj: " << (jj + 1)
          << std::endl;
      }
      (out_thetas_.at(ii)).resize(jj + 1);
    }
    (out_thetas_.at(ii)).at(jj) = res_theta;

    // At the optimal theta, evaluate the QAOA ansatz and measure
    // a few shots - return the state that corresponds to the mode of the distribution

    std::string targetCircuit;
    std::reverse(opt_params.begin(), opt_params.end());
    targetCircuit = measurement_circ(qn, qaoa_step, ham, extended_param, opt_params);

    // configure this single experiment run:
    auto qpu = acc;
    HeterogeneousMap mqbacc;
    mqbacc.insert("shots", 1024);
    qpu->updateConfiguration(mqbacc);

    // buffer settings:
    auto buffer3 = xacc::qalloc(qn);
    buffer3->setName("q");
    xacc::storeBuffer(buffer3);

    // compile the circuit
    auto xasmCompiler = xacc::getCompiler("xasm");
    auto irqft3 = xasmCompiler->compile(targetCircuit, qpu);
    std::vector<std::shared_ptr<CompositeInstruction>> placedCircuits;
    placedCircuits.push_back(irqft3->getComposite("evaled_qaoa"));

    // print out circuit depth
    for (auto& circ:placedCircuits){
      std::cout << "Ansatz depth: " << circ->depth() << std::endl;
    }

    // Print out optimal circuit when in verbose mode:
    auto qasmCompiler = xacc::getCompiler("staq");
    auto ir2 = irqft3->getComposite("evaled_qaoa");
    if (debug_qristal_) {
      std::cout
        << "* Optimal circuit:" << std::endl
        << qasmCompiler->translate(ir2) << std::endl;
    }

    // execute the experiment and get the measurements:
    std::string guessedState="";
    std::vector<std::map<std::string, int>> allresults;
    std::map<std::string, int> placed_cct_simc;
    qpu->execute(buffer3, placedCircuits[0]);
    auto results = buffer3->getMeasurementCounts();

    // find highest number of counts
    unsigned currentMax = 0;
    // map has key first value second:
    for (auto const& bitstringcount : results){
      if (bitstringcount.second > currentMax) {
        currentMax = bitstringcount.second;
        guessedState = bitstringcount.first;
      }
    }
    if (debug_qristal_) {
        std::cout
        << "Predicted eigenstate (assuming a classical Hamiltonian): " << guessedState
        << std::endl;
    }

    if (out_eigenstates_.size() < (ii + 1)) {
      if (debug_qristal_) {
        std::cout << "Resizing ii: " << (ii + 1) << std::endl;
      }
      out_eigenstates_.resize(ii + 1);
    }
    if ((out_eigenstates_.at(ii)).size() < (jj + 1)) {
      if (debug_qristal_) {
        std::cout
          << "ii: " << ii
          << ", resizing jj: " << (jj + 1)
          << std::endl;
      }
      (out_eigenstates_.at(ii)).resize(jj + 1);
    }
    (out_eigenstates_.at(ii)).at(jj) = guessedState;

    // Evaluate QAOA at optimal theta, then observe it and get back a list of
    // kernels - these are then used to estimate the quantum execution
    // time for determining the cost function and the Jacobian of the cost function

    std::shared_ptr<xacc::CompositeInstruction> kernel;
    xacc::HeterogeneousMap m;
    kernel = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<Instruction>("qaoa")
    );
    m.insert("nbQubits", qn);
    m.insert("nbSteps", qaoa_step);
    m.insert("cost-ham", observable);
    if (extended_param) {
        m.insert("parameter-scheme", "Extended");
    }
    else {
        m.insert("parameter-scheme", "Standard");
    }

    kernel->expand(m);

    auto tmp_theta = opt_params;
    std::reverse(tmp_theta.begin(), tmp_theta.end());
    auto evaled = kernel->operator()(tmp_theta);
    auto kernels = observable->observe(evaled);

    // Instantiate Profiler - save to out_quantum_energy_calc_times_
    double accum_quantum_est_ms = 0.0;
    for (auto &kel : kernels) {
      auto profile = qristal::Profiler(kel, qn);
      auto quantum_est =
          profile.get_total_initialisation_maxgate_readout_time_ms(0.0, sn);
      accum_quantum_est_ms += quantum_est[profile.KEY_TOTAL_TIME];
    }

    std::map<int,double> res_quantum_energy_calc_time{{0, accum_quantum_est_ms * nIters}};

    if (out_quantum_energy_calc_times_.size() < (ii + 1)) {
      if (debug_qristal_) {
        std::cout << "Resizing ii: " << (ii + 1) << std::endl;
      }
      out_quantum_energy_calc_times_.resize(ii + 1);
    }
    if ((out_quantum_energy_calc_times_.at(ii)).size() < (jj + 1)) {
      if (debug_qristal_) {
        std::cout
          << "ii: " << ii
          << ", resizing jj: " << (jj + 1)
          << std::endl;
      }
      (out_quantum_energy_calc_times_.at(ii)).resize(jj + 1);
    }
    (out_quantum_energy_calc_times_.at(ii)).at(jj) = res_quantum_energy_calc_time;

    // If grads is enabled, use the optimum theta (opt_params) to calculate the
    // Jacobian + the quantum estimated time to perform the Jacobian calculation
    // nIter times
    bool tgrad = grads_valid.get(ii, jj);
    if (debug_qristal_) {
      std::cout << "Gradient enabled [grad]: " << tgrad << std::endl;
    }

    if (tgrad) {
      auto strategy =
        xacc::getService<xacc::AlgorithmGradientStrategy>(tgradient_strategy);
      strategy->initialize({std::make_pair("observable", observable)});

      // At the optimum theta, generate gradient circuits based on
      // gradient_strategy
      auto bufferg = xacc::qalloc(qn);

      auto gradientInstructions =
        strategy->getGradientExecutions(kernel, opt_params);
      acc->execute(bufferg, gradientInstructions);

      // Based on gradient_strategy, post-process to return quantities of
      // interest
      std::vector<double> dx(opt_params.size());
      strategy->compute(dx, bufferg->getChildren());
      std::cout << "Jacobian #elements: " << dx.size() << std::endl;
      std::cout << "Jacobian at optimum: " << dx << std::endl;

      double accum_grad_quantum_est_ms = 0.0;
      for (auto &kel : gradientInstructions) {
        auto profile = qristal::Profiler(kel, qn);
        auto quantum_est =
          profile.get_total_initialisation_maxgate_readout_time_ms(0.0, sn);
        accum_grad_quantum_est_ms += quantum_est[profile.KEY_TOTAL_TIME];
      }

      std::map<int,double> res_quantum_jacobian_calc_time{{0, accum_grad_quantum_est_ms * nIters}};

      if (out_quantum_jacobian_calc_times_.size() < (ii + 1)) {
        if (debug_qristal_) {
          std::cout << "Resizing ii: " << (ii + 1) << std::endl;
        }
        out_quantum_jacobian_calc_times_.resize(ii + 1);
      }
      if ((out_quantum_jacobian_calc_times_.at(ii)).size() < (jj + 1)) {
        if (debug_qristal_) {
          std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
        }
        (out_quantum_jacobian_calc_times_.at(ii)).resize(jj + 1);
      }
      (out_quantum_jacobian_calc_times_.at(ii)).at(jj) = res_quantum_jacobian_calc_time;
    }
  }

}

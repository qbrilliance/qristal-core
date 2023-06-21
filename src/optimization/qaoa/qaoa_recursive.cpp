// Copyright (c) Quantum Brilliance Pty Ltd
#include "qb/core/optimization/qaoa/qaoa_recursive.hpp"
#include "qb/core/profiler.hpp"
#include "qb/core/pretranspiler.hpp"
namespace qb {
namespace op {

// n_c (threshold number of variables)
void QaoaRecursive::set_n_c(const std::size_t &in_n_c) {
  QaoaRecursive::n_cs_.clear();
  QaoaRecursive::n_cs_.push_back({in_n_c});
}
void QaoaRecursive::set_n_cs(const VectorN &in_n_cs) {
  QaoaRecursive::n_cs_ = in_n_cs;
}
const VectorN & QaoaRecursive::get_n_cs() const { return QaoaRecursive::n_cs_; }

// Start of help text
const char* QaoaRecursive::help_n_cs_ = R"(
        n_c:

        Threshold value for the number of variables. The recursion stops when the number of variables reaches n_c.

        n_cs:

        A 2d-array version of n_c.

)";

const std::string QaoaRecursive::get_summary() const {
  using namespace xacc;
  std::ostringstream out;

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

  out << "* n_c:" << std::endl << 
  "    Threshold number of variables" << std::endl << 
  "  = ";
  for (auto item : get_n_cs()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;

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

  out << "* noise:" << std::endl <<
  "    Enable QB noise model" << std::endl <<
  "  = ";
  for (auto item : get_noises()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;

  out << "* extended_param:" << std::endl <<
  "    Enable extended rQAOA parameters" << std::endl <<
  "  = ";
  for (auto item : get_extended_params()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;

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

  out << "* functol:" << std::endl <<
  "    Tolerance used by the optimiser" << std::endl <<
  "  = ";
  for (auto item : get_functols()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;

  out << "* optimum_energy_abstol:" << std::endl <<
  "    Expect optimum energy in the interval [optimum_energy_lowerbound, optimum_energy_lowerbound + optimum_energy_abstol]" << std::endl <<
  "  = ";
  for (auto item : get_optimum_energy_abstols()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;
  
  out << "* optimum_energy_lowerbound:" << std::endl <<
  "    Expect optimum energy in the interval [optimum_energy_lowerbound, optimum_energy_lowerbound + optimum_energy_abstol]" << std::endl <<
  "  = ";
  for (auto item : get_optimum_energy_lowerbounds()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out << std::endl;
  }
  out << std::endl << std::endl;

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

  out << "* eigenstate:" << std::endl <<
  "    Bitstring of rQAOA optimum state (for classical Hamiltonian)" << std::endl <<
  " = ";
  for (auto item : get_out_eigenstates()) {
    for (auto itel : item) {
      out << " " << itel;
    }
    out  << std::endl;
  }
  out << std::endl << std::endl;

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

  return out.str();
}

// measurement_circ_rqaoa allows to define which qubits to measure: i and j from array_of_indices array
std::string QaoaRecursive::measurement_circ_rqaoa(
  const int &n_qubits,
  const int &qaoa_steps,
  const std::string &H_string,
  const bool &extended_param,
  const std::vector<double> &params,
  const std::vector<int> &array_of_indices
) {
  //auto H = xacc::quantum::getObservable("pauli", H_string);
  
  // Observable from Hamiltonian string
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

  // Add measurement lines to ansatz circuit. Measure qubits i and j
  for (int num = 0; num < 2; num++) {
    circuit << "Measure(q[" << array_of_indices[num] << "]);" << std::endl;
  }
  circuit << "}" << std::endl;
  return circuit.str();
}

int QaoaRecursive::is_ii_consistent() {
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
  if ((N_ii = singleton_or_eqlength(n_cs_, N_ii)) == INVALID) {
    std::cout << "[n_c] shape is invalid" << std::endl;
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

int QaoaRecursive::is_jj_consistent() {
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
  for (auto el : hams_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[ham] shape is invalid" << std::endl;
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
  for (auto el : n_cs_) {
    if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
      std::cout << "[n_c] shape is invalid" << std::endl;
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

void QaoaRecursive::run(const size_t &ii, const size_t &jj) {
    using namespace xacc;

  // Construct validations
  ValidatorTwoDimOp<VectorString, std::string> colnames_valid(
    colnames_,
    " name of condition in columns [colname] "
  );

  ValidatorTwoDimOp<VectorString, std::string> rownames_valid(
    rownames_,
    " name of experiment in rows [rowname] "
  );

  ValidatorTwoDimOp<VectorN, size_t> qaoa_steps_valid(
    qaoa_steps_, QaoaBase::QAOA_STEPS_LOWERBOUND,
    QaoaBase::QAOA_STEPS_UPPERBOUND,
    " number of QAOA layers [qaoa_step] "
  );

  if (qaoa_steps_valid.is_data_empty()) {
    throw std::range_error("Number of QAOA layers [qaoa_step] cannot be empty");
  }

  ValidatorTwoDimOp<VectorBool, bool> extended_params_valid(
    extended_params_,
    false,
    true,
    " enable rQAOA with extended parameters [extended_param] "
  );
  if (extended_params_valid.is_data_empty()) {
    throw std::range_error("Enable rQAOA extended parameters [extended_param] cannot be empty");
  }   

  ValidatorTwoDimOp<VectorString, std::string> hams_valid(
    hams_,
    " Hamiltonian for rQAOA [ham] "
  );
  if (hams_valid.is_data_empty()) {
    throw std::range_error("A Hamiltonian [ham] must be specified");
  }

  ValidatorTwoDimOp<VectorN, size_t> sns_valid(
    sns_,
    QaoaRecursive::SNS_LOWERBOUND,
    QaoaRecursive::SNS_UPPERBOUND,
    " number of shots [sn] "
  );
  if (sns_valid.is_data_empty()) {
    throw std::range_error("Number of shots [sn] cannot be empty");
  }

  ValidatorTwoDimOp<VectorN, size_t> n_cs_valid(
    n_cs_, QaoaRecursive::N_CS_LOWERBOUND,
    QaoaRecursive::N_CS_UPPERBOUND,
    " threshold number of variables [n_c] "
  );
  if (n_cs_valid.is_data_empty()) {
    throw std::range_error("Threshold number of variables [n_c] cannot be empty");
  }

  ValidatorTwoDimOp<VectorN, size_t> qns_valid(
    qns_,
    QaoaRecursive::QNS_LOWERBOUND,
    QaoaRecursive::QNS_UPPERBOUND,
    " number of qubits [qn] "
  );
  if (qns_valid.is_data_empty()) {
    throw std::range_error("Number of qubits [qn] cannot be empty");
  }

  ValidatorTwoDimOp<VectorN, size_t> maxevals_valid(
    maxevals_,
    QaoaRecursive::MAXEVALS_LOWERBOUND,
    QaoaRecursive::MAXEVALS_UPPERBOUND,
    " number of optimiser evaluations [maxeval] "
  );
  if (maxevals_valid.is_data_empty()) {
    throw std::range_error("Number of optimiser evaluation [maxeval] cannot be empty");
  }

  ValidatorTwoDimOp<VectorString, std::string> accs_valid(
    accs_,
    VALID_ACCS,
    " name of back-end simulator [acc] "
  );
  if (accs_valid.is_data_empty()) {
    throw std::range_error("A back-end simulator [acc] must be specified");
  }

  ValidatorTwoDimOp<VectorString, std::string> methods_valid(
    methods_,
    VALID_OPTIMISER_METHODS,
    " optimiser algorithm [method] "
  );
  if (methods_valid.is_data_empty()) {
    throw std::range_error("An optmiser method [method] must be specified");
  }

  ValidatorTwoDimOp<VectorBool, bool> grads_valid(
    grads_,
    false,
    true,
    " enable return of gradient calculation at the optimum [grad] "
  );
  if (grads_valid.is_data_empty()) {
    throw std::range_error("Enable gradient calculation at the optimum [grad] cannot be empty");
  }   
  
  ValidatorTwoDimOp<VectorString, std::string> gradient_strategys_valid(
    gradient_strategys_,
    VALID_GRADIENT_STRATEGYS,
    " gradient calculation method [gradient_strategy] "
  );
  if (gradient_strategys_valid.is_data_empty()) {
    throw std::range_error("A gradient strategy [gradient_strategy] must be specified");
  }

  ValidatorTwoDimOp<VectorBool, bool> noises_valid(
    noises_,
    false,
    true,
    " enable the QB noise model [noise] "
  );
  if (noises_valid.is_data_empty()) {
    throw std::range_error("Enable the QB noise model [noise] cannot be empty");
  }  
  // Shape consistency
  int N_ii = is_ii_consistent();
  int N_jj = is_jj_consistent();
  if (N_ii < 0) {
    throw std::range_error("Leading dimension has inconsistent length");
  }

  if (debug_qbos_) {
    std::cout 
      << "Invoked run on Experiment " << ii
      << ", name: " << rownames_valid.get(ii,jj) 
      << " ; Condition " << jj 
      << ", name: " << colnames_valid.get(ii,jj) 
      << std::endl;
  }
  int sn = sns_valid.get(ii,jj);
  if (debug_qbos_) {
    std::cout << "Number of shots [sn]: " << sn << std::endl;      
  }
  int n_c = n_cs_valid.get(ii,jj);
  if (debug_qbos_) {
    std::cout << "Threshold number of variables [n_c]: " << n_c << std::endl;      
  }
  int qn = qns_valid.get(ii,jj);
  if (debug_qbos_) {
    std::cout << "Number of qubits [qn]: " << qn << std::endl;      
  } 
  int maxeval = maxevals_valid.get(ii,jj);
  if (debug_qbos_) {
    std::cout << "Number of optimiser evaluations [maxeval]: " << maxeval << std::endl;      
  }
  bool is_deterministic = false;
  if (sn == 0) {
    is_deterministic = true;
    sn = 1;
  }
  if (debug_qbos_) {
    if (is_deterministic) {
      std::cout << "Using deterministic rQAOA..." << std::endl;
    }
    else {
      std::cout << "Using stochastic rQAOA..." << std::endl;      
    }
  }
  
  // qaoa_step
  int qaoa_step = qaoa_steps_valid.get(ii,jj);
  if (debug_qbos_) {
      std::cout << "Number of QAOA layers [qaoa_step]: " << qaoa_step << std::endl;
  }

  // use extended parameters in rQAOA
  bool extended_param = extended_params_valid.get(ii,jj);
  if (debug_qbos_) {
      std::cout << "Use extended parameters for rQAOA [extended_param]: " << extended_param << std::endl;
  }

  // Hamiltonian
  std::string ham = hams_valid.get(ii,jj);
  if (debug_qbos_) {
    std::cout << "Hamiltonian [ham]: " << ham << std::endl;       
  }

  // Noise
  //bool noise = noises_valid.get(ii, jj);
  bool noise = false; // no noise model atm

  //auto noiseModel = std::make_shared<xacc::quantum::QuantumBrillianceNoiseModel>();
  //noiseModel->setup_48_qubits();

  // Accelerator : "vqe-mode"=true is non-stochastic for qpp back-end
  std::string tacc = accs_valid.get(ii, jj);
  auto acc = xacc::getAccelerator(tacc, {{"vqe-mode", is_deterministic}, {"shots", sn}});

  // If noise is enabled, we force the use of aer:
  /*
  if (noise) {
    tacc = "aer";
    acc = xacc::getAccelerator(
        tacc, {{"shots", sn}, {"noise-model", noiseModel->toJson()}});
    if (debug_qbos_)
      std::cout << "# Noise model: enabled - 48 qubit" << std::endl;
  } else {
    if (debug_qbos_)
      std::cout << "# Noise model: disabled" << std::endl;
  }
  if (debug_qbos_) {
    std::cout << "Accelerator [acc]: " << tacc << std::endl;
  }
  */

  // Optimiser
  std::string tmethod = methods_valid.get(ii,jj);
  std::string nlopt_mlpack_select = "nlopt";

  // Use MLPACK if available
  if (VALID_MLPACK_OPTIMISER_METHODS.find(tmethod) == VALID_MLPACK_OPTIMISER_METHODS.end()) {
    if (debug_qbos_) {
      std::cout << "Assuming an optimiser: " << tmethod << " supported by the nlopt library..." << std::endl;      
    }
  } else {
    nlopt_mlpack_select = "mlpack";
    if (debug_qbos_) {
      std::cout << "Assuming an optimiser: " << tmethod << " supported by the MLPACK library..." << std::endl;      
    }
  }

  auto optim = xacc::getOptimizer(nlopt_mlpack_select);
    
  // instantiate XACC rQAOA
  std::string tgradient_strategy = gradient_strategys_valid.get(ii,jj);
  if (debug_qbos_) {
    std::cout << "Gradient strategy [gradient_strategy]: " << tgradient_strategy << std::endl;      
  }

  // count how many recursions we need in accordance with a given threshold number of variables===========================================================================================================================================================
  int num_of_iterations = qn - n_c;

  if (debug_qbos_){
    std::cout << "num of iterations " << num_of_iterations << std::endl;
  }
  
  // here a single recursion starts++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  // Observable from Hamiltonian string
  std::shared_ptr<xacc::Observable> H_counter = std::make_shared<xacc::quantum::PauliOperator>();
  H_counter->fromString(ham);

  int ham_counter = 0;
  float energy_shift = 0.0;
  int initial_qn = qn;

  int z_index_history[qn][2*num_of_iterations+1];

  for (int w = 0; w < qn; w++) {
    z_index_history[w][0] = w;
  }

  for (int w = 0; w < qn; w++) {
    for (int q = 1; q < 2*num_of_iterations+1; q++){
      z_index_history[w][q] = 1000000;
    }
  }


  for (int j = 0; j < num_of_iterations; j++) {
    //noiseModel->set_m_nbQubits(qn);
    //noiseModel->set_qb_connectivity_to_limit(qn);

    H_counter->fromString(ham);

    ham_counter = 0;

    for (const auto &term : H_counter->getNonIdentitySubTerms()) {
      ham_counter++;  
    }

    if (debug_qbos_) {
      std::cout << "Iteration " << j << std::endl;
      std::cout << "Ham counter " << ham_counter << std::endl; 
    }

    // create new thetas 
    int size_theta;
    if (extended_param) { size_theta = (qn + ham_counter)*qaoa_step; }
    else { size_theta = 2*qaoa_step; }

    if (debug_qbos_) {
      std::cout << "size_theta " << size_theta << std::endl; 
    }
    std::vector<double> theta(size_theta);
    for(int l=0; l<size_theta; ++l){
      theta[l] = 0.25;          // set each element's value
    }
    std::reverse(theta.begin(), theta.end());

    if (debug_qbos_) {
      std::cout << "Parameters to optimise [theta]: " << theta << std::endl; 
      std::cout << "Current hamiltonian " << ham << std::endl; 
      std::cout << "Number of qubits " << qn << std::endl; 
    }

    optim->setOptions(
      xacc::HeterogeneousMap{
        std::make_pair("initial-parameters", theta),
        std::make_pair("nlopt-maxeval", maxeval),
        std::make_pair("nlopt-optimizer", tmethod),
        std::make_pair("mlpack-max-iter", maxeval),
        std::make_pair("mlpack-optimizer", tmethod)
      }
    );

    // Observable from Hamiltonian string
    std::shared_ptr<Observable> observable = std::make_shared<xacc::quantum::PauliOperator>();
    observable->fromString(ham);

    auto rqaoa = xacc::getService<Algorithm>("QAOA");
    if (extended_param) {
      rqaoa->initialize(
        {
          {"steps", qaoa_step},
          {"parameter-scheme", "Extended"},
          {"accelerator", acc},
          {"observable", observable},
          {"gradient_strategy", tgradient_strategy}, 
          {"optimizer", optim}
        }
      );
      // Order of valid settings, starting from the most preferred: parameter-shift, central, forward, backward, [autodiff - does not allow u3 in ansatz]
    } else {  // standard rQAOA
      rqaoa->initialize(
        {
          {"steps", qaoa_step},
          {"parameter-scheme", "Standard"},
          {"accelerator", acc},
          {"observable", observable},
          {"gradient_strategy",
          tgradient_strategy}, {"optimizer", optim}
        }
      );
    }

    // Allocate some qubits and execute
    auto buffer = xacc::qalloc(qn);

    // Last validations prior to execution go here...

    // All validations passed... proceed to execute rQAOA
    rqaoa->execute(buffer);

    // Energy - store output
    const double opt_val = (*buffer)["opt-val"].as<double>();

    if (debug_qbos_) {
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

    if (debug_qbos_) {
      std::cout << "optimal cost value: " << opt_val << std::endl;
    }

    // Element 0 of the vector is the optimum value
    energies.insert(energies.begin(), opt_val);
    
    for (auto &childBuff : buffer->getChildren()) {
      if (childBuff->hasExtraInfoKey("energy")) {
        double energy = (*childBuff)["energy"].as<double>();
        // std::reverse(param.begin(), param.end());
        energies.insert(energies.end(), energy);
      }
      stepidx++;
    }

    // Save energy trace to: out_energys_ [VectorMapND]
    if (out_energys_.size() < (ii + 1)) {
      if (debug_qbos_) {
        std::cout << "Resizing ii: " << (ii + 1) << std::endl;
      }
      out_energys_.resize(ii + 1);
    }
    if ((out_energys_.at(ii)).size() < (jj + 1)) {
      if (debug_qbos_) {
        std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
      }
      (out_energys_.at(ii)).resize(jj + 1);
    }

    ND res_energy;
    vec_to_map(res_energy, energies);
    (out_energys_.at(ii)).at(jj) = res_energy;

    // Element 0 is where theta at the optimum is saved to 
    auto opt_params = (*buffer)["opt-params"].as<std::vector<double>>();        
    std::reverse(opt_params.begin(), opt_params.end());

    if (debug_qbos_){
      std::cout << "optimal params " << opt_params << std::endl; 
    }

    std::vector<double> alliters_theta;
    alliters_theta.insert(alliters_theta.end(), opt_params.begin(), opt_params.end());
    
    // theta parameters - store output
    // Save theta trace to: out_thetas_ [VectorMapND]
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
    ND res_theta;
    vec_to_map(res_theta, alliters_theta);
    if (out_thetas_.size() < (ii + 1)) {
      if (debug_qbos_) {
        std::cout << "Resizing ii: " << (ii + 1) << std::endl;
      }
      out_thetas_.resize(ii + 1);
    }
    if ((out_thetas_.at(ii)).size() < (jj + 1)) {
      if (debug_qbos_) {
        std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
      }
      (out_thetas_.at(ii)).resize(jj + 1);
    }
    (out_thetas_.at(ii)).at(jj) = res_theta;
    

    // At the optimal theta, evaluate the rQAOA ansatz and measure
    // a few shots - return the state that corresponds to the mode of the distribution
    
    std::shared_ptr<xacc::Observable> H = std::make_shared<xacc::quantum::PauliOperator>();
    H->fromString(ham);
    
    std::vector<float> list_of_expval;
    std::vector<std::string> list_of_terms;
    std::vector<std::string> list_of_coefficients;
    std::reverse(opt_params.begin(), opt_params.end());

    for (const auto &term : H->getNonIdentitySubTerms()) {
      auto H_elem = term->toString();
      
      // create an array of indices
      int index_i = (int) H_elem[H_elem.size()-4]-48;
      int index_j = (int) H_elem[H_elem.size()-1]-48;

      int round_bracket = H_elem.find('(');
      int comma = H_elem.find(',');

      std::string H_coefficient = H_elem.substr(round_bracket+1, comma-round_bracket-1);

      std::vector<int> array_of_indices = { index_i, index_j };

      std::string targetCircuit;
      targetCircuit = measurement_circ_rqaoa(qn, qaoa_step, ham, extended_param, opt_params, array_of_indices);

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

      // Print out optimal circuit when in verbose mode:
      auto qasmCompiler = xacc::getCompiler("staq");
      auto ir2 = irqft3->getComposite("evaled_qaoa");

      // execute the experiment and get the measurements:
      std::string guessedState="";
      std::vector<std::map<std::string, int>> allresults;
      std::map<std::string, int> placed_cct_simc;
      qpu->execute(buffer3, placedCircuits[0]);
      auto results = buffer3->getMeasurementCounts();
      
      //convert measurement result to expectation value ZiZj
      float probabilities[4];

      int i = 0;
      for ( auto&[key, value]: results ){   
        probabilities[i] = float(value)/1024.0;
        i++;
      }

      float expval_zizj = probabilities[0]-probabilities[1]-probabilities[2]+probabilities[3];
      
      list_of_expval.push_back(expval_zizj);
      list_of_terms.push_back(H_elem.substr(H_elem.size()-5, H_elem.size()-1));
      list_of_coefficients.push_back(H_coefficient);
    }

    std::vector<float> list_of_abs_expval = list_of_expval;
    for(int i = 0; i < list_of_expval.size(); i++) {
      if(list_of_expval[i] < 0) list_of_abs_expval[i] *= -1; //make positive
    }

    float max_expval = *std::max_element(std::begin(list_of_abs_expval), std::end(list_of_abs_expval));
    int index_max_expval = std::distance(list_of_abs_expval.begin(), std::max_element(list_of_abs_expval.begin(), list_of_abs_expval.end()));

    if (debug_qbos_) {  
      std::cout << "list of expval " << list_of_expval << std::endl;
      std::cout << "index max abs(expval) " << index_max_expval << std::endl;
      std::cout << "list of terms " << list_of_terms << std::endl;
      std::cout << "list of coeff " << list_of_coefficients << std::endl;
    }

    // impose the constraint Zj = sgn(max expval) Zi and substitute it into the Hamiltonian

    float sign_of_expval = copysignf(1.0, list_of_expval[index_max_expval]);

    //replace Zj to sgn(max expval) Zi  in ham

    //get the operator which will be replaced in Hamiltonian
    std::string op_to_be_replaced = list_of_terms[index_max_expval].substr(0, 2);
    std::string replace_with = list_of_terms[index_max_expval].substr(3, 5);

    char a = op_to_be_replaced[1];
    int ia = a - '0';

    char b = replace_with[1];
    int ib = b - '0';

    z_index_history[ia][2*j+1] = ia; // ideally here should be something like NULL but c++ does not support this
    z_index_history[ib][2*j+1] = ib*sign_of_expval; // ideally here should be something like NULL but c++ does not support this

    if (debug_qbos_) {  
      std::cout << "to be replaced " << op_to_be_replaced << std::endl;
    }

    // find this operator in Hamiltonian and replace
    std::string new_ham;
    bool is_replaced;

    for (int i = 0; i < list_of_expval.size(); i++) {
      is_replaced = false;

      if (list_of_terms[i].find(op_to_be_replaced) != std::string::npos) {
        std::string::size_type position_in_ham = list_of_terms[i].find(op_to_be_replaced);
        list_of_terms[i].replace(position_in_ham, op_to_be_replaced.length(), replace_with);
        is_replaced = true;
      }

      if (is_replaced==true & sign_of_expval==-1){
        new_ham = new_ham + "- "+ list_of_coefficients[i] +" " + list_of_terms[i] + " ";
        if (list_of_terms[i].substr(0, 2) == list_of_terms[i].substr(3, 5)){
          energy_shift = energy_shift - std::stof(list_of_coefficients[i]);
        }
      } else {
        new_ham = new_ham + "+ "+ list_of_coefficients[i] +" " + list_of_terms[i] + " ";

        if (list_of_terms[i].substr(0, 2) == list_of_terms[i].substr(3, 5)){
          energy_shift = energy_shift + std::stof(list_of_coefficients[i]);
        }
      }
    }

    if (debug_qbos_) {
      std::cout << "new ham " << new_ham << '\n';
      std::cout << "energy shift " << energy_shift << '\n';
    }

    // analyze new_ham and change indices to reduce the number of qubits-------------------------------------------

    //boundary index
    int boundary_index = op_to_be_replaced[1]-48;

    if (debug_qbos_) {
      std::cout << "boundary_index " << boundary_index << '\n';
    }
    
    // analyze new hamiltonian: after getObservable we are sure that XACC optimized new_ham, there are no more elements with double indices etc. 
    H_counter->fromString(new_ham);

    ham_counter = 0;
    std::vector<std::string> new_list_of_terms;
    std::vector<std::string> new_list_of_coefficients;

    for (const auto &term : H_counter->getNonIdentitySubTerms()) {
      ham_counter++;  
      auto H_elem = term->toString();
      new_list_of_terms.push_back(H_elem.substr(H_elem.size()-5, H_elem.size()-1));

      int round_bracket = H_elem.find('(');
      int comma = H_elem.find(',');
      std::string H_coefficient = H_elem.substr(round_bracket+1, comma-round_bracket-1);
      new_list_of_coefficients.push_back(H_coefficient);
    }

    if (debug_qbos_) {
      std::cout << "Ham counter " << ham_counter << std::endl; 
      std::cout << "new list of terms " << new_list_of_terms << std::endl;
    }

    // collect indices from new_ham
    int ham_indices[2*ham_counter];

    for (int i = 0; i < ham_counter; i++) {
      ham_indices[2*i] = (int)new_list_of_terms[i][1];
      ham_indices[2*i+1] = (int)new_list_of_terms[i][4];

      if (ham_indices[2*i] > boundary_index+48){ 
        ham_indices[2*i] = ham_indices[2*i] - 1;
        new_list_of_terms[i][1] = ham_indices[2*i]; 
      } 

      if (ham_indices[2*i+1] > boundary_index+48){ 
        ham_indices[2*i+1] = ham_indices[2*i+1] - 1;
        new_list_of_terms[i][4] = ham_indices[2*i+1]; 
      }
    }

    if (debug_qbos_) {
      std::cout << "new_list_of_terms " << new_list_of_terms << std::endl; 
    }

    std::string new_ham2;

    for (int k = 0; k < ham_counter; k++) {
      new_ham2 = new_ham2 + "+ "+ new_list_of_coefficients[k] +" " + new_list_of_terms[k] + " ";
    }

    qn=qn-1; //reduce number of qubits

    // record index shift history
    for (int w = 0; w < initial_qn; w++) { 
      if (z_index_history[w][2*j] < boundary_index) { 
        z_index_history[w][2*(j+1)] = z_index_history[w][2*j]; 
      } else if (z_index_history[w][2*j] > boundary_index) { 
        z_index_history[w][2*(j+1)] = z_index_history[w][2*j]-1; 
      }
    }
    
    //sort ascending
    for (int w = 0; w < initial_qn-1; w++) { 
      if (z_index_history[w][2*(j+1)] > z_index_history[w+1][2*(j+1)]) { 
        int l = z_index_history[w+1][2*(j+1)];
        z_index_history[w+1][2*(j+1)] = z_index_history[w][2*(j+1)];
        z_index_history[w][2*(j+1)] = l;
      }
    }

    // make new ham
    //-----------------------------------------------------------------------------------------------------------------
  
    ham = new_ham2;
    if (debug_qbos_) {
      std::cout << "Hamiltonian after qubit reduction " << new_ham2 << std::endl; 
    }
  }

  //end of recursion ==================================================================================================
  if (debug_qbos_) {
    std::cout 
      << "-------------------------------------------end of recursion ------------------------------------------------" 
      << std::endl;
  }

  //find solution for the small system

  // theta
  H_counter->fromString(ham);

  ham_counter = 0;

  for (const auto &term : H_counter->getNonIdentitySubTerms()) {
    ham_counter++;
  }

  // create new thetas 
  int size_theta;
  if (extended_param) { 
    size_theta = (qn + ham_counter)*qaoa_step; 
  } else { 
    size_theta = 2*qaoa_step; 
  }

  if (debug_qbos_) {
      std::cout << "size_theta " << size_theta << std::endl; 
  }

  std::vector<double> theta(size_theta);
  for(int l=0; l<size_theta; ++l){
    theta[l] = 0.25;          // set each element's value
  }
  if (debug_qbos_) {
    std::cout << "Parameters to optimise [theta]: " << theta << std::endl; 
  }
  std::reverse(theta.begin(), theta.end());

  // Observable from Hamiltonian string
  std::shared_ptr<Observable> observable = std::make_shared<xacc::quantum::PauliOperator>();
  observable->fromString(ham);
  if (debug_qbos_) {
    std::cout << "final Hamiltonian " << ham << std::endl; 
  }

  // Noise
/*
  noiseModel->setup_48_qubits();
  noiseModel->set_m_nbQubits(qn);
  noiseModel->set_qb_connectivity_to_limit(qn);

  // If noise is enabled, we force the use of aer:
  if (noise) {
      tacc = "aer";
      acc = xacc::getAccelerator(tacc, {{"shots", sn}, {"noise-model", noiseModel->toJson()}});
      if (debug_qbos_)
      std::cout << "# Noise model: enabled - 48 qubit" << std::endl;
  } else {
      if (debug_qbos_)
      std::cout << "# Noise model: disabled" << std::endl;
  }
  */

  if (debug_qbos_) {
      std::cout << "Accelerator [acc]: " << tacc << std::endl;
  }

  optim->setOptions(
    xacc::HeterogeneousMap{
      std::make_pair("initial-parameters", theta),
      std::make_pair("nlopt-maxeval", maxeval),
      std::make_pair("nlopt-optimizer", tmethod),
      std::make_pair("mlpack-max-iter", maxeval),
      std::make_pair("mlpack-optimizer", tmethod)
    }
  );

  auto qaoa = xacc::getService<Algorithm>("QAOA");
  if (extended_param) {
    qaoa->initialize(
      {
        {"steps", qaoa_step},
        {"parameter-scheme", "Extended"},
        {"accelerator", acc},
        {"observable", observable},
        {"gradient_strategy", tgradient_strategy},
        {"optimizer", optim}
      }
    );
  } else {  // standard QAOA
    qaoa->initialize(
      {
        {"steps", qaoa_step},
        {"parameter-scheme", "Standard"},
        {"accelerator", acc},
        {"observable", observable},
        {"gradient_strategy", tgradient_strategy}, 
        {"optimizer", optim}
      }
    );
  }

  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(qn);

  // Last validations prior to execution go here...

  // All validations passed... proceed to execute QAOA
  qaoa->execute(buffer);

  // Energy - store output
  const double opt_val = (*buffer)["opt-val"].as<double>();

  if (debug_qbos_) {
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

  if (debug_qbos_) {
    std::cout << "optimal cost value: " << opt_val << std::endl;
    std::cout << "total shift: " << energy_shift << std::endl;
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

  // Save energy trace to: out_energys_ [VectorMapND]
  if (out_energys_.size() < (ii + 1)) {
    if (debug_qbos_) {
      std::cout << "Resizing ii: " << (ii + 1) << std::endl;
    }
    out_energys_.resize(ii + 1);
  }
  if ((out_energys_.at(ii)).size() < (jj + 1)) {
    if (debug_qbos_) {
      std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
    }
    (out_energys_.at(ii)).resize(jj + 1);
  }

  ND res_energy;
  vec_to_map(res_energy, energies);
  (out_energys_.at(ii)).at(jj) = res_energy;

  // Element 0 is where theta at the optimum is saved to 
  auto opt_params = (*buffer)["opt-params"].as<std::vector<double>>();        
  std::reverse(opt_params.begin(), opt_params.end());

  if (debug_qbos_){
    std::cout << "optimal params " << opt_params << std::endl; 
  }

  std::vector<double> alliters_theta;
  alliters_theta.insert(alliters_theta.end(), opt_params.begin(), opt_params.end());
    
  // theta parameters - store output
  // Save theta trace to: out_thetas_ [VectorMapND]
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
  ND res_theta;
  vec_to_map(res_theta, alliters_theta);
  if (out_thetas_.size() < (ii + 1)) {
    if (debug_qbos_) {
      std::cout << "Resizing ii: " << (ii + 1) << std::endl;
    }
    out_thetas_.resize(ii + 1);
  }
  if ((out_thetas_.at(ii)).size() < (jj + 1)) {
    if (debug_qbos_) {
      std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
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

  // Print out optimal circuit when in verbose mode:
  auto qasmCompiler = xacc::getCompiler("staq");
  auto ir2 = irqft3->getComposite("evaled_qaoa");

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

  if (debug_qbos_) {  
    std::cout << "    Predicted eigenstate (assuming a classical Hamiltonian): " << guessedState << std::endl;
  }

  // Analyze guessedState and reconstruct the original state according to z_index_history------------------------------------------------------------------------------------

  int reconstructed_state[initial_qn];
  std::reverse(guessedState.begin(), guessedState.end());

  // populate reconstructed state according to the guessedState

  for (int s = 0; s < initial_qn; s++ ){
    if (z_index_history[s][2*num_of_iterations] < 900000) { 
      char elem = guessedState[s];
      int ielem = elem - '0';
      reconstructed_state[z_index_history[s][2*num_of_iterations]] = ielem; 
    } 
  }

  for (int v = num_of_iterations; v > 0; v--){
    // find first changed index
    int removed_index;

    for (int s = 0; s < initial_qn; s++ ){
      if (z_index_history[s][2*v-1] < 900000) { 
        removed_index = z_index_history[s][2*v-1]; 
        break;
      } 
    } 

    // shift the indices to the right from boundary
    for (int s = initial_qn - 2 ; s >= 0; s-- ){
      if (s >= removed_index) { 
        reconstructed_state[s+1] = reconstructed_state[s];
      } 
    } 

    // find the second changed index
    int replacing_index;

    for (int s = removed_index+1; s < initial_qn; s++ ){
      if (z_index_history[s][2*v-1] < 900000) { 
        replacing_index = z_index_history[s][2*v-1]; 
        break;
      } 
    } 

    // add removed qubit in accordance to z_index_history (populate boundary index)

    if (replacing_index > 0) { 
      reconstructed_state[removed_index] = reconstructed_state[replacing_index]; 
    } else if (replacing_index < 0) { 
      if (reconstructed_state[abs(replacing_index)] == 1) { 
        reconstructed_state[removed_index] = 0; 
      } else if (reconstructed_state[abs(replacing_index)] == 0) {
        reconstructed_state[removed_index] = 1; 
      }
    }
  }

  std::reverse(reconstructed_state, reconstructed_state + initial_qn);

  if (debug_qbos_) {
    std::cout << " reconstructed state : " ;
    for (int s = 0; s < initial_qn; s++ ){
      std::cout << reconstructed_state[s] << " " ;
    }
    std::cout << "" << std::endl;
  }

  // convert reconstructed_state to string
  std::ostringstream os;
    for (int i: reconstructed_state) {
        os << i;
    }
 
  std::string reconstructed_state_str(os.str());

  //-------------------------------------------------------------------------------------------------------------------
  
  if (out_eigenstates_.size() < (ii + 1)) {
    if (debug_qbos_) {
      std::cout << "Resizing ii: " << (ii + 1) << std::endl;
    }
    out_eigenstates_.resize(ii + 1);
  }
  if ((out_eigenstates_.at(ii)).size() < (jj + 1)) {
    if (debug_qbos_) {
      std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
    }
    (out_eigenstates_.at(ii)).resize(jj + 1);
  }
  (out_eigenstates_.at(ii)).at(jj) = reconstructed_state_str;
  
  // Evaluate rQAOA at optimal theta, then observe it and get back a list of
  // kernels - these are then used to estimate the quantum execution
  // time for determining the cost function and the Jacobian of the cost function
  
  std::shared_ptr<xacc::CompositeInstruction> kernel;
  xacc::HeterogeneousMap m;
  kernel = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<Instruction>("qaoa"));
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
    auto profile = qb::Profiler(kel, qn);
    auto quantum_est = profile.get_total_initialisation_maxgate_readout_time_ms(0.0, sn);
    accum_quantum_est_ms += quantum_est[profile.KEY_TOTAL_TIME];
  }

  ND res_quantum_energy_calc_time{{0, accum_quantum_est_ms * nIters}};

  if (out_quantum_energy_calc_times_.size() < (ii + 1)) {
    if (debug_qbos_) {
      std::cout << "Resizing ii: " << (ii + 1) << std::endl;
    }
    out_quantum_energy_calc_times_.resize(ii + 1);
  }
  if ((out_quantum_energy_calc_times_.at(ii)).size() < (jj + 1)) {
    if (debug_qbos_) {
      std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
    }
    (out_quantum_energy_calc_times_.at(ii)).resize(jj + 1);
  }
  (out_quantum_energy_calc_times_.at(ii)).at(jj) = res_quantum_energy_calc_time;

  // If grads is enabled, use the optimum theta (opt_params) to calculate the
  // Jacobian + the quantum estimated time to perform the Jacobian calculation
  // nIter times
  bool tgrad = grads_valid.get(ii, jj);
  if (debug_qbos_) {
    std::cout << "Gradient enabled [grad]: " << tgrad << std::endl;
  }
  if (tgrad) {
    auto strategy = xacc::getService<xacc::AlgorithmGradientStrategy>(tgradient_strategy);
    strategy->initialize({std::make_pair("observable", observable)});

    // At the optimum theta, generate gradient circuits based on
    // gradient_strategy
    auto bufferg = xacc::qalloc(qn);

    auto gradientInstructions = strategy->getGradientExecutions(kernel, opt_params);
    acc->execute(bufferg, gradientInstructions);

    // Based on gradient_strategy, post-process to return quantities of
    // interest
    std::vector<double> dx(opt_params.size());
    strategy->compute(dx, bufferg->getChildren());
    std::cout << "Jacobian #elements: " << dx.size() << std::endl;
    std::cout << "Jacobian at optimum: " << dx << std::endl;

    double accum_grad_quantum_est_ms = 0.0;
    for (auto &kel : gradientInstructions) {
      auto profile = qb::Profiler(kel, qn);
      auto quantum_est =
          profile.get_total_initialisation_maxgate_readout_time_ms(0.0, sn);
      accum_grad_quantum_est_ms += quantum_est[profile.KEY_TOTAL_TIME];
    }

    ND res_quantum_jacobian_calc_time{{0, accum_grad_quantum_est_ms * nIters}};

    if (out_quantum_jacobian_calc_times_.size() < (ii + 1)) {
      if (debug_qbos_) {
        std::cout << "Resizing ii: " << (ii + 1) << std::endl;
      }
      out_quantum_jacobian_calc_times_.resize(ii + 1);
    }
    if ((out_quantum_jacobian_calc_times_.at(ii)).size() < (jj + 1)) {
      if (debug_qbos_) {
        std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
      }
      (out_quantum_jacobian_calc_times_.at(ii)).resize(jj + 1);
    }
    (out_quantum_jacobian_calc_times_.at(ii)).at(jj) =
        res_quantum_jacobian_calc_time;
  }
  
}

} // namespace op
} // namespace qbOS

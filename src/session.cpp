// Copyright (c) Quantum Brilliance Pty Ltd

// STL
#include <algorithm>
#include <memory>
#include <regex>
#include <random>
#include <sstream>
#include <stdexcept>

// Boost
#include <boost/dynamic_bitset.hpp>

// QB
#include "qb/core/backend.hpp"
#include "qb/core/backend_utils.hpp"
#include "qb/core/backends/qb_hardware/qb_qpu.hpp"
#include "qb/core/pretranspiler.hpp"
#include "qb/core/profiler.hpp"
#include "qb/core/session.hpp"
#include "qb/core/passes/circuit_opt_passes.hpp"
#include "qb/core/circuit_builder.hpp"

// XACC
#include "CompositeInstruction.hpp"
#include "AcceleratorBuffer.hpp"
#include "xacc.hpp"

// CUDAQ support
#ifdef WITH_CUDAQ
  #include "qb/core/cudaq/sim_pool.hpp"
  #include "qb/core/cudaq/cudaq_acc.hpp"
#endif

// dlopen
#include <dlfcn.h>

// Helper functions
namespace
{
  /// Wrapper of std::scoped_lock to allow for optional locking,
  /// e.g., doing nothing if no mutex is given.
  /// Note: std::scoped_lock is templated, i.e., std::scoped_lock<> (no op) and std::scoped_lock<std::mutex> are different types,
  /// hence cannot use the ternary operator to switch between them.
  class optional_scoped_lock {
    /// The real lock (if a mutex is given)
    std::unique_ptr<std::scoped_lock<std::mutex>> m_lock;

  public:
    optional_scoped_lock(std::mutex *mutex) {
      if (mutex) {
        /// Construct the lock
        m_lock = std::make_unique<std::scoped_lock<std::mutex>>(*mutex);
      }
    }
    ~optional_scoped_lock() = default;
  };

  // Trim from left (remove leading white spaces, e.g., spaces, tab, empty lines)
  inline std::string ltrim(const std::string &in_str,
                           const char *t = " \t\n\r\f\v")
  {
    auto s = in_str;
    s.erase(0, s.find_first_not_of(t));
    return s;
  }

  double getExpectationValueZ(std::shared_ptr<xacc::AcceleratorBuffer> buffer)
  {
    double aver = 0.0;
    auto has_even_parity = [](const std::string &x) -> bool
    {
      int c = std::count(x.begin(), x.end(), '1');
      return c%2 == 0;
    };
    if (buffer->hasExtraInfoKey("ro-fixed-exp-val-z"))
    {
      aver = mpark::get<double>(buffer->getInformation("ro-fixed-exp-val-z"));
    }
    else if (buffer->hasExtraInfoKey("exp-val-z"))
    {
      aver = mpark::get<double>(buffer->getInformation("exp-val-z"));
    }
    else
    {
      if (buffer->getMeasurementCounts().empty())
      {
        xacc::error("called getExpectationValueZ() on an AcceleratorBuffer with "
                    "no measurements!");
      }

      for (auto &kv : buffer->getMeasurementCounts())
      {
        auto par = has_even_parity(kv.first);
        auto p = buffer->computeMeasurementProbability(kv.first);
        if (!par) p = -p;
        aver += p;
      }
    }
    return aver;
  }

} // namespace


namespace qb
{
  /// Default session constructor
  session::session()
      : debug_(false), name_m{{}}, number_m{{}}, infiles_{{}},
        include_qbs_{{SDK_DIR "/include/qb/core/qblib.inc"}},
        remote_backend_database_path_{SDK_DIR "/remote_backends.yaml"},
        instrings_{{}}, irtarget_ms_{{}}, accs_{{"qpp"}},
        aer_sim_types_{}, randoms_{{}},
        xasms_{{{false}}}, quil1s_{{{false}}}, noplacements_{{{false}}},
        nooptimises_{{{true}}}, nosims_{{{false}}}, noises_{{{false}}},
        output_oqm_enableds_{{{false}}}, log_enableds_{{{false}}},
        notimings_{{{false}}}, qns_{{}}, rns_{{}}, sns_{{}}, betas_{{}},
        thetas_{{}}, 
        initial_bond_dimensions_{{}}, initial_kraus_dimensions_{{}}, max_bond_dimensions_{{}},
        max_kraus_dimensions_{{}}, svd_cutoffs_{{}}, rel_svd_cutoffs_{{}}, noise_models_{{}},
        acc_uses_lsbs_{{{}}}, acc_uses_n_bits_{{{}}}, output_amplitudes_{{}},
        out_raws_{{{}}}, out_bitstrings_{{{}}}, out_divergences_{{{}}},
        out_transpiled_circuits_{{{}}}, out_qobjs_{{{}}}, out_qbjsons_{{{}}},
        out_single_qubit_gate_qtys_{{{}}}, out_double_qubit_gate_qtys_{{{}}},
        out_total_init_maxgate_readout_times_{{{}}}, out_z_op_expects_{{{}}},
        executor_(std::make_shared<Executor>()), error_mitigations_{}, state_vec_{},
        in_get_state_vec_(false) {
    xacc::Initialize();
    xacc::setIsPyApi();
    xacc::set_verbose(debug_);
  #ifdef WITH_CUDAQ
    // Populate VALID_ACCS with additional CUDAQ backend
    for (const auto &qoda_sim :
        cudaq_sim_pool::get_instance().available_simulators()) {
      VALID_ACCS.emplace(qoda_sim);
    }
  #endif
  }

  session::session(const std::string &name) : session() {
    name_m.push_back({name});
  }
  session::session(const bool debug) : session() { debug_ = debug; }

  /**
    Desc:
    Generates random circuits of arbitrary size and form.

    Notes:
    - The user needs to study and eventually insert large enough circuit depth for
  proper random behaviour. This would allow for the appearance of all basic gates
  and reaching Porter-Thomas distribution
      ([Boixo2018] argues sub-linear scaling is enough to achieve this). Note some
  basic gates are not available through XACC framework but all can be implemented
  in terms of others gates we list here. In addition if the optimisations and
  placement features can make the actual depth larger than this fixed amount,
  however, it always atays as O(n_q).
    - We include only maximally two-operand gates.
    - Currently does not include any middle measurements and conditional
  operations.
    - Currently does not include any middle resets.
    - Inputting a fixed random seed is not yet implemented.
    - Currently we are not (weakly) conditioning any of the quantum wires to the
  classical bit values.

    Args:
    [n_q (int)] number of quantum registers -- must be equal or greater than 3
    [seed (int)] sets the random seed

    Returns:
    [RandomQcircuit (string)] generated random quantum circuit in OpenCasm format

    Raises:
    NONE
  **/
  std::string session::random_circuit(const int n_q, const int depth) {
    std::vector<std::string> one_q_ops = {"id", "u1",  "u2", "u3", "x",
                                          "y",  "z",   "h",  "s",  "sdg",
                                          "t",  "tdg", "rx", "ry", "rz"};
    std::vector<std::string> two_q_ops = {"cx",  "cy",  "cz",  "swap", "rzz",
                                          "cu1", "crz", "cu3", "ch"};
    // Note that three-operand gates are not available in XACC accelerators yet,
    // but can be considered as sequentially implemented in terms of others here.

    std::vector<std::string> one_params = {"u1",  "rx",  "ry", "rz",
                                           "rzz", "crz", "cu1"};

    std::vector<std::string> two_params = {"u2"};
    std::vector<std::string> three_params = {"u3", "cu3"};

    std::default_random_engine re{static_cast<long unsigned int>(time(0))};

    // initialising the quantum circuit
    std::stringstream RandomQcircuit;
    RandomQcircuit << "__qpu__ void QBCIRCUIT(qreg q) {" << std::endl
                   << "  OPENQASM 2.0;" << std::endl
                   << "  include \"qelib1.inc\";" << std::endl;

    RandomQcircuit << "  creg c[" << n_q << "];" << std::endl;

    for (int level = 1; level <= depth; level++) {

      std::vector<int> remaining_qubits;
      for (int i = 0; i < n_q; i++) {
        remaining_qubits.push_back(i);
      }

      while (!remaining_qubits.empty()) {

        int remaining_size = remaining_qubits.size();
        int max_possible_operands = std::min(remaining_size, 2);
        int num_operands = std::rand() % max_possible_operands + 1;

        // shuffle remaining_qubits if needed ...
        std::vector<int> operands;
        for (int j = 1; j <= num_operands; j++) {
          int this_int = choose_random(remaining_qubits);
          operands.push_back(this_int);
          remaining_qubits.erase(std::remove(remaining_qubits.begin(),
                                             remaining_qubits.end(), this_int),
                                 remaining_qubits.end());
        }

        // create operands_str ...
        std::stringstream operands_stream;
        operands_stream << " ";
        for (int i : operands) {
          operands_stream << "q[" << i << "],";
        }
        std::string operands_str = operands_stream.str();
        operands_str.pop_back();

        // update remaining_qubits with its last remaining members if needed ...
        std::string operation;
        if (num_operands == 1)
          operation = choose_random(one_q_ops);
        else if (num_operands == 2)
          operation = choose_random(two_q_ops);

        int num_angles = 0;
        if (std::find(one_params.begin(), one_params.end(), operation) !=
            one_params.end())
          num_angles = 1;
        else if (std::find(two_params.begin(), two_params.end(), operation) !=
                 two_params.end())
          num_angles = 2;
        else if (std::find(three_params.begin(), three_params.end(), operation) !=
                 three_params.end())
          num_angles = 3;

        // setting the angles randomly ...
        // we can create angles_str at the same time ...
        std::stringstream angles_stream;
        std::string angles_str;
        if (num_angles > 0) {
          angles_stream << "(";
          for (int j = 1; j <= num_angles; j++) {
            std::uniform_real_distribution<double> unif(-((j / 2) % 2) * M_PI,
                                                        M_PI);
            double angle = unif(re);
            angles_stream << angle << ",";
          }
          angles_str = angles_stream.str();
          angles_str.pop_back();
          angles_str += ")";
        }

        RandomQcircuit << "  " << operation << angles_str << operands_str << ";"
                       << std::endl;
      }
    }

    // adding the measurement part to the circuit ...
    for (int i = 0; i < n_q; i++) {
      RandomQcircuit << "  measure q[" << i << "] -> c[" << i << "];"
                     << std::endl;
    }

    // finalising RandomQcircuit string
    RandomQcircuit << "}" << std::endl;
    return RandomQcircuit.str();
  }

  /**
   * get_jensen_shannon - calculate the divergence between two discrete
   *probability distributions supported in the same space
   *
   * Input:
   *
   *     [std::map<std::string, int>]        in_q : the counts from a quantum
   *simulation.  The string key is assumed to be a BCD index for in_p
   *
   *     [std::vector<std::complex<double>>] in_p : the amplitudes for the
   *theoretical distribution of states from which in_q has been sampled.  begin()
   *corresponds to |00...0>. end() corresponds to |111...1>.  Increments in the
   *state label correspond to increments in the iterator.
   *
   * Output:
   *
   *     [double]  divergence : 0.5*(D_KL(in_p||m) + D_KL(in_q||m))
   *
   * where:
   *
   *   m = 0.5*(in_p + in_q)
   *
   *   D_KL(P||Q) = P' * (log(P)-log(Q))  : P,Q are column vectors.  Exclude all
   *elements of Q that are zero
   **/

  double session::get_jensen_shannon_divergence(const std::map<std::string, int> &in_q,
                                                  const std::map<std::string, std::complex<double>> &in_p) {
    double divergence = 0.0;
    int sum_in_q = 0;
    for (auto in_q_elem = in_q.begin(); in_q_elem != in_q.end(); in_q_elem++) {
      sum_in_q += in_q_elem->second;
    }

    std::map<std::string, int>::const_iterator in_q_elem;
    std::string n_str = in_q.begin()->first;
    unsigned int n_q = n_str.size();

    for (auto in_p_elem = in_p.begin(); in_p_elem != in_p.end(); in_p_elem++) {
      double nipe = std::norm(in_p_elem->second);
      // Search in_q for a state with the label matching in_p_elem
      std::string statelabel = in_p_elem->first;
      std::reverse(statelabel.begin(), statelabel.end());
      if (debug_) {
        std::cout << "[debug]: statelabel: " << statelabel << " , nipe: " << nipe << std::endl;
      }
      in_q_elem = in_q.find(statelabel);

      if (in_q_elem != in_q.end()) {
        double rfq = (1.0 / sum_in_q) * (in_q_elem->second);
        if (debug_) {
          std::cout << "[debug]: rfq: " << rfq << std::endl;
        }
        double m = 0.5 * (rfq + nipe);
        if ((in_q_elem->second > 0) && (nipe > 0)) {
          divergence += 0.5 * (nipe * (std::log(nipe) - std::log(m)) +
                               rfq * (std::log(rfq) - std::log(m)));
        } else if ((in_q_elem->second > 0) && (nipe == 0)) {
          divergence += 0.5 * rfq * (std::log(rfq) - std::log(m));
        } else if ((in_q_elem->second == 0) && (nipe > 0)) {
          divergence += 0.5 * nipe * (std::log(nipe) - std::log(m));
        } else {
          if (debug_) {
            std::cout
                << "Ignoring a state that has zero probability in both P and Q"
                << std::endl;
          }
        }
      } else {
        divergence +=
            0.5 * nipe * std::log(2); // divergence += 0.5*nipe*(std::log(nipe) -
                                      // std::log(m)); m = 0.5*nipe
      }
    }

    // Check for entries of in_q that are sparse (zero) for in_p
    for (auto in_q_elem = in_q.begin(); in_q_elem != in_q.end(); in_q_elem++) {
      std::string statelabel = in_q_elem->first;
      std::reverse(statelabel.begin(), statelabel.end());
      double rfq = (1.0 / sum_in_q) * (in_q_elem->second);
      auto in_p_el = in_p.find(statelabel);

      if (in_p_el == in_p.end()) {
        divergence +=
            0.5 * rfq * std::log(2); // divergence += 0.5*rfq*(std::log(rfq) -
                                     // std::log(m)); m = 0.5*rfq
      }
    }

    return divergence;
  }

  void session::get_jensen_shannon(const size_t &ii, const size_t &jj) {
    // Validations
    if (debug_) {
      std::cout << "Invoked get_jensen_shannon on circuit: " << ii
                << ", setting:" << jj << std::endl;
    }
    const int INVALID = -1;
    const int SINGLETON = 1;
    int N_ii = SINGLETON;
    int N_jj = SINGLETON;

    if ((N_ii = singleton_or_eqlength(out_bitstrings_, N_ii)) == INVALID) {
      throw std::range_error("[out_bitstrings] shape is invalid");
    }
    if ((N_ii = singleton_or_eqlength(output_amplitudes_, N_ii)) == INVALID) {
      throw std::range_error("[output_amplitudes] shape is invalid");
    }
    if ((N_ii = singleton_or_eqlength(acc_uses_lsbs_, N_ii)) == INVALID) {
      throw std::range_error("[acc_uses_lsbs] shape is invalid");
    }
    for (auto el : out_bitstrings_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[out_bitstrings] shape is invalid");
      }
    }
    for (auto el : output_amplitudes_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[output_amplitudes] shape is invalid");
      }
    }
    for (auto el : acc_uses_lsbs_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[acc_uses_lsbs] shape is invalid");
      }
    }

    if (out_divergences_.size() < (ii + 1)) {
      if (debug_) {
        std::cout << "Resizing ii: " << (ii + 1) << std::endl;
      }
      out_divergences_.resize(ii + 1);
    }
    if (out_divergences_.at(ii).size() < (jj + 1)) {
      if (debug_) {
        std::cout << "ii: " << ii << ", resizing jj: " << (jj + 1) << std::endl;
      }
      out_divergences_.at(ii).resize(jj + 1);
    }

    double jsdivergence = get_jensen_shannon_divergence(
        out_bitstrings_.at(ii).at(jj), output_amplitudes_.at(ii).at(jj));
    ND jsdivergence_nd;
    jsdivergence_nd.insert(std::make_pair(0, jsdivergence));
    out_divergences_.at(ii).at(jj) = jsdivergence_nd;
  }

  void session::get_jensen_shannon() {
    if (debug_) std::cout << "Invoked get_jensen_shannon()" << std::endl;

    // Shape consistency
    const int INVALID = -1;
    const int SINGLETON = 1;
    int N_ii = SINGLETON;
    int N_jj = SINGLETON;

    if ((N_ii = singleton_or_eqlength(out_bitstrings_, N_ii)) == INVALID) {
      throw std::range_error("[out_bitstrings] shape is invalid");
    }
    if ((N_ii = singleton_or_eqlength(output_amplitudes_, N_ii)) == INVALID) {
      throw std::range_error("[output_amplitudes] shape is invalid");
    }
    for (auto el : out_bitstrings_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[out_bitstrings] shape is invalid");
      }
    }
    for (auto el : output_amplitudes_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[output_amplitudes] shape is invalid");
      }
    }

    if (N_ii < 0) {
      throw std::range_error("Leading dimension has inconsistent length");
    }

    if (N_jj < 0) {
      throw std::range_error("Second dimension has inconsistent length");
    }
    if (debug_) {
      std::cout << "N_ii: " << N_ii << std::endl;
    }
    if (debug_) {
      std::cout << "N_jj: " << N_jj << std::endl;
    }

    for (int ii = 0; ii < N_ii; ii++) {
      for (int jj = 0; jj < N_jj; jj++) {
        get_jensen_shannon(ii, jj);
      }
    }
  }

  std::string session::aer_circuit_transpiler(std::string &circuit) {

    std::string NEWcircuit = circuit;
    std::istringstream stream{circuit};
    std::string current_line;

    // Repeat till end is reached
    while (std::getline(stream, current_line)) {
      size_t pos = NEWcircuit.find(current_line);

      size_t pos_cy = current_line.find("cy q[");
      size_t pos_rzz = current_line.find("rzz(");

      // size_t pos_q = current_line.find("q");
      size_t open1 = current_line.find("[");
      size_t open2 = current_line.find("[", open1 + 1);
      size_t close1 = current_line.find("]");
      size_t close2 = current_line.find("]", close1 + 1);
      size_t par1 = current_line.find("(");
      size_t par2 = current_line.find(")");
      // size_t com1 = current_line.find(",");
      // size_t com2 = current_line.find(",", com1 + 1);
      // size_t com3 = current_line.find(",", com2 + 1);

      if (pos_cy != std::string::npos) {
        // find the qubit numbering
        std::string q_i = current_line.substr(open1 + 1, close1 - open1 - 1);
        std::string q_f = current_line.substr(open2 + 1, close2 - open2 - 1);
        // building up the replacement circuit
        std::stringstream repl_circuit;
        repl_circuit << "  sdg q[" << q_f << "];" << std::endl;
        repl_circuit << "  cx q[" << q_i << "],q[" << q_f << "];" << std::endl;
        repl_circuit << "  s q[" << q_f << "];";
        // Replace this occurrence
        NEWcircuit.replace(pos, current_line.size(), repl_circuit.str());
      } else if (pos_rzz != std::string::npos) {
        // std::string angle = current_line.substr(pos_rzz+4,8);
        std::string angle = current_line.substr(par1 + 1, par2 - par1 - 1);
        // std::string q_i = current_line.substr(pos_q+2,1);
        std::string q_i = current_line.substr(open1 + 1, close1 - open1 - 1);
        std::string q_f = current_line.substr(open2 + 1, close2 - open2 - 1);
        // building up the replacement circuit
        std::stringstream repl_circuit;
        repl_circuit << "  cx q[" << q_i << "],q[" << q_f << "];" << std::endl;
        repl_circuit << "  rz(" << angle << ") q[" << q_f << "];" << std::endl;
        repl_circuit << "  cx q[" << q_i << "],q[" << q_f << "];";
        // Replace this occurrence
        NEWcircuit.replace(pos, current_line.size(), repl_circuit.str());
      }
    }

    return NEWcircuit;
  }

  /// Retrieve and validate run configurations for index pair (ii, jj) using the table index convention.
  run_i_j_config session::get_run_config(size_t ii, size_t jj)
  {
    run_i_j_config config;
    /// Number of shots
    {
      ValidatorTwoDim<VectorN, size_t> sns_valid(sns_, session::SNS_LOWERBOUND, session::SNS_UPPERBOUND,
                                                 " number of shots [sn] ");
      if (sns_valid.is_data_empty())
      {
        throw std::range_error("Number of shots [sn] cannot be empty");
      }

      config.num_shots = sns_valid.get(ii, jj);
    }

    /// Number of qubits
    {
      ValidatorTwoDim<VectorN, size_t> qns_valid(qns_, session::QNS_LOWERBOUND, session::QNS_UPPERBOUND,
                                                 " number of qubits [qn] ");
      if (qns_valid.is_data_empty())
      {
        throw std::range_error("Number of qubits [qn] cannot be empty");
      }
      config.num_qubits = qns_valid.get(ii, jj);
    }

    /// Number of repetitions
    {
      ValidatorTwoDim<VectorN, size_t> rns_valid(rns_, session::RNS_LOWERBOUND, session::RNS_UPPERBOUND,
                                                 " number of repetitions [rn] ");
      if (rns_valid.is_data_empty())
      {
        throw std::range_error("Number of repetitions [rn] cannot be empty");
      }
      config.num_repetitions = rns_valid.get(ii, jj);
    }

    /// Enable output transpilation and resource estimation
    {
      ValidatorTwoDim<VectorBool, bool> output_oqm_enableds_valid(
          output_oqm_enableds_, false, true, " enable output of transpiled circuit [output_oqm_enabled] ");
      if (output_oqm_enableds_valid.is_data_empty())
      {
        throw std::range_error("Enable output of transpiled circuit "
                               "[output_oqm_enabled] cannot be empty");
      }
      config.oqm_enabled = output_oqm_enableds_valid.get(ii, jj);
    }

    /// Name of the accelerator
    {
      ValidatorTwoDim<VectorString, std::string> accs_valid(accs_, VALID_ACCS, " name of back-end simulator [acc] ");
      if (accs_valid.is_data_empty())
      {
        throw std::range_error("A back-end simulator [acc] must be specified");
      }
      config.acc_name = accs_valid.get(ii, jj);
    }

    /// QB custom OpenQASM include file
    {
      ValidatorTwoDim<VectorString, std::string> include_qbs_valid(include_qbs_);
      if (include_qbs_valid.is_data_empty())
      {
        throw std::range_error("A file for custom OpenQASM gates [include_qb] must be specified");
      }
      config.openqasm_qb_include_filepath = include_qbs_valid.get(ii, jj);
    }

    /// Input source string type (OpenQASM/XASM/Quil)
    {
      // Default is OpenQASM
      config.source_type = source_string_type::OpenQASM;
      ValidatorTwoDim<VectorBool, bool> xasms_valid(xasms_, false, true, " interpret circuit in XASM format [xasm] ");
      if (xasms_valid.is_data_empty())
      {
        throw std::range_error("Flag for XASM [xasm] cannot be empty");
      }

      ValidatorTwoDim<VectorBool, bool> quil1s_valid(quil1s_, false, true,
                                                     " interpret circuit in Quil v1 format [quil1] ");
      if (quil1s_valid.is_data_empty())
      {
        throw std::range_error("Flag for Quil v1 [quil1] cannot be empty");
      }

      const bool is_xasm = xasms_valid.get(ii, jj);
      const bool is_quil = quil1s_valid.get(ii, jj);
      // Users shouldn't set both (invalid)
      if (is_xasm && is_quil)
      {
        throw std::logic_error("XASM and Quil cannot be enabled at the same time.");
      }
      if (is_xasm)
      {
        config.source_type = source_string_type::XASM;
      }
      if (is_quil)
      {
        config.source_type = source_string_type::Quil;
      }
    }

    /// Disable circuit placement flag
    {
      ValidatorTwoDim<VectorBool, bool> noplacements_valid(noplacements_, false, true,
                                                           " disable placement mapping [noplacement] ");
      if (noplacements_valid.is_data_empty())
      {
        throw std::range_error("Flag for disabling placement mapping [noplacement] cannot be empty");
      }
      config.no_placement = noplacements_valid.get(ii, jj);
    }

    /// Disable circuit optimisation flag
    {
      ValidatorTwoDim<VectorBool, bool> nooptimises_valid(nooptimises_, false, true,
                                                          " disable circuit optimiser [nooptimise] ");
      if (nooptimises_valid.is_data_empty())
      {
        throw std::range_error("Flag for disabling circuit optimiser [nooptimise] cannot be empty");
      }
      config.no_optimise = nooptimises_valid.get(ii, jj);
    }

    /// Skip simulation flag
    {
      ValidatorTwoDim<VectorBool, bool> nosims_valid(nosims_, false, true, " disable circuit simulator [nosim] ");
      if (nosims_valid.is_data_empty())
      {
        throw std::range_error("Flag for disabling circuit simulator [nosim] cannot be empty");
      }
      config.no_sim = nosims_valid.get(ii, jj);
    }

    /// Enable/disable noise flag
    {
      ValidatorTwoDim<VectorBool, bool> noises_valid(noises_, false, true, " enable the QB noise model [noise] ");
      if (noises_valid.is_data_empty())
      {
        throw std::range_error("Enable the QB noise model [noise] cannot be empty");
      }
      config.noise = noises_valid.get(ii, jj);
    }

    /// QB tensor network simulators initial bond dimension
    {
      ValidatorTwoDim<VectorN, size_t> initial_bond_dimensions_valid(
          initial_bond_dimensions_, session::INITIAL_BOND_DIMENSION_LOWERBOUND, session::INITIAL_BOND_DIMENSION_UPPERBOUND,
          " Initial bond dimension [initial-bond-dimension] ");
      if (initial_bond_dimensions_valid.is_data_empty())
      {
        throw std::range_error("Initial bond dimension [initial-bond-dimension] cannot be empty");
      }
      config.initial_bond_tnqvm = initial_bond_dimensions_valid.get(ii, jj);
    }

    /// QB purification simulator initial kraus dimension
    {
      ValidatorTwoDim<VectorN, size_t> initial_kraus_dimensions_valid(
          initial_kraus_dimensions_, session::INITIAL_KRAUS_DIMENSION_LOWERBOUND, session::INITIAL_KRAUS_DIMENSION_UPPERBOUND,
          " Initial kraus dimension [initial-kraus-dimension] ");
      if (initial_kraus_dimensions_valid.is_data_empty())
      {
        throw std::range_error("Initial kraus dimension [initial-kraus-dimension] cannot be empty");
      }
      config.initial_kraus_tnqvm = initial_kraus_dimensions_valid.get(ii, jj);
    }

    /// TNQVM-MPS and QB tensor network simulators max bond dimension
    {
      ValidatorTwoDim<VectorN, size_t> max_bond_dimensions_valid(
          max_bond_dimensions_, session::MAX_BOND_DIMENSION_LOWERBOUND, session::MAX_BOND_DIMENSION_UPPERBOUND,
          " Maximum bond dimension [max-bond-dimension] ");
      if (max_bond_dimensions_valid.is_data_empty())
      {
        throw std::range_error("Maximum bond dimension [max-bond-dimension] cannot be empty");
      }
      config.max_bond_tnqvm = max_bond_dimensions_valid.get(ii, jj);
    }

    /// QB purification simulator max kraus dimension
    {
      ValidatorTwoDim<VectorN, size_t> max_kraus_dimensions_valid(
          max_kraus_dimensions_, session::MAX_KRAUS_DIMENSION_LOWERBOUND, session::MAX_KRAUS_DIMENSION_UPPERBOUND,
          " Maximum kraus dimension [max-kraus-dimension] ");
      if (max_kraus_dimensions_valid.is_data_empty())
      {
        throw std::range_error("Maximum kraus dimension [max-kraus-dimension] cannot be empty");
      }
      config.max_kraus_tnqvm = max_kraus_dimensions_valid.get(ii, jj);
    }

    /// TNQVM-MPS and QB tensor network SVD cutoff limit
    {
      ND svd_lowerbound{{0, -1.0e9}}; // This limit is currently ignored
      ND svd_upperbound{{0, 1.0e9}};  // This limit is currently ignored
      ValidatorTwoDim<VectorMapND, ND> svd_cutoffs_valid(svd_cutoffs_, svd_lowerbound, svd_upperbound,
                                                         " MPS SVD cutoff [svd-cutoff] ");

      if (svd_cutoffs_valid.is_data_empty())
      {
        throw std::range_error("MPS SVD cutoff [svd-cutoff] cannot be empty");
      }
      config.svd_cutoff_tnqvm = svd_cutoffs_valid.get(ii, jj).at(0);
    }

    /// TNQVM-MPS and QB tensor network relative SVD cutoff limit
    {
      ND rel_svd_lowerbound{{0, -1.0e9}}; // This limit is currently ignored
      ND rel_svd_upperbound{{0, 1.0e9}};  // This limit is currently ignored
      ValidatorTwoDim<VectorMapND, ND> rel_svd_cutoffs_valid(rel_svd_cutoffs_, rel_svd_lowerbound, rel_svd_upperbound,
                                                         " MPS relative SVD cutoff [svd-cutoff] ");

      if (rel_svd_cutoffs_valid.is_data_empty())
      {
        throw std::range_error("MPS relative SVD cutoff [rel-svd-cutoff] cannot be empty");
      }
      config.rel_svd_cutoff_tnqvm = rel_svd_cutoffs_valid.get(ii, jj).at(0);
    }

    /// Choice of noise model
    {
      if (noise_models_[0].empty())
      {
        config.noise_model = NoiseModel("default", config.num_qubits);
      }
      else
      {
        ValidatorTwoDim<std::vector<std::vector<NoiseModel>>, NoiseModel> noise_models_valid(noise_models_);
        config.noise_model = noise_models_valid.get(ii, jj);
      }
    }

    /// Choice of noise mitigation
    {
      // There were error mitigation settings
      if (!error_mitigations_.empty()) {
        ValidatorTwoDim<VectorString, std::string> error_mitigations_valid(
            error_mitigations_, VALID_ERROR_MITIGATIONS,
            " name of error mitigation module [error_mitigation] ");
        config.noise_mitigation = error_mitigations_valid.get(ii, jj);
      }
    }

    /// User-provided random seed
    {
      if (!seeds_.empty()) {
        ValidatorTwoDim<VectorN, size_t> seeds_valid(
            seeds_, 0, std::numeric_limits<int>::max(), " random seed [seed] ");
        config.simulator_seed = seeds_valid.get(ii, jj);
      }
    }

    /// AER simulator type
    {
      if (!aer_sim_types_.empty()) {
        ValidatorTwoDim<VectorString, std::string> aer_sim_types_valid(
            aer_sim_types_, VALID_AER_SIM_TYPES,
            " name of AER simulation method [aer_sim_type] ");
        config.aer_sim_type = aer_sim_types_valid.get(ii, jj);
      }
    }

    return config;
  }

  /// Ensure that all result tables are resized/expanded to accommodate (ii, jj) experiment index.
  void session::ensure_results_table_size(size_t ii, size_t jj)
  {
    // Helper to resize the rows and columns (as nested vectors) to make sure it can accommodate (i, j) index
    const auto resize_for_i_j = [=](auto& table_to_resize, const std::string& table_name)
    {
      if (table_to_resize.size() < (ii + 1))
      {
        if (debug_)
        {
          std::cout << "# " << table_name << " - Resizing ii: " << (ii + 1) << "\n";
        }
        table_to_resize.resize(ii + 1);
      }
      if (table_to_resize.at(ii).size() < (jj + 1))
      {
        if (debug_)
        {
          std::cout << "# " << table_name << " - ii: " << ii << ", resizing jj: " << (jj + 1) << "\n";
        }
        table_to_resize.at(ii).resize(jj + 1);
      }
    };

    // Resize all tables:
    resize_for_i_j(acc_uses_lsbs_, "acc_uses_lsbs_");
    resize_for_i_j(acc_uses_n_bits_, "acc_uses_n_bits_");
    resize_for_i_j(output_amplitudes_, "output_amplitudes_");
    resize_for_i_j(out_raws_, "out_raws_");
    resize_for_i_j(out_bitstrings_, "out_bitstrings_");
    resize_for_i_j(out_divergences_, "out_divergences_");
    resize_for_i_j(out_transpiled_circuits_, "out_transpiled_circuits_");
    resize_for_i_j(out_qobjs_, "out_qobjs_");
    resize_for_i_j(out_qbjsons_, "out_qbjsons_");
    resize_for_i_j(out_single_qubit_gate_qtys_, "out_single_qubit_gate_qtys_");
    resize_for_i_j(out_double_qubit_gate_qtys_, "out_double_qubit_gate_qtys_");
    resize_for_i_j(out_total_init_maxgate_readout_times_, "out_total_init_maxgate_readout_times_");
    resize_for_i_j(out_z_op_expects_, "out_z_op_expects_");
  }

  /// Retrieve the target circuit string for (i, j) task
  std::string session::get_target_circuit_qasm_string(size_t ii, size_t jj, const run_i_j_config& run_config) {
   std::string target_circuit;
   const auto file_or_string_or_random_or_ir =
        validate_infiles_instrings_randoms_irtarget_ms_nonempty(ii, jj);
    // Invalid input
    if (file_or_string_or_random_or_ir == session::circuit_input_types::INVALID) {
      throw std::invalid_argument("Please check your settings again.");
    }

    if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_INFILE) {
        // File input: load from file
        std::ifstream tifs((infiles_[ii])[0]);
        if (tifs.is_open()) {
          std::string outbuf(std::istreambuf_iterator<char>(tifs), {});
          if (ltrim(outbuf).rfind("OPENQASM", 0) == 0) {
            // Check for raw OpenQASM string.
            // Source string starts with "OPENQASM" (not already being wrapped in
            // __qpu__)
            target_circuit = convertRawOpenQasmToQBKernel(outbuf);
          } else {
            target_circuit = outbuf;
          }
        }
    } else if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_INSTRING_QPU) {
      // String input
      target_circuit = (instrings_[ii])[0];
    } else if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_RANDOM) {
      // Random input: generate random circuit
      target_circuit = random_circuit(run_config.num_qubits, (randoms_[ii])[0]);
      {
        if (instrings_.size() < (ii + 1))
        {
          if (debug_)
          {
            std::cout << "Resizing ii: " << (ii + 1) << std::endl;
          }
          instrings_.resize(ii + 1);
        }
        if (debug_)
        {
          std::cout << "Recording the random circuit to instring" << std::endl;
        }
        if (instrings_.at(ii).size() == 0)
        {
          instrings_.at(ii).resize(1);
        }

        instrings_.at(ii).at(0) += "\n# Random circuit created:\n\n";
        instrings_.at(ii).at(0) += target_circuit;
      }
    } else if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_IR) {
      if (debug_) {
        std::cout << "Using a directly created XACC IR" << std::endl;
      }
    } else {
      throw std::invalid_argument("This circuit type is not implemented");
    }

    if (file_or_string_or_random_or_ir != session::circuit_input_types::VALID_IR) {
      // Pre-transpile
      // Note: does not apply to XASM and Quil1 input formats.
      if (debug_) {
        std::cout << "[debug]: Circuit before pre-transpile:" << std::endl
                  << target_circuit << std::endl;
      }

      const std::string acc_pre = run_config.acc_name;
      const bool xasm     = (run_config.source_type == source_string_type::XASM);
      const bool quil1    = (run_config.source_type == source_string_type::Quil);
      if (!xasm && !quil1 && (acc_pre.compare("tnqvm") == 0)) {
        // target_circuit = exatn_circuit_transpiler(target_circuit);
        if (debug_) {
          std::cout << "[debug]: No pre-transpiling will be performed:"
                    << std::endl
                    << target_circuit << std::endl;
        }
      } else if (!xasm && !quil1 && (acc_pre.compare("aer") == 0)) {
        target_circuit = aer_circuit_transpiler(target_circuit);
        if (debug_) {
          std::cout << "[debug]: Circuit after aer pre-transpile:" << std::endl
                    << target_circuit << std::endl;
        }
      } else {
        if (debug_) {
          std::cout << "[debug]: No pre-transpiling will be performed"
                    << std::endl;
        }
      }

      // QB multi-control gates
      // Note: Does not apply to XASM and Quil1 formats
      if (!xasm && !quil1) {
        if (debug_) {
          std::cout << "[debug]: Start to insert QB specific gates"
                    << std::endl;
        }
        Pretranspile qbgpre = Pretranspile("QB specific gates inserted");
        qbgpre.add_n_control_gates(target_circuit);
        if (debug_) {
          std::cout << "[debug]: Circuit after inserting QB specific gates:"
                    << std::endl;
          std::cout << target_circuit << std::endl;
        }
      }

      // Replace QBTHETA_n with theta[n]
      // Note: Does not apply to XASM and Quil1 formats
      if (!xasm && !quil1 && (validate_thetas_option() >= 0)) {
        ND lowerbound{{0, -1.0e9}}; // Currently this limit is ignored
        ND upperbound{{0, 1.0e9}};  // Currently this limit is ignored

        ValidatorTwoDim<VectorMapND, ND> thetas_valid(
            thetas_, lowerbound, upperbound,
            " values for circuit parameters QBTHETA_n ");
        Pretranspile tpre =
            Pretranspile("Substituting QBTHETA_n with theta[n]");
        ND theta = thetas_valid.get(ii, jj);
        std::vector<double> qbtheta;
        if (!theta.empty()) {
          map_to_vec<ND, std::vector<double>>(theta, qbtheta);
        }
        for (size_t j = 0; j < qbtheta.size(); j++) {
          std::stringstream starget;
          std::stringstream sval;

          starget << "QBTHETA_" << j;
          sval << qbtheta.at(j);
          if (debug_) {
            std::cout << starget.str() << " = " << sval.str() << std::endl;
          }
          tpre.set_parameter(starget.str(), sval.str());
        }
        tpre.run(target_circuit);
      }

      // Insert include file for QB: include_qb
      if (!xasm && !quil1) {
        std::string incqb = run_config.openqasm_qb_include_filepath;
        if (debug_)
          std::cout << "[debug]: Include file for QB: " << incqb << std::endl;
        std::ifstream tifs(incqb);
        if (tifs.is_open()) {
          std::string target_includeqb(std::istreambuf_iterator<char>(tifs),
                                        {});
          std::stringstream anchor_second;
          anchor_second << "include \"qelib1.inc\";" << std::endl
                        << target_includeqb;
          target_circuit = std::regex_replace(
              target_circuit, std::regex("include \"qelib1.inc\";"),
              anchor_second.str());
          if (debug_)
            std::cout << "[debug]: Circuit after custom include file for QB:"
                      << std::endl
                      << target_circuit << std::endl;
        } else {
          if (debug_)
            std::cout
                << "[debug]: Could not find the QB custom include file named: "
                << incqb << std::endl;
        }
      }
    }
    return target_circuit;
  }

  void session::execute_on_simulator(
      std::shared_ptr<xacc::Accelerator> qpu,
      std::shared_ptr<xacc::AcceleratorBuffer> buffer_b,
      std::vector<std::shared_ptr<xacc::CompositeInstruction>> &circuits,
      const run_i_j_config &run_config) {
    try {
      // There were error mitigation settings
      if (!run_config.noise_mitigation.empty()) {
        if (run_config.noise) {
          auto noise_mitigated_acc = [&]() {
            if (run_config.noise_mitigation == "rich-extrap") {
              // Noise scaling factors that we use
              const std::vector<int> noise_scalings{1, 3, 5};
              return xacc::getAcceleratorDecorator(
                  run_config.noise_mitigation, qpu,
                  {{"scale-factors", noise_scalings}});
            }

            return xacc::getAcceleratorDecorator(run_config.noise_mitigation, qpu);
          }();
          if (noise_mitigated_acc) {
            qpu = noise_mitigated_acc;
          } else {
            std::cout << "Noise mitigation module '"
                      << run_config.noise_mitigation
                      << "' was not properly initialized. Please check "
                         "your installation.\n";
          }
        } else {
          // Noise was disabled, nothing to do
          if (debug_) {
            std::cout
                << "# Noise was set to False. Error mitigation is skipped."
                << std::endl;
          }
        }
      }

      qpu->execute(buffer_b, circuits.at(0));
    } catch (...) {
      throw std::invalid_argument(
          "The simulation of your input circuit failed");
    }
  }
  
  /// Get the simulator based on `run_i_j_config`
  std::shared_ptr<xacc::Accelerator> session::get_sim_qpu(bool execute_on_hardware, const run_i_j_config &run_config)
  {
    // If a hardware accelerator was selected, we set acc = "tnqvm"
    std::string acc = (execute_on_hardware ? "tnqvm" : run_config.acc_name);

    bool noises = run_config.noise;
    int max_bond_dimension = run_config.max_bond_tnqvm;
    int max_kraus_dimension = run_config.max_kraus_tnqvm;
    int initial_bond_dimension = run_config.initial_bond_tnqvm;
    int initial_kraus_dimension = run_config.initial_kraus_tnqvm;
    double svd_cutoff = run_config.svd_cutoff_tnqvm;
    double rel_svd_cutoff = run_config.rel_svd_cutoff_tnqvm;
    // Optional random seed: randomized by default.
    int random_seed = []() {
      static std::random_device dev;
      static std::mt19937 rng(dev());
      static std::uniform_int_distribution<std::mt19937::result_type> dist(
          0, std::numeric_limits<int>::max());
      return dist(rng);
    }();
    // User-provided random seed
    if (run_config.simulator_seed.has_value()) {
      random_seed = run_config.simulator_seed.value();
      if (debug_) {
        std::cout << "# Seed value: " << random_seed << std::endl;
      }
    }
#ifdef WITH_CUDAQ
    // If a CUDAQ backend sim was requested, returns its xacc::Accelerator
    // wrapper.
    if (xacc::container::contains(
            cudaq_sim_pool::get_instance().available_simulators(), acc)) {
      if (debug_) {
        std::cout << "# Using CUDA Quantum Simulator backend: " << acc
                  << std::endl;
      }
      return std::make_shared<qb::cudaq_acc>(acc);
    }
#endif

    // Load emulator if emulator tensor network backends are selected.
    if (acc == "qb-mps" || acc == "qb-purification" || acc == "qb-mpdo") {
      static const char *EMULATOR_NOISE_MODEL_LIB_NAME = "libqbemulator.so";
      void *handle = dlopen(EMULATOR_NOISE_MODEL_LIB_NAME, RTLD_LOCAL | RTLD_LAZY);

      if (handle == NULL) {
        std::cout << "The accelerator you are searching for may be available in the Qristal Emulator plugin. Please see https://quantumbrilliance.com/quantum-brilliance-emulator." << std::endl;
      }
    }

    auto qpu = xacc::getAccelerator(acc, {{"seed", random_seed}});
    if (acc.compare("tnqvm") == 0) {
      xacc::set_verbose(false);
      qpu = xacc::getAccelerator(
          "tnqvm", {
                       std::make_pair("tnqvm-visitor", "exatn-mps"),
                       std::make_pair("max-bond-dim", max_bond_dimension),
                       std::make_pair("svd-cutoff", svd_cutoff)
                   });
    } else if (acc.compare("aer") == 0) {
      xacc::HeterogeneousMap aer_options{{"seed", random_seed}};

      // Omit shots if the state vector is requested. This triggers Xacc AER to use
      // the statevector simulation type instead of qasm simulation type. The former
      // then populates its ExecutionInfo::WaveFuncKey.
      if (!in_get_state_vec_) {
        aer_options.insert("shots", run_config.num_shots);
      } else if (in_get_state_vec_ == true && run_config.num_shots > 0) {
        std::cout << "Warning: Requesting AER state vector will ignore shot sampling!\n";
      }

      if (!run_config.aer_sim_type.empty()) {
        aer_options.insert("sim-type", run_config.aer_sim_type);
        if (debug_)
          std::cout << "# Using AER simulation method: " << run_config.aer_sim_type
                    << std::endl;
      } else {
        if (debug_)
          std::cout << "# Using default AER simulation method." << std::endl;
      }
      if (run_config.noise) {
        aer_options.insert("noise-model", run_config.noise_model.to_json());
        aer_options.insert("qobj-compiler",
                           run_config.noise_model.get_qobj_compiler());
        if (debug_)
          std::cout << "# Noise model: enabled" << std::endl;
      } else {
        if (debug_)
          std::cout << "# Noise model: disabled" << std::endl;
      }
      // Get AER and initialize it with proper settings.
      qpu = xacc::getAccelerator(acc, aer_options);
    } else if (acc.compare("qb-lambda") == 0) {
      std::string lambda_url = "ec2-3-26-79-252.ap-southeast-2.compute."
                               "amazonaws.com"; // Default AWS reverse proxy
                                                // server for the QB Lambda
                                                // workstation
      //"10.10.8.50:4000"; // internal address
      // TODO add checking for existence of url key
      if (remote_backend_database_["qb-lambda"]) 
      {
        lambda_url = remote_backend_database_["qb-lambda"]["url"].as<std::string>();
        if (debug_) std::cout << "Execute on Lambda workstation @ " << lambda_url << std::endl;
      }

      if (noises) {
        qpu = xacc::getAccelerator(
            "qb-lambda", {{"device", "GPU"},
                          {"url", lambda_url},
                          {"noise-model", run_config.noise_model.to_json()}});
        if (debug_) {
          std::cout << "# Noise model: enabled - 48 qubit" << std::endl;
        }
      } else {
        qpu = xacc::getAccelerator("qb-lambda",
                                   {{"device", "GPU"}, {"url", lambda_url}});
        if (debug_) {
          std::cout << "# Noise model: disabled" << std::endl;
        }
      }
    } else if (acc == "qsim" && noises) {
      // Use qsim via Cirq wrapper to handle noise if requested
      // The "cirq-qsim" backend is part of the external emulator package
      // to be used with the qb emulator noise models only.
      if (run_config.noise_model.name != "qb-nm1" &&
          run_config.noise_model.name != "qb-nm2" &&
          run_config.noise_model.name != "qb-nm3") {
        // We don't support arbitrary noise model in qsim yet.
        // Hence, ignore the noise request. Log info to let users know.
        std::cout
            << "# The 'qsim' accelerator doesn't support noise configuration '"
            << run_config.noise_model.name << "'." << std::endl;
        std::cout << "# If you wish to use qsim with noise, please install the "
                     "emulator package and select one of the QB noise models "
                     "(e.g. 'qb-nm1', 'qb-nm2' or 'qb-nm3')."
                  << std::endl;
        std::cout << "# Disabling noise." << std::endl;
      } else {
        // Switch to "cirq-qsim" from emulator package
        qpu = xacc::getAccelerator("cirq-qsim");
        if (debug_) {
          std::cout << "# Noise model for qsim (from emulator package): enabled"
                    << std::endl;
        }
      }
    } else if (acc == "qb-mps") {
      if (noises) {
        // Use MPS via qb-mps wrapper to handle noise if requested
        // The "qb-mps" backend is part of the external emulator package
        // to be used with the qb emulator noise models only.
        if (run_config.noise_model.name != "qb-nm1" &&
            run_config.noise_model.name != "qb-nm2" &&
            run_config.noise_model.name != "qb-nm3" &&
            run_config.noise_model.name != "qb-qdk1" &&
            run_config.noise_model.name != "qb-dqc2") {
          // We don't support arbitrary noise model in qb-mps yet.
          // Hence, ignore the noise request. Log info to let users know.
          std::cout
              << "# The 'qb-mps' accelerator doesn't support noise configuration '"
              << run_config.noise_model.name << "'." << std::endl;
          std::cout << "# If you wish to use qb-mps with noise, please install the "
                      "emulator package and select one of the QB noise models "
                      "(e.g. 'qb-nm1', 'qb-nm2' or 'qb-nm3')."
                    << std::endl;
          std::cout << "# Disabling noise." << std::endl;
        } else {
          // Switch to "qb-mps" from emulator package
          static const char *EMULATOR_NOISE_MODEL_LIB_NAME = "libqbemulator.so";
          void *handle = dlopen(EMULATOR_NOISE_MODEL_LIB_NAME, RTLD_LOCAL | RTLD_LAZY);
          using func_type = qb::NoiseModel *(const char *);
          auto *get_emulator_noise_model =
              reinterpret_cast<func_type *>(dlsym(handle, "get_emulator_noise_model"));

          // run_config.noise_model.name is of type string but the emulator requires the noise
          // model's name to be of type const char.
          // Convert noise model name's type from string to char.
          char* noise_model_name_as_char = new char[run_config.noise_model.name.length() + 1];
          std::strcpy(noise_model_name_as_char, run_config.noise_model.name.c_str());
          auto *noise_model_name = get_emulator_noise_model(noise_model_name_as_char);

          qpu = xacc::getAccelerator("qb-mps", {
              {"initial-bond-dim", initial_bond_dimension},
              {"max-bond-dim", max_bond_dimension},
              {"abs-truncation-threshold", svd_cutoff},
              {"rel-truncation-threshold", rel_svd_cutoff},
              {"noise-model", noise_model_name}});
          if (debug_) {
            std::cout << "# Noise model for qb-mps (from emulator package): enabled"
                      << std::endl;
          }
        }
      } else { // Noiseless simulation
        qpu = xacc::getAccelerator("qb-mps", {
            {"initial-bond-dim", initial_bond_dimension},
            {"max-bond-dim", max_bond_dimension},
            {"abs-truncation-threshold", svd_cutoff},
            {"rel-truncation-threshold", rel_svd_cutoff}});
      }
    } else if (acc == "qb-purification") {
      if (noises) {
        // Use purification via qb-purification wrapper to handle noise if requested
        // The "qb-purification" backend is part of the external emulator package
        // to be used with the qb emulator noise models only.
        if (run_config.noise_model.name != "qb-nm1" &&
            run_config.noise_model.name != "qb-nm2" &&
            run_config.noise_model.name != "qb-nm3" &&
            run_config.noise_model.name != "qb-qdk1" &&
            run_config.noise_model.name != "qb-dqc2") {
          // We don't support arbitrary noise model in qb-purification yet.
          // Hence, ignore the noise request. Log info to let users know.
          std::cout
              << "# The 'qb-purification' accelerator doesn't support noise configuration '"
              << run_config.noise_model.name << "'." << std::endl;
          std::cout << "# If you wish to use qb-purification with noise, please install the "
                      "emulator package and select one of the QB noise models "
                      "(e.g. 'qb-nm1', 'qb-nm2' or 'qb-nm3')."
                    << std::endl;
          std::cout << "# Disabling noise." << std::endl;
        } else {
          // Switch to "qb-purification" from emulator package
          static const char *EMULATOR_NOISE_MODEL_LIB_NAME = "libqbemulator.so";
          void *handle = dlopen(EMULATOR_NOISE_MODEL_LIB_NAME, RTLD_LOCAL | RTLD_LAZY);
          using func_type = qb::NoiseModel *(const char *);
          auto *get_emulator_noise_model =
              reinterpret_cast<func_type *>(dlsym(handle, "get_emulator_noise_model"));

          // run_config.noise_model.name is of type string but the emulator requires the noise
          // model's name to be of type const char.
          // Convert noise model name's type from string to char.
          char* noise_model_name_as_char = new char[run_config.noise_model.name.length() + 1];
          std::strcpy(noise_model_name_as_char, run_config.noise_model.name.c_str());
          auto *noise_model_name = get_emulator_noise_model(noise_model_name_as_char);

          qpu = xacc::getAccelerator("qb-purification", {
              {"initial-bond-dim", initial_bond_dimension},
              {"initial-kraus-dim", initial_kraus_dimension},
              {"max-bond-dim", max_bond_dimension},
              {"max-kraus-dim", max_kraus_dimension},
              {"abs-truncation-threshold", svd_cutoff},
              {"rel-truncation-threshold", rel_svd_cutoff},
              {"noise-model", noise_model_name}});
          if (debug_) {
            std::cout << "# Noise model for qb-purification (from emulator package): enabled"
                      << std::endl;
          }
        }
      } else { // Noiseless simulation
        qpu = xacc::getAccelerator("qb-purification", {
            {"initial-bond-dim", initial_bond_dimension},
            {"initial-kraus-dim", initial_kraus_dimension},
            {"max-bond-dim", max_bond_dimension},
            {"max-kraus-dim", max_kraus_dimension},
            {"abs-truncation-threshold", svd_cutoff},
            {"rel-truncation-threshold", rel_svd_cutoff}});
      }
    } else if (acc == "qb-mpdo") {
      if (noises) {
        // Use MPDO via qb-mpdo wrapper to handle noise if requested
        // The "qb-mpdo" backend is part of the external emulator package
        // to be used with the qb emulator noise models only.
        if (run_config.noise_model.name != "qb-nm1" &&
            run_config.noise_model.name != "qb-nm2" &&
            run_config.noise_model.name != "qb-nm3" &&
            run_config.noise_model.name != "qb-qdk1" &&
            run_config.noise_model.name != "qb-dqc2") {
          // We don't support arbitrary noise model in qb-mpdo yet.
          // Hence, ignore the noise request. Log info to let users know.
          std::cout
              << "# The 'qb-mpdo' accelerator doesn't support noise configuration '"
              << run_config.noise_model.name << "'." << std::endl;
          std::cout << "# If you wish to use qb-mpdo with noise, please install the "
                      "emulator package and select one of the QB noise models "
                      "(e.g. 'qb-nm1', 'qb-nm2' or 'qb-nm3')."
                    << std::endl;
          std::cout << "# Disabling noise." << std::endl;
        } else {
          // Switch to "qb-mpdo" from emulator package
          static const char *EMULATOR_NOISE_MODEL_LIB_NAME = "libqbemulator.so";
          void *handle = dlopen(EMULATOR_NOISE_MODEL_LIB_NAME, RTLD_LOCAL | RTLD_LAZY);
          using func_type = qb::NoiseModel *(const char *);
          auto *get_emulator_noise_model =
              reinterpret_cast<func_type *>(dlsym(handle, "get_emulator_noise_model"));

          // run_config.noise_model.name is of type string but the emulator requires the noise
          // model's name to be of type const char.
          // Convert noise model name's type from string to char.
          char* noise_model_name_as_char = new char[run_config.noise_model.name.length() + 1];
          std::strcpy(noise_model_name_as_char, run_config.noise_model.name.c_str());
          auto *noise_model_name = get_emulator_noise_model(noise_model_name_as_char);

          qpu = xacc::getAccelerator("qb-mpdo", {
              {"initial-bond-dim", initial_bond_dimension},
              {"max-bond-dim", max_bond_dimension},
              {"abs-truncation-threshold", svd_cutoff},
              {"rel-truncation-threshold", rel_svd_cutoff},
              {"noise-model", noise_model_name}});
          if (debug_) {
            std::cout << "# Noise model for qb-mpdo (from emulator package): enabled"
                      << std::endl;
          }
        }
      } else { // Noiseless simulation
        qpu = xacc::getAccelerator("qb-mpdo", {
            {"initial-bond-dim", initial_bond_dimension},
            {"max-bond-dim", max_bond_dimension},
            {"abs-truncation-threshold", svd_cutoff},
            {"rel-truncation-threshold", rel_svd_cutoff}});
      }
    }
    return qpu;
  }
  
  /// Run (execute) task specified by the (ii, jj) index pair
  void session::run(const size_t ii, const size_t jj) {
   run_internal(ii, jj, /*acc = */ nullptr);
  }

  void session::run() {
    if (debug_) std::cout << "Invoked run()" << std::endl;

    // Shape consistency
    const int N_ii = is_ii_consistent();
    const int N_jj = is_jj_consistent();
    if (N_ii < 0) {
      throw std::range_error("Leading dimension has inconsistent length");
    }

    if (N_jj < 0) {
      throw std::range_error("Second dimension has inconsistent length");
    }
    if (debug_) {
      std::cout << "N_ii: " << N_ii << std::endl;
    }
    if (debug_) {
      std::cout << "N_jj: " << N_jj << std::endl;
    }

    // Clear all stored results:
    out_raws_.clear();
    out_raws_.resize(N_ii);
    for (auto el : out_raws_) {
      el.resize(N_jj);
    }
    out_bitstrings_.clear();
    out_bitstrings_.resize(N_ii);
    for (auto el : out_bitstrings_) {
      el.resize(N_jj);
    }
    out_divergences_.clear();
    out_divergences_.resize(N_ii);
    for (auto el : out_divergences_) {
      el.resize(N_jj);
    }
    out_transpiled_circuits_.clear();
    out_transpiled_circuits_.resize(N_ii);
    for (auto el : out_transpiled_circuits_) {
      el.resize(N_jj);
    }
    out_qobjs_.clear();
    out_qobjs_.resize(N_ii);
    for (auto el : out_qobjs_) {
      el.resize(N_jj);
    }
    out_qbjsons_.clear();
    out_qbjsons_.resize(N_ii);
    for (auto el : out_qbjsons_) {
      el.resize(N_jj);
    }
    out_single_qubit_gate_qtys_.clear();
    out_single_qubit_gate_qtys_.resize(N_ii);
    for (auto el : out_single_qubit_gate_qtys_) {
      el.resize(N_jj);
    }
    out_double_qubit_gate_qtys_.clear();
    out_double_qubit_gate_qtys_.resize(N_ii);
    for (auto el : out_double_qubit_gate_qtys_) {
      el.resize(N_jj);
    }
    out_total_init_maxgate_readout_times_.clear();
    out_total_init_maxgate_readout_times_.resize(N_ii);
    for (auto el : out_total_init_maxgate_readout_times_) {
      el.resize(N_jj);
    }
    acc_uses_lsbs_.clear();
    acc_uses_lsbs_.resize(N_ii);
    for (auto el : acc_uses_lsbs_) {
      el.resize(N_jj);
    }
    acc_uses_n_bits_.clear();
    acc_uses_n_bits_.resize(N_ii);
    for (auto el : acc_uses_n_bits_) {
      el.resize(N_jj);
    }

    for (size_t ii = 0; ii < N_ii; ii++) {
      for (size_t jj = 0; jj < N_jj; jj++) {
        run(ii, jj);
      }
    }
  }

  void session::qb12() {
    if (debug_) {
      std::cout
          << "Setting defaults for Quantum Brilliance 12 qubit, 1024-shot, "
             "noiseless simulation, MPS bond dimension = 256, SVD cutoff = 1.0e-8"
          << std::endl;
    }
    set_qn(12);
    set_sn(1024);
    set_rn(1);
    set_noise(false);
    set_initial_bond_dimension(1);
    set_initial_kraus_dimension(1);
    set_max_bond_dimension(256);
    set_max_kraus_dimension(256);
    ND scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    ND rel_scut{{0, 1.0e-4}};
    set_rel_svd_cutoff(rel_scut);
    set_output_oqm_enabled(true);
  }

  void session::aws32dm1() {
    if (debug_) {
      std::cout
          << "Setting AWS Braket DM1 with 32 asynchronous workers, "
             "noiseless, 17-qubits, 256 shots"
          << std::endl;
    }
    const int wn = 32; // 32 workers
    set_qn(17);
    set_sn(256);
    set_rn(1);
    set_noise(false);
    set_max_bond_dimension(256);
    ND scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    set_output_oqm_enabled(true);
    set_acc("aws_acc");
    std::stringstream async_workers;
    async_workers << "{\"accs\": [";
    for (int iw=0; iw<(wn-1); iw++) {
        async_workers << "{\"acc\": \"aws_acc\"},";
    }
    async_workers << "{\"acc\": \"aws_acc\"}]}";
    set_parallel_run_config(async_workers.str());
  }

  void session::aws32sv1() {
    if (debug_) {
      std::cout
          << "Setting AWS Braket SV1 with 32 asynchronous workers, "
             "noiseless, 34-qubits, 256 shots"
          << std::endl;
    }
    const int wn = 32; // 32 workers
    set_qn(34);
    set_sn(256);
    set_rn(1);
    set_noise(false);
    set_max_bond_dimension(256);
    ND scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    set_output_oqm_enabled(true);
    set_acc("aws_acc");
    std::stringstream async_workers;
    async_workers << "{\"accs\": [";
    for (int iw=0; iw<(wn-1); iw++) {
        async_workers << "{\"acc\": \"aws_acc\"},";
    }
    async_workers << "{\"acc\": \"aws_acc\"}]}";
    set_parallel_run_config(async_workers.str());
  }

  void session::aws8tn1() {
    if (debug_) {
      std::cout
          << "Setting AWS Braket TN1 with 8 asynchronous workers, "
             "noiseless, 48-qubits, 256 shots"
          << std::endl;
    }
    const int wn = 8; // 8 workers
    set_qn(48);
    set_sn(256);
    set_rn(1);
    set_noise(false);
    set_max_bond_dimension(256);
    ND scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    set_output_oqm_enabled(true);
    set_acc("aws_acc");
    std::stringstream async_workers;
    async_workers << "{\"accs\": [";
    for (int iw=0; iw<(wn-1); iw++) {
        async_workers << "{\"acc\": \"aws_acc\"},";
    }
    async_workers << "{\"acc\": \"aws_acc\"}]}";
    set_parallel_run_config(async_workers.str());
  }

  /// Util method to compile input source string into IR
  /// This method is thread-safe, thus can be used to compile multiple source strings in parallel.
  std::shared_ptr<xacc::CompositeInstruction> session::compile_input(const std::string& in_source_string, int in_num_qubits,
                                                                  source_string_type in_source_type)
  {
    // Retrieve the compiler instance for the QASM dialect
    auto compiler = [&]() -> std::shared_ptr<xacc::Compiler>
    {
      switch (in_source_type)
      {
      case source_string_type::OpenQASM:
        return xacc::getCompiler("staq");
      case source_string_type::XASM:
        return xacc::getCompiler("xasm");
      case source_string_type::Quil:
        return xacc::getCompiler("quil");
      default:
        __builtin_unreachable();
      }
    }();

    // Get buffer name from the kernel signature
    const std::string buffer_name = [](const std::string& kernel_src)
    {
      const std::regex kernelSignatureRegex(R"(__qpu__[ \t]void[ \t]QBCIRCUIT\(qreg[ \t]([a-zA-Z_$][a-zA-Z_$0-9]*)\))");
      std::smatch matches;
      std::string qRegName = "q";
      if (std::regex_search(kernel_src, matches, kernelSignatureRegex))
      {
        qRegName = matches[1].str();
      }
      return qRegName;
    }(in_source_string);

    auto src_str = in_source_string;
    // For OpenQASM: inject the buffer declaration into the source code so that the compiler doesn't need to do global
    // look-up into the static buffer map. This is to guarantee thread-safety in case multiple kernels using the same
    // qubit register name.
    if (in_source_type == source_string_type::OpenQASM && in_source_string.find("__qpu__") != std::string::npos)
    {
      std::string tmp            = "";
      const auto first_brace_pos = in_source_string.find_first_of("{");
      const auto last_brace_pos  = in_source_string.find_last_of("}");
      const auto sub             = in_source_string.substr(first_brace_pos + 1, last_brace_pos - first_brace_pos - 1);
      auto lines                 = xacc::split(sub, '\n');
      // Flag to insert register declaration only once after the first include.
      bool added_reg_decl = false;
      for (auto& l : lines)
      {
        xacc::trim(l);
        tmp += l + "\n";
        if (l.find("include") != std::string::npos)
        {
          // Add the qreg declaration with the proper size
          tmp += "qreg " + buffer_name + "[" + std::to_string(in_num_qubits) + "];\n";
          added_reg_decl = true;
        }
      }
      // Replace the source string
      src_str = tmp;
    }


    // Compile source string to IR
    auto irtarget = compiler->compile(src_str);
    // Get the kernel composite instruction (quantum circuit)
    return irtarget->getComposites().front();
  }

  void session::set_parallel_run_config(const std::string &in_config) {
    // Check if this is a file:
    std::ifstream config_file(in_config);
    if (config_file) {
      const std::string config_str((std::istreambuf_iterator<char>(config_file)),
                                   std::istreambuf_iterator<char>());
      executor_->initialize(config_str);
    } else {
      executor_->initialize(in_config);
    }
  }

  Executor &session::get_executor() { return *executor_; }

  std::shared_ptr<async_job_handle> session::run_async(const std::size_t ii, const std::size_t jj, std::shared_ptr<xacc::Accelerator> accelerator) {
    static std::mutex shared_mutex;
    return run_internal(ii, jj, accelerator, &shared_mutex);
  }

  std::shared_ptr<async_job_handle> session::run_internal(const std::size_t ii, const std::size_t jj,
                     std::shared_ptr<xacc::Accelerator> accelerator, std::mutex* optional_mutex)
  {
    if (debug_ && accelerator) {
      std::stringstream msg;
      msg << "thread " << std::this_thread::get_id() << " run_async (ii,jj): ("
          << ii << "," << jj << ") with acc: " << accelerator->name() << "(" << accelerator << ")" << std::endl;
      std::cout << msg.str();
      msg.str("");
    }
    // Retrieve and validate run configuration for this (ii, jj) task
    auto run_config = get_run_config(ii, jj);

    // Check shape consistency
    // This is thread-safe.
    {
      int N_ii = is_ii_consistent();
      int N_jj = is_jj_consistent();

      if (N_ii < 0) {
        throw std::range_error("Leading dimension has inconsistent length");
      }

      if (N_jj < 0) {
        throw std::range_error("Second dimension has inconsistent length");
      }
    }

    {
      optional_scoped_lock lock(optional_mutex);
      // Resize result tables if necessary.
      // (resizing vectors => need lock)
      ensure_results_table_size(ii, jj);
    }

    // Determine what type of input is for (ii, jj)
    // This is thread-safe (different threads looks at different vector
    // elements)
    const auto file_or_string_or_random_or_ir =
        validate_infiles_instrings_randoms_irtarget_ms_nonempty(ii, jj);
    if (file_or_string_or_random_or_ir ==
        session::circuit_input_types::INVALID) {
      throw std::invalid_argument("Please check your settings again.");
    }

    if (file_or_string_or_random_or_ir == session::circuit_input_types::VALID_CUDAQ) {
#ifdef WITH_CUDAQ
      // Running CUDAQ in an async. context is illegal!
      assert(!optional_mutex);
      // Execute CUDAQ kernel input
      run_cudaq(ii, jj, run_config);
      return nullptr;
#else
      throw std::runtime_error(
          "CUDAQ is not supported. Please build qb::core with CUDAQ.");
#endif
    }

    bool noplacement = run_config.no_placement;
    bool nooptimise = run_config.no_optimise;
    bool nosim = run_config.no_sim;
    std::stringstream debug_msg;
    if (debug_) {
      switch (run_config.source_type) {
      case source_string_type::XASM:
        debug_msg << "# XASM compiler: xasm" << std::endl;
        break;
      case source_string_type::Quil:
        debug_msg << "# Quil v1 compiler: quil" << std::endl;
        break;
      case source_string_type::OpenQASM:
        debug_msg << "# OpenQASM compiler: staq" << std::endl;
      }
    }

    size_t n_qubits = run_config.num_qubits;
    if (debug_)
      debug_msg << "# Qubits: " << n_qubits << std::endl;
    int shots = run_config.num_shots;
    if (debug_)
      debug_msg << "# Shots: " << shots << std::endl;
    size_t n_samples = run_config.num_repetitions;
    if (debug_)
      debug_msg << "# Repetitions: " << n_samples << std::endl;
    bool output_oqm_enableds = run_config.oqm_enabled;
    if (debug_) {
      if (output_oqm_enableds) {
        debug_msg << "# Output transpiled circuit: enabled" << std::endl;
      } else {
        debug_msg << "# Output transpiled circuit: disabled" << std::endl;
      }
    }
    if (debug_) {
      // Flush all the debug logs in one go
      std::cout << "thread " << std::this_thread::get_id() << "\n"
                << debug_msg.str() << std::endl;
      debug_msg.str("");
    }

    // Load the remote backend database
    remote_backend_database_ = YAML::LoadFile(remote_backend_database_path_);

    // Has the user asked for a hardware backend?  Check that the backend is in the remote backend database, but not AWS or Lambda.
    const bool exec_on_hardware = run_config.acc_name != "aws_acc" and 
                                  run_config.acc_name != "qb-lambda" and
                                  remote_backend_database_[run_config.acc_name];

    // Collect all the simulator options (thread safe)
    const xacc::HeterogeneousMap mqbacc = backend_config(remote_backend_database_, run_config);

    // ==============================================
    // Construct/initialize the Accelerator instance
    // ==============================================
    // Note: there are two cases here: 
    // (1) Asynchronous execution: an accelerator instance is explicitly provided 
    // i.e., run_async(i, j), whereby an accelerator from a pre-defined pool is provided.
    // (2) Synchronous/sequential execution whereby the accelerator name for this (i, j) job is set in this session object,
    // hence, we just construct/retrieve it accordingly. 
    auto qpu = accelerator ? accelerator : get_sim_qpu(exec_on_hardware, run_config);
    auto acc = std::make_shared<qb::backend>();
    qpu->updateConfiguration(mqbacc);
    acc->updateConfiguration(mqbacc);

    auto buffer_b = std::make_shared<xacc::AcceleratorBuffer>(n_qubits);
    std::vector<std::shared_ptr<xacc::CompositeInstruction>> citargets;
    {
      // Lock up until the circuit is executed
      optional_scoped_lock lock(optional_mutex);
      // ==============================================
      // ----------------- Compilation ----------------
      // ==============================================
      if (file_or_string_or_random_or_ir ==
          session::circuit_input_types::VALID_IR) {
        // Direct IR input (e.g., circuit builder)
        citargets.push_back(irtarget_ms_.at(ii).at(0));
      } else {
        // String input -> compile
        const std::string target_circuit =
            get_target_circuit_qasm_string(ii, jj, run_config);
        // Note: compile_input may not be thread-safe, e.g., XACC's staq
        // compiler plugin was not defined as Clonable, hence only one instance
        // is available from the service registry.
        citargets.push_back(
            compile_input(target_circuit, n_qubits, run_config.source_type));
      }

      // ==============================================
      // -----------------  Placement  ----------------
      // ==============================================
      xacc::HeterogeneousMap m;
      // Transform the target to account for QB topology: XACC
      // "swap-shortest-path"
      if (!noplacement) {
        if (debug_)
          debug_msg << "# Quantum Brilliance topology placement: enabled"
                    << std::endl;
        std::string placement = "swap-shortest-path";
        if (!placements_.empty()) {
          ValidatorTwoDim<VectorString, std::string> placements_valid(
              placements_, VALID_HARDWARE_PLACEMENTS,
              " name of placement module");
          const std::string placement_opt = placements_valid.get(ii, jj);

          if (debug_) {
            debug_msg << "# Placement: " << placement_opt << std::endl;
          }
          if (xacc::getIRTransformation(placement_opt)) {
            placement = placement_opt;
          } else {
            std::cout
                << "Placement module '" << placement_opt
                << "' cannot be located. Please check your installation.\n";
          }
        }
        if (run_config.acc_name == "aws_acc") {
          m.merge(qpu->getProperties());
        }

        if (debug_) {
          // Flush all the placement debug logs in one go
          std::cout << "thread " << std::this_thread::get_id() << "\n"
                    << debug_msg.str() << std::endl;
          debug_msg.str("");
        }

        // Disable QASM inlining during placement.
        // e.g., we don't want to map gates to the IBM gateset (defined in
        // qelib1.inc) during placement.
        m.insert("no-inline", true);
        // TODO: investigate thread-safety of placement modules
        // and remove this from the scoped_lock if possible.
        auto A = xacc::getIRTransformation(placement);
        A->apply(citargets.at(0), acc, m);
      }

      // ==============================================
      // ----------  Circuit Optimization  ------------
      // ==============================================
      // Perform circuit optimisation: XACC "circuit-optimizer"
      if (!nooptimise) {
        if (debug_)
          debug_msg << "# Quantum Brilliance circuit optimiser: enabled"
                    << std::endl;
        const auto opt_passes = [&]() -> Passes {
          if (!circuit_opts_.empty()) {
            ValidatorTwoDim<Table2d<Passes>, Passes> opts_valid(circuit_opts_);
            return opts_valid.get(ii, jj);
          }

          // By default, if not otherwise specified, we apply the circuit
          // optimizer pass
          return {create_circuit_optimizer_pass()};
        }();
        // Apply those optimization passes
        // TODO: investigate thread-safety of optimization passes
        // and remove this from the scoped_lock if possible.
        for (const auto &pass : opt_passes) {
          if (debug_) {
            debug_msg << "# Apply optimization pass: " << pass->get_name()
                      << std::endl;
          }
          // Wrap the composite IR (citargets[0]) as a CircuitBuilder to send on
          // to the optimization pass. Set copy_nodes to false to keep the root
          // node (citargets.at(0)) intact.
          CircuitBuilder ir_as_circuit(citargets.at(0), /*copy_nodes*/ false);
          pass->apply(ir_as_circuit);
        }
        if (debug_) {
          // Flush all the circuit optimization  debug logs in one go
          std::cout << "thread " << std::this_thread::get_id() << "\n"
                    << debug_msg.str() << std::endl;
          debug_msg.str("");
        }
      }
    } // End scoped_lock

    // ==============================================
    // ----------  Execution  ------------
    // ==============================================
    buffer_b->resetBuffer();
    xacc::ScopeTimer timer_for_qpu(
        "Walltime, in ms, for simulator to execute quantum circuit", false);
    if (!nosim && !exec_on_hardware) {
      try {
        if (debug_) {
          debug_msg << "# "
                    << " Prior to qpu->execute..."
                    << "\n";
          std::cout << "thread " << std::this_thread::get_id() << "\n"
                    << debug_msg.str() << std::endl;
          debug_msg.str("");
        }
        if (qpu->name() == "aws_acc" &&
            std::dynamic_pointer_cast<qb::remote_accelerator>(qpu)) {
          auto as_remote_acc =
              std::dynamic_pointer_cast<qb::remote_accelerator>(qpu);
          // Asynchronous offload the circuit.
          auto aws_job_handle = as_remote_acc->async_execute(citargets.at(0));
          aws_job_handle->add_done_callback([=](auto &handle) {
            auto buffer_temp =
                std::make_shared<xacc::AcceleratorBuffer>(buffer_b->size());
            handle.load_result(buffer_temp);
            auto qb_transpiler = std::make_shared<qb::backend>();
            this->process_run_result(
                ii, jj, run_config, citargets.at(0), qpu, mqbacc, buffer_temp,
                timer_for_qpu.getDurationMs(), qb_transpiler);
          });
          return aws_job_handle;
        } else {
          // Blocking execution of a local simulator instance
          execute_on_simulator(qpu, buffer_b, citargets, run_config);
        }
      } catch (...) {
        throw std::invalid_argument(
            "The simulation of your input circuit failed");
      }
    } else if (exec_on_hardware) {
      // Hardware execution
      // We don't expect to run this in an async. context (e.g., an acceletor instance from a pool)
      assert(!accelerator && !optional_mutex);
      std::shared_ptr<xacc::quantum::qb_qpu> hardware_device = std::make_shared<xacc::quantum::qb_qpu>(debug_);
      hardware_device->updateConfiguration(mqbacc);
      if (debug_) std::cout << "# " << run_config.acc_name << " accelerator: initialised" << std::endl;

      // Execute (and polling wait)
      execute_on_qb_hardware(hardware_device, buffer_b, citargets, run_config, debug_);

      // Store the JSON sent to the QB hardware for the user to inspect
      out_qbjsons_.at(ii).at(jj) = hardware_device->get_qbjson();

    }
    // ==============================================
    // ------------  Post processing  ---------------
    // ==============================================

    {
      optional_scoped_lock lock(optional_mutex);
      /// Post-processing results for run_async with local simulators:
      /// i.e., simulation occurs on this thread.
      // Note: this will write to STL vectors, hence, needing a scoped_lock.
      const double xacc_scope_timer_qpu_ms = timer_for_qpu.getDurationMs();
      process_run_result(ii, jj, run_config, citargets.at(0), qpu, mqbacc,
                         buffer_b, xacc_scope_timer_qpu_ms, acc);
    }
    return nullptr;
  }

  void session::process_run_result(
      const std::size_t ii, const std::size_t jj,
      const run_i_j_config &run_config,
      std::shared_ptr<xacc::CompositeInstruction> ir_target,
      std::shared_ptr<xacc::Accelerator> sim_qpu,
      const xacc::HeterogeneousMap &sim_qpu_configs,
      std::shared_ptr<xacc::AcceleratorBuffer> buffer_b,
      double xacc_scope_timer_qpu_ms,
      std::shared_ptr<qb::backend> qb_transpiler) {
    if (debug_) {
      std::cout << std::endl;
      buffer_b->print();
      std::cout << std::endl;
      std::cout << "Walltime elapsed for the simulator to perform the "
                   "requested number of shots of the quantum circuit, in ms: "
                << xacc_scope_timer_qpu_ms << std::endl;
      std::cout << std::endl;
      std::cout << "Bit order [0=>LSB, 1=>MSB]: " << sim_qpu->getBitOrder()
                << " : Important note : TNQVM reports incorrect bit order - it "
                   "uses LSB"
                << std::endl;
    }

    // Store indicator of LSB pattern
    // 0 => LSB
    acc_uses_lsbs_.at(ii).at(jj) = (sim_qpu->getBitOrder() == 0);
    // Workaround for incorrect TNQVM reporting of LSB/MSB ordering
    if (sim_qpu->name().compare("tnqvm") == 0) {
      acc_uses_lsbs_.at(ii).at(jj) = false;
    }
    // Workaround for aer reverse ordering
    // Also for aer, keep the qobj so that a user can call Aer standalone
    // later
    if (sim_qpu->name().compare("aer") == 0) {
      acc_uses_lsbs_.at(ii).at(jj) = true;
      out_qobjs_.at(ii).at(jj) = sim_qpu->getNativeCode(ir_target, sim_qpu_configs);
    }

    // Get counts
    std::map<std::string, int> qpu_counts = buffer_b->getMeasurementCounts();

    // Get Z operator expectation:
    if (!run_config.no_sim) {
      if (buffer_b->hasExtraInfoKey("ro-fixed-exp-val-z") ||
          buffer_b->hasExtraInfoKey("exp-val-z") ||
          (!buffer_b->getMeasurementCounts().empty())) {
        double z_expectation_val = getExpectationValueZ(buffer_b);
        if (debug_) {
          std::cout << "* Z-operator expectation value: " << z_expectation_val
                    << std::endl;
        }
        // Save Z-operator expectation value to VectorMapND
        ND res_z{{0, z_expectation_val}};
        out_z_op_expects_.at(ii).at(jj) = res_z;
      } else {
        xacc::warning("No Z operator expectation available");
      }
    }

    // Get the state vector from qpp or AER
    if ((sim_qpu->name().compare("qpp") == 0 ||
        (sim_qpu->name().compare("aer") == 0 && run_config.aer_sim_type == "statevector"))
        && in_get_state_vec_ == true) {
      state_vec_ = sim_qpu->getExecutionInfo<xacc::ExecutionInfo::WaveFuncPtrType>(
                        xacc::ExecutionInfo::WaveFuncKey);
    }

    // Flip the bit string (key in map qpu_counts) if AER backend is used. This is
    // to ensure that the printed bit string in out_raw has the same order as other
    // backends.
    if (sim_qpu->name().compare("aer") == 0) {
      std::map<std::string, int> qpu_counts_aer;
      for (const auto &[bit_string, count] : qpu_counts) {
        std::string bit_string_reverse = bit_string;
        std::reverse(bit_string_reverse.begin(), bit_string_reverse.end());
        qpu_counts_aer.insert(std::make_pair(bit_string_reverse, count));
      }
      qpu_counts = qpu_counts_aer;
    }

    // Save the counts to out_bitstrings_ and raw map data in out_raws
    populate_measure_counts_data(ii, jj, qpu_counts);

    // Transpile to QB native gates (acc)
    if (run_config.oqm_enabled) {
      auto buffer_qb =
          std::make_shared<xacc::AcceleratorBuffer>(run_config.num_qubits);
      try {
        qb_transpiler->execute(buffer_qb, ir_target);
      } catch (...) {
        throw std::invalid_argument(
            "Transpiling to QB native gates for your input circuit failed");
      }

      // Save the transpiled circuit string
      out_transpiled_circuits_.at(ii).at(jj) = qb_transpiler->getTranspiledResult();

      // Invoke the Profiler
      Profiler timing_profile(qb_transpiler->getTranspiledResult(), run_config.num_qubits, debug_);

      // Save single qubit gate qtys to VectorMapNN
      out_single_qubit_gate_qtys_.at(ii).at(jj) = timing_profile.get_count_1q_gates_on_q();

      // Save two-qubit gate qtys to VectorMapNN
      out_double_qubit_gate_qtys_.at(ii).at(jj) = timing_profile.get_count_2q_gates_on_q();

      // Save timing results to VectorMapND
      out_total_init_maxgate_readout_times_.at(ii).at(jj) =
          timing_profile.get_total_initialisation_maxgate_readout_time_ms(xacc_scope_timer_qpu_ms, run_config.num_shots);
    }
  }

  // Wrap raw OpenQASM string in a QB Kernel:
  // - Move qreg to a kernel argument
  // - Denote the kernel name as 'QBCIRCUIT'
  std::string session::convertRawOpenQasmToQBKernel(const std::string &in_rawQasm) 
  {
    // Pattern:
    // - start with qreg
    // - follow by any number of spaces
    // - a valid variable name (first character is alpha or _, followed by alphanumeric)
    // - register size
    // - spaces or no spaces, then ';'
    const std::regex qregDeclarationRegex(R"(qreg[ \t]+([a-zA-Z_$][a-zA-Z_$0-9]*)\[\d+\]\s*;)");
    std::smatch matches;
    if (std::regex_search(in_rawQasm, matches, qregDeclarationRegex))
    {
      // Note: first match is the entire string, second match is the qreg
      // variable name grouping.
      const std::string qRegDeclararion = matches[0].str();
      const std::string qRegName = matches[1].str();
      // Erase the declaration line from the body and construct the kernel
      // signature accordingly.
      std::string q2oqm = in_rawQasm;
      q2oqm.erase(q2oqm.find(qRegDeclararion), qRegDeclararion.length());
      const std::string qbstr =
          "__qpu__ void QBCIRCUIT(qreg " + qRegName + ") {\n" + q2oqm + '}';
      return qbstr;
    }
    else
    {
      // There is no qreg declaration, e.g., an empty OpenQASM source with no
      // body. Just handle it graciously.
      const std::string qbstr =
          "__qpu__ void QBCIRCUIT(qreg q) {\n" + in_rawQasm + '}';
      return qbstr;
    }
  }
} // namespace qb

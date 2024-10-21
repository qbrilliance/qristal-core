// Copyright (c) Quantum Brilliance Pty Ltd

// STL
#include <algorithm>
#include <vector>
#include <memory>
#include <regex>
#include <random>
#include <sstream>
#include <stdexcept>
#include <random>

// Boost
#include <boost/dynamic_bitset.hpp>

// Qristal
#include "qristal/core/backend.hpp"
#include "qristal/core/backend_utils.hpp"
#include "qristal/core/backends/qb_hardware/qb_qpu.hpp"
#include "qristal/core/pretranspiler.hpp"
#include "qristal/core/profiler.hpp"
#include "qristal/core/session.hpp"
#include "qristal/core/passes/circuit_opt_passes.hpp"
#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/metrics/ConfusionMatrix.hpp"

// XACC
#include "CompositeInstruction.hpp"
#include "AcceleratorBuffer.hpp"
#include "xacc.hpp"

// CUDAQ support
#ifdef WITH_CUDAQ
  #include "qristal/core/cudaq/sim_pool.hpp"
  #include "qristal/core/cudaq/cudaq_acc.hpp"
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


namespace qristal
{
  /// Default session constructor
  session::session()
      : name_m{{}}, number_m{{}}, infiles_{{}},
        include_qbs_{{SDK_DIR "/include/qb/core/qblib.inc"}},
        remote_backend_database_path_{SDK_DIR "/remote_backends.yaml"},
        instrings_{{}}, irtarget_ms_{{}}, accs_{{"qpp"}},
        aer_sim_types_{}, randoms_{{}},
        xasms_{{{false}}}, quil1s_{{{false}}}, noplacements_{{{false}}},
        nooptimises_{{{true}}}, nosims_{{{false}}}, noises_{{{false}}},
        output_oqm_enableds_{{{false}}},
        notimings_{{{false}}}, qns_{{}}, sns_{{}},
        calc_out_counts_{{{false}}}, calc_jacobians_{{{false}}}, parameter_vectors_{{{}}},
        initial_bond_dimensions_{{}}, initial_kraus_dimensions_{{}}, max_bond_dimensions_{{}},
        max_kraus_dimensions_{{}}, svd_cutoffs_{{}}, rel_svd_cutoffs_{{}}, noise_models_{{}},
        acc_outputs_qbit0_left_{{{}}}, acc_uses_n_bits_{{{}}}, expected_amplitudes_{{}},
        results_{{{}}}, out_probs_{{{}}}, out_counts_{{{}}},
        out_divergences_{{{}}}, out_transpiled_circuits_{{{}}}, out_qobjs_{{{}}},
        out_qbjsons_{{{}}}, out_single_qubit_gate_qtys_{{{}}}, out_double_qubit_gate_qtys_{{{}}},
        out_total_init_maxgate_readout_times_{{{}}}, out_z_op_expects_{{{}}},
        executor_(std::make_shared<Executor>()), error_mitigations_{}, state_vec_{},
        in_get_state_vec_(false), measure_sample_sequentials_{{"auto"}} {
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
  session::session(const bool debug, const bool msb) : session() { debug_ = debug; out_counts_ordered_by_MSB_ = msb; }

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
    RandomQcircuit << "__qpu__ void qristal_circuit(qreg q) {" << std::endl
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
   *     [std::vector<int>] counts: the counts from a quantum simulation.
   *
   *     [std::vector<std::complex<double>>] amplitudes : the amplitudes for the
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
   *   D_KL(X||Y) = X' * (log(X)-log(Y))  : X,Y are column vectors.  Exclude all
   *elements of X or Y that are zero
   **/

  double session::get_jensen_shannon_divergence(const std::map<std::vector<bool>, int>& counts,
                                                const std::map<std::vector<bool>, std::complex<double>>& amplitudes) {
    double divergence = 0.0;
    double sum_counts = std::accumulate(counts.begin(), counts.end(), 0,
                         [](auto prev_sum, auto &entry) { return prev_sum + entry.second; });
    double d_pm, d_qm;
    for (const auto& [bits, amp] : amplitudes) {
      // Calculate the mixture vector element m(i) = 0.5*(p(i) + q(i))
      double p_i = counts.find(bits) == counts.end() ? 0 : counts.at(bits) / sum_counts;
      double q_i = std::norm(amp);
      double m_i = 0.5 * (p_i + q_i);
      // get the Kullback-Leibler divergence of probs p and q wrt m
      if (m_i > 0 && p_i > 0) d_pm += p_i * std::log(p_i/m_i);
      if (m_i > 0 && q_i > 0) d_qm += q_i * std::log(q_i/m_i);
    }

    return (0.5*d_pm) + (0.5*d_qm);
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

    if ((N_ii = singleton_or_eqlength(out_counts_, N_ii)) == INVALID) {
      throw std::range_error("[out_counts_] shape is invalid");
    }
    if ((N_ii = singleton_or_eqlength(expected_amplitudes_, N_ii)) == INVALID) {
      throw std::range_error("[expected_amplitudes] shape is invalid");
    }
    if ((N_ii = singleton_or_eqlength(acc_outputs_qbit0_left_, N_ii)) == INVALID) {
      throw std::range_error("[acc_outputs_qbit0_left] shape is invalid");
    }
    for (auto el : out_counts_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[out_counts_] shape is invalid");
      }
    }
    for (auto el : expected_amplitudes_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[expected_amplitudes] shape is invalid");
      }
    }
    for (auto el : acc_outputs_qbit0_left_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[acc_outputs_qbit0_left] shape is invalid");
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

    double jsdivergence = get_jensen_shannon_divergence(results_.at(ii).at(jj), expected_amplitudes_.at(ii).at(jj));
    std::map<int,double> jsdivergence_nd;
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
    if ((N_ii = singleton_or_eqlength(out_counts_, N_ii)) == INVALID) {
      throw std::range_error("[out_counts_] shape is invalid");
    }
    if ((N_ii = singleton_or_eqlength(expected_amplitudes_, N_ii)) == INVALID) {
      throw std::range_error("[expected_amplitudes] shape is invalid");
    }
    for (auto el : out_counts_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[out_counts_] shape is invalid");
      }
    }
    for (auto el : expected_amplitudes_) {
      if ((N_jj = singleton_or_eqlength(el, N_jj)) == INVALID) {
        throw std::range_error("[expected_amplitudes] shape is invalid");
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
      ValidatorTwoDim<size_t> sns_valid(sns_, session::SNS_LOWERBOUND, session::SNS_UPPERBOUND,
                                                 " number of shots [sn] ");
      if (sns_valid.is_data_empty())
      {
        throw std::range_error("Number of shots [sn] cannot be empty");
      }

      config.num_shots = sns_valid.get(ii, jj);
    }

    /// Number of qubits
    {
      ValidatorTwoDim<size_t> qns_valid(qns_, session::QNS_LOWERBOUND, session::QNS_UPPERBOUND,
                                                 " number of qubits [qn] ");
      if (qns_valid.is_data_empty())
      {
        throw std::range_error("Number of qubits [qn] cannot be empty");
      }
      config.num_qubits = qns_valid.get(ii, jj);
    }

    /// Enable out_counts calculation
    {
      ValidatorTwoDim<bool> calc_out_counts_valid(calc_out_counts_, false, true,
                                                 "enabled out_counts calculation [calc_out_counts]");
      if (calc_out_counts_valid.is_data_empty())
      {
        throw std::range_error("Cannot determine whether to calculate out_counts");
      }
      config.calc_out_counts = calc_out_counts_valid.get(ii, jj);
    }

    /// Enable gradient calculation for parametrized circuit
    {
      ValidatorTwoDim<bool> calc_jacobians_valid(calc_jacobians_, false, true,
                                                 "enabled jacobian calculation [calc_jacobian]");
      if (calc_jacobians_valid.is_data_empty())
      {
        throw std::range_error("Cannot determine whether to calculate gradients");
      }
      config.calc_jacobian = calc_jacobians_valid.get(ii, jj);
    }

    /// Enable output transpilation and resource estimation
    {
      ValidatorTwoDim<bool> output_oqm_enableds_valid(
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
      ValidatorTwoDim<std::string> accs_valid(accs_, VALID_ACCS, " name of back-end simulator [acc] ");
      if (accs_valid.is_data_empty())
      {
        throw std::range_error("A back-end simulator [acc] must be specified");
      }
      config.acc_name = accs_valid.get(ii, jj);
    }

    /// Qristal custom OpenQASM include file
    {
      ValidatorTwoDim<std::string> include_qbs_valid(include_qbs_);
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
      ValidatorTwoDim<bool> xasms_valid(xasms_, false, true, " interpret circuit in XASM format [xasm] ");
      if (xasms_valid.is_data_empty())
      {
        throw std::range_error("Flag for XASM [xasm] cannot be empty");
      }

      ValidatorTwoDim<bool> quil1s_valid(quil1s_, false, true,
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
      ValidatorTwoDim<bool> noplacements_valid(noplacements_, false, true,
                                                           " disable placement mapping [noplacement] ");
      if (noplacements_valid.is_data_empty())
      {
        throw std::range_error("Flag for disabling placement mapping [noplacement] cannot be empty");
      }
      config.no_placement = noplacements_valid.get(ii, jj);
    }

    /// Disable circuit optimisation flag
    {
      ValidatorTwoDim<bool> nooptimises_valid(nooptimises_, false, true,
                                                          " disable circuit optimiser [nooptimise] ");
      if (nooptimises_valid.is_data_empty())
      {
        throw std::range_error("Flag for disabling circuit optimiser [nooptimise] cannot be empty");
      }
      config.no_optimise = nooptimises_valid.get(ii, jj);
    }

    /// Skip simulation flag
    {
      ValidatorTwoDim<bool> nosims_valid(nosims_, false, true, " disable circuit simulator [nosim] ");
      if (nosims_valid.is_data_empty())
      {
        throw std::range_error("Flag for disabling circuit simulator [nosim] cannot be empty");
      }
      config.no_sim = nosims_valid.get(ii, jj);
    }

    /// Enable/disable noise flag
    {
      ValidatorTwoDim<bool> noises_valid(noises_, false, true, " enable noise modelling [noise] ");
      if (noises_valid.is_data_empty())
      {
        throw std::range_error("Enable noise modelling [noise] cannot be empty");
      }
      config.noise = noises_valid.get(ii, jj);
    }

    /// QB tensor network simulators initial bond dimension
    {
      ValidatorTwoDim<size_t> initial_bond_dimensions_valid(
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
      ValidatorTwoDim<size_t> initial_kraus_dimensions_valid(
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
      ValidatorTwoDim<size_t> max_bond_dimensions_valid(
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
      ValidatorTwoDim<size_t> max_kraus_dimensions_valid(
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
      std::map<int,double> svd_lowerbound{{0, -1.0e9}}; // This limit is currently ignored
      std::map<int,double> svd_upperbound{{0, 1.0e9}};  // This limit is currently ignored
      ValidatorTwoDim<std::map<int,double>> svd_cutoffs_valid(svd_cutoffs_, svd_lowerbound, svd_upperbound,
                                                         " MPS SVD cutoff [svd-cutoff] ");

      if (svd_cutoffs_valid.is_data_empty())
      {
        throw std::range_error("MPS SVD cutoff [svd-cutoff] cannot be empty");
      }
      config.svd_cutoff_tnqvm = svd_cutoffs_valid.get(ii, jj).at(0);
    }

    /// TNQVM-MPS and QB tensor network relative SVD cutoff limit
    {
      std::map<int,double> rel_svd_lowerbound{{0, -1.0e9}}; // This limit is currently ignored
      std::map<int,double> rel_svd_upperbound{{0, 1.0e9}};  // This limit is currently ignored
      ValidatorTwoDim<std::map<int,double>> rel_svd_cutoffs_valid(rel_svd_cutoffs_, rel_svd_lowerbound, rel_svd_upperbound,
                                                         " MPS relative SVD cutoff [svd-cutoff] ");

      if (rel_svd_cutoffs_valid.is_data_empty())
      {
        throw std::range_error("MPS relative SVD cutoff [rel-svd-cutoff] cannot be empty");
      }
      config.rel_svd_cutoff_tnqvm = rel_svd_cutoffs_valid.get(ii, jj).at(0);
    }

    /// QB tensor network accelerator measurement sampling method
    {
      ValidatorTwoDim<std::string> measure_sample_sequential_valid(
          measure_sample_sequentials_, VALID_MEASURE_SAMPLING_OPTIONS, " Measurement sampling method [measure-sample-sequential] ");
      if (measure_sample_sequential_valid.is_data_empty())
      {
        throw std::range_error("Measure sampling method [measure-sample-sequential] cannot be empty");
      }
      config.measure_sample_sequential = measure_sample_sequential_valid.get(ii, jj);
    }

    /// Choice of noise model
    {
      if (noise_models_[0].empty())
      {
        config.noise_model_owned = std::make_shared<NoiseModel>("default", config.num_qubits);
        config.noise_model = config.noise_model_owned.get();
      }
      else
      {
        ValidatorTwoDim<NoiseModel*> noise_models_valid(noise_models_);
        config.noise_model = noise_models_valid.get(ii, jj);
      }
    }

    /// Choice of noise mitigation
    {
      // There were error mitigation settings
      if (!error_mitigations_.empty()) {
        ValidatorTwoDim<std::string> error_mitigations_valid(
            error_mitigations_, VALID_ERROR_MITIGATIONS,
            " name of error mitigation module [error_mitigation] ");
        config.noise_mitigation = error_mitigations_valid.get(ii, jj);
      }
    }

    /// User-provided random seed
    {
      if (!seeds_.empty()) {
        ValidatorTwoDim<size_t> seeds_valid(
            seeds_, 0, std::numeric_limits<int>::max(), " random seed [seed] ");
        config.simulator_seed = seeds_valid.get(ii, jj);
      }
    }

    /// AER simulator type
    {
      if (!aer_sim_types_.empty()) {
        ValidatorTwoDim<std::string> aer_sim_types_valid(
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
    resize_for_i_j(acc_outputs_qbit0_left_, "acc_outputs_qbit0_left_");
    resize_for_i_j(acc_uses_n_bits_, "acc_uses_n_bits_");
    resize_for_i_j(results_, "results_");
    resize_for_i_j(expected_amplitudes_, "expected_amplitudes_");
    resize_for_i_j(out_probs_, "out_probs_");
    resize_for_i_j(out_counts_, "out_counts_");
    resize_for_i_j(out_prob_gradients_, "out_prob_gradients_");
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
      if (!xasm && !quil1 && acc_pre == "tnqvm") {
        // target_circuit = exatn_circuit_transpiler(target_circuit);
        if (debug_) {
          std::cout << "[debug]: No pre-transpiling will be performed:"
                    << std::endl
                    << target_circuit << std::endl;
        }
      } else if (!xasm && !quil1 && acc_pre == "aer") {
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

      // Qristal multi-control gates
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
  std::shared_ptr<xacc::Accelerator> session::get_sim_qpu(bool execute_on_hardware, run_i_j_config &run_config)
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
    std::string measure_sample_sequential = run_config.measure_sample_sequential;
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
      return std::make_shared<qristal::cudaq_acc>(acc);
    }
#endif

    // Load emulator if an emulator backend is selected.
    if (acc == "qb-mps" || acc == "qb-purification" || acc == "qb-mpdo" || (acc == "qsim" && noises)) {
      // Load emulator library
      static const char *EMULATOR_NOISE_MODEL_LIB_NAME = "libqristal_emulator.so";
      void *handle = dlopen(EMULATOR_NOISE_MODEL_LIB_NAME, RTLD_LOCAL | RTLD_LAZY);
      if (handle == NULL) {
        std::cout << "The accelerator you are searching for may be available in the Qristal Emulator plugin. Please see https://quantumbrilliance.com/quantum-brilliance-emulator." << std::endl;
      }
    }

    auto qpu = xacc::getAccelerator(acc, {{"seed", random_seed}});
    if (acc == "tnqvm") {
      xacc::set_verbose(false);
      qpu = xacc::getAccelerator(
          "tnqvm", {
                       std::make_pair("tnqvm-visitor", "exatn-mps"),
                       std::make_pair("max-bond-dim", max_bond_dimension),
                       std::make_pair("svd-cutoff", svd_cutoff)
                   });
    } else if (acc == "aer") {
      xacc::HeterogeneousMap aer_options{{"seed", random_seed}};

      // Omit shots if the state vector is requested. This triggers Xacc AER to use
      // the statevector simulation type instead of qasm simulation type. The former
      // then populates its ExecutionInfo::WaveFuncKey.
      if (!in_get_state_vec_) {
        aer_options.insert("shots", run_config.num_shots);
      } else if (in_get_state_vec_ == true && run_config.num_shots > 0) {
        std::cout << "Warning: Requesting AER state vector will ignore shot sampling!\n";
      }
      // If state vector is requested, check that the statevector backend is chosen;
      // throw an error otherwise
      if (in_get_state_vec_ and run_config.aer_sim_type != "statevector") {
        throw std::invalid_argument("Requesting the state vector data requires using the 'statevector' backend.");
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
        aer_options.insert("noise-model", run_config.noise_model->to_json());
        aer_options.insert("qobj-compiler",
                           run_config.noise_model->get_qobj_compiler());
        if (debug_)
          std::cout << "# Noise model: enabled" << std::endl;
      } else {
        if (debug_)
          std::cout << "# Noise model: disabled" << std::endl;
      }
      // Get AER and initialize it with proper settings.
      qpu = xacc::getAccelerator(acc, aer_options);
    } else if (acc == "qsim" && noises) {
      // Use qsim via Cirq wrapper to handle noise if requested.
      // The "cirq-qsim" backend is part of the external emulator package
      // to be used with the qb emulator noise models only.
      xacc::HeterogeneousMap qpu_options;
      if (debug_) {
        std::cout << "# Noise model for qsim (from emulator package): enabled" << std::endl;
      }
      qpu = xacc::getAccelerator("cirq-qsim", {{"noise-model", run_config.noise_model}});
    } else if (acc == "qb-mps" || acc == "qb-purification" || acc == "qb-mpdo") {
      xacc::HeterogeneousMap qpu_options {
        {"initial-bond-dim", initial_bond_dimension},
        {"max-bond-dim", max_bond_dimension},
        {"abs-truncation-threshold", svd_cutoff},
        {"rel-truncation-threshold", rel_svd_cutoff},
        {"measurement-sampling-sequential", measure_sample_sequential}};
      if (acc == "qb-purification") { // Additional options for qb-purification
        qpu_options.insert("initial-kraus-dim", initial_kraus_dimension);
        qpu_options.insert("max-kraus-dim", max_kraus_dimension);
      }

      if (noises) { // Use tensor network backend via xacc wrapper to handle noise if requested.
        if (debug_) {
          std::cout << "# Noise model for " << acc << " (from emulator package): enabled" << std::endl;
        }
        qpu_options.insert("noise-model", run_config.noise_model);
      }
      qpu = xacc::getAccelerator(acc, qpu_options);
    }

    return qpu;
  }

  /// Run (execute) task specified by the (ii, jj) index pair
  void session::run(const size_t ii, const size_t jj) {
    run_internal(ii, jj, /*acc = */ nullptr);
  }

  void session::validate_run() {

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
    results_.clear();
    results_.resize(N_ii);
    for (auto el : results_) {
      el.resize(N_jj);
    }
    out_probs_.clear();
    out_probs_.resize(N_ii);
    for (auto el : out_probs_) {
      el.resize(N_jj);
    }
    out_counts_.clear();
    out_counts_.resize(N_ii);
    for (auto el : out_counts_) {
      el.resize(N_jj);
    }
    out_prob_gradients_.clear();
    out_prob_gradients_.resize(N_ii);
    for (auto el : out_prob_gradients_) {
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
  }

  void session::run() {
    if (debug_) std::cout << "Invoked run()" << std::endl;
    const int N_ii = is_ii_consistent();
    const int N_jj = is_jj_consistent();
    validate_run();
    for (size_t ii = 0; ii < N_ii; ii++) {
      for (size_t jj = 0; jj < N_jj; jj++) {
        run(ii, jj);
      }
    }
  }

  void session::run_with_SPAM(size_t n_shots) {
    std::cerr << "╭────────────────────────────────────────────────────────╮" << std::endl;
    std::cerr << "│ Warning: Called run() with automatic SPAM measurement! │" << std::endl;
    std::cerr << "│        I will execute a new SPAM benchmark now!        │" << std::endl;
    std::cerr << "╰────────────────────────────────────────────────────────╯" << std::endl;
    //(1) create a copy of this session and set the numbers of shots 
    session sim_cp = *this;
    if (n_shots == 0) {
      n_shots = sns_[0][0];
    }
    sim_cp.set_sn(n_shots);

    //(2) execute and evaluate a SPAM benchmark
    std::set<size_t> qubits; 
    for (size_t q = 0; q < qns_[0][0]; ++q) {
      qubits.insert(q);
    }
    qristal::benchmark::SPAMBenchmark workflow(qubits, sim_cp);
    qristal::benchmark::ConfusionMatrix<qristal::benchmark::SPAMBenchmark> metric(workflow);
    auto confusion = metric.evaluate(true).begin()->second;

    //(3) enable automatic SPAM correction 
    set_SPAM_confusion_matrix(confusion);

    //(4) continue with normal run()
    run();
  }

  void session::run_gradients(const size_t ii, const size_t jj) {
    // Skip gradients if no counts have been returned by the backend
    if (out_counts_.at(ii).at(jj).empty()) return;
    // Initialize
    std::shared_ptr<xacc::CompositeInstruction> target_circuit = irtarget_ms_.at(ii).at(jj);
    std::vector<double> param_vals = parameter_vectors_.at(ii).at(jj);
    Table2d<std::shared_ptr<xacc::CompositeInstruction>> gradient_circs;
    size_t num_params = param_vals.size();
    if (target_circuit->nVariables() <= 0 or num_params != target_circuit->nVariables()) throw
     std::logic_error("This circuit is not parametrized correctly, so gradients cannot be calculated.");

    // Calculate param-shift gradient
    for (size_t i = 0; i < 2*num_params; i+=2) {
      std::vector<double> vals(param_vals);
      vals[i/2] += M_PI_2; // Increase by pi/2
      auto evaled_circ_plus = (*target_circuit)(vals);
      vals[i/2] -= M_PI; // Reduce by pi to get -pi/2 overall
      auto evaled_circ_minus = (*target_circuit)(vals);
      gradient_circs.push_back({evaled_circ_plus});
      gradient_circs.push_back({evaled_circ_minus});
    }

    // Create new session object to run shifted gradient circuits
    qristal::session gradient_sess(*this);
    gradient_sess.set_irtarget_ms(gradient_circs);
    gradient_sess.set_calc_jacobian(false);
    gradient_sess.set_calc_out_counts(true);
    gradient_sess.run();
    run_i_j_config run_config = get_run_config(ii, jj);
    size_t num_qubits = run_config.num_qubits;
    size_t num_shots = run_config.num_shots;
    size_t num_outputs = ipow(2, num_qubits);

    // Construct the jacobian
    Table2d<double>& jacobian = out_prob_gradients_.at(ii).at(jj);
    std::vector<double>& probs_all = out_probs_.at(ii).at(jj);
    try {
      jacobian.resize(num_params,std::vector<double>(num_outputs));
      probs_all.resize(num_outputs);
    }
    catch (std::exception& e) {
      throw std::logic_error("Your RAM use is too fragmented to allocate a large enough std::vector<double> to hold all relevant gradients.\n"
                             "Please use less circuit parameters, free up more memory, or use set_calc_jacobian(false).");
    }
    for (size_t i = 0; i < 2*num_params; i += 2) {
      // Find the bitstring indices that are non-zero in at least one run
      std::vector<int> counts_plus = gradient_sess.get_out_counts().at(i).at(0);
      std::vector<int> counts_minus = gradient_sess.get_out_counts().at(i+1).at(0);
      // Only loop over non-zero bitstrings and populate jacobian
      for (size_t j = 0; j < num_outputs; j++) {
        jacobian[i/2][j] = 0.5 * (counts_plus[j] - counts_minus[j]) / (1.0 * num_shots);
      }
    }
    for (size_t i = 0; i < num_outputs; i++) {
      probs_all.at(i) = out_counts_.at(ii).at(jj).at(i) / (1.0 * num_shots);
    }
  }

  void session::init() {
    if (debug_) {
      std::cout
          << "Setting defaults for Quantum Brilliance 12 qubit, 1024-shot, "
             "noiseless simulation, MPS bond dimension = 256, SVD cutoff = 1.0e-8"
          << std::endl;
    }
    set_qn(12);
    set_sn(1024);
    set_noise(false);
    set_initial_bond_dimension(1);
    set_initial_kraus_dimension(1);
    set_max_bond_dimension(256);
    set_max_kraus_dimension(256);
    std::map<int,double> scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    std::map<int,double> rel_scut{{0, 1.0e-4}};
    set_rel_svd_cutoff(rel_scut);
    set_output_oqm_enabled(true);
    set_measure_sample_sequential("auto");
    set_aer_sim_type("statevector");
  }

  void session::aws_setup(uint wn) {
    if (debug_) std::cout << "Setting AWS Braket defaults." << std::endl;
    set_noise(false);
    set_initial_bond_dimension(1);
    set_initial_kraus_dimension(1);
    set_max_bond_dimension(256);
    set_max_kraus_dimension(256);
    std::map<int,double> scut{{0, 1.0e-8}};
    set_svd_cutoff(scut);
    std::map<int,double> rel_scut{{0, 1.0e-4}};
    set_rel_svd_cutoff(rel_scut);
    set_output_oqm_enabled(true);
    set_acc("aws-braket");
    std::stringstream async_workers;
    async_workers << "{\"accs\": [";
    for (int iw=0; iw<(wn-1); iw++) {
        async_workers << "{\"acc\": \"aws-braket\"},";
    }
    async_workers << "{\"acc\": \"aws-braket\"}]}";
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
      const std::regex kernelSignatureRegex(R"(__qpu__[ \t]void[ \t]qristal_circuit\(qreg[ \t]([a-zA-Z_$][a-zA-Z_$0-9]*)\))");
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
          "CUDAQ is not supported. Please build qristal::core with CUDAQ.");
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
    const bool exec_on_hardware = run_config.acc_name != "aws-braket" and
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
    auto acc = std::make_shared<qristal::backend>();
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
        std::shared_ptr<xacc::CompositeInstruction> in_circ = irtarget_ms_.at(ii).at(0);
        if (in_circ->nVariables() > 0) {
          in_circ = in_circ->operator()(parameter_vectors_.at(ii).at(0));
        }
        citargets.push_back(in_circ);
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
          ValidatorTwoDim<std::string> placements_valid(
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
        if (run_config.acc_name == "aws-braket") {
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
            ValidatorTwoDim<Passes> opts_valid(circuit_opts_);
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
        if (qpu->name() == "aws-braket" && std::dynamic_pointer_cast<qristal::remote_accelerator>(qpu)) {
          // Asynchronously offload the circuit to AWS Braket
          auto as_remote_acc = std::dynamic_pointer_cast<qristal::remote_accelerator>(qpu);
          auto aws_job_handle = as_remote_acc->async_execute(citargets.at(0));
          aws_job_handle->add_done_callback([=](auto &handle) {
            auto buffer_temp = std::make_shared<xacc::AcceleratorBuffer>(buffer_b->size());
            handle.load_result(buffer_temp);
            auto qb_transpiler = std::make_shared<qristal::backend>();
            this->process_run_result(
                ii, jj, run_config, citargets.at(0), qpu, mqbacc, buffer_temp,
                timer_for_qpu.getDurationMs(), qb_transpiler);
          });
          return aws_job_handle;
        } else {
          // Blocking (synchronous) execution of a local simulator instance
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
      std::shared_ptr<qristal::backend> qb_transpiler) {
    if (debug_) {
      std::cout << std::endl;
      buffer_b->print();
      std::cout << std::endl;
      std::cout << "Walltime elapsed for the simulator to perform the "
                   "requested number of shots of the quantum circuit, in ms: "
                << xacc_scope_timer_qpu_ms << std::endl;
      std::cout << std::endl;
      std::cout << "Reported sim native bit order: [0=>qbit 0 left, 1=>qbit 0 right]: " << sim_qpu->getBitOrder() << std::endl;
      if (sim_qpu->name() == "tnqvm") std::cout << "Note that you are using TNQVM, and it reports the wrong native bit order." << std::endl;
    }

    // Store indicator of native bitstring pattern, correcting erroneous TNQVM value
    acc_outputs_qbit0_left_.at(ii).at(jj) = (sim_qpu->getBitOrder() == 0) or sim_qpu->name() == "tnqvm";

    // Keep the qobj so that a user can call Aer standalone later.
    if (sim_qpu->name() == "aer") {
      out_qobjs_.at(ii).at(jj) = sim_qpu->getNativeCode(ir_target, sim_qpu_configs);
    }

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
        // Save Z-operator expectation value to Table2d<std::map<int,double>>
        std::map<int,double> res_z{{0, z_expectation_val}};
        out_z_op_expects_.at(ii).at(jj) = res_z;
      } else {
        xacc::warning("No Z operator expectation available");
      }
    }

    // Get the state vector from qpp or AER
    if (in_get_state_vec_ and
       (sim_qpu->name() == "qpp" || (sim_qpu->name() == "aer" && run_config.aer_sim_type == "statevector"))) {
      state_vec_ = sim_qpu->getExecutionInfo<xacc::ExecutionInfo::WaveFuncPtrType>(
                        xacc::ExecutionInfo::WaveFuncKey);

      if (out_counts_ordered_by_MSB_) {
        std::vector<std::complex<double>> state_vec_tmp = *state_vec_;
        // Generate bitstring configurations
        const size_t num_qubits = std::log2(state_vec_->size());
        for (size_t i = 0; i < state_vec_->size(); i++) {
          auto bitstring = boost::dynamic_bitset<>(num_qubits, i);
          std::vector<bool> vb;
          for (size_t j = 0; j < bitstring.size(); j++) {
            vb.emplace_back(bitstring[j]);
          }
          state_vec_tmp[bitstring_index(vb)] = state_vec_->at(i);
        }
        *state_vec_ = state_vec_tmp;
      }
    }

    // Get counts
    const auto& counts_map = buffer_b->getMeasurementCounts();
    if (!run_config.no_sim) {
      // Save the counts to results_
      populate_measure_counts_data(ii, jj, counts_map);
      // If required to calculate gradients, do it now
      if (run_config.calc_jacobian) run_gradients(ii, jj);
    }

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

      // Save single qubit gate qtys to Table2d<std::map<int,int>>
      out_single_qubit_gate_qtys_.at(ii).at(jj) = timing_profile.get_count_1q_gates_on_q();

      // Save two-qubit gate qtys to Table2d<std::map<int,int>>
      out_double_qubit_gate_qtys_.at(ii).at(jj) = timing_profile.get_count_2q_gates_on_q();

      // Save timing results to Table2d<std::map<int,double>>
      out_total_init_maxgate_readout_times_.at(ii).at(jj) =
          timing_profile.get_total_initialisation_maxgate_readout_time_ms(xacc_scope_timer_qpu_ms, run_config.num_shots);
    }

    // Perform SPAM correction (if set) and store corrected results separately
    if (perform_SPAM_correction_) {
      std::cerr << "╭────────────────────────────────────────────────────────╮" << std::endl;
      std::cerr << "│     Warning: Automatic SPAM correction is enabled!     │" << std::endl;
      std::cerr << "│ `results` will be overwritten by SPAM-corrected counts │" << std::endl;
      std::cerr << "│  Native results can be retrieved from `results_native` │" << std::endl;
      std::cerr << "╰────────────────────────────────────────────────────────╯" << std::endl;
      results_native_ = results_; 
      //overwrite results_ with SPAM-corrected counts
      results_.clear();
      for (const auto& counts_list : results_native_) {
        std::vector<std::map<std::vector<bool>, int>> SPAM_corrected;
        for (const auto& counts: counts_list){
          SPAM_corrected.push_back(
            qristal::apply_SPAM_correction(counts, SPAM_correction_mat_)
          );
        }
        results_.push_back(SPAM_corrected);
      }
    }
  }

  // Wrap raw OpenQASM string in a QB Kernel:
  // - Move qreg to a kernel argument
  // - Denote the kernel name as 'qristal_circuit'
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
          "__qpu__ void qristal_circuit(qreg " + qRegName + ") {\n" + q2oqm + '}';
      return qbstr;
    }
    else
    {
      // There is no qreg declaration, e.g., an empty OpenQASM source with no
      // body. Just handle it graciously.
      const std::string qbstr =
          "__qpu__ void qristal_circuit(qreg q) {\n" + in_rawQasm + '}';
      return qbstr;
    }
  }

  // Convert a bit vector to an integer assuming either LSB or MSB encoding
  size_t session::bitstring_index(const std::vector<bool>& bitvec) {
    size_t result = 0;
    if (out_counts_ordered_by_MSB_) {
      for (uint i = 0; i < bitvec.size(); i++) if (bitvec[i]) result += 1<<(bitvec.size()-i-1);
    } else {
      for (uint i = 0; i < bitvec.size(); i++) if (bitvec[i]) result += 1<<i;
    }
    return result;
  }

  // Randomly draw (and remove) a single shot from the results map
  std::vector<bool> session::draw_shot(const size_t i, const size_t j) {

    // Save this as a private member variable after the i,j job array stuff has been removed
    size_t shots_remaining = std::accumulate(results_.at(i).at(j).begin(), results_.at(i).at(j).end(), 0, [](auto sum, auto& e) { return sum+e.second; });
    if (shots_remaining == 0) throw std::out_of_range("Unable to draw shot as no shots remain.");

    // RNG
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<> gen_shot_index(0, shots_remaining-1);

    // Randomly generate an integer representing the index of the shot to draw
    const size_t shot_index = gen_shot_index(rng);

    // Iterate through the results map entries until the drawn shot has been reached
    size_t sum = 0;
    std::vector<bool> bitvec;
    for (auto entry = results_.at(i).at(j).begin(); entry != results_.at(i).at(j).end(); ++entry) {
      assert(entry->second > 0);
      sum += entry->second;
      // If the drawn shot has been reached, save the bitvector and break the loop
      if (sum >= shot_index) {
        bitvec = entry->first;
        // If this was the last shot for this bitvector, remove it
        if (--(entry->second) == 0) results_.at(i).at(j).erase(entry);
        break;
      }
    }
    return bitvec;
  }

}

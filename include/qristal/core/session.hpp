// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

#include "qristal/core/async_executor.hpp"
#include "qristal/core/remote_async_accelerator.hpp"
#include "qristal/core/cmake_variables.hpp"
#include "qristal/core/utils.hpp"
#include "qristal/core/session_utils.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include "qristal/core/passes/base_pass.hpp"
// CUDAQ support
#ifdef WITH_CUDAQ
  #include "cudaq/utils/cudaq_utils.h"
#endif

// STL
#include <cmath>
#include <complex>
#include <functional>
#include <map>
#include <string>
#include <vector>

// YAML
#include "yaml-cpp/yaml.h"

// Forward declarations
namespace xacc::quantum { class qb_qpu; }
namespace qristal { class backend; }


namespace qristal
{

  /// A session of the Qristal SDK.
  class session
  {

    private:

      // Debugging
      bool debug_ = false;

      // Debugging
      bool out_counts_ordered_by_MSB_ = false;

      // Remote backend database
      std::string remote_backend_database_path_;
      YAML::Node remote_backend_database_;

      // Names
      Table2d<std::string> name_m;
      std::vector<std::vector<std::vector<int>>> number_m;
      Table2d<std::string> infiles_;
      Table2d<std::string> include_qbs_;
      Table2d<std::string> instrings_;
      std::vector<std::pair<std::string, std::function<void()>>> cudaq_kernels_;

      std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> irtarget_ms_;

      Table2d<std::string> accs_;

      Table2d<std::string> aer_sim_types_;

      Table2d<size_t> randoms_;
      Table2d<std::string> placements_;
      /// Circuit optimization passes to apply
      Table2d<Passes> circuit_opts_;
      Table2d<bool> xasms_;
      Table2d<bool> quil1s_;
      Table2d<bool> noplacements_;
      Table2d<bool> nooptimises_;
      Table2d<bool> nosims_;
      Table2d<bool> noises_;
      Table2d<bool> output_oqm_enableds_;
      Table2d<bool> log_enableds_;
      Table2d<bool> notimings_;
      Table2d<bool> calc_out_counts_;
      Table2d<bool> calc_jacobians_;

      Table2d<size_t> qns_;
      Table2d<size_t> rns_;
      Table2d<size_t> sns_;
      Table2d<size_t> seeds_;

      Table2d<std::map<int,double>> betas_;
      Table2d<std::map<int,double>> thetas_;

      Table2d<std::vector<double>> parameter_vectors_;

      // ExaTN-MPS and QB tensor network settings
      Table2d<size_t> max_bond_dimensions_;
      Table2d<size_t> initial_bond_dimensions_;
      Table2d<size_t> max_kraus_dimensions_;
      Table2d<size_t> initial_kraus_dimensions_;
      Table2d<std::map<int,double>> svd_cutoffs_;
      Table2d<std::map<int,double>> rel_svd_cutoffs_;
      Table2d<std::string> measure_sample_sequentials_;

      // Noise models
      std::vector<std::vector<NoiseModel>> noise_models_;

      // Variables not wrapped to Python
      Table2d<size_t> acc_uses_n_bits_;

      Table2d<std::map<std::vector<bool>, std::complex<double>>> expected_amplitudes_;

      // For storing results
      Table2d<std::map<std::vector<bool>, int>> results_;
      Table2d<std::vector<double>> out_probs_;
      Table2d<std::vector<int>> out_counts_;
      Table2d<std::map<int,double>> out_divergences_;
      Table2d<std::string> out_transpiled_circuits_;
      Table2d<std::string> out_qobjs_;
      Table2d<std::string> out_qbjsons_;
      Table2d<Table2d<double>> out_prob_gradients_;
      Table2d<bool> acc_outputs_qbit0_left_;
      //
      Table2d<std::map<int,int>> out_single_qubit_gate_qtys_;
      Table2d<std::map<int,int>> out_double_qubit_gate_qtys_;
      Table2d<std::map<int,double>> out_total_init_maxgate_readout_times_;
      Table2d<std::map<int,double>> out_z_op_expects_;

      // Parallel (async) executor
      std::shared_ptr<Executor> executor_;

      // State vector from qpp or aer
      bool in_get_state_vec_;
      std::shared_ptr<std::vector<std::complex<double>>> state_vec_;

      // Error mitigation
      Table2d<std::string> error_mitigations_;

      // Constants
      const int INVALID = -1;
      const int VALID = 0;

      /// Valid input types:
      enum class circuit_input_types { INVALID = -1, VALID_INSTRING_QPU = 1, VALID_RANDOM, VALID_INFILE, VALID_IR, VALID_CUDAQ };

      // Bounds
      const size_t RANDOMS_UPPERBOUND = 1000;
      const size_t SNS_LOWERBOUND = 1;
      const size_t SNS_UPPERBOUND = 1000000;
      const size_t QNS_LOWERBOUND = 1;
      const size_t QNS_UPPERBOUND = 10000;
      const size_t RNS_LOWERBOUND = 1;
      const size_t RNS_UPPERBOUND = 1000000;
      const size_t MAX_BOND_DIMENSION_LOWERBOUND = 1;
      const size_t MAX_BOND_DIMENSION_UPPERBOUND = 50000;
      const size_t INITIAL_BOND_DIMENSION_LOWERBOUND = 1;
      const size_t INITIAL_BOND_DIMENSION_UPPERBOUND = 50000;
      const size_t MAX_KRAUS_DIMENSION_LOWERBOUND = 1;
      const size_t MAX_KRAUS_DIMENSION_UPPERBOUND = 50000;
      const size_t INITIAL_KRAUS_DIMENSION_LOWERBOUND = 1;
      const size_t INITIAL_KRAUS_DIMENSION_UPPERBOUND = 50000;


      // Valid strings
      std::unordered_set<std::string> VALID_ACCS = {
          "aer",
          "aws-braket",
          "tnqvm",
          "qpp",
          "qsim",
          "loopback",
          "qb-lambda",
          "sparse-sim",
          "cirq-qsim",
          "qb-mps",
          "qb-purification",
          "qb-mpdo"
      };

      // Valid AER simulator types
      std::unordered_set<std::string> VALID_AER_SIM_TYPES = {
        "statevector",
        "density_matrix",
        "matrix_product_state"
      };

      std::unordered_set<std::string> VALID_ERROR_MITIGATIONS = {
        // Simple readout mitigation
        "ro-error",
        // Richardson extrapolation (to the zero noise level)
        "rich-extrap",
        // Readout mitigation by multiplying error assignment matrix
        // (inverse of the SPAM matrix)
        "assignment-error-kernel"
      };

      // Valid placements
      std::unordered_set<std::string> VALID_HARDWARE_PLACEMENTS = {
          "swap-shortest-path", "noise-aware"};

      // Valid measurement sampling options
      std::unordered_set<std::string> VALID_MEASURE_SAMPLING_OPTIONS = {
          "auto", "on", "off"};

    public:
      /**
       * @brief Construct a new session object
       *
       * Some parameters are uninitialized, e.g., number of qubits (`qns_`).
       * These parameters can be set manually (using corresponding setter methods) or via provided presets, e.g., init().
       */
      session();

      /**
       * @brief Construct a new session object with a specific name
       *
       * @param name Session name
       */
      session(const std::string &name);

      /**
       * @brief Construct a new session object with a specific debug flag
       *
       * @param debug Debug flag. Printing debug messages to console if true.
       */
      session(const bool debug);

      /**
       * @brief Construct a new session object with a specific debug flag, using
       * a specific ordering for out_counts and associated QML-relevant outputs.
       *
       * @param debug Debug flag. Printing debug messages to console if true.
       * @param msb   MSB flag. If true, use MSB to determine ordering of out_counts
       *              vector, out_probs vector and out_prob_gradients table; else use LSB.
       */
      session(const bool debug, const bool msb);

      // Setters and Getters

      /**
       * @brief Set the input QASM source file
       *
       * @param infile Full path to the source file
       */
      void set_infile(const std::string &infile);

      /**
       * @brief Set the list input QASM source files
       *
       * @param infiles A list of paths to source files
       */
      void set_infiles(const Table2d<std::string> &infiles);

      /**
       * @brief Get the list input QASM source files
       *
       * @return List input QASM source files
       */
      const Table2d<std::string> &get_infiles() const;

      //
      /**
       * @brief Set the input QASM source string.
       *
       * @param instring Input source string
       */
      void set_instring(const std::string &instring);
      /**
       * @brief Set the list of input QASM source strings.
       *
       * @param instrings Input source strings
       */
      void set_instrings(const Table2d<std::string> &instrings);
      /**
       * @brief Get the input QASM source strings of the session.
       *
       * @return List of source strings
       */
      const Table2d<std::string> &get_instrings() const;

      /**
       * @brief Set the irtarget (`xacc::CompositeInstruction`) object
       *
       * @note This `xacc::CompositeInstruction` can be manually constructed (i.e., building the IR tree using XACC).
       * If the irtarget is provided instead of QASM strings or files, the QASM compilation step will be skipped.
       *
       * @param irtarget_m Input IR object
       */
      void set_irtarget_m(const std::shared_ptr<xacc::CompositeInstruction> &irtarget_m);
      /**
       * @brief Set the list of irtarget (`xacc::CompositeInstruction`) objects
       *
       * @param irtarget_ms List of input IR objects
       */
      void set_irtarget_ms(const std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> &irtarget_ms);
      /**
       * @brief Get the list of input IR objects
       *
       * @return List of input IR objects
       */
      const std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> &get_irtarget_ms() const;

#ifdef WITH_CUDAQ
      /**
       * @brief Set the input CUDAQ kernel
       *
       * @param in_kernel Input CUDAQ kernel (templated callable returning void)
       */
      template <typename CudaQKernel, typename... Args>
      void set_cudaq_kernel(CudaQKernel &&in_kernel, Args &&...args) {
        const auto kernel_name = cudaq::getKernelName(in_kernel);
        std::function<void()> wrapped_kernel = [&in_kernel, ... args = std::forward<Args>(args)]() mutable {
          in_kernel(std::forward<Args>(args)...);
        };
        cudaq_kernels_.emplace_back(std::make_pair(kernel_name, std::move(wrapped_kernel)));
      }
#endif

      //

      //
      /**
       * @brief Set the path to the OpenQASM include file
       *
       * @param include_qb Path to the OpenQASM include file
       */
      void set_include_qb(const std::string &include_qb);
      /**
       * @brief Set the list of paths to the OpenQASM include files.
       *
       * @param include_qbs Paths to the OpenQASM include files
       */
      void set_include_qbs(const Table2d<std::string> &include_qbs);
      /**
       * @brief Get the list of paths to the OpenQASM include files.
       *
       * @return Paths to the OpenQASM include files
       */
      const Table2d<std::string> &get_include_qbs() const;

      /**
       * @brief Set the parameter values for execution.
       *
       * @param vals_vec New parameter vector for runtime substitution
       */
      void set_parameter_vector(const std::vector<double> &vals_vec);
      /**
       * @brief Set the list of parameter values for execution.
       *
       * @param vals_vecs New parameter vectors for runtime substitution
       */
      void set_parameter_vectors(Table2d<std::vector<double>> vals_vecs);
      /**
       * @brief Get the parameter values for runtime circuit execution.
       *
       * @return A vector-of-vectors of parameter values
       */
      const Table2d<std::vector<double>> &get_parameter_vectors() const;

      /**
       * @brief Determine whether jacobians will be calculated for parametrized circuits.
       *
       * @param calculate_gradients_m Whether to calculate jacobians for parametrized circuits
       */
      void set_calc_jacobian(bool calculate_gradients_m);
      /**
       * @brief Determine whether jacobians will be calculated for specific parametrized circuits.
       *
       * @param calculate_gradients_ms Whether to calculate jacobians for specific parametrized circuits
       */
      void set_calc_jacobians(Table2d<bool> calculate_gradients_ms);
      /**
       * @brief Get the jacobians calculation flags.
       *
       * @return A 1-d array of jacobians calculation flags
       */
      const Table2d<bool> &get_calc_jacobians() const;

      /**
       * @brief Determine whether non-compact output counts vector will be calculated.
       *
       * @param calculate_out_counts_m Whether to calculate non-compact output counts vector.
       */
      void set_calc_out_counts(bool calculate_out_counts_m);
      /**
       * @brief Determine whether non-compact output counts vector will be calculated for specific circuits.
       *
       * @param calculate_out_counts_ms Whether to calculate non-compact output counts vector for specific circuits.
       */
      void set_calc_out_countss(Table2d<bool> calculate_out_counts_ms);
      /**
       * @brief Get the non-compact counts vector calculation flags.
       *
       * @return A 1-d array of non-compact counts vector calculation flags
       */
      const Table2d<bool> &get_calc_out_counts() const;

      /**
       * @brief Set the path to the qpu config JSON file.
       * @brief Set the path to the remote backend database yaml file.
       *
       * @param remote_backend_database_path Path to the remote backend database yaml file.
       */
      void set_remote_backend_database_path(const std::string &remote_backend_database);
      /**
       * @brief Get the path to the remote backend database yaml file.
       *
       * @return Path to the remote backend database yaml file.
       */
      const std::string &get_remote_backend_database_path() const;

      /**
       * @brief Set the backend accelerator.
       *
       * @param acc Name of the accelerator
       */
      void set_acc(const std::string &acc);
      /**
       * @brief Set the list of backend accelerators.
       *
       * @param accs List of backend accelerator names
       */
      void set_accs(const Table2d<std::string> &accs);

      /**
       * @brief Get the list of backend accelerators.
       *
       * @return List of backend accelerator names
       */
      const Table2d<std::string> &get_accs() const;

      /**
       * @brief Set the AER backend simulator type
       *
       * @param sim_type Simulator type
       */
      void set_aer_sim_type(const std::string &sim_type);
      /**
       * @brief Set the AER backend simulator types
       *
       * @param sim_types Simulator type
       */
      void set_aer_sim_types(const Table2d<std::string> &sim_types);
      /**
       * @brief Get the AER backend simulator type
       *
       * @return Simulator type
       */
      const Table2d<std::string> &get_aer_sim_types() const;

      /**
       * @brief Set the depth of the auto-generated random circuit
       *
       * @param in_random Circuit depth
       */
      void set_random(const size_t &in_random);
      /**
       * @brief Set the depths of the auto-generated random circuits
       *
       * @param in_random Circuit depth values
       */
      void set_randoms(const Table2d<size_t> &in_random);
      /**
       * @brief Set the depths of the auto-generated random circuits
       *
       * @return Circuit depth values
       */
      const Table2d<size_t> &get_randoms() const;

      /**
       * @brief Set the XASM input flag
       *
       * True if the input is in XASM dialect.
       *
       * @param in_xasm XASM input flag
       */
      void set_xasm(const bool &in_xasm);
      /**
       * @brief Set the XASM input flags
       *
       * @param in_xasm XASM input flags
       */
      void set_xasms(const Table2d<bool> &in_xasm);
      /**
       * @brief Get the XASM input flag
       *
       * @return XASM input flags
       */
      const Table2d<bool> &get_xasms() const;

      /**
       * @brief Set the Quil input flag
       *
       * True if the input is in Quil (v1) dialect.
       *
       * @param in_quil1 Quil input flag
       */
      void set_quil1(const bool &in_quil1);
      /**
       * @brief Set the Quil input flags
       *
       * @param in_quil1 Quil input flags
       */
      void set_quil1s(const Table2d<bool> &in_quil1);
      /**
       * @brief Get the Quil input flags
       *
       * @return Quil input flags
       */
      const Table2d<bool> &get_quil1s() const;

      /**
       * @brief Set the noplacement flag
       *
       * True to disable circuit placement.
       *
       * @param in_noplacement noplacement flag
       */
      void set_noplacement(const bool &in_noplacement);
      /**
       * @brief Set the noplacement flags
       *
       * @param in_noplacement noplacement flags
       */
      void set_noplacements(const Table2d<bool> &in_noplacement);
      /**
       * @brief Get the noplacement flag
       *
       * @return noplacement flags
       */
      const Table2d<bool> &get_noplacements() const;

      /**
       * @brief Set the circuit placement method
       *
       * @param in_placement Name of the circuit placement module
       */
      void set_placement(const std::string &in_placement);
      /**
       * @brief Set the circuit placement methods
       *
       * @param in_placements Names of the circuit placement modules
       */
      void set_placements(const Table2d<std::string> &in_placements);
      /**
       * @brief Get the circuit placement methods
       *
       * @return Names of the circuit placement modules
       */
      const Table2d<std::string> &get_placements() const;

      /**
       * @brief Set the nooptimise flag
       *
       * True to disable circuit optimization
       *
       * @param in_nooptimise nooptimise flag
       */
      void set_nooptimise(const bool &in_nooptimise);
      /**
       * @brief Set the nooptimise flags
       *
       * @param in_nooptimise nooptimise flags
       */
      void set_nooptimises(const Table2d<bool> &in_nooptimise);
      /**
       * @brief Get the nooptimise flags
       *
       * @return nooptimise flags
       */
      const Table2d<bool> &get_nooptimises() const;

      /**
       * @brief Set the circuit optimization passes
       *
       * @param in_opts Sequence of optimization passes to apply
       */
      void set_circuit_opt(const Passes& in_passes);
      /**
       * @brief Set the circuit optimization passes
       *
       * @param in_opts 2-D table of sequences of optimization passes to apply
       */
      void set_circuit_opts(const Table2d<Passes> &in_passes);
      /**
       * @brief Get the circuit optimization passes
       *
       * @return 2-D table of sequences of optimization passes to apply
       */
      const Table2d<Passes> &get_circuit_opts() const;

      /**
       * @brief Set the nosim flag
       *
       * True to disable circuit simulation, e.g., dry-run to inspect transpilation and resource estimation only.
       *
       * @param in_nosim nosim flag
       */
      void set_nosim(const bool &in_nosim);
      /**
       * @brief Set the nosim flags
       *
       * @param in_nosim nosim flags
       */
      void set_nosims(const Table2d<bool> &in_nosim);
      /**
       * @brief Get the nosim flags
       *
       * @return nosim flags
       */
      const Table2d<bool> &get_nosims() const;

      /**
       * @brief Set the noise simulation flag
       *
       * True to enable noisy simulation.
       *
       * @param in_noise Noise flag
       */
      void set_noise(const bool &in_noise);
      /**
       * @brief Set the noise simulation flags
       *
       * @param in_noise Noise flags
       */
      void set_noises(const Table2d<bool> &in_noise);
      /**
       * @brief Get the noise simulation flags
       *
       * @return Noise flags
       */
      const Table2d<bool> &get_noises() const;

      /**
       * @brief Get the full state vector (works with QPP backend only!)
       *
       * @return Full complex state vector as std::vector<std::complex<double>>
       */
      const std::shared_ptr<std::vector<std::complex<double>>> &get_state_vec_raw() const;
      /**
       * @brief Set the flag to retrieve the state vector
       *
       * @param in_get_state_vec Flag to retrieve state vector (works with QPP backend only!)
       */
      void get_state_vec(const bool &in_get_state_vec);

      /**
       * @brief Set the output transpilation and resource estimation flag
       *
       * True to enable output transpilation and resource estimation
       *
       * @param in_output_oqm_enabled Config. value
       */
      void set_output_oqm_enabled(const bool &in_output_oqm_enabled);
      /**
       * @brief Set the output oqm enableds object
       *
       * @param in_output_oqm_enabled Config. values
       */
      void set_output_oqm_enableds(const Table2d<bool> &in_output_oqm_enabled);
      /**
       * @brief Get the output oqm enableds object
       *
       * @return Config. values
       */
      const Table2d<bool> &get_output_oqm_enableds() const;

      /// @private
      // This function is not being used.
      void set_log_enabled(const bool &in_log_enabled);
      /// @private
      void set_log_enableds(const Table2d<bool> &in_log_enabled);
      /// @private
      const Table2d<bool> &get_log_enableds() const;

      /**
       * @brief Set the notiming configuration flag
       *
       * @param in_notiming Config. value
       */
      void set_notiming(const bool &in_notiming);
      /**
       * @brief Set the notiming configuration flags
       *
       * @param in_notiming Config. values
       */
      void set_notimings(const Table2d<bool> &in_notiming);
      /**
       * @brief Get the notiming configuration flags
       *
       * @return Config. values
       */
      const Table2d<bool> &get_notimings() const;

      /**
       * @brief Set the number of qubits
       *
       * @param in_qn Number of qubits
       */
      void set_qn(const size_t &in_qn);
      /**
       * @brief Set the numbers of qubits
       *
       * @param in_qn Numbers of qubits
       */
      void set_qns(const Table2d<size_t> &in_qn);
      /**
       * @brief Get the numbers of qubits
       *
       * @return Number of qubits
       */
      const Table2d<size_t> &get_qns() const;

      /**
       * @brief Set the number of repetitions
       *
       * @param in_rn Number of repetitions
       */
      void set_rn(const size_t &in_rn);
      /**
       * @brief Set the numbers of repetitions
       *
       * @param in_rn Numbers of repetitions
       */
      void set_rns(const Table2d<size_t> &in_rn);
      /**
       * @brief Get the numbers of repetitions
       *
       * @return Numbers of repetitions
       */
      const Table2d<size_t> &get_rns() const;

      /**
       * @brief Set the number of measurement shots
       *
       * @param in_sn Number of shots
       */
      void set_sn(const size_t &in_sn);
      /**
       * @brief Set the number of measurement shots
       *
       * @param in_sn Number of shots
       */
      void set_sns(const Table2d<size_t> &in_sn);
      /**
       * @brief Get the number of measurement shots
       *
       * @return Number of shots
       */
      const Table2d<size_t> &get_sns() const;

      /// @private
      // unused
      void set_beta(const std::map<int,double> &in_beta);
      /// @private
      // unused
      void set_betas(const Table2d<std::map<int,double>> &in_beta);
      /// @private
      // unused
      const Table2d<std::map<int,double>> &get_betas() const;

      /**
       * @brief Set the angle variables (theta)
       *
       * @param in_theta Theta values
       */
      void set_theta(const std::map<int,double> &in_theta);
      /**
       * @brief Set the angle variables (theta)
       *
       * @param in_theta Theta values
       */
      void set_thetas(const Table2d<std::map<int,double>> &in_theta);
      /**
       * @brief Get  the angle variables (theta)
       *
       * @return Theta values
       */
      const Table2d<std::map<int,double>> &get_thetas() const;

      /**
       * @brief Set the initial bond dimension (MPS simulator)
       * @note This is only needed if using the "tnqvm" backend accelerator.
       *
       * @param in_initial_bond_dimension Initial MPS bond dimension value
       */
      void set_initial_bond_dimension(const size_t &in_initial_bond_dimension);
      /**
       * @brief Set the initial bond dimension (MPS simulator)
       *
       * @param in_initial_bond_dimension Initial MPS bond dimension value
       */
      void set_initial_bond_dimensions(const Table2d<size_t> &in_initial_bond_dimension);
      /**
       * @brief Get the initial bond dimension (MPS simulator)
       *
       * @return Initial MPS bond dimension value
       */
      const Table2d<size_t> &get_initial_bond_dimensions() const;

      /**
       * @brief Set the initial kraus dimension (MPS simulator)
       * @note This is only needed if using the "tnqvm" backend accelerator.
       *
       * @param in_initial_kraus_dimension Initial MPS kraus dimension value
       */
      void set_initial_kraus_dimension(const size_t &in_initial_kraus_dimension);
      /**
       * @brief Set the initial kraus dimension (MPS simulator)
       *
       * @param in_initial_kraus_dimension Initial MPS kraus dimension value
       */
      void set_initial_kraus_dimensions(const Table2d<size_t> &in_initial_kraus_dimension);
      /**
       * @brief Get the initial kraus dimension (MPS simulator)
       *
       * @return Initial MPS kraus dimension value
       */
      const Table2d<size_t> &get_initial_kraus_dimensions() const;

      /**
       * @brief Set the maximum bond dimension (MPS simulator)
       * @note This is only needed if using the "tnqvm" backend accelerator.
       *
       * @param in_max_bond_dimension Max MPS bond dimension value
       */
      void set_max_bond_dimension(const size_t &in_max_bond_dimension);
      /**
       * @brief Set the maximum bond dimension (MPS simulator)
       *
       * @param in_max_bond_dimension Max MPS bond dimension value
       */
      void set_max_bond_dimensions(const Table2d<size_t> &in_max_bond_dimension);
      /**
       * @brief Get the maximum bond dimension (MPS simulator)
       *
       * @return Max MPS bond dimension value
       */
      const Table2d<size_t> &get_max_bond_dimensions() const;

      /**
       * @brief Set the maximum kraus dimension (MPS simulator)
       * @note This is only needed if using the "tnqvm" backend accelerator.
       *
       * @param in_max_kraus_dimension Max MPS kraus dimension value
       */
      void set_max_kraus_dimension(const size_t &in_max_kraus_dimension);
      /**
       * @brief Set the maximum kraus dimension (MPS simulator)
       *
       * @param in_max_kraus_dimension Max MPS kraus dimension value
       */
      void set_max_kraus_dimensions(const Table2d<size_t> &in_max_kraus_dimension);
      /**
       * @brief Get the maximum kraus dimension (MPS simulator)
       *
       * @return Max MPS kraus dimension value
       */
      const Table2d<size_t> &get_max_kraus_dimensions() const;

      /**
       * @brief Set the SVD cutoff limit (MPS simulator)
       * @note This is only needed if using the "tnqvm" backend accelerator.
       *
       * @param in_svd_cutoff SVD cutoff value
       */
      void set_svd_cutoff(const std::map<int,double> &in_svd_cutoff);
      /**
       * @brief Set the SVD cutoff limit (MPS simulator)
       *
       * @param in_svd_cutoff SVD cutoff value
       */
      void set_svd_cutoffs(const Table2d<std::map<int,double>> &in_svd_cutoff);
      /**
       * @brief Get the SVD cutoff limit (MPS simulator)
       *
       * @return SVD cutoff value
       */
      const Table2d<std::map<int,double>> &get_svd_cutoffs() const;

      /**
       * @brief Set the relative SVD cutoff limit (MPS simulator)
       * @note This is only needed if using the "tnqvm" backend accelerator.
       *
       * @param in_rel_svd_cutoff SVD cutoff value
       */
      void set_rel_svd_cutoff(const std::map<int,double> &in_rel_svd_cutoff);
      /**
       * @brief Set the relative SVD cutoff limit (MPS simulator)
       *
       * @param in_rel_svd_cutoff SVD cutoff value
       */
      void set_rel_svd_cutoffs(const Table2d<std::map<int,double>> &in_rel_svd_cutoff);
      /**
       * @brief Get the relative SVD cutoff limit (MPS simulator)
       *
       * @return Relative SVD cutoff value
       */
      const Table2d<std::map<int,double>> &get_rel_svd_cutoffs() const;

      /**
       * @brief Set the measurement sampling method - "off" uses the cutensorNet
       * contraction method of the entire tensor network state. Program terminates
       * with error meassage if cutensorNet fails.
       * "on" uses the cutensor sequential contraction method.
       * "auto" (default) uses the cutensorNet contraction method and automatically
       * swithes to the cutensor sequential contraction method if the cutensorNet
       * method fails.
       * @note This is only needed if using the emulator tensor network accelerator
       *
       * @param in_measure_sample_sequential Measure sampling option value
       */
      void set_measure_sample_sequential(const std::string &in_measure_sample_sequential);
      /**
       * @brief Set the measurement sampling methods
       *
       * @param in_measure_sample_sequential Measure sampling option values
       */
      void set_measure_sample_sequentials(const Table2d<std::string> &in_measure_sample_sequential);
      /**
       * @brief Get the measurement sampling method
       *
       * @return Measure sampling option values
       */
      const Table2d<std::string> &get_measure_sample_sequentials() const;

      /**
       * @brief Set the noise model
       *
       * @param model The noise model to use
       */
      void set_noise_model(const NoiseModel& model);
      /**
       * @brief Set the noise models
       *
       * @param models The noise models to use
       */
      void set_noise_models(const std::vector<std::vector<NoiseModel>>& noise_models);
      /**
       * @brief Get the noise models
       *
       * @return The noise models to use
       */
      const std::vector<std::vector<NoiseModel>>& get_noise_models() const;

      /**
       * @brief Set the expected amplitudes for Jensen–Shannon divergence calculation
       *
       * @param amp Amplitude values
       */
      void set_expected_amplitudes(const std::map<std::vector<bool>, std::complex<double>> &amp);
      /**
       * @brief Set the expected amplitudes for Jensen–Shannon divergence calculation
       *
       * @param amp Amplitude values
       */
      void set_expected_amplitudess(const std::vector<std::vector<std::map<
          std::vector<bool>, std::complex<double>>>> &amp);
      /**
       * @brief Get the expected amplitudes for Jensen–Shannon divergence calculation
       *
       * @return Amplitude values
       */
      const std::vector<std::vector<std::map<std::vector<bool>, std::complex<double>>>> &get_expected_amplitudes() const;

      /**
       * @brief Set the debug flag (verbose logging)
       *
       * @param debug Config. value
       */
      void set_debug(const bool &debug);
      /**
       * @brief Get the debug flag
       *
       * @return Config. value
       */
      const bool &get_debug() const;

      /**
       * @brief Get the output measurement counts as a map
       *
       * @return Measurement counts map
       */
      const Table2d<std::map<std::vector<bool>,int>> &results() const;

      /**
       * @brief Get the output measurement counts as a vector
       *
       * @return Measurement counts vector
       */
      const Table2d<std::vector<int>> &get_out_counts() const;

      /**
       * @brief Get the output probabilities
       *
       * @return Measurement probabilities vector
       */
      const Table2d<std::vector<double>> &get_out_probs() const;

      /**
       * @brief Get the output probability gradients
       *
       * @return Table of probability jacobians w.r.t. runtime parameters
       */
      const Table2d<Table2d<double>> &get_out_prob_jacobians() const;

      /**
       * @brief Get the output Jensen–Shannon divergence results
       *
       * @return Divergence results
       */
      const Table2d<std::map<int,double>> &get_out_divergences() const;

      /**
       * @brief Get the output transpiled circuits
       *
       * @return Output transpiled circuits
       */
      const Table2d<std::string> &get_out_transpiled_circuits() const;

      /**
       * @brief Get the output QObj Json strings
       *
       * @return QObj Json strings
       */
      const Table2d<std::string> &get_out_qobjs() const;

      /**
       * @brief Get the output QB Json strings (hardware execution)
       *
       * @return QB Json strings
       */
      const Table2d<std::string> &get_out_qbjsons() const;

      /**
       * @brief Get the output single-qubit gate counts
       *
       * @return Single-qubit gate counts
       */
      const Table2d<std::map<int,int>> & get_out_single_qubit_gate_qtys() const;

      /**
       * @brief Get the output two-qubit gate counts
       *
       * @return Two-qubit gate counts
       */
      const Table2d<std::map<int,int>> & get_out_double_qubit_gate_qtys() const ;

      /**
       * @brief Get the output total circuit execution time (hardware runtime estimation)
       *
       * @return Estimated hardware runtime.
       */
      const Table2d<std::map<int,double>> & get_out_total_init_maxgate_readout_times() const;

      /**
       * @brief Get the output expected value in the Z basis
       *
       * @return expected value in the Z basis
       */
      const Table2d<std::map<int,double>> & get_out_z_op_expects() const;

      /**
       * @brief Set the noise mitigation method
       *
       * @param noise_mitigate Noise mitigation method
       */
      void set_noise_mitigation(const std::string &noise_mitigate);
      /**
       * @brief Set the noise mitigation methods
       *
       * @param noise_mitigates Noise mitigation methods
       */
      void set_noise_mitigations(const Table2d<std::string> &noise_mitigates);

      /**
       * @brief Get the noise mitigation methods
       *
       * @return Noise mitigation methods
       */
      const Table2d<std::string> &get_noise_mitigations() const;

      /**
       * @brief Set the random seed value
       *
       * @param in_seed Seed value
       */
      void set_seed(const size_t &in_seed);
      /**
       * @brief Set random seed values
       *
       * @param in_seeds Seed values
       */
      void set_seeds(const Table2d<size_t> &in_seeds);
      /**
       * @brief Get random seed values
       *
       * @return Seed values
       */
      const Table2d<size_t> &get_seeds() const;

      /**
       * @brief Get the summary of all session configurations
       *
       * @return Session configuration summary
       */
      const std::string get_summary() const;

      /**
       * @brief Set the names of tasks
       *
       * @param name_ Task names
       */
      void setName(const Table2d<std::string> &name_);
      /**
       * @brief Set the name of task
       *
       * @param name_ Task name
       */
      void setName(const std::string &name_);
      /**
       * @brief Get names of tasks
       *
       * @return Task names
       */
      const Table2d<std::string> &getName() const;

      /**
       * @brief Compute the Jensen-Shannon divergence result for the (ii, jj) result
       *
       * @param ii Row index
       * @param jj Column index
       */
      void get_jensen_shannon(const size_t &ii, const size_t &jj);
      /**
       * @brief Compute all the Jensen-Shannon divergence results
       *
       */
      void get_jensen_shannon();

      /// Run a quantum task at the (ii, jj) index in the experiment table.
      void run(const size_t ii, const size_t jj);
      /// Validate the run i.e. ensure all configurations are set in a
      /// valid manner.
      void validate_run();
      /**
       * @brief Execute all quantum tasks
       *
       */
      void run();

      /// Set the multi-qpu run configurations:
      /// e.g., the list of QPUs paricipate in this run
      void set_parallel_run_config(const std::string &in_config);
      /// Threadsafe execution of (ii, jj) task using the provided accelerator.
      /// Returns a job handle if the job is posted to a remote accelerator (e.g. AWS Braket).
      /// Otherwise, returns null if this function completes the run locally.
      std::shared_ptr<async_job_handle> run_async(const std::size_t ii, const std::size_t jj,
                     std::shared_ptr<xacc::Accelerator> acc);

      /// Get the QPU pool executor.
      Executor& get_executor();
      /// Shortcuts for setting defaults
      /// 12 qubits, 1024 shots, noiseless
      void init();
      /**
       * @brief AWS defaults
       *
       * @param wn Number of asynchronous workers
       */
      void aws_setup(uint wn);

      /**
       * @brief Returns the (base-10) integer vector index for the probabilities/
       * counts vector, corresponding to a bitstring for the quantum experiment
       * at (ii, jj).
       *
       * @param bitvec The bit-vector to be converted to the vector index
       */
      inline size_t bitstring_index(const std::vector<bool>& bitvec);

    private:
      int validate_sns_nonempty();
      int validate_qns_nonempty();
      int validate_rns_nonempty();
      int validate_thetas_option();
      int validate_instrings();
      void validate_acc(const std::string &acc);
      void validate_noise_mitigation(const std::string &noise_mitigate);
      void validate_aer_sim_type(const std::string &sim_type);
      void validate_measure_sample_options(const std::string &measure_sample_options);
      int is_ii_consistent();
      int is_jj_consistent();
      circuit_input_types validate_infiles_instrings_randoms_irtarget_ms_nonempty(const size_t ii, const size_t jj);

      // Methods
      std::string random_circuit(const int n_q, const int depth);

      double get_jensen_shannon_divergence(const std::map<std::vector<bool>, int>& counts,
                                           const std::map<std::vector<bool>, std::complex<double>>& amplitudes);

      std::string aer_circuit_transpiler(std::string &circuit);
      /// Ensure that all result tables are resized/expanded to accommodate (ii, jj) experiment index.
      void ensure_results_table_size(size_t ii, size_t jj);
      /// Retrieve and validate run configurations for index pair (ii, jj) using the table index convention.
      run_i_j_config get_run_config(size_t ii, size_t jj);
      /// Helper to populate result tables (e.g. counts, expectation values, resource estimations) post-execution.
      void process_run_result(const std::size_t ii, const std::size_t jj, const run_i_j_config& run_config,
                              std::shared_ptr<xacc::CompositeInstruction> kernel_ir,
                              std::shared_ptr<xacc::Accelerator> sim_qpu,
                              const xacc::HeterogeneousMap& sim_qpu_configs,
                              std::shared_ptr<xacc::AcceleratorBuffer> buffer_b, double runtime_ms,
                              std::shared_ptr<qristal::backend> qb_transpiler);
      /// Util method to compile input source string into IR
      /// This method is thread-safe, thus can be used to compile multiple source strings in parallel.
      std::shared_ptr<xacc::CompositeInstruction> compile_input(const std::string& in_source_string, int in_num_qubits, source_string_type in_source_type);

      /// Retrieve the target circuit string for (i, j) task:
      /// This will involve loading file (if file mode is selected), generate random circuit string (if random mode is selected), etc.
      std::string get_target_circuit_qasm_string(size_t ii, size_t jj, const run_i_j_config& run_config);
      /// Wrap raw OpenQASM string in a QB Kernel:
      /// - Move qreg to a kernel argument
      /// - Denote the kernel name as 'qristal_circuit'
      static std::string
      convertRawOpenQasmToQBKernel(const std::string &in_rawQasm);

      /// Get the simulator based on `run_i_j_config`
      std::shared_ptr<xacc::Accelerator> get_sim_qpu(bool execute_on_hardware, run_i_j_config& run_config);

      /// Calculate the gradients for the parametrized quantum task at the
      /// (ii, jj) index in the experiment table.
      void run_gradients(const size_t ii, const size_t jj);

      /// Execute the circuit on a simulator
      void execute_on_simulator(
          std::shared_ptr<xacc::Accelerator> acc,
          std::shared_ptr<xacc::AcceleratorBuffer> buffer_b,
          std::vector<std::shared_ptr<xacc::CompositeInstruction>> &circuits,
          const run_i_j_config &run_config);

      /// Internal (ii, jj) task execution.
      /// acc: if given (not null), this will be used for simulation. Otherwise, constructed from the run configuration for (ii, jj).
      /// optional_mutex: if not null, performed locking as appropriate to make this execution thread safe (e.g., accessing data members of this session instance).
      std::shared_ptr<async_job_handle> run_internal(const std::size_t ii, const std::size_t jj,
                     std::shared_ptr<xacc::Accelerator> acc, std::mutex* optional_mutex = nullptr);
#ifdef WITH_CUDAQ
      // Run CUDAQ kernel assigned as (i, j) task of this session
      void run_cudaq(size_t ii, size_t jj, const run_i_j_config &run_config);
#endif

      /// Populate QPU execution results for task (i, j) to the session data
      /// Templated measure_counts_map to support different type of map-like data.
      template <typename CountMapT>
      void populate_measure_counts_data(size_t ii, size_t jj, const CountMapT& measure_counts_map) {

        if (measure_counts_map.empty()) {
          if (debug_) std::cout << "Zero counts returned!" << std::endl;
          return;
        }
        run_i_j_config run_config = get_run_config(ii, jj);
        size_t num_qubits = run_config.num_qubits;
        size_t num_shots = run_config.num_shots;

        // Check that the number of qubits is set correctly
        int qbits_meas = measure_counts_map.begin()->first.length();
        if (qbits_meas > num_qubits) {
          throw std::logic_error("Not enough qubits! Set qn to at least " + std::to_string(qbits_meas));
        }

        // Convert count map keys from strings with assumed endianness and directionality to std::vector<bool>, and save results
        results_.at(ii).at(jj).clear();
        for (const auto& [bitstring, count] : measure_counts_map) {
          std::vector<bool> bitvector(qbits_meas);
          for (int i = 0; i < qbits_meas; i++) {
            int j = acc_outputs_qbit0_left_.at(ii).at(jj) ? i : qbits_meas - (i+1);
            bitvector.at(j) = (bitstring.at(i) != '0');
          }
          results_.at(ii).at(jj)[bitvector] = count;
        }

        // If requested, save the results to the out_counts vector for computing gradients
        if (run_config.calc_out_counts) {
          const double num_entries = std::pow(2, num_qubits);
          double scalefactor = 1;
          // We need 2^nq ints for out_counts plus (potentially) num_params * 2^nq doubles for jacobians + 2^nq doubles for probs
          if (run_config.calc_jacobian) {
            int num_params = parameter_vectors_.at(ii).at(jj).size();
            scalefactor += (num_params + 1) * double(sizeof(num_entries))/sizeof(num_params);
          }
          auto& counts = out_counts_.at(ii).at(jj);
          counts = std::vector<int>(0);
          // Check that there is enough free memory to store everything
          if (counts.max_size() > scalefactor*num_entries)
          {
            try { counts.resize((long long int)num_entries); }
            catch(std::exception& e) {
              std::string err = "You RAM use is too fragmented to allocate a large enough "
               "std::vector<int> to hold integer bitstring representations.\nPlease free up more memory, use set_calc_out_counts(false),";
              if (run_config.calc_jacobian) err += ", use less circuit parameters, or use set_calc_jacobian(false)";
              err += ".";
              throw std::logic_error(err);
            }
            for (const auto &[bitvec, count] : results_.at(ii).at(jj)) {
              counts[bitstring_index(bitvec)] = count;
            }
          }
          else {
            std::ostringstream err;
            err << "There are too many " << num_qubits << "-bit bitstrings to fit in a "
                << "std::vector<int> in integer representation on this system for this circuit." << std::endl
                << "Maximum qubits that can be fitted in memory for this circuit: " << (int)std::floor(std::log(counts.max_size()/scalefactor)/std::log(2)) << "."
                << std::endl << "Please use set_calc_out_counts(false)";
            if (run_config.calc_jacobian) err << ", use less circuit parameters, or use set_calc_jacobian(false)";
            std::cout << ".";
            throw std::logic_error(err.str());
          }
        }
      }
  };

}

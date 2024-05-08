// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

#include "qb/core/async_executor.hpp"
#include "qb/core/remote_async_accelerator.hpp"
#include "qb/core/cmake_variables.hpp"
#include "qb/core/utils.hpp"
#include "qb/core/session_utils.hpp"
#include "qb/core/noise_model/noise_model.hpp"
#include "qb/core/passes/base_pass.hpp"

// CUDAQ support
#ifdef WITH_CUDAQ
  #include "cudaq/utils/cudaq_utils.h"
#endif

// STL
#include <complex>
#include <functional>
#include <map>
#include <string>
#include <vector>

// YAML
#include "yaml-cpp/yaml.h"

// Forward declarations
namespace xacc::quantum { class qb_qpu; }
namespace qb { class backend; }


namespace qb
{

  /// A session of the QB SDK quantum programming and execution framework.
  class session
  {

    private:

      // Debugging
      bool debug_;

      // Remote backend database
      std::string remote_backend_database_path_;
      YAML::Node remote_backend_database_;

      // Names
      VectorString name_m;
      std::vector<std::vector<std::vector<int>>> number_m;
      VectorString infiles_;
      VectorString include_qbs_;
      VectorString instrings_;
      std::vector<std::pair<std::string, std::function<void()>>> cudaq_kernels_;

      std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> irtarget_ms_;

      VectorString accs_;

      VectorString aer_sim_types_;

      VectorN randoms_;
      VectorString placements_;
      /// Circuit optimization passes to apply
      Table2d<Passes> circuit_opts_;
      VectorBool xasms_;
      VectorBool quil1s_;
      VectorBool noplacements_;
      VectorBool nooptimises_;
      VectorBool nosims_;
      VectorBool noises_;
      VectorBool output_oqm_enableds_;
      VectorBool log_enableds_;
      VectorBool notimings_;

      VectorN qns_;
      VectorN rns_;
      VectorN sns_;
      VectorN seeds_;

      VectorMapND betas_;
      VectorMapND thetas_;

      // ExaTN-MPS and QB tensor network settings
      VectorN max_bond_dimensions_;
      VectorN initial_bond_dimensions_;
      VectorN max_kraus_dimensions_;
      VectorN initial_kraus_dimensions_;
      VectorMapND svd_cutoffs_;
      VectorMapND rel_svd_cutoffs_;
      VectorString measure_sample_sequentials_;

      // Noise models
      std::vector<std::vector<NoiseModel>> noise_models_;

      // Variables not wrapped to Python
      VectorBool acc_uses_lsbs_;
      VectorN acc_uses_n_bits_;

      std::vector<std::vector<std::map<std::string, std::complex<double>>>> output_amplitudes_;

      // For storing results
      VectorString out_raws_;
      std::vector<std::vector<std::map<std::string, int>>> out_bitstrings_;
      VectorMapND out_divergences_;
      VectorString out_transpiled_circuits_;
      VectorString out_qobjs_;
      VectorString out_qbjsons_;
      //
      VectorMapNN out_single_qubit_gate_qtys_;
      VectorMapNN out_double_qubit_gate_qtys_;
      VectorMapND out_total_init_maxgate_readout_times_;
      VectorMapND out_z_op_expects_;

      // Parallel (async) executor
      std::shared_ptr<Executor> executor_;

      // State vector from qpp
      bool in_get_state_vec_;
      std::shared_ptr<std::vector<std::complex<double>>> state_vec_;

      // Error mitigation
      VectorString error_mitigations_;

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
          "aws_acc",
          "tnqvm",
          "qpp",
          "qsim",
          "dqc_gen1",
          "qdk_gen1",
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
       * These parameters can be set manually (using corresponding setter methods) or via provided presets, e.g., qb12().
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

      // Setters and Getters
      // Notes:
      //  - setters and getters implementation should be defined in python_module/core/methods_getter_setter.cpp
      //  - help strings (Python) should be defined in python_module/core/methods_py_help_strings.cpp
      
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
      void set_infiles(const VectorString &infiles);
      
      /**
       * @brief Get the list input QASM source files
       * 
       * @return List input QASM source files
       */
      const VectorString &get_infiles() const;
      
      /// @private
      static const char *help_infiles_;
      
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
      void set_instrings(const VectorString &instrings);
      /**
       * @brief Get the input QASM source strings of the session.
       * 
       * @return List of source strings
       */
      const VectorString &get_instrings() const;
      /// @private
      static const char *help_instrings_;
      //
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
      /// @private
      static const char *help_irtarget_ms_;

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
      void set_include_qbs(const VectorString &include_qbs);
      /**
       * @brief Get the list of paths to the OpenQASM include files.
       * 
       * @return Paths to the OpenQASM include files
       */
      const VectorString &get_include_qbs() const;
      /// @private
      static const char *help_include_qbs_;
      //
      /**
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
      /// @private
      static const char *help_remote_backend_database_path_;
      //
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
      void set_accs(const VectorString &accs);
      
      /**
       * @brief Get the list of backend accelerators. 
       * 
       * @return List of backend accelerator names 
       */
      const VectorString &get_accs() const;
      /// @private
      static const char *help_accs_;
      //      
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
      void set_aer_sim_types(const VectorString &sim_types);
      /**
       * @brief Get the AER backend simulator type
       * 
       * @return Simulator type
       */
      const VectorString &get_aer_sim_types() const;
      /// @private
      static const char *help_aer_sim_types_;
      //
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
      void set_randoms(const VectorN &in_random);
      /**
       * @brief Set the depths of the auto-generated random circuits
       * 
       * @return Circuit depth values
       */
      const VectorN &get_randoms() const;
      /// @private
      static const char *help_randoms_;
      //
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
      void set_xasms(const VectorBool &in_xasm);
      /**
       * @brief Get the XASM input flag
       * 
       * @return XASM input flags
       */
      const VectorBool &get_xasms() const;
      /// @private
      static const char *help_xasms_;
      //
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
      void set_quil1s(const VectorBool &in_quil1);
      /**
       * @brief Get the Quil input flags
       * 
       * @return Quil input flags
       */
      const VectorBool &get_quil1s() const;
      /// @private
      static const char *help_quil1s_;
      //
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
      void set_noplacements(const VectorBool &in_noplacement);
      /**
       * @brief Get the noplacement flag
       * 
       * @return noplacement flags 
       */
      const VectorBool &get_noplacements() const;
      /// @private
      static const char *help_noplacements_;
      //
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
      void set_placements(const VectorString &in_placements);
      /**
       * @brief Get the circuit placement methods
       * 
       * @return Names of the circuit placement modules
       */
      const VectorString &get_placements() const;
      /// @private
      static const char *help_placements_;
      //
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
      void set_nooptimises(const VectorBool &in_nooptimise);
      /**
       * @brief Get the nooptimise flags
       * 
       * @return nooptimise flags 
       */
      const VectorBool &get_nooptimises() const;
      /// @private
      static const char *help_nooptimises_;
      
      //
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
      /// @private
      static const char *help_circuit_opts_;
      //
      
      //
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
      void set_nosims(const VectorBool &in_nosim);
      /**
       * @brief Get the nosim flags
       * 
       * @return nosim flags  
       */
      const VectorBool &get_nosims() const;
      /// @private
      static const char *help_nosims_;
      //
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
      void set_noises(const VectorBool &in_noise);
      /**
       * @brief Get the noise simulation flags
       * 
       * @return Noise flags 
       */
      const VectorBool &get_noises() const;
      /// @private
      static const char *help_noises_;
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
      /// @private
      static const char* help_state_vec_;
      //
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
      void set_output_oqm_enableds(const VectorBool &in_output_oqm_enabled);
      /**
       * @brief Get the output oqm enableds object
       * 
       * @return Config. values 
       */
      const VectorBool &get_output_oqm_enableds() const;
      /// @private
      static const char *help_output_oqm_enableds_;
      //
      /// @private 
      // This function is not being used.
      void set_log_enabled(const bool &in_log_enabled);
      /// @private 
      void set_log_enableds(const VectorBool &in_log_enabled);
      /// @private 
      const VectorBool &get_log_enableds() const;
      /// @private
      static const char *help_log_enableds_;
      //
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
      void set_notimings(const VectorBool &in_notiming);
      /**
       * @brief Get the notiming configuration flags
       * 
       * @return Config. values
       */
      const VectorBool &get_notimings() const;
      /// @private
      static const char *help_notimings_;
      //
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
      void set_qns(const VectorN &in_qn);
      /**
       * @brief Get the numbers of qubits
       * 
       * @return Number of qubits 
       */
      const VectorN &get_qns() const;
      /// @private
      static const char *help_qns_;
      //
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
      void set_rns(const VectorN &in_rn);
      /**
       * @brief Get the numbers of repetitions
       * 
       * @return Numbers of repetitions
       */
      const VectorN &get_rns() const;
      /// @private
      static const char *help_rns_;
      //
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
      void set_sns(const VectorN &in_sn);
      /**
       * @brief Get the number of measurement shots
       * 
       * @return Number of shots
       */
      const VectorN &get_sns() const;
      /// @private
      static const char *help_sns_;
      //
      /// @private
      // unused
      void set_beta(const ND &in_beta);
      /// @private
      // unused
      void set_betas(const VectorMapND &in_beta);
      /// @private
      // unused
      const VectorMapND &get_betas() const;
      /// @private
      static const char *help_betas_;
      //
      /**
       * @brief Set the angle variables (theta)
       * 
       * @param in_theta Theta values
       */
      void set_theta(const ND &in_theta);
      /**
       * @brief Set the angle variables (theta)
       * 
       * @param in_theta Theta values
       */
      void set_thetas(const VectorMapND &in_theta);
      /**
       * @brief Get  the angle variables (theta)
       * 
       * @return Theta values
       */
      const VectorMapND &get_thetas() const;
      /// @private
      static const char *help_thetas_;
      //
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
      void set_initial_bond_dimensions(const VectorN &in_initial_bond_dimension);
      /**
       * @brief Get the initial bond dimension (MPS simulator)
       * 
       * @return Initial MPS bond dimension value
       */
      const VectorN &get_initial_bond_dimensions() const;
      /// @private
      static const char *help_initial_bond_dimensions_;
      //
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
      void set_initial_kraus_dimensions(const VectorN &in_initial_kraus_dimension);
      /**
       * @brief Get the initial kraus dimension (MPS simulator)
       * 
       * @return Initial MPS kraus dimension value
       */
      const VectorN &get_initial_kraus_dimensions() const;
      /// @private
      static const char *help_initial_kraus_dimensions_;
      //
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
      void set_max_bond_dimensions(const VectorN &in_max_bond_dimension);
      /**
       * @brief Get the maximum bond dimension (MPS simulator)
       * 
       * @return Max MPS bond dimension value
       */
      const VectorN &get_max_bond_dimensions() const;
      /// @private
      static const char *help_max_bond_dimensions_;
      //
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
      void set_max_kraus_dimensions(const VectorN &in_max_kraus_dimension);
      /**
       * @brief Get the maximum kraus dimension (MPS simulator)
       * 
       * @return Max MPS kraus dimension value
       */
      const VectorN &get_max_kraus_dimensions() const;
      /// @private
      static const char *help_max_kraus_dimensions_;
      //
      /**
       * @brief Set the SVD cutoff limit (MPS simulator)
       * @note This is only needed if using the "tnqvm" backend accelerator.
       * 
       * @param in_svd_cutoff SVD cutoff value
       */
      void set_svd_cutoff(const ND &in_svd_cutoff);
      /**
       * @brief Set the SVD cutoff limit (MPS simulator)
       * 
       * @param in_svd_cutoff SVD cutoff value
       */
      void set_svd_cutoffs(const VectorMapND &in_svd_cutoff);
      /**
       * @brief Get the SVD cutoff limit (MPS simulator)
       * 
       * @return SVD cutoff value
       */
      const VectorMapND &get_svd_cutoffs() const;
      /// @private
      static const char *help_svd_cutoffs_;
      //
      /**
       * @brief Set the relative SVD cutoff limit (MPS simulator)
       * @note This is only needed if using the "tnqvm" backend accelerator.
       * 
       * @param in_rel_svd_cutoff SVD cutoff value
       */
      void set_rel_svd_cutoff(const ND &in_rel_svd_cutoff);
      /**
       * @brief Set the relative SVD cutoff limit (MPS simulator)
       * 
       * @param in_rel_svd_cutoff SVD cutoff value
       */
      void set_rel_svd_cutoffs(const VectorMapND &in_rel_svd_cutoff);
      /**
       * @brief Get the relative SVD cutoff limit (MPS simulator)
       * 
       * @return Relative SVD cutoff value
       */
      const VectorMapND &get_rel_svd_cutoffs() const;
      /// @private
      static const char *help_rel_svd_cutoffs_;
      //
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
      void set_measure_sample_sequentials(const VectorString &in_measure_sample_sequential);
      /**
       * @brief Get the measurement sampling method
       * 
       * @return Measure sampling option values
       */
      const VectorString &get_measure_sample_sequentials() const;
      /// @private
      static const char *help_measure_sample_sequentials_;
      //
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
      /// @private
      static const char *help_noise_models_;
      //
      /**
       * @brief Set the amplitudes for Jensen–Shannon divergence calculation
       * 
       * @param in_output_amplitude Amplitude values
       */
      void set_output_amplitude(const std::map<std::string, std::complex<double>> &in_output_amplitude);
      /**
       * @brief Set the amplitudes for Jensen–Shannon divergence calculation
       * 
       * @param in_output_amplitude Amplitude values
       */
      void set_output_amplitudes(const std::vector<std::vector<std::map<
          std::string, std::complex<double>>>> &in_output_amplitude);
      /**
       * @brief Get the amplitudes for Jensen–Shannon divergence calculation
       * 
       * @return Amplitude values
       */
      const std::vector<std::vector<std::map<std::string, std::complex<double>>>> &get_output_amplitudes() const;
      /// @private
      static const char *help_output_amplitudes_;
      //
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
      /// @private
      static const char *help_debug_;
      //
      
      /**
       * @brief Get the raw output bitstring results
       * 
       * @return Bitstring results 
       */
      const VectorString &get_out_raws() const;
      /// @private
      static const char *help_out_raws_;
      //
      /**
       * @brief Get the output measurement counts 
       * 
       * @return Measurement count map
       */
      const std::vector<std::vector<std::map<std::string, int>>> &get_out_bitstrings() const;
      /// @private
      static const char *help_out_bitstrings_;
      //
      /**
       * @brief Get the output Jensen–Shannon divergence results
       * 
       * @return Divergence results
       */
      const VectorMapND &get_out_divergences() const;
      /// @private
      static const char *help_out_divergences_;
      
      /**
       * @brief Get the output transpiled circuits
       * 
       * @return Output transpiled circuits
       */
      const VectorString &get_out_transpiled_circuits() const;
      /// @private
      static const char *help_out_transpiled_circuits_;
      //
      /**
       * @brief Get the output QObj Json strings
       * 
       * @return QObj Json strings
       */
      const VectorString &get_out_qobjs() const;
      /// @private
      static const char *help_out_qobjs_;
      //
      /**
       * @brief Get the output QB Json strings (hardware execution)
       * 
       * @return QB Json strings
       */
      const VectorString &get_out_qbjsons() const;
      /// @private
      static const char *help_out_qbjsons_;

      //
      /**
       * @brief Get the output single-qubit gate counts
       * 
       * @return Single-qubit gate counts 
       */
      const VectorMapNN & get_out_single_qubit_gate_qtys() const;
      /// @private
      static const char* help_out_single_qubit_gate_qtys_;
      //
      /**
       * @brief Get the output two-qubit gate counts
       * 
       * @return Two-qubit gate counts 
       */
      const VectorMapNN & get_out_double_qubit_gate_qtys() const ;
      /// @private
      static const char* help_out_double_qubit_gate_qtys_;
      //
      /**
       * @brief Get the output total circuit execution time (hardware runtime estimation)
       * 
       * @return Estimated hardware runtime.
       */
      const VectorMapND & get_out_total_init_maxgate_readout_times() const;
      /// @private
      static const char* help_out_total_init_maxgate_readout_times_;
      //
      /**
       * @brief Get the output expected value in the Z basis
       * 
       * @return expected value in the Z basis
       */
      const VectorMapND & get_out_z_op_expects() const;
      /// @private
      static const char* help_out_z_op_expects_;
      //
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
      void set_noise_mitigations(const VectorString &noise_mitigates);
      
      /**
       * @brief Get the noise mitigation methods
       * 
       * @return Noise mitigation methods
       */
      const VectorString &get_noise_mitigations() const;
      /// @private
      static const char *help_noise_mitigations_;
      //
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
      void set_seeds(const VectorN &in_seeds);
      /**
       * @brief Get random seed values
       * 
       * @return Seed values
       */
      const VectorN &get_seeds() const;
      /// @private
      static const char *help_seeds_;
      //

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
      void setName(const VectorString &name_);
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
      const VectorString &getName() const;

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
      void qb12();
      /// AWS Braket DM1, 32 async workers, 17 qubits, 256 shots, noiseless
      void aws32dm1();
      /// AWS Braket SV1, 32 async workers, 34 qubits, 256 shots, noiseless 
      void aws32sv1();
      /// AWS Braket TN1, 8 async workers, 49 qubits, 256 shots, noiseless
      void aws8tn1();

          
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

      double get_jensen_shannon_divergence(const std::map<std::string, int> &in_q,
                                           const std::map<std::string, std::complex<double>> &in_p);

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
                              std::shared_ptr<qb::backend> qb_transpiler);
      /// Util method to compile input source string into IR
      /// This method is thread-safe, thus can be used to compile multiple source strings in parallel.
      std::shared_ptr<xacc::CompositeInstruction> compile_input(const std::string& in_source_string, int in_num_qubits, source_string_type in_source_type);

      /// Retrieve the target circuit string for (i, j) task:
      /// This will involve loading file (if file mode is selected), generate random circuit string (if random mode is selected), etc.
      std::string get_target_circuit_qasm_string(size_t ii, size_t jj, const run_i_j_config& run_config);
      /// Wrap raw OpenQASM string in a QB Kernel:
      /// - Move qreg to a kernel argument
      /// - Denote the kernel name as 'QBCIRCUIT'
      static std::string
      convertRawOpenQasmToQBKernel(const std::string &in_rawQasm);
      
      /// Get the simulator based on `run_i_j_config`
      std::shared_ptr<xacc::Accelerator> get_sim_qpu(bool execute_on_hardware, const run_i_j_config& run_config);

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
      void populate_measure_counts_data(size_t ii, size_t jj,
                                        const CountMapT &measure_counts_map) {
        // Save the counts to a string-to-int map
        std::map<std::string, int> keystring_all;
        for (const auto &[bit_string, count] : measure_counts_map) {
          std::string keystring = bit_string;
          if (acc_uses_lsbs_.at(ii).at(jj)) {
            // 0 => LSB
            std::reverse(keystring.begin(), keystring.end());
          }
          if (keystring.size() < 32) {
            keystring_all.insert(std::make_pair(keystring, count));
          } else {
            if (debug_) {
              std::cout << "Cannot represent bitstring as integer. Please use "
                           ".out_raw method instead.\n";
            }
            break;
          }
        }

        // Store bitstring and measured counts
        out_bitstrings_.at(ii).at(jj) = keystring_all;

        // Save results to JSON
        nlohmann::json qpu_counts_js = measure_counts_map;
        out_raws_.at(ii).at(jj) = qpu_counts_js.dump(4);
      }
  };

} // namespace qb

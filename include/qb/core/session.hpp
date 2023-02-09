// Copyright (c) 2021 Quantum Brilliance Pty Ltd

#include "qb/core/QuantumBrillianceAccelerator.hpp"
#include "qb/core/QuantumBrillianceRemoteAccelerator.hpp"
#include "qb/core/async_executor.hpp"
#include "qb/core/remote_async_accelerator.hpp"
#include "qb/core/cmake_variables.hpp"
#include "qb/core/utils.hpp"
#include "qb/core/profiler.hpp"
#include "qb/core/pretranspiler.hpp"

#pragma once

namespace qb
{

  /// A session of the QB SDK quantum programming and execution framework.
  class session
  {

    private:

      // Debugging
      bool debug_;

      // Names
      VectorString name_m;
      std::vector<std::vector<std::vector<int>>> number_m;
      VectorString infiles_;
      VectorString include_qbs_;
      VectorString qpu_configs_;
      VectorString instrings_;

      std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> irtarget_ms_;

      VectorString accs_;

      // AWS Braket related members
      VectorString aws_device_names_;  // AWS Braket hosted simulator or hosted hardware QPU to run circuits on.  For validation, see: VALID_AWS_DEVICES.  Has effect only when accs_ == "aws_acc".
      VectorString aws_formats_;
      VectorBool aws_verbatims_;       // Verbatim mode on AWS Braket hardware QPUs (Rigetti)
      VectorString aws_s3s_;           // Name of S3 Bucket that will store AWS Braket results.  For validation, see: VALID_AWS_S3_PREFIXS.  Has effect only when accs_ == "aws_acc".
      VectorString aws_s3_paths_;      // Path inside S3 Bucket where AWS Braket results are kept.  Automatically created if neccessary.  Has effect only when accs_ == "aws_acc".

      VectorString aer_sim_types_;

      VectorN randoms_;
      VectorString placements_;

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

      /// QB hardware configuration: contrast thresholds (non-negative real number)
      /// The balanced SSR contrast below which a shot will be ignored.  
      /// init_contrast_thresholds_ applies during initialisation.
      /// qubit_contrast_thresholds_ applies on a per-qubit basis during final readout
      /// use_default_contrast_settings_ excludes all contrast threshold settings from being sent to the QB hardware
      VectorBool use_default_contrast_settings_;
      VectorMapND init_contrast_thresholds_;
      VectorMapND qubit_contrast_thresholds_;

      // ExaTN-MPS settings
      VectorN max_bond_dimensions_;
      VectorMapND svd_cutoffs_;

      // Noise models
      VectorString noise_models_;    // In-built noise model versions.  For validation, see VALID_NOISE_MODEL_NAMES.

      // Variables not wrapped to Python
      VectorBool acc_uses_lsbs_;
      VectorN acc_uses_n_bits_;

      VectorMapNC output_amplitudes_;

      // For storing results
      VectorString out_raws_;
      VectorMapNN out_counts_;
      VectorMapND out_divergences_;
      VectorString out_transpiled_circuits_;
      VectorString out_qobjs_;
      VectorString out_qbjsons_;
      //
      VectorMapNN out_single_qubit_gate_qtys_;
      VectorMapNN out_double_qubit_gate_qtys_;
      VectorMapND out_total_init_maxgate_readout_times_;
      VectorMapND out_z_op_expects_;



      // Thread locking
      std::mutex m_;
      // Parallel (async) executor
      std::shared_ptr<Executor> executor_;

      // Error mitigation
      VectorString error_mitigations_;

      // Constants
      const int INVALID = -1;
      const int VALID = 0;

      /// Valid input types:
      enum class circuit_input_types { INVALID = -1, VALID_INSTRING_QPU = 1, VALID_RANDOM, VALID_INFILE, VALID_IR };

      // Bounds
      const int RANDOMS_UPPERBOUND = 1000;
      const size_t SNS_LOWERBOUND = 1;
      const size_t SNS_UPPERBOUND = 1000000;
      const size_t SNS_DM1_LOWERBOUND = 1;
      const size_t SNS_DM1_UPPERBOUND = 10000;
      const size_t SNS_SV1_LOWERBOUND = 1;
      const size_t SNS_SV1_UPPERBOUND = 10000;
      const size_t SNS_TN1_LOWERBOUND = 1;
      const size_t SNS_TN1_UPPERBOUND = 999;
      const size_t QNS_LOWERBOUND = 1;
      const size_t QNS_UPPERBOUND = 10000;
      const size_t QNS_DM1_LOWERBOUND = 1;
      const size_t QNS_DM1_UPPERBOUND = 17;
      const size_t QNS_SV1_LOWERBOUND = 1;
      const size_t QNS_SV1_UPPERBOUND = 34;
      const size_t QNS_TN1_LOWERBOUND = 1;
      const size_t QNS_TN1_UPPERBOUND = 48;
      const size_t RNS_LOWERBOUND = 1;
      const size_t RNS_UPPERBOUND = 1000000;
      const size_t MAX_BOND_DIMENSION_LOWERBOUND = 1;
      const size_t MAX_BOND_DIMENSION_UPPERBOUND = 50000;


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
          "sparse-sim"
      };

      std::unordered_set<std::string> VALID_QB_HARDWARE_ACCS = {
          "dqc_gen1",
          "qdk_gen1",
          "loopback"
      };

      // Valid AWS backend strings
      std::unordered_set<std::string> VALID_AWS_DEVICES = {
          "SV1",
          "DM1",
          "TN1",
          "LocalSimulator",
          "Rigetti"
      };

      // Valid AWS backend formats
      std::unordered_set<std::string> VALID_AWS_FORMATS = {
          "braket",
          "openqasm3"
      };

      // Valid AWS Braket S3 bucket name prefixs
      std::unordered_set<std::string> VALID_AWS_S3_PREFIXS = {
          "amazon-braket-"
      };

      // Valid AER simulator types
      std::unordered_set<std::string> VALID_AER_SIM_TYPES = {
        "statevector",
        "density_matrix",
        "matrix_product_state"
      };

      std::unordered_set<std::string> VALID_QB_HARDWARE_COMMANDS = {
          "circuit"
      };

      std::map<std::string,std::string> VALID_QB_HARDWARE_URLS = {
          {"dqc_gen1", "https://172.17.0.1:8443"},
          {"qdk_gen1", "https://172.17.0.1:8443"},
          {"loopback", "http://127.0.0.1:8000"}
      };

      std::map<std::string,std::string> VALID_QB_HARDWARE_POSTPATHS = {
          {"dqc_gen1", ""},
          {"qdk_gen1", ""}
      };

      std::map<std::string,int> VALID_QB_HARDWARE_POLLING_SECS = {
          {"dqc_gen1", 3},
          {"qdk_gen1", 3}
      };

      std::map<std::string,int> VALID_QB_HARDWARE_POLLING_RETRY_LIMITS = {
          {"dqc_gen1", 10},
          {"qdk_gen1", 10}
      };

      std::map<std::string,int> VALID_QB_HARDWARE_OVER_REQUEST_FACTORS = {
          {"dqc_gen1", 4},
          {"qdk_gen1", 4}
      };

      std::map<std::string,bool> VALID_QB_HARDWARE_RESAMPLE_ENABLEDS = {
          {"dqc_gen1", false},
          {"qdk_gen1", false}
      };

      std::map<std::string,bool> VALID_QB_HARDWARE_RECURSIVE_REQUEST_ENABLEDS = {
          {"dqc_gen1", true},
          {"qdk_gen1", true}
      };

      std::map<std::string,int> VALID_QB_HARDWARE_RESAMPLE_ABOVE_PERCENTAGES = {
          {"dqc_gen1", 95},
          {"qdk_gen1", 95},
          {"loopback", 95}
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
          "swap-shortest-path", "tket"};

      // Valid noise_model values
      std::unordered_set<std::string> VALID_NOISE_MODEL_NAMES = {
        "default",
        "qb-nm1",
        "qb-nm2"
      };

      // More QB hardware options (similar to the above) go here
      const double CONTRAST_UPPERBOUND = 1.0;
      const double CONTRAST_LOWERBOUND = 0.0;

      const int VALID_QB_HARDWARE_INIT = 0;
      const int VALID_QB_HARDWARE_REQUEST_ID = 0;
      const int VALID_QB_HARDWARE_POLL_ID = 0;
      const int VALID_QB_HARDWARE_CYCLES = 1;

      const int POLLING_NOT_READY = 300;
      const int POLLING_PROCESS_FAILED = 500;
      const int POLLING_SUCCESS = 0;

    public:
      /**
       * @brief Construct a new session object
       * 
       * Some parameters are uninitialized, e.g., number of qubits (`qns_`).
       * These parameters can be set manually (using corresponding setter methods) or via provided presets, e.g., qb12().
       */
      session()
          : debug_(false), name_m{{}}, number_m{{}}, infiles_{{}},
            // FIXME some tests will probably fail if the SDK_SOURCE_DIR path to qblib.inc and qpu_config.json is not also added.
            include_qbs_{{SDK_DIR "/include/qb/core/qblib.inc"}},
            qpu_configs_{{SDK_DIR "/include/qb/core/qpu_config.json"}},
            instrings_{{}}, irtarget_ms_{{}},
            accs_{{"qpp"}},
            aws_device_names_{{"DM1"}}, aws_formats_{{"openqasm3"}}, aws_verbatims_{{{false}}},
            aws_s3s_{{"amazon-braket-QBSDK"}}, aws_s3_paths_{{"output"}},
            aer_sim_types_{},
            randoms_{{}}, xasms_{{{false}}},
            quil1s_{{{false}}}, noplacements_{{{false}}},
            nooptimises_{{{true}}}, nosims_{{{false}}}, noises_{{{false}}},
            output_oqm_enableds_{{{false}}}, log_enableds_{{{false}}},
            notimings_{{{false}}}, qns_{{}}, rns_{{}}, sns_{{}}, betas_{{}},
            thetas_{{}},
            use_default_contrast_settings_{{{true}}},
            init_contrast_thresholds_{{{}}},
            // init_contrast_thresholds_{{{std::pair<int,double>(0,0.1)}}},
            qubit_contrast_thresholds_{{{}}},
            // qubit_contrast_thresholds_{{{std::pair<int,double>(0,0.1),std::pair<int,double>(1,0.1)}}},
            max_bond_dimensions_{{}}, svd_cutoffs_{{}}, noise_models_{{"default"}},
            acc_uses_lsbs_{{{}}}, acc_uses_n_bits_{{{}}},
            output_amplitudes_{{}}, out_raws_{{{}}}, out_counts_{{{}}},
            out_divergences_{{{}}}, out_transpiled_circuits_{{{}}},
            out_qobjs_{{{}}}, out_qbjsons_{{{}}},
            out_single_qubit_gate_qtys_{{{}}}, out_double_qubit_gate_qtys_{{{}}},
            out_total_init_maxgate_readout_times_{{{}}}, out_z_op_expects_{{{}}},
            executor_(std::make_shared<Executor>()), error_mitigations_{} {
        xacc::Initialize();
        xacc::setIsPyApi();
        xacc::set_verbose(debug_);
      }
      
      /**
       * @brief Construct a new session object with a specific name
       * 
       * @param name Session name
       */
      session(const std::string &name) : session() { name_m.push_back({name}); }
      
      /**
       * @brief Construct a new session object with a specific debug flag
       * 
       * @param debug Debug flag. Printing debug messages to console if true.
       */
      session(const bool debug) : session() { debug_ = debug; }

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
       * @brief Set the path to the qpu config JSON file.
       * 
       * @param qpu_config Path to the qpu config JSON file
       */
      void set_qpu_config(const std::string &qpu_config);
      /**
       * @brief Set the list of paths to the qpu config JSON files.
       * 
       * @param qpu_configs List of paths to the qpu config JSON files
       */
      void set_qpu_configs(const VectorString &qpu_configs);
      /**
       * @brief Get the list of paths to the qpu config JSON files.
       * 
       * @return List of paths to the qpu config JSON files
       */
      const VectorString &get_qpu_configs() const;
      /// @private
      static const char *help_qpu_configs_;
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
       * @brief Set the AWS Braket device name, e.g., SV1, DM1, etc.
       * 
       * @param device_name Name of AWS Braket device
       */
      void set_aws_device_name(const std::string &device_name);
      /**
       * @brief Set AWS Braket device names.
       * 
       * @param device_names Names of AWS Braket devices
       */
      void set_aws_device_names(const VectorString &device_names);
      /**
       * @brief Get AWS Braket device names.
       * 
       * @return Names of AWS Braket devices
       */
      const VectorString &get_aws_device_names() const;
      /// @private
      static const char *help_aws_device_names_;
      //
      /**
       * @brief Set the format to submit circuits to AWS Braket  
       * 
       * @param format Format type (e.g., openqasm3)
       */
      void set_aws_format(const std::string &format);
      /**
       * @brief Set the format to submit circuits to AWS Braket  
       * 
       * @param formats List of format types (e.g., openqasm3)
       */
      void set_aws_formats(const VectorString &formats);
      /**
       * @brief Get format to submit circuits to AWS Braket 
       * 
       * @return List of format types
       */
      const VectorString &get_aws_formats() const;
      /// @private
      static const char *help_aws_formats_;
      //
      /**
       * @brief Set the AWS S3 bucket to store results
       * 
       * @param bucket_name Name of S3 bucket
       */
      void set_aws_s3(const std::string &bucket_name);       // Setter for scalar case. Used by pybind11.
      /**
       * @brief Set the AWS S3 bucket to store results
       * 
       * @param bucket_names Names of S3 bucket
       */
      void set_aws_s3s(const VectorString &bucket_names);    // Setter for vector case. User by pybind11.
      /**
       * @brief Get the AWS S3 bucket to store results
       * 
       * @return Names of S3 bucket
       */
      const VectorString &get_aws_s3s() const;               // Getter that works for scalar and vector cases
      /// @private
      static const char *help_aws_s3s_;                      // Help/summary screen text
      //
      /**
       * @brief Set the AWS S3 path 
       * 
       * @param path AWS S3 path
       */
      void set_aws_s3_path(const std::string &path);
      /**
       * @brief Set the AWS S3 paths 
       * 
       * @param paths AWS S3 paths
       */
      void set_aws_s3_paths(const VectorString &paths);
      /**
       * @brief Get the AWS S3 paths 
       * 
       * @return AWS S3 paths
       */
      const VectorString &get_aws_s3_paths() const;
      /// @private
      static const char *help_aws_s3_paths_;
      //
      /**
       * @brief Set the AWS verbatim mode
       * 
       * @param verbatim Verbatim mode
       */
      void set_aws_verbatim(const bool &verbatim);
      /**
       * @brief Set the AWS verbatim modes
       * 
       * @param verbatims Verbatim modes
       */
      void set_aws_verbatims(const VectorBool &verbatims);
      /**
       * @brief Get the AWS verbatim mode
       * 
       * @return Verbatim modes
       */
      const VectorBool &get_aws_verbatims() const;
      /// @private
      static const char *help_aws_verbatims_;
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
       * @brief Get the use_default_contrast_setting configuration flags
       * 
       * @return Config. values
       */
      const VectorBool &get_use_default_contrast_settings() const;
      /// @private
      static const char *help_use_default_contrast_settings_;
      //
      /**
       * @brief Set QB hardware contrast threshold above which initialisation is deemed successful
       *
       * @param in_init_contrast_threshold Non-negative threshold value
       */
       void set_init_contrast_threshold(const double &in_init_contrast_threshold);
      /**
       * @brief Set QB hardware contrast threshold above which initialisation is deemed successful
       *
       * @param in_init_contrast_thresholds Non-negative threshold values
       */
       void set_init_contrast_thresholds(const VectorMapND &in_init_contrast_thresholds);
      /**
      * @brief Get the contrast threshold for initialisation
      *
      * @return Initialisation contrast thresholds
      */
      const VectorMapND &get_init_contrast_thresholds() const;
      /// @private
      static const char *help_init_contrast_thresholds_;
      //
      /**
      * @brief Set QB hardware contrast thresholds during final readout on a per-qubit basis
      *
      * @param in_qubit_contrast_threshold Non-negative qubit readout threshold
      */
      void set_qubit_contrast_threshold(const ND &in_qubit_contrast_threshold);
      /**
      * @brief Set QB hardware contrast thresholds during final readout on a per-qubit basis
      *
      * @param in_qubit_contrast_thresholds Non-negative qubit readout threshold
      */
      void set_qubit_contrast_thresholds(const VectorMapND &in_qubit_contrast_thresholds);
      /**
      * @brief Get the qubit contrast thresholds for final readout
      *
      * @return Contrast thresholds used for final readout 
      */
      const VectorMapND &get_qubit_contrast_thresholds() const;
      /// @private
      static const char *help_qubit_contrast_thresholds_;
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
       * @brief Set the noise model name
       * 
       * @param noise_model Name of the noise model to use
       */
      void set_noise_model(const std::string &noise_model);
      /**
       * @brief Set the noise model name
       * 
       * @param noise_models Name of the noise model to use
       */
      void set_noise_models(const VectorString &noise_models);
      /**
       * @brief Get the noise model name
       * 
       * @return Name of the noise model to use
       */
      const VectorString &get_noise_models() const;
      /// @private
      static const char *help_noise_models_;
      //
      /**
       * @brief Set the amplitudes for Jensen–Shannon divergence calculation
       * 
       * @param in_output_amplitude Amplitude values
       */
      void set_output_amplitude(const NC &in_output_amplitude);
      /**
       * @brief Set the amplitudes for Jensen–Shannon divergence calculation
       * 
       * @param in_output_amplitude Amplitude values
       */
      void set_output_amplitudes(const VectorMapNC &in_output_amplitude);
      /**
       * @brief Get the amplitudes for Jensen–Shannon divergence calculation
       * 
       * @return Amplitude values
       */
      const VectorMapNC &get_output_amplitudes() const;
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
      const VectorMapNN &get_out_counts() const;
      /// @private
      static const char *help_out_counts_;
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

      /// @private
      // Not implemented
      void profile(const size_t &ii, const size_t &jj);
      /// @private
      // Not implemented
      void profile();
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
      void run(const size_t &ii, const size_t &jj);
      /// @private
      // Not implemented
      void run(const size_t &ii);
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

      /// For QB hardware: contrast threshold helper methods
      
      /**
       * @brief Contrast thresholds for QB hardware.  Applies globally throughout a Qristal session.
       * 
       * @param init_ct Input contrast threshold used for initialisation
       * @param q0_ct Input contrast threshold used for final readout of qubit[0]
       * @param q1_ct Input contrast threshold used for final readout of qubit[1]
       */
      void set_contrasts(const double &init_ct, const double &q0_ct, const double &q1_ct);

      /**
      * @brief Removes all contrast thresholds stored in a Qristal session
      */
      void reset_contrasts();

      /// Supported types of the input source string
      enum class source_string_type
      {
        XASM,
        Quil,
        OpenQASM
      };
      
      /// Internal struct captures (i, j) run configurations retrieved and validated from the overall config tables.
      /// @private
      struct run_i_j_config
      {
        /// Number of measurement shots (sns_)
        int num_shots;
        /// Number of qubits (qns_)
        int num_qubits;
        /// Number of repetitions (rns_)
        int num_repetitions;
        /// Enable post-execution transpilation and resource estimation
        bool oqm_enabled;
        /// Name of the backend accelerator
        std::string acc_name;
        /// Full path to the OpenQASM include file where custom QB gates are defined
        std::string openqasm_qb_include_filepath;
        /// Full path to the QPU configuration JSON file
        std::string qpu_config_json_filepath;
        /// QB hardware: prevent contrast thresholds from being sent from Qristal
        bool use_default_contrast_setting;
        /// QB hardware: init contrast thresholds
        ND init_contrast_thresholds;
        /// QB hardware: qubit contrast thresholds
        ND qubit_contrast_thresholds;
        /// Type (assembly dialect) of the input source string
        source_string_type source_type;
        /// Disable placement if true
        bool no_placement;
        /// Disable circuit optimisation if true
        bool no_optimise;
        /// Disable simulation (accelerator execution) if true
        /// e.g., just want to run transpilation for resource estimation.
        bool no_sim;
        /// Enable noisy simulation/emulation
        bool noise;
        /// Name of the noise model
        std::string noise_model;
        /// *
        /// Backend-specific configurations
        /// *
        /// [TNQVM] Max MPS bond dimension to keep
        int max_bond_tnqvm;
        /// [TNQVM] SVD cut-off limit for singular value truncation
        double svd_cutoff_tnqvm;
        /// [AWS Braket] Name of the AWS device name (e.g., SV1, TN1)
        std::string aws_device_name;
        /// [AWS Braket] Enable 'verbatim' mode
        bool aws_verbatim;
        /// [AWS Braket] Format of the AWS source string that we use to submit the request
        std::string aws_format;
        /// [AWS Braket] Name of the S3 bucket to store the result
        std::string aws_s3;
        /// [AWS Braket] Path (key) within the bucket where the result is stored
        std::string aws_s3_path;
      };
    
    private:
      int validate_sns_nonempty();
      int validate_qns_nonempty();
      int validate_rns_nonempty();
      int validate_thetas_option();
      int validate_instrings();
      void validate_acc(const std::string &acc);
      void validate_noise_mitigation(const std::string &noise_mitigate);
      void validate_aws_s3(const std::string &bucket_name);
      void validate_aws_format(const std::string &format);
      void validate_aws_device_name(const std::string &device_name);
      void validate_noise_model(const std::string &noise_model);
      void validate_aer_sim_type(const std::string &sim_type);

      template <class TT> int eqlength(const TT &in_d, const int N_ii);
      template <class TT> int singleton_or_eqlength(const TT &in_d, const int N_ii);
      int is_ii_consistent();
      int is_jj_consistent();
       circuit_input_types validate_infiles_instrings_randoms_irtarget_ms_nonempty(const int &ii, const int &jj);

      // Methods
      std::string random_circuit(const int &n_q, const int &depth);

      template <typename TT> double get_probability(const TT &in_elem);

      template <typename TQ, class TV>
      TQ get_key(const std::map<TQ, TV> &in_q, const int &key_in_q);
      template <class TT, class TV>
      int get_key(const std::map<int, TT> &in_q, const std::pair<const int, TV> &el,
                  const int &key_in_q);

      template <typename TQ, typename TP>
      double get_jensen_shannon_divergence(const std::map<TQ, int> &in_q,
                                           const TP &in_p, const bool &in_use_lsb);

      std::string aer_circuit_transpiler(std::string &circuit);
      /// Ensure that all result tables are resized/expanded to accommodate (ii, jj) experiment index.
      void ensure_results_table_size(size_t ii, size_t jj);
      /// Retrieve and validate run configurations for index pair (ii, jj) using the table index convention.
      run_i_j_config get_run_config(size_t ii, size_t jj);
      /// Helper to populate result tables (e.g. counts, expectation values, resource estimations) post-execution.
      void process_run_result(const std::size_t ii, const std::size_t jj, const run_i_j_config& run_config,
                              std::shared_ptr<xacc::AcceleratorBuffer> buffer_b, double runtime_ms,
                              std::shared_ptr<xacc::quantum::QuantumBrillianceAccelerator> qb_transpiler);
      /// Util method to compile input source string into IR
      /// This method is thread-safe, thus can be used to compile multiple source strings in parallel.
      std::shared_ptr<xacc::CompositeInstruction> compile_input(const std::string& in_source_string, int in_num_qubits, source_string_type in_source_type);

      /// Retrieve the target circuit string for (i, j) task:
      /// This will involve loading file (if file mode is selected), generate random circuit string (if random mode is selected), etc.
      std::string get_target_circuit_qasm_string(size_t ii, size_t jj, const run_i_j_config& run_config);
  };

} // namespace qb

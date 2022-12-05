// Copyright (c) 2021 Quantum Brilliance Pty Ltd
#pragma once

#include "qb/core/QuantumBrillianceAccelerator.hpp"
#include "qb/core/QuantumBrillianceRemoteAccelerator.hpp"
#include "qb/core/async_executor.hpp"
#include "qb/core/remote_async_accelerator.hpp"
#include "qb/core/cmake_variables.hpp"
#include "qb/core/utils.hpp"
#include "qb/core/profiler.hpp"
#include "qb/core/pretranspiler.hpp"

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


      const int VALID_QB_HARDWARE_INIT = 0;
      const int VALID_QB_HARDWARE_REQUEST_ID = 0;
      const int VALID_QB_HARDWARE_POLL_ID = 0;
      const int VALID_QB_HARDWARE_CYCLES = 1;

      const int POLLING_NOT_READY = 300;
      const int POLLING_PROCESS_FAILED = 500;
      const int POLLING_SUCCESS = 0;

    public:
      session()
          : debug_(false), name_m{{}}, number_m{{}}, infiles_{{}},
            // FIXME some tests will probably fail if the SDK_SOURCE_DIR path to qblib.inc and qpu_config.json is not also added.
            include_qbs_{{SDK_DIR "/include/qb/core/qblib.inc"}},
            qpu_configs_{{SDK_DIR "/include/qb/core/qpu_config.json"}},
            instrings_{{}}, irtarget_ms_{{{}}},
            accs_{{"tnqvm"}},
            aws_device_names_{{"DM1"}}, aws_formats_{{"openqasm3"}}, aws_verbatims_{{{false}}},
            aws_s3s_{{"amazon-braket-QBSDK"}}, aws_s3_paths_{{"output"}},
            aer_sim_types_{},
            randoms_{{}}, xasms_{{{false}}},
            quil1s_{{{false}}}, noplacements_{{{false}}},
            nooptimises_{{{true}}}, nosims_{{{false}}}, noises_{{{false}}},
            output_oqm_enableds_{{{false}}}, log_enableds_{{{false}}},
            notimings_{{{false}}}, qns_{{}}, rns_{{}}, sns_{{}}, betas_{{}},
            thetas_{{}}, max_bond_dimensions_{{}}, svd_cutoffs_{{}}, noise_models_{{"default"}},
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
      session(const std::string &name) : session() { name_m.push_back({name}); }
      session(const bool debug) : session() { debug_ = debug; }

      // Setters and Getters
      // Notes:
      //  - setters and getters implementation should be defined in python_module/core/methods_getter_setter.cpp
      //  - help strings (Python) should be defined in python_module/core/methods_py_help_strings.cpp
      void set_infile(const std::string &infile);
      void set_infiles(const VectorString &infiles);
      const VectorString &get_infiles() const;
      static const char *help_infiles_;
      //
      void set_instring(const std::string &instring);
      void set_instrings(const VectorString &instrings);
      const VectorString &get_instrings() const;
      static const char *help_instrings_;
      //
      void set_irtarget_m(const std::shared_ptr<xacc::CompositeInstruction> &irtarget_m);
      void set_irtarget_ms(const std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> &irtarget_ms);
      const std::vector<std::vector<std::shared_ptr<xacc::CompositeInstruction>>> &get_irtarget_ms() const;
      static const char *help_irtarget_ms_;
      //
      void set_include_qb(const std::string &include_qb);
      void set_include_qbs(const VectorString &include_qbs);
      const VectorString &get_include_qbs() const;
      static const char *help_include_qbs_;
      //
      void set_qpu_config(const std::string &qpu_config);
      void set_qpu_configs(const VectorString &qpu_configs);
      const VectorString &get_qpu_configs() const;
      static const char *help_qpu_configs_;
      //
      void set_acc(const std::string &acc);
      void set_accs(const VectorString &accs);
      void validate_acc(const std::string &acc);
      const VectorString &get_accs() const;
      static const char *help_accs_;
      //
      void set_aws_device_name(const std::string &device_name);
      void set_aws_device_names(const VectorString &device_names);
      void validate_aws_device_name(const std::string &device_name);
      const VectorString &get_aws_device_names() const;
      static const char *help_aws_device_names_;
      //
      void set_aws_format(const std::string &format);
      void set_aws_formats(const VectorString &formats);
      void validate_aws_format(const std::string &format);
      const VectorString &get_aws_formats() const;
      static const char *help_aws_formats_;
      //
      void set_aws_s3(const std::string &bucket_name);       // Setter for scalar case. Used by pybind11.
      void set_aws_s3s(const VectorString &bucket_names);    // Setter for vector case. User by pybind11.
      void validate_aws_s3(const std::string &bucket_name);  // Validator at the time of user data input
      const VectorString &get_aws_s3s() const;               // Getter that works for scalar and vector cases
      static const char *help_aws_s3s_;                      // Help/summary screen text
      //
      void set_aws_s3_path(const std::string &path);
      void set_aws_s3_paths(const VectorString &paths);
      const VectorString &get_aws_s3_paths() const;
      static const char *help_aws_s3_paths_;
      //
      void set_aws_verbatim(const bool &verbatim);
      void set_aws_verbatims(const VectorBool &verbatims);
      const VectorBool &get_aws_verbatims() const;
      static const char *help_aws_verbatims_;
      //
      void set_aer_sim_type(const std::string &sim_type);
      void set_aer_sim_types(const VectorString &sim_types);
      void validate_aer_sim_type(const std::string &sim_type);
      const VectorString &get_aer_sim_types() const;
      static const char *help_aer_sim_types_;
      //
      void set_random(const size_t &in_random);
      void set_randoms(const VectorN &in_random);
      const VectorN &get_randoms() const;
      static const char *help_randoms_;
      //
      void set_xasm(const bool &in_xasm);
      void set_xasms(const VectorBool &in_xasm);
      const VectorBool &get_xasms() const;
      static const char *help_xasms_;
      //
      void set_quil1(const bool &in_quil1);
      void set_quil1s(const VectorBool &in_quil1);
      const VectorBool &get_quil1s() const;
      static const char *help_quil1s_;
      //
      void set_noplacement(const bool &in_noplacement);
      void set_noplacements(const VectorBool &in_noplacement);
      const VectorBool &get_noplacements() const;
      static const char *help_noplacements_;
      //
      void set_placement(const std::string &in_placement);
      void set_placements(const VectorString &in_placements);
      const VectorString &get_placements() const;
      static const char *help_placements_;
      //
      void set_nooptimise(const bool &in_nooptimise);
      void set_nooptimises(const VectorBool &in_nooptimise);
      const VectorBool &get_nooptimises() const;
      static const char *help_nooptimises_;
      //
      void set_nosim(const bool &in_nosim);
      void set_nosims(const VectorBool &in_nosim);
      const VectorBool &get_nosims() const;
      static const char *help_nosims_;
      //
      void set_noise(const bool &in_noise);
      void set_noises(const VectorBool &in_noise);
      const VectorBool &get_noises() const;
      static const char *help_noises_;
      //
      void set_output_oqm_enabled(const bool &in_output_oqm_enabled);
      void set_output_oqm_enableds(const VectorBool &in_output_oqm_enabled);
      const VectorBool &get_output_oqm_enableds() const;
      static const char *help_output_oqm_enableds_;
      //
      void set_log_enabled(const bool &in_log_enabled);
      void set_log_enableds(const VectorBool &in_log_enabled);
      const VectorBool &get_log_enableds() const;
      static const char *help_log_enableds_;
      //
      void set_notiming(const bool &in_notiming);
      void set_notimings(const VectorBool &in_notiming);
      const VectorBool &get_notimings() const;
      static const char *help_notimings_;
      //
      void set_qn(const size_t &in_qn);
      void set_qns(const VectorN &in_qn);
      const VectorN &get_qns() const;
      static const char *help_qns_;
      //
      void set_rn(const size_t &in_rn);
      void set_rns(const VectorN &in_rn);
      const VectorN &get_rns() const;
      static const char *help_rns_;
      //
      void set_sn(const size_t &in_sn);
      void set_sns(const VectorN &in_sn);
      const VectorN &get_sns() const;
      static const char *help_sns_;
      //
      void set_beta(const ND &in_beta);
      void set_betas(const VectorMapND &in_beta);
      const VectorMapND &get_betas() const;
      static const char *help_betas_;
      //
      void set_theta(const ND &in_theta);
      void set_thetas(const VectorMapND &in_theta);
      const VectorMapND &get_thetas() const;
      static const char *help_thetas_;
      //
      void set_max_bond_dimension(const size_t &in_max_bond_dimension);
      void set_max_bond_dimensions(const VectorN &in_max_bond_dimension);
      const VectorN &get_max_bond_dimensions() const;
      static const char *help_max_bond_dimensions_;
      //
      void set_svd_cutoff(const ND &in_svd_cutoff);
      void set_svd_cutoffs(const VectorMapND &in_svd_cutoff);
      const VectorMapND &get_svd_cutoffs() const;
      static const char *help_svd_cutoffs_;
      //
      void set_noise_model(const std::string &noise_model);
      void set_noise_models(const VectorString &noise_models);
      void validate_noise_model(const std::string &noise_model);
      const VectorString &get_noise_models() const;
      static const char *help_noise_models_;
      //
      void set_output_amplitude(const NC &in_output_amplitude);
      void set_output_amplitudes(const VectorMapNC &in_output_amplitude);
      const VectorMapNC &get_output_amplitudes() const;
      static const char *help_output_amplitudes_;
      //
      void set_debug(const bool &debug);
      const bool &get_debug() const;
      static const char *help_debug_;
      //
      void set_out_raw(const std::string &out_raw);
      void set_out_raws(const VectorString &out_raws);
      const VectorString &get_out_raws() const;
      static const char *help_out_raws_;
      //
      void set_out_count(const NN &out_count);
      void set_out_counts(const VectorMapNN &out_counts);
      const VectorMapNN &get_out_counts() const;
      static const char *help_out_counts_;
      //
      void set_out_divergence(const ND &out_divergence);
      void set_out_divergences(const VectorMapND &out_divergences);
      const VectorMapND &get_out_divergences() const;
      static const char *help_out_divergences_;
      //
      void set_out_transpiled_circuit(const std::string &out_transpiled_circuit);
      void set_out_transpiled_circuits(const VectorString &out_transpiled_circuits);
      const VectorString &get_out_transpiled_circuits() const;
      static const char *help_out_transpiled_circuits_;
      //
      void set_out_qobj(const std::string &out_qobj);
      void set_out_qobjs(const VectorString &out_qobjs);
      const VectorString &get_out_qobjs() const;
      static const char *help_out_qobjs_;
      //
      void set_out_qbjson(const std::string &out_qbjson);
      void set_out_qbjsons(const VectorString &out_qbjsons);
      const VectorString &get_out_qbjsons() const;
      static const char *help_out_qbjsons_;

      //
      void set_out_single_qubit_gate_qty(const NN & out_single_qubit_gate_qty);
      void set_out_single_qubit_gate_qtys(const VectorMapNN & out_single_qubit_gate_qtys);
      const VectorMapNN & get_out_single_qubit_gate_qtys() const;
      static const char* help_out_single_qubit_gate_qtys_;
      //
      void set_out_double_qubit_gate_qty(const NN & out_double_qubit_gate_qty);
      void set_out_double_qubit_gate_qtys(const VectorMapNN & out_double_qubit_gate_qtys);
      const VectorMapNN & get_out_double_qubit_gate_qtys() const ;
      static const char* help_out_double_qubit_gate_qtys_;
      //
      void set_out_total_init_maxgate_readout_time(const ND & out_total_init_maxgate_readout_time);
      void set_out_total_init_maxgate_readout_times(const VectorMapND & out_total_init_maxgate_readout_times);
      const VectorMapND & get_out_total_init_maxgate_readout_times() const;
      static const char* help_out_total_init_maxgate_readout_times_;
      //
      void set_out_z_op_expect(const ND & out_z_op_expect);
      void set_out_z_op_expects(const VectorMapND & set_out_z_op_expects);
      const VectorMapND & get_out_z_op_expects() const;
      static const char* help_out_z_op_expects_;
      //
      void set_noise_mitigation(const std::string &noise_mitigate);
      void set_noise_mitigations(const VectorString &noise_mitigates);
      void validate_noise_mitigation(const std::string &noise_mitigate);
      const VectorString &get_noise_mitigations() const;
      static const char *help_noise_mitigations_;
      //
      void set_seed(const size_t &in_seed);
      void set_seeds(const VectorN &in_seeds);
      const VectorN &get_seeds() const;
      static const char *help_seeds_;
      //

      const std::string get_summary() const;

      void setName(const VectorString &name_);
      void setName(const std::string &name_);
      const VectorString &getName() const;

      // Validation methods
      circuit_input_types validate_infiles_instrings_randoms_irtarget_ms_nonempty(const int &ii, const int &jj);

      int validate_sns_nonempty();
      int validate_qns_nonempty();
      int validate_rns_nonempty();
      int validate_thetas_option();
      int validate_instrings();
      template <class TT> int eqlength(const TT &in_d, const int N_ii);
      template <class TT> int singleton_or_eqlength(const TT &in_d, const int N_ii);
      int is_ii_consistent();
      int is_jj_consistent();

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
      void get_jensen_shannon(const size_t &ii, const size_t &jj);
      void get_jensen_shannon();

      std::string aer_circuit_transpiler(std::string &circuit);

      void profile(const size_t &ii, const size_t &jj);
      void profile();

      /// Ensure that all result tables are resized/expanded to accommodate (ii, jj) experiment index.
      void ensure_results_table_size(size_t ii, size_t jj);
      /// Run a quantum task at the (ii, jj) index in the experiment table.
      void run(const size_t &ii, const size_t &jj);
      void run(const size_t &ii);
      void run();
      // Set the multi-qpu run configurations:
      // e.g., the list of QPUs paricipate in this run
      void set_parallel_run_config(const std::string &in_config);
      /// Threadsafe execution of (ii, jj) task using the provided accelerator.
      /// Returns a job handle if the job is posted to a remote accelerator (e.g. AWS Braket).
      /// Otherwise, returns null if this function completes the run locally.
      std::shared_ptr<async_job_handle> run_async(const std::size_t ii, const std::size_t jj,
                     std::shared_ptr<xacc::Accelerator> acc);

      /// Get the QPU pool executor.
      Executor& get_executor();
      // Shortcuts for setting defaults
      void qb12(); // 12 qubits, 1024 shots, noiseless
      void aws32dm1(); // AWS Braket DM1, 32 async workers, 17 qubits, 256 shots, noiseless
      void aws32sv1(); // AWS Braket SV1, 32 async workers, 34 qubits, 256 shots, noiseless
      void aws8tn1(); // AWS Braket TN1, 8 async workers, 49 qubits, 256 shots, noiseless

      /// Supported types of the input source string
      enum class source_string_type
      {
        XASM,
        Quil,
        OpenQASM
      };
      /// Internal struct captures (i, j) run configurations retrieved and validated from the overall config tables.
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

// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

#include <qristal/core/circuit_language.hpp>
#include <qristal/core/cmake_variables.hpp>
#include <qristal/core/noise_model/noise_model.hpp>
#include <qristal/core/passes/base_pass.hpp>
#include <qristal/core/remote_async_accelerator.hpp>
#include <qristal/core/utils.hpp>

// MPI
#ifdef USE_MPI
  #include <qristal/core/mpi/mpi_manager.hpp>
  #include <qristal/core/mpi/results_types.hpp>
#endif

// CUDAQ support
#ifdef WITH_CUDAQ
  #include <cudaq/utils/cudaq_utils.h>
#endif

// STL
#include <cmath>
#include <complex>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <string_view>

// YAML
#include <yaml-cpp/yaml.h>


// Forward declarations
namespace xacc::quantum { class qb_qpu; }
namespace qristal { class backend; }

namespace qristal
{

  #ifdef USE_MPI
  /**
   * @brief Checks the configuration of the session is valid for MPI-related runs.
   *
   * @note @ref acc is modified depending on whether @ref mpi_hardware_accelerators is set.
   */
  void validate_mpi_config(int32_t num_mpi_processes, int32_t mpi_process_id,
                           const std::vector<std::string> &mpi_hardware_accelerators, std::string &session_accelerator,
                           const YAML::Node &remote_backend_database);
#endif

  /// A session of the Qristal SDK.
  class session
  {

    private:

      bool all_bitstring_counts_ordered_by_MSB_ = false;
      YAML::Node remote_backend_database_;

      /// The number of shots remaining to be drawn from \ref results_ using \ref draw_shot
      size_t shots_remaining_;

      /// The number of shots to be run on the current process (differs from \ref sn if running with multiple MPI processes).
      size_t sn_this_process;

      size_t acc_uses_n_bit_;

      /**
       * @brief The results of a Quantum calculation.
       * The map value (count) is the number of times the map key (Qubit states
       * with the same bit indexation as the quantum processor registers) was
       * measured after all shots were run.
       *
       * @note The Qubit states are represented as a vector of booleans for the
       * reasons outlined below:
       * - High qubit counts can quickly exhaust all possible values able to
       *   be encoded by even a 64-bit integer, causing integer overflow.
       * - Values are agnostic with respect to both endianness and ordering
       *   because a vector with indices correspond unambiguously to the
       *   register number of each qubit. Specifically:
       *   - Ordering: if qubits were saved as e.g. a string "0010110", a
       *     convention about whether the value of qubit 0 goes on the left of
       *     the string or the right of it must be chosen.
       *   - Endianness: if instead a maximally compact representation is opted
       *     for where an integer that corresponds to the whole bitstring is
       *     used (e.g. @ref all_bitstring_counts_), a convention about whether
       *     bit 0 is the most or the least significant bit in encoding that
       *     integer must be chosen.
       *
       */
      std::map<std::vector<bool>, int> results_;

      /**
       * @brief Provides counts for every possible combination of qubit
       * measurements, ordered according to the selected encoding (MSB, LSB).
       * Requires @ref calc_all_bitstring_counts to be set to true.
       *
       * E.g. If 2 qubits are used in the calculation, the out counts vector
       * will contain the count of each result in the below order.
       *
       * | Encoding | Order of Results |
       * |----------|------------------|
       * | MSB      | 00, 01, 10, 11   |
       * | LSB      | 00, 10, 01, 11   |
       *
       * @note The mapping from bitstring to vector index can be obtained
       * from the function @ref bitstring_index.
       */
      std::vector<int> all_bitstring_counts_;

      /**
       * @brief Probability distribution of output results. Order of
       * probabilities is identical to the order of the @ref all_bitstring_counts_.
       *
       * @note The mapping from bitstring to vector index can be obtained
       * from the function @ref bitstring_index.
       */
      std::vector<double> all_bitstring_probabilities_;

      /**
       * @brief A 2D array of probability gradients with dimensions of the
       * number of parameters by the number of qubits. Requires @ref
       * calc_gradients to be set to true.
       *
       * This array stores gradients of the bitstring probabilities with
       * respect to the circuit parameters, in the following format (where
       * @c y is the probability list and @c x is the parameter list):
       *
       * \f[
       * \begin{bmatrix}
       * \frac{dy_0}{dx_0} & \frac{dy_0}{dx_1} & ... & \frac{dy_0}{dx_n} \\
       * \frac{dy_1}{dx_0} & \frac{dy_1}{dx_1} & ... & \frac{dy_1}{dx_n} \\
       * ... \\
       * \frac{dy_m}{dx_0} & \frac{dy_m}{dx_1} & ... & \frac{dy_m}{dx_n}
       * \end{bmatrix}
       * \f]
       *
       * As the gradients are returned as a list-of-lists, they can be
       * accessed in row major format, and indexing the above matrix can be
       * done accordingly, i.e. @c all_bitstring_probability_gradients()[0][1]
       * corresponds to the @c dy_0/dx_1 value. @c x_i correspond to the parameters
       * set using \ref circuit_parameters() (i.e. the parameters ordered by their
       * first appearance in the circuit). @c y_i are the output probabilities of
       * different bitstrings, indexed in the same manner as \ref all_bitstring_counts_.
       * Explicitly, the index @c i corresponding to a specific bitstring can be
       * obtained by calling @c bitstring_index(bitstring), with bitstring
       * given as a list of bit values.
       */
      std::vector<std::vector<double>> all_bitstring_probability_gradients_;

      std::string transpiled_circuit_;
      std::string qobj_;
      std::string qbjson_;
      bool acc_outputs_qbit0_left_;
      std::map<int,int> one_qubit_gate_depths_;
      std::map<int,int> two_qubit_gate_depths_;
      std::map<int,double> timing_estimates_;
      double z_op_expectation_;

      /// The XACC accelerator in use
      std::shared_ptr<xacc::Accelerator> qpu_ = nullptr;

      /// State vector from qpp or aer
      std::shared_ptr<std::vector<std::complex<double>>> state_vec_;

      /// Whether or not to apply SPAM error mitigation
      bool perform_SPAM_correction_ = false;

      /**
       * @brief When error mitigation is performed for the session, the raw
       * results are stored in here.
       * @see results_
       */
      std::map<std::vector<bool>, int> results_native_;

      /// Supported circuit input sources
      enum class circuit_origin {
        instring,
        infile,
        random_circuit,
        IR,
        CUDAQ
      };

      /// Bounds on session parameters
      /// @{
      using ibounds = const std::pair<size_t, size_t>;
      using dbounds = const std::pair<double, double>;
      ibounds random_circuit_depth_bounds{0,1000};
      ibounds sn_bounds{1,1000000000};
      ibounds qn_bounds{1,10000};
      ibounds max_bond_bounds{1,50000};
      ibounds initial_bond_bounds{1,50000};
      ibounds max_kraus_bounds{1,50000};
      ibounds initial_kraus_bounds{1,50000};
      dbounds svd_cutoff_bounds{0, 1e9};
      /// @}


      /// Sets of recognised values for string-type session parameters.
      /// @{
      /// Emulator backends
      static const std::unordered_set<std::string_view> EMULATOR_BACKENDS;
      /// Non-emulator backends
      static const std::unordered_set<std::string_view> NON_EMULATOR_BACKENDS;
      /// Backends that support GPU execution
      static const std::unordered_set<std::string_view> GPU_BACKENDS;
      /// Backends that support noise
      static const std::unordered_set<std::string_view> NOISY_BACKENDS;
      /// Backends that *only* support noise, i.e. will not run with noise = false
      static const std::unordered_set<std::string_view> EXCLUSIVELY_NOISY_BACKENDS;
      /// Valid AER simulator types
      static const std::unordered_set<std::string_view> VALID_AER_SIM_TYPES;
      /// Recommended maximum qubit number for selected accelerator type
      static const std::unordered_map<std::string_view, size_t> MAX_QUBITS_ACCS;
      /// Recognised noise mitigation strategies
      static const std::unordered_set<std::string_view> VALID_NOISE_MITIGATIONS;
      /// Valid placement strategies
      static const std::unordered_set<std::string_view> VALID_HARDWARE_PLACEMENTS;
      /// Valid measurement sampling options
      static const std::unordered_set<std::string_view> VALID_MEASURE_SAMPLING_OPTIONS;
      /// @}

      #ifdef USE_MPI
        private:
          mpi::MpiManager mpi_manager_{};

        public:

          /**
           * @brief Controls whether a session object should communicate over MPI.
           * @details This is used to ensure that session objects created in order
           * to compute gradients does not try to communicate with other processes.
           * This is the responsibility of the master session object.
           * @warning The default configuration for a session object is to
           * communicate over MPI. If making copies of session objects, keep this in
           * mind.
           */
          bool mpi_acceleration_enabled{true};

          /**
           * @brief Sets the accelerators for running with MPI. Each MPI process
           * uses its rank to index into this array to set its accelerator backend.
           *
           * @note If a single process is running and this is set, @ref acc will be ignored.
           * @note If this is not set, then all MPI processes will use the same backend accelerator
           * configured in @ref acc.
           * @note Only hardware backends can be used to partition workloads across MPI as results may
           * be misleading or cause errors when combining different backends.
           *
           */
          std::vector<std::string> mpi_hardware_accelerators;

          /**
           * @brief Light-weight convenience wrapper function for printing only from the MPI supervisor
           * process.
           *
           * @param output_stream Stream to print to (e.g. std::cout or std::cerr)
           * @param message The message to print
           */
          void supervisor_print(const std::string& message, std::ostream& output_stream = std::cout) {
            if (mpi_manager_.get_process_id() == 0) {
              output_stream << message;
            }
          }

          int32_t get_mpi_process_id()
          {
            return mpi_manager_.get_process_id();
          }

          int32_t get_total_mpi_processes()
          {
            return mpi_manager_.get_total_processes();
          }

    #endif

    public:
      // Building docs results in the following error for these type aliases, so they are excluded from parsing.
      //      /workspaces/core/build/docs/_cpp_api/classqristal_1_1session.rst:13:Invalid
      //      C++ declaration: Expected end of definition. [error at 41]
      //        ResultsMapQubitsType =
      //              decltype(results_)::value_type::value_type::key_type
      //              ------------------^
      //      gmake[2]: *** [CMakeFiles/ReadTheDocsHtmlBuild.dir/build.make:125:
      //           docs/_build/html/index.html] Error 2
      //      gmake[1]: *** [CMakeFiles/Makefile2:2413:
      //           CMakeFiles/ReadTheDocsHtmlBuild.dir/all] Error 2
      //      gmake: *** [Makefile:166: all] Error 2
      /// @cond DOXYGEN_SHOULD_SKIP_THIS
      using ResultsMapQubitsType = decltype(results_)::key_type;
      using ResultsMapCountType = decltype(results_)::mapped_type;
      using ResultsMapType = decltype(results_);
      using NativeResultsMapType = decltype(results_native_);
      using CountType = decltype(all_bitstring_counts_)::value_type;
      using OutCountsType = decltype(all_bitstring_counts_);
      using ProbabilityType = decltype(all_bitstring_probabilities_)::value_type;
      using OutProbabilitiesType = decltype(all_bitstring_probabilities_);
      using ProbabilityGradientType = decltype(all_bitstring_probability_gradients_)::value_type::value_type;
      using OutProbabilityGradientsType = decltype(all_bitstring_probability_gradients_);
      /// @endcond

      /**
       * @brief Construct a new session object
       *
       * Some parameters are uninitialized, e.g., number of qubits (`qn`).
       * These parameters can be set manually.
       */
      session();

      /**
       * @brief Construct a new session object using a specific ordering for all_bitstring_counts_ and associated QML-relevant outputs.
       *
       * @param msb   MSB flag. If true, use MSB to determine ordering of all_bitstring_counts_
       *              all_bitstring_probabilities and all_bitstring_probability_gradients table; else use LSB.
       */
      session(const bool msb);

      /// Full path to an input QASM source file
      std::string infile;

      /// An input QASM source string.
      std::string instring;

      /**
       * @brief Provides a `xacc::CompositeInstruction` IR target.
       *
       * If the IR target is provided instead of QASM strings or files, the QASM compilation step will be skipped.
       * The IR target can be manually constructed by building the IR tree using XACC.
       */
      std::shared_ptr<xacc::CompositeInstruction> irtarget;

      /// List of GPU device numbers
      std::vector<size_t> gpu_device_ids;

      /// A CUDAQ kernel to execute, with any free parameters already fully specified.
      std::function<void()> cudaq_kernel;

      /**
       * @brief Create fully specified input CUDAQ kernel from a parameterised kernel and some argument values at which to evaluate it.
       *
       * @param in_kernel Input CUDAQ kernel (templated callable returning void)
       * @param args Concrete argument values with which to call the CUDAQ kernel
       */
      template <typename CudaQKernel, typename... Args>
      std::function<void()> bind_args_to_cudaq_kernel(CudaQKernel &&in_kernel, Args &&...args) {
        return [&in_kernel, ... args = std::forward<Args>(args)]() mutable { in_kernel(std::forward<Args>(args)...); };
      }

      /// Path to an OpenQASM file to include at the beginning of every circuit
      /// Contains custom Quantum Brilliance gate definitions.
      std::string include_qb = QRISTAL_DIR + "/include/qristal/core/qristal.inc";

      /// Vector of circuit parameters to use for runtime substitution
      std::vector<double> circuit_parameters;

      /// Whether or not gradients will be calculated for parametrized circuits.
      bool calc_gradients = false;

      /// Whether or not a non-compact output counts vector will be calculated.
      bool calc_all_bitstring_counts = false;

      /// @brief The path to the remote backend database yaml file.
      /// The path to a YAML file with configuration data for remote backends (including hardware).
      std::string remote_backend_database_path = QRISTAL_DIR + "/remote_backends.yaml";

      /// Chosen backend accelerator.
      std::string acc = "qpp";

      /// The simulator type for the AER backend accelerator
      std::string aer_sim_type = "statevector";

      /// The maximum number of OpenMP threads that AER can use
      size_t aer_omp_threads = 0;

      /// The depth of random circuit to be generated.
      size_t random_circuit_depth = 0;

      /// The frontend language in which the input circuit is written
      circuit_language input_language = circuit_language::OpenQASM;

      /// Enable noisy simulation.
      bool noise = false;

      /// A noise mitigation method to apply
      std::string noise_mitigation;

      /// Correction matrix to use for SPAM error correction
      Eigen::MatrixXd SPAM_correction_matrix;

      /// Disable circuit placement IR transformations (both pure topological and noise-based placement).
      bool noplacement = true;

      /// The name of the module to use for circuit placement IR transformation.
      std::string placement = "swap-shortest-path";

      /// Disable circuit optimization IR transformations
      bool nooptimise = false;

      /// Circuit optimization passes to apply
      Passes circuit_opts;

      /**
       * @brief Whether or not to actually execute the circuit upon calling run.
       *
       * Set false to disable circuit simulation, for e.g., inspecting transpilation or resource estimation only.
       */
      bool execute_circuit = true;

      /// Compute the state vector with qpp or aer
      bool calc_state_vec = false;

       /// Enable output transpilation and resource estimation
      bool output_oqm_enabled = true;

      /// Disable timing estimation
      bool notiming = false;

      /// The number of qubits to simulate
      size_t qn = 0;

      /// The number of measurement shots to be performed
      size_t sn = 0;

      /// Seed value for the session's random number generator.
      /// Zero indicates that the seed should be generated by std::random_device.
      size_t seed = 0;

      /**
       * @brief Initial bond dimension for tensor network simulators.
       * @note This is only needed when using tensor network backend accelerators.
       */
      size_t initial_bond_dimension = 1;

      /**
       * @brief Maximum bond dimension for tensor network simulators.
       * @note This is only needed when using tensor network backend accelerators.
       */
      size_t max_bond_dimension = 256;

      /**
       * @brief Initial Kraus dimension for the purification simulator.
       * @note This is only needed when using the emulator's purification backend accelerator.
       */
      size_t initial_kraus_dimension = 1;

      /**
       * @brief Maximum Kraus dimension for the purification simulator.
       * @note This is only needed when using the emulator's purification backend accelerator.
       */
      size_t max_kraus_dimension = 256;

      /**
       * @brief Absolute SVD cutoff for tensor network simulators.
       * @note This is only needed when using tensor network backend accelerators.
       */
      double svd_cutoff = 1.0e-8;

      /**
       * @brief Relative SVD cutoff for tensor network simulators.
       * @note This is only needed when using tensor network backend accelerators.
       */
      double rel_svd_cutoff = 1.0e-4;

      /**
       * @brief Set the measurement sampling method. Options:
       * "cutensornet" uses the single-shot cutensorNet contraction method of the entire
       * tensor network state. Program terminates with error meassage if cutensorNet fails.
       * "cutensornet_multishot" uses the multi-shot cutensorNet contraction method.
       * "sequential" uses the cutensor sequential contraction method.
       * "auto" (default) uses the cutensorNet contraction method and automatically
       * swithes to the cutensor sequential contraction method if the cutensorNet
       * method fails.
       * @note This is only needed if using a tensor network accelerator
       */
      std::string measure_sample_method = "auto";

      /// Noise model to use when noise = true
      std::shared_ptr<NoiseModel> noise_model;

      /// Debug mode (verbose logging)
      bool debug = false;

      /**
       * @brief Get the output measurement counts as a map
       *
       * @return Measurement counts map
       */
      const std::map<std::vector<bool>,int>& results() const;

      /**
       * @brief Get the native output measurement counts as a map
       *
       * @return Native measurement counts map
       *
       * @details Beware: The native results are only stored separately, if a confusion or
       * correction matrix was supplied to session, enabling automatic SPAM correction!
       */
      const std::map<std::vector<bool>,int>& results_native() const;

      /**
       * @brief Get the full state vector (works with qpp and aer backends only!)
       *
       * @return Full complex state vector as std::vector<std::complex<double>>
       */
      const std::vector<std::complex<double>>& state_vec() const;

      /**
       * @brief Get the output measurement counts as a vector
       *
       * @return Measurement counts vector
       */
      const std::vector<int>& all_bitstring_counts() const;

      /**
       * @brief Get the output probabilities
       *
       * @return Measurement probabilities vector
       */
      const std::vector<double>& all_bitstring_probabilities() const;

      /**
       * @brief Get the output probability gradients
       *
       * @return Table of probability jacobians w.r.t. runtime parameters
       */
      const std::vector<std::vector<double>>& all_bitstring_probability_gradients() const;

      /**
       * @brief Get the output transpiled circuit
       *
       * @return Output transpiled circuit as an OpenQASM string.
       */
      std::string transpiled_circuit() const;

      /**
       * @brief Get the output Aer QObj JSON string
       *
       * @return QObj Json string
       */
      std::string qobj() const;

      /**
       * @brief Get the output QB JSON string (QB hardware JSON POST payload)
       *
       * @return QB Json string.
       */
      std::string qbjson() const;

      /**
       * @brief Get the number of one-qubit gates applied to each individual qubit.
       *
       * @return Keys: qubit indices; values: number of one-qubit gates.
       */
      std::map<int,int> one_qubit_gate_depths() const;

      /**
       * @brief Get the number of two-qubit gates applied to each individual qubit.
       *
       * @return Keys: qubit indices; values: number of two-qubit gates.
       */
      std::map<int,int> two_qubit_gate_depths() const ;

      /**
       * @brief Get estimated circuit execution times on hardware.
       *
       * @return Estimated hardware runtimes, in ms. Keys:
       *    0: Total time
       *    1: Initialisation component
       *    2: Gate (max. depth) component
       *    3: Readout component
       *    4: Total time (from classical simulation)
       *    5: PC transfer to controller time
       */
      std::map<int,double> timing_estimates() const;

      /**
       * @brief Get the output expected value in the Z basis, from the shot counts observed.
       *
       * @return Expected value in the Z basis
       */
      double z_op_expectation() const;

      /// @brief Set the SPAM correction matrix by providing an equivalent SPAM confusion matrix
      void set_SPAM_confusion_matrix(Eigen::MatrixXd mat);

      /// @brief Retrieve an equivalent confusion matrix from the SPAM correction matrix
      Eigen::MatrixXd get_SPAM_confusion_matrix() const;

      /// Validate the run i.e. ensure all configurations are set in a valid manner.
      void validate();

      #ifdef WITH_CUDAQ
        /// Execute all quantum tasks requiring CUDA-Q
        void run_cudaq();
      #endif

      /**
       * @brief Execute a standard SPAM benchmark, and use the measured confusion
       * matrix to automatically correct SPAM errors in a consecutive `run()`
       *
       * Arguments:
       * @param n_shots : The number of shots to be used for the SPAM benchmark.
       * Defaults to 0, taking the same number of shots as set in sns_.
       */
      void run_with_SPAM(size_t n_shots = 0);

      /// Execute all quantum tasks
      /// Returns a job handle if the job is posted to a remote accelerator (e.g. AWS Braket).
      /// Otherwise, returns null if this function completes the run locally.
      std::shared_ptr<async_job_handle> run();

      /// Cancel any in-flight asynchronous execution of run()
      void cancel_run();

      /**
       * @brief Returns the (base-10) integer vector index for the probabilities/
       * counts vector, corresponding to a bitstring.
       *
       * @param bitvec The bit-vector to be converted to the vector index
       */
      size_t bitstring_index(const std::vector<bool> &bitvec);

      /// Randomly draw (and remove) a single shot from the results map
      std::vector<bool> draw_shot();

    private:

      std::string random_circuit(const size_t n_q, const size_t depth);

      /// @brief Try to work out the form of the circuit input.
      ///
      /// Forms checked first get precedence; fields associated with
      /// other forms get ignored as soon as a valid form is found.
      circuit_origin deduce_circuit_origin();

      /// Helper to populate result tables (e.g. counts, expectation values, resource estimations) post-execution.
      void process_run_result(std::shared_ptr<xacc::CompositeInstruction> kernel_ir,
                              std::shared_ptr<xacc::Accelerator> sim_qpu,
                              const xacc::HeterogeneousMap& sim_qpu_configs,
                              std::shared_ptr<xacc::AcceleratorBuffer> buffer_b, double runtime_ms,
                              std::shared_ptr<qristal::backend> qb_transpiler);

      /// @brief Util method to compile input source string into IR
      ///
      /// This method is thread-safe, thus can be used to compile multiple source strings in parallel.
      std::shared_ptr<xacc::CompositeInstruction> compile_input(const std::string& in_source_string, int in_num_qubits, circuit_language in_source_type);

      /// @brief Retrieve the target circuit string.
      /// This will involve loading a file, generating a random circuit string, etc, depending on the value of \p input_origin.
      /// @param input_origin The origin of the input circuit.
      /// @return The target circuit as a std::string.
      std::string get_target_circuit_qasm_string(circuit_origin input_origin);

      /// Wrap raw OpenQASM string in a QB Kernel:
      /// - Move qreg to a kernel argument
      /// - Denote the kernel name as 'qristal_circuit'
      static std::string convertRawOpenQasmToQBKernel(const std::string &in_rawQasm);

      /// @brief Combine all backend options into a dict (xacc::HeterogeneousMap).
      /// @return A xacc::HeterogeneousMap containing the settings for the backend in use.
      xacc::HeterogeneousMap configure_backend(const YAML::Node& rbdb);

      /// Get the simulator
      std::shared_ptr<xacc::Accelerator> get_sim_qpu(bool execute_on_hardware);

      /// @brief Calculate the gradients for the parametrized quantum task.
      /// This will calculate the gradients of the probabilities of all possible output bitstrings
      /// of the circuit, with respect to each circuit parameter. The session does this by creating
      /// two new session objects and using them to compute the gradients using the "parameter-shift"
      /// rule, where the circuit is run again using slightly shifted parameters.
      void run_gradients();

      /// Execute the circuit on a simulator
      void execute_on_simulator(
          std::shared_ptr<xacc::Accelerator> acc,
          std::shared_ptr<xacc::AcceleratorBuffer> buffer_b,
          std::shared_ptr<xacc::CompositeInstruction>& circuit);

      /// Populate a given counts map with results from QPU execution.
      /// Templated measure_counts_map to support different type of map-like data.
      template <typename CountMapT>
      void populate_measure_counts_data(const CountMapT& measure_counts_map) {

        if (measure_counts_map.empty()) {
          if (debug) std::cout << "Zero counts returned!" << std::endl;
          return;
        }

        // Check that the number of qubits is set correctly
        int qbits_meas = measure_counts_map.begin()->first.length();
        if (qbits_meas > qn) {
          throw std::logic_error("Not enough qubits! Set qn to at least " + std::to_string(qbits_meas));
        }

        // Convert count map keys from strings with assumed endianness and directionality to std::vector<bool>, and save results
        results_.clear();
        for (const auto& [bitstring, count] : measure_counts_map) {
          std::vector<bool> bitvector(qbits_meas);
          for (int i = 0; i < qbits_meas; i++) {
            int j = acc_outputs_qbit0_left_ ? i : qbits_meas - (i+1);
            bitvector.at(j) = (bitstring.at(i) != '0');
          }
          results_[bitvector] = count;
        }

        // If requested, save the results to the all_bitstring_counts for computing gradients
        if (calc_all_bitstring_counts) {
          for (const auto &[bitvec, count] : results_) all_bitstring_counts_[bitstring_index(bitvec)] = count;
        }
      }

  };

}

#ifdef USE_MPI

  namespace qristal {

  /**
   * @brief These checks exist to ensure the result types of the session object
   * exactly match the types used to serialise those results over MPI. If these
   * types change, the implementation will break and undefined behaviour will
   * be introduced.
   * @note This macro and associated static assertion are the simplest mechanism
   * available to implement serialisation data type checks without causing
   * additional obfuscation of the types of the results variables in the session
   * class, by assigning them additional type aliases.
   */
  #define CHECK_SESSION_RESULT_TYPE(type1, type2)                                \
    static_assert(                                                               \
        std::is_same_v<type1, type2>,                                            \
        "Results types in the session class must exactly match those used in "   \
        "MPI serialisation. Serialisation/deserialisation functions have been "  \
        "written which rely on types being identical. If the types require "     \
        "changing, these functions need to be reviewed so that the necessary "   \
        "changes can be made to avoid undefined behaviour.")

  CHECK_SESSION_RESULT_TYPE(session::ResultsMapQubitsType, mpi::Qubits);
  CHECK_SESSION_RESULT_TYPE(session::ResultsMapCountType, mpi::Count);
  CHECK_SESSION_RESULT_TYPE(session::ResultsMapType, mpi::ResultsMap);
  CHECK_SESSION_RESULT_TYPE(session::NativeResultsMapType, mpi::ResultsMap);

  CHECK_SESSION_RESULT_TYPE(session::CountType, mpi::Count);
  CHECK_SESSION_RESULT_TYPE(session::OutCountsType, mpi::OutCounts);

  CHECK_SESSION_RESULT_TYPE(session::ProbabilityType, mpi::Probability);
  CHECK_SESSION_RESULT_TYPE(session::OutProbabilitiesType, mpi::OutProbabilities);
  CHECK_SESSION_RESULT_TYPE(session::ProbabilityGradientType, mpi::Probability);
  CHECK_SESSION_RESULT_TYPE(session::OutProbabilityGradientsType,
                            mpi::OutProbabilityGradients);

  // // Also ensure the count type used in the results map is the same used in
  // all_bitstring_counts
  CHECK_SESSION_RESULT_TYPE(session::ResultsMapCountType, session::CountType);

  } // namespace qristal

#endif

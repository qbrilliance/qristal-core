// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include <qristal/core/backend.hpp>
#include <qristal/core/backend_utils.hpp>
#include <qristal/core/backends/qb_hardware/qb_qpu.hpp>
#include <qristal/core/benchmark/metrics/ConfusionMatrix.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/passes/circuit_opt_passes.hpp>
#include <qristal/core/pretranspiler.hpp>
#include <qristal/core/profiler.hpp>
#include <qristal/core/session.hpp>

// MPI
#ifdef USE_MPI
  #include <qristal/core/mpi/results_types.hpp>
  #include <qristal/core/mpi/workload_partitioning.hpp>
#endif

// CUDAQ support
#ifdef WITH_CUDAQ
  #include <qristal/core/cudaq/sim_pool.hpp>
  #include <qristal/core/cudaq/cudaq_acc.hpp>
#endif

// STL
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <optional>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>

// XACC
#include <AcceleratorBuffer.hpp>
#include <CompositeInstruction.hpp>
#include <xacc.hpp>

// Boost
#include <boost/dynamic_bitset.hpp>

// dlopen
#include <dlfcn.h>

// fmt
#include <fmt/format.h>

// range v3
#include <range/v3/view/zip.hpp>


// Helper functions
namespace
{
  // Whitespace characters
  constexpr std::string_view whitespace = " \t\n\r\f\v";

  // Trim whitespace off both ends of a string
  inline void trim(std::string& s) {
    s.erase(0, s.find_first_not_of(whitespace));
    s.erase(s.find_last_not_of(whitespace) + 1);
  }

  // Check if a string starts with a given substring, ignoring whitespace.
  inline bool starts_with_ex_whitespace(std::string sub, std::string s) {
    s.erase(0, s.find_first_not_of(whitespace));
    return s.starts_with(sub);
  }

  double getExpectationValueZ(std::shared_ptr<xacc::AcceleratorBuffer> buffer) {
    double aver = 0.0;
    auto has_even_parity = [](const std::string &x) -> bool {
      int c = std::count(x.begin(), x.end(), '1');
      return c%2 == 0;
    };
    if (buffer->hasExtraInfoKey("ro-fixed-exp-val-z")) {
      aver = mpark::get<double>(buffer->getInformation("ro-fixed-exp-val-z"));
    } else if (buffer->hasExtraInfoKey("exp-val-z")) {
      aver = mpark::get<double>(buffer->getInformation("exp-val-z"));
    } else {
      if (buffer->getMeasurementCounts().empty()) {
        xacc::error("called getExpectationValueZ() on an AcceleratorBuffer with no measurements!");
      }
      for (auto &kv : buffer->getMeasurementCounts()) {
        bool par = has_even_parity(kv.first);
        auto p = buffer->computeMeasurementProbability(kv.first);
        if (!par) p = -p;
        aver += p;
      }
    }
    return aver;
  }
}


namespace qristal
{
  /// Default session constructor
  session::session() {
    xacc::Initialize();
    xacc::setIsPyApi();
    circuit_opts = {create_circuit_optimizer_pass()};
  }
  session::session(const bool msb) : session() { all_bitstring_counts_ordered_by_MSB_ = msb; }

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
    [RandomQcircuit (string)] generated random quantum circuit in OpenQasm format

    Raises:
    NONE
  **/
  std::string session::random_circuit(const size_t n_q, const size_t depth) {
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

  /// Retrieve the target circuit string
  std::string session::get_target_circuit_qasm_string(circuit_origin input_origin) {
    std::string target_circuit;
    if (input_origin == circuit_origin::infile) {
        // File input: load from file
        std::ifstream tifs(infile);
        if (tifs.is_open()) {
          std::string outbuf(std::istreambuf_iterator<char>(tifs), {});
          // Check for raw OpenQASM string: starts with "OPENQASM" (not already wrapped in __qpu__)
          target_circuit = starts_with_ex_whitespace("OPENQASM", outbuf) ? convertRawOpenQasmToQBKernel(outbuf) : outbuf;
        }
    } else if (input_origin == circuit_origin::instring) {
      // String input
      target_circuit = instring;
    } else if (input_origin == circuit_origin::random_circuit) {
      // Random input: generate random circuit
      if (nooptimise) {
        std::cout << "Warning! Optimising a random circuit may take a long time!" << std::endl;
      }
      target_circuit = random_circuit(qn, random_circuit_depth);
      if (debug) std::cout << "Recording the random circuit to instring" << std::endl;
      instring += "\n# Random circuit created:\n\n";
      instring += target_circuit;
    } else if (input_origin == circuit_origin::IR) {
      if (debug) {
        std::cout << "Using a directly created XACC IR" << std::endl;
      }
    } else {
      throw std::invalid_argument("This circuit type is not implemented");
    }

    // Pre-transpile for OpenQASM.
    if (input_origin != circuit_origin::IR and input_language == circuit_language::OpenQASM) {
      if (debug) std::cout << "[debug]: Circuit before pre-transpile:" << std::endl << target_circuit << std::endl;

      if (acc == "aer") {
        target_circuit = aer_circuit_transpiler(target_circuit);
        if (debug) std::cout << "[debug]: Circuit after aer pre-transpile:" << std::endl << target_circuit << std::endl;
      }

      // Qristal multi-control gates
      if (debug) std::cout << "[debug]: Start to insert QB specific gates" << std::endl;
      Pretranspile qbgpre = Pretranspile("QB specific gates inserted");
      qbgpre.add_n_control_gates(target_circuit);
      if (debug) std::cout << "[debug]: Circuit after inserting QB specific gates:" << std::endl << target_circuit << std::endl;

      // Insert include file for QB: include_qb
      if (debug) std::cout << "[debug]: Include file for QB: " << include_qb << std::endl;
      std::ifstream tifs(include_qb);
      if (tifs.is_open()) {
        std::string target_includeqb(std::istreambuf_iterator<char>(tifs), {});
        std::stringstream anchor_second;
        anchor_second << "include \"qelib1.inc\";" << std::endl << target_includeqb;
        target_circuit = std::regex_replace(target_circuit, std::regex("include \"qelib1.inc\";"), anchor_second.str());
        if (debug) std::cout << "[debug]: Circuit after custom include file for QB:" << std::endl << target_circuit << std::endl;
      } else if (debug) std::cout << "[debug]: Could not find the QB custom include file named: " << include_qb << std::endl;
    }

    return target_circuit;
  }

  void session::execute_on_simulator(
      std::shared_ptr<xacc::Accelerator> qpu,
      std::shared_ptr<xacc::AcceleratorBuffer> buffer_b,
      std::shared_ptr<xacc::CompositeInstruction>& circuit) {
    try {
      // There were error mitigation settings
      if (!noise_mitigation.empty()) {
        if (noise) {
          auto noise_mitigated_acc = [&]() {
            if (noise_mitigation == "rich-extrap") {
              // Noise scaling factors that we use
              const std::vector<int> noise_scalings{1, 3, 5};
              return xacc::getAcceleratorDecorator(noise_mitigation, qpu, {{"scale-factors", noise_scalings}});
            }
            return xacc::getAcceleratorDecorator(noise_mitigation, qpu);
          }();
          if (noise_mitigated_acc) {
            qpu = noise_mitigated_acc;
          } else {
            std::cout << "Noise mitigation module '"
                      << noise_mitigation
                      << "' was not properly initialized. Please check "
                         "your installation.\n";
          }
        } else {
          // Noise was disabled, nothing to do
          if (debug) {
            std::cout
                << "# Noise was set to False. Error mitigation is skipped."
                << std::endl;
          }
        }
      }

      qpu->execute(buffer_b, circuit);
    } catch (...) {
      throw std::invalid_argument(
          "The simulation of your input circuit failed");
    }
  }

  /// Get the simulator
  std::shared_ptr<xacc::Accelerator> session::get_sim_qpu(bool execute_on_hardware)
  {
    // If a hardware accelerator was selected, we select "tnqvm" as the sim acc
    std::string sim_acc = (execute_on_hardware ? "tnqvm" : acc);

    #ifdef WITH_CUDAQ
      // If a CUDAQ backend sim was requested, returns its xacc::Accelerator wrapper.
      if (xacc::container::contains(cudaq_sim_pool::get_instance().available_simulators(), sim_acc)) {

        if (debug) std::cout << "# Using CUDA Quantum Simulator backend: " << sim_acc << std::endl;

        if (sim_acc != "cudaq:qpp" && gpu_device_ids.empty()) {
          throw std::invalid_argument("No GPU device specified. Please specify GPU devices in gpu_device_ids option.");
        }

        xacc::HeterogeneousMap qpu_options {
          {"shots", static_cast<int>(sn_this_process)},
          {"initial-bond-dim", initial_bond_dimension},
          {"max-bond-dim", max_bond_dimension},
          {"abs-truncation-threshold", svd_cutoff},
          {"rel-truncation-threshold", rel_svd_cutoff},
          {"measurement-sampling-method", measure_sample_method},
          {"gpu-device-ids", gpu_device_ids}
        };

        // Additional options for qb-purification
        if (sim_acc == "cudaq:qb_purification") {
          qpu_options.insert("initial-kraus-dim", initial_kraus_dimension);
          qpu_options.insert("max-kraus-dim", max_kraus_dimension);
        }

        if (noise) {
          if (debug) std::cout << "# Noise model for " << sim_acc << " (from emulator package): enabled" << std::endl;
          qpu_options.insert("noise-model", noise_model.get());
        }

        auto cudaq_accelerator = std::make_shared<qristal::cudaq_acc>(sim_acc);
        cudaq_accelerator->initialize(qpu_options);

        return cudaq_accelerator;
      }
    #endif

    // Load emulator if an emulator backend is selected.
    if (EMULATOR_BACKENDS.count(sim_acc) != 0) {
      // Load emulator library
      static const char *EMULATOR_LIB_NAME = "libqristal_emulator.so";
      void *handle = dlopen(EMULATOR_LIB_NAME, RTLD_LOCAL | RTLD_LAZY);
      if (handle == NULL) {
        std::cout
            << "The accelerator you are searching for may be available in the "
               "Qristal Emulator plugin. Please see "
               "https://quantumbrilliance.com/quantum-brilliance-emulator."
            << std::endl;
      }
    }

    // Set up the options for the accelerator
    xacc::HeterogeneousMap qpu_options {{"seed", static_cast<int>(seed)}};

    // Additional settings for TNQVM
    if (sim_acc == "tnqvm") {
      xacc::set_verbose(false);
      qpu_options.insert("tnqvm-visitor", "exatn-mps");
      qpu_options.insert("max-bond-dim", max_bond_dimension);
      qpu_options.insert("svd-cutoff", svd_cutoff);
    }

    // Additional settings for AER
    if (sim_acc == "aer") {
      // Omit shots if the state vector is requested. This triggers Xacc AER to use
      // the statevector simulation type instead of qasm simulation type. The former
      // then populates its ExecutionInfo::WaveFuncKey.
      if (not calc_state_vec) {
        qpu_options.insert("shots", static_cast<int>(sn_this_process));
      } else if (sn_this_process > 0) {
        std::cout << "Warning: Requesting AER state vector will ignore shot sampling!\n";
      }
      // If state vector is requested, check that the statevector backend is chosen;
      // throw an error otherwise
      if (calc_state_vec and aer_sim_type != "statevector") {
        throw std::invalid_argument("Requesting the state vector data requires using the 'statevector' backend.");
      }

      if (!aer_sim_type.empty()) {
        qpu_options.insert("sim-type", aer_sim_type);
        if (debug)
          std::cout << "# Using AER simulation method: " << aer_sim_type
                    << std::endl;
      } else {
        if (debug)
          std::cout << "# Using default AER simulation method." << std::endl;
      }

      if (aer_omp_threads == 0) {
        char* omp_num_threads = std::getenv("OMP_NUM_THREADS");
        if (omp_num_threads != NULL) aer_omp_threads = strtol(omp_num_threads, nullptr, 10);
        else if (debug) std::cout << "# AER will determine how many threads to use for itself." << std::endl;
      }
      if (aer_omp_threads != 0) {
        qpu_options.insert("max_parallel_threads", aer_omp_threads);
        if (debug)std::cout << "# Allowing AER simulator to use up to " << aer_omp_threads
                             << " OpenMP threads." << std::endl;
      }

      if (noise) {
        qpu_options.insert("noise-model", noise_model->to_json());
        qpu_options.insert("qobj-compiler", noise_model->get_qobj_compiler());
      }
    }

    // Additional settings for emulator backends
    if (EMULATOR_BACKENDS.count(sim_acc) != 0) {
      // Tensor network settings
      if (sim_acc != "qb-statevector") {
        if (gpu_device_ids.empty()) throw std::invalid_argument("Please specify GPU devices in gpu_device_ids option.");
        qpu_options.insert("initial-bond-dim", initial_bond_dimension);
        qpu_options.insert("max-bond-dim", max_bond_dimension);
        qpu_options.insert("abs-truncation-threshold", svd_cutoff);
        qpu_options.insert("rel-truncation-threshold", rel_svd_cutoff);
        qpu_options.insert("measurement-sampling-method", measure_sample_method);
        if (sim_acc == "qb-purification") { // Additional options for qb-purification
          qpu_options.insert("initial-kraus-dim", initial_kraus_dimension);
          qpu_options.insert("max-kraus-dim", max_kraus_dimension);
        }
      }

      if (!gpu_device_ids.empty()) qpu_options.insert("gpu-device-ids", gpu_device_ids);

      if (noise) qpu_options.insert("noise-model", noise_model);
    }

    return xacc::getAccelerator(sim_acc, qpu_options);
  }

  /// Helper function to check that a setting is in a set of allowed values
  inline void check_allowed(std::unordered_set<std::string_view> list, std::string_view val, std::string_view name) {
    if (list.find(val) != list.end()) return;
    std::stringstream err;
    err << "Qristal: valid settings for " << name << ":" << std::endl;
    for (auto it : list) err << "* \"" << it << "\"" << std::endl;
    throw std::range_error(err.str());
  }

  /// Helper function to check that a setting is within allowed numerical bounds
  template <typename T>
  inline void check_bounds(T val, std::pair<T,T> bounds, std::string_view name) {
    if (val >= bounds.first and val <= bounds.second) return;
    std::stringstream err;
    err << "Qristal: value " << val << " for " << name << " is not within allowed range [" << bounds.first << "," << bounds.second << "]" << std::endl;
    throw std::range_error(err.str());
  }

  // Check that a session object is consistently configured.
  void session::validate() {

    // Make sure that calc_all_bitstring_counts is set if calc_gradients is set
    if (calc_gradients and not calc_all_bitstring_counts) {
      std::cerr << "╭────────────────────────────────────────────────────────────╮" << std::endl;
      std::cerr << "│ Warning: calc_gradients implies calc_all_bitstring_counts. │" << std::endl;
      std::cerr << "│         Setting calc_all_bitstring_counts = true.          │" << std::endl;
      std::cerr << "╰────────────────────────────────────────────────────────────╯" << std::endl;
      calc_all_bitstring_counts = true;
    }

    // Load the remote backend database and add names of hardware devices to valid backends
    remote_backend_database_ = YAML::LoadFile(remote_backend_database_path);
    std::unordered_set<std::string> REMOTE_BACKENDS;
    for (YAML::const_iterator it = remote_backend_database_.begin(); it != remote_backend_database_.end(); ++it) {
      REMOTE_BACKENDS.emplace(it->first.as<std::string>());
    }
    std::unordered_set<std::string_view> VALID_BACKENDS = EMULATOR_BACKENDS;
    VALID_BACKENDS.insert(NON_EMULATOR_BACKENDS.begin(), NON_EMULATOR_BACKENDS.end());
    VALID_BACKENDS.insert(REMOTE_BACKENDS.begin(), REMOTE_BACKENDS.end());

    // Add CUDAQ backends to VALID_BACKENDS
    #ifdef WITH_CUDAQ
      std::unordered_set<std::string> CUDAQ_BACKENDS;
      for (const auto &cudaq_sim : cudaq_sim_pool::get_instance().available_simulators()) {
        CUDAQ_BACKENDS.emplace(cudaq_sim);
      }
      VALID_BACKENDS.insert(CUDAQ_BACKENDS.begin(), CUDAQ_BACKENDS.end());
    #endif

    // Check that chosen options are allowed
    check_allowed(VALID_BACKENDS, acc, "acc");
    check_allowed(VALID_AER_SIM_TYPES, aer_sim_type, "aer_sim_type");
    check_allowed(VALID_MEASURE_SAMPLING_OPTIONS, measure_sample_method, "measure_sample_method");
    check_allowed(VALID_NOISE_MITIGATIONS, noise_mitigation, "noise_mitigation");

    // Check that the chosen placement options are allowed
    if (not noplacement) {
      // Make sure that noise is turned on if placement is turned on
      if (not noise) throw std::invalid_argument("Placement requires connectivity map, which can only be obtained if noise=true.");
      // Make sure the chosen placement transformation is allowed, and can be loaded
      check_allowed(VALID_HARDWARE_PLACEMENTS, placement, "placement");
      if (not xacc::getIRTransformation(placement)) {
        std::cout << "Placement module '" << placement << "' cannot be located. Please check your installation." << std::endl;
      }
    }

    // Check that the chosen optimisation options are allowed
    if (not nooptimise) {
      if (input_language == circuit_language::XASM) throw std::invalid_argument(
       "Circuits provided in XASM cannot be run through the Qristal optimisation pipeline. Please set nooptimise=true.");
    }

    // Make sure chosen options are within allowed bounds
    if (execute_circuit) check_bounds(sn, sn_bounds, "sn");
    check_bounds(qn, qn_bounds, "qn");
    check_bounds(initial_bond_dimension, initial_bond_bounds, "initial_bond_dimension");
    check_bounds(max_bond_dimension, max_bond_bounds, "max_bond_dimension");
    check_bounds(initial_kraus_dimension, initial_kraus_bounds, "initial_kraus_dimension");
    check_bounds(max_kraus_dimension, max_kraus_bounds, "max_kraus_dimension");
    check_bounds(svd_cutoff, svd_cutoff_bounds, "svd_cutoff");
    check_bounds(rel_svd_cutoff, svd_cutoff_bounds, "rel_svd_cutoff");
    check_bounds(random_circuit_depth, random_circuit_depth_bounds, "random_circuit_depth");

    // Generate random seed if one has not been chosen.
    if (seed == 0) {
      if (debug) std::cout << "# No seed specified; generating one using std::random_device." << std::endl;
      seed = []() {
        static std::random_device dev;
        static std::mt19937 rng(dev());
        static std::uniform_int_distribution<std::mt19937::result_type> dist(0, std::numeric_limits<size_t>::max());
        return dist(rng);
      }();
    }
    if (debug) std::cout << "# Seed value: " << seed << std::endl;

    // Emit a warning if the user's choice of backend and number of qubits runs the risk of overflowing memory
    std::string acc_annotated = acc;
    if (acc == "qb-statevector") acc_annotated += (gpu_device_ids.empty() ? "-cpu" : "-gpu");
    if (acc == "aer") acc_annotated += ("-" + aer_sim_type);
    auto it = MAX_QUBITS_ACCS.find(acc_annotated);
    if (it != MAX_QUBITS_ACCS.end() and qn > it->second) {
      std::cout << "Warning: The selected accelerator may not support the requested number of qubits." << std::endl;
    }

    // Check validity of noise settings
    if (noise) {

      // Make sure that the selected backend supports noise
      if (NOISY_BACKENDS.find(acc) == NOISY_BACKENDS.end()) {
        throw std::runtime_error("Backend " + acc + " does not support noise. Please turn off noise or use a different backend.");
      }

      // If a noisy sim is requested but no noise model specified, construct the default model.
      if (not noise_model) noise_model = std::make_shared<NoiseModel>("default", qn);

      // Emit a warning if the chosen noise model has noise defined in terms of gates that are not natively supported by the chosen backend.
      std::vector<std::string> basis_gates = noise_model->get_qobj_basis_gates();
      std::unordered_map<std::string, std::map<std::vector<size_t>, std::vector<NoiseChannel>>> noise_channels = noise_model->get_noise_channels();
      for (const auto &[gate, channel] : noise_channels) {
        auto it = std::find(basis_gates.begin(), basis_gates.end(), gate);
        if (it == basis_gates.end()) {
          std::cout << "Warning: Supplied gate " << gate << " is not a basis gate of the chosen backend; noise "
                    << "will not be applied to this gate. Please set the correct qobj_compiler." << std::endl
                    << "Current qobj_compiler " << noise_model->get_qobj_compiler() << " has basis gates: { "
                    << fmt::to_string(fmt::join(basis_gates, " ")) << "}" << std::endl;
        }
      }

      if (debug) std::cout << "# Noise enabled with model " << noise_model->name << "." << std::endl;

    } else {

      // Make sure that the selected backend supports running without noise
      if (EXCLUSIVELY_NOISY_BACKENDS.find(acc) != EXCLUSIVELY_NOISY_BACKENDS.end()) {
        throw std::runtime_error("Backend " + acc + " only works with noise. Please turn on noise or use a different backend.");
      }

      if (debug) std::cout << "# Noise disabled. " << std::endl;
    }

    // Make sure that any provided SPAM correction matrix has the right dimension, and enable SPAM correction if so.
    if (SPAM_correction_matrix.rows() != 0 or SPAM_correction_matrix.cols() != 0) {
      size_t dim = std::pow(2, qn);
      assert(SPAM_correction_matrix.rows() == dim && SPAM_correction_matrix.cols() == dim && "Mismatching dimensions of SPAM correction matrix and numbers of qubits!");
      perform_SPAM_correction_ = true;
    }

    // Clear all non-optional outputs
    results_.clear();
    transpiled_circuit_.clear();
    qobj_.clear();
    qbjson_.clear();
    one_qubit_gate_depths_.clear();
    two_qubit_gate_depths_.clear();
    timing_estimates_.clear();

    // Clear and size the optional outputs
    const long long int num_outputs = ipow(2, qn);
    const size_t num_params = circuit_parameters.size();

    if (calc_all_bitstring_counts) {
      all_bitstring_counts_.clear();
      constexpr double size_ratio_dbl_to_int = double(sizeof(double(0)))/sizeof(int(0));
      // We need 2^nq ints for all_bitstring_counts plus (potentially) num_params * 2^nq doubles for jacobians + 2^nq doubles for probs
      double scalefactor = 1 + (calc_gradients ? size_ratio_dbl_to_int * (num_params + 1): 0);
      // Check that there is enough free memory to store everything
      if (all_bitstring_counts_.max_size() > scalefactor*double(num_outputs))
      {
        try { all_bitstring_counts_.resize(num_outputs); }
        catch(std::exception& e) {
          std::string err = "Your RAM use is too fragmented to allocate a large enough "
           "std::vector<int> to hold integer bitstring representations.\nPlease free up more memory, use calc_all_bitstring_counts = false,";
          if (calc_gradients) err += ", use less circuit parameters, or use set_calc_gradients(false)";
          err += ".";
          throw std::logic_error(err);
        }
      }
      else {
        std::ostringstream err;
        err << "There are too many " << qn << "-bit bitstrings to fit in a "
            << "std::vector<int> in integer representation on this system for this circuit." << std::endl
            << "Maximum qubits that can be fitted in memory for this circuit: " << (int)std::floor(std::log(all_bitstring_counts_.max_size()/scalefactor)/std::log(2)) << "."
            << std::endl << "Please use calc_all_bitstring_counts = false";
        if (calc_gradients) err << ", use less circuit parameters, or use calc_gradients = false";
        std::cout << ".";
        throw std::logic_error(err.str());
      }
    }

    if (calc_gradients) {
      all_bitstring_probabilities_.clear();
      all_bitstring_probability_gradients_.clear();
      try {
        all_bitstring_probabilities_.resize(num_outputs);
        all_bitstring_probability_gradients_.resize(num_params,std::vector<double>(num_outputs));
      }
      catch (std::exception& e) {
        throw std::logic_error("Your RAM use is too fragmented to allocate a large enough std::vector<double> to hold all relevant gradients.\n"
                               "Please use less circuit parameters, free up more memory, or use calc_gradients = false.");
      }
    }

  }


  // Work out where the circuit to be run comes from.
  session::circuit_origin session::deduce_circuit_origin() {

    // 1. User-provided instring.
    if (not instring.empty()) {

      // Remove leading/trailing white spaces
      trim(instring);

      // 1.1 Check if the string starts with "__qpu"
      if (instring.find("__qpu") == 0) {
        // instring is a quantum circuit kernel
        if (debug) std::cout << "[debug]: __qpu detected - instring contains a quantum kernel." << std::endl;
        return circuit_origin::instring;
      }

      // 1.2 Check for raw OpenQASM string.
      else if (starts_with_ex_whitespace("OPENQASM", instring)) {
        instring = convertRawOpenQasmToQBKernel(instring);
        if (debug) std::cout << "[debug]: Raw OpenQASM source detected. Converted to a QB quantum kernel." << std::endl;
        return circuit_origin::instring;
      }

      // 1.3 Oops, instring is just a text description. Better look elsewhere.
      else if (debug) std::cout << "[debug]: instring will be used as descriptive text." << std::endl;
    }

    // 2. Random circuit generation, compilation and execution.
    if (random_circuit_depth > 0) return circuit_origin::random_circuit;

    // 3. User-provided input filename.
    if (not infile.empty()) {
      std::ifstream tifs(infile);
      if (tifs.is_open()) {
        if (debug) std::cout << "[debug]: Opened input file named: " << infile << "." << std::endl;
        return circuit_origin::infile;
      } else {
        throw std::invalid_argument("Input file not found: " + infile);
      }
    }

    // 4. User-provided IR tree.
    else if (irtarget) {
      if (debug) std::cout << "[debug]: XACC IR target detected." << std::endl;
      return circuit_origin::IR;
    }

    // 5. CUDAQ input
    else if (cudaq_kernel) {
      if (debug) std::cout << "[debug]: found CUDAQ kernel." << std::endl;
      return circuit_origin::CUDAQ;
    }

    // 6. None of the above - bail out.
    else throw std::invalid_argument("Error: no circuit set. Please set instring, random_circuit_depth, infile, irtarget or cudaq_kernel to a valid value.");

  }

  void session::run_with_SPAM(size_t n_shots) {
    std::cerr << "╭────────────────────────────────────────────────────────╮" << std::endl;
    std::cerr << "│ Warning: Called run() with automatic SPAM measurement! │" << std::endl;
    std::cerr << "│        I will execute a new SPAM benchmark now!        │" << std::endl;
    std::cerr << "╰────────────────────────────────────────────────────────╯" << std::endl;
    //(1) create a copy of this session and set the numbers of shots
    session sim_cp = *this;
    if (n_shots == 0) n_shots = sn;
    sim_cp.sn = n_shots;

    //(2) execute and evaluate a SPAM benchmark
    std::set<size_t> qubits;
    for (size_t q = 0; q < qn; ++q) {
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

  void session::run_gradients() {

    // Skip gradients if no counts have been returned by the backend
    if (all_bitstring_counts_.empty()) return;

    // Initialize
    std::shared_ptr<xacc::CompositeInstruction> target_circuit = irtarget;
    std::vector<std::shared_ptr<xacc::CompositeInstruction>> gradient_circs;
    const size_t num_params = circuit_parameters.size();

    // Calculate param-shift gradient
    for (size_t i = 0; i < 2*num_params; i+=2) {
      std::vector<double> vals(circuit_parameters);
      vals[i/2] += M_PI_2; // Increase by pi/2
      auto evaled_circ_plus = (*target_circuit)(vals);
      vals[i/2] -= M_PI; // Reduce by pi to get -pi/2 overall
      auto evaled_circ_minus = (*target_circuit)(vals);
      gradient_circs.push_back(evaled_circ_plus);
      gradient_circs.push_back(evaled_circ_minus);
    }

    // Create new session objects to run shifted gradient circuits.
    std::vector<session> gradient_sessions(gradient_circs.size(), *this);
    for (auto [s, circ] : ::ranges::views::zip(gradient_sessions, gradient_circs)) {
      s.calc_all_bitstring_counts = true;
      s.irtarget = circ;
      s.calc_gradients = false;
      #ifdef USE_MPI
        s.mpi_acceleration_enabled = false;
      #endif
      s.run();
    }

    // Construct the probabilities
    const size_t num_outputs = ipow(2, qn);
    for (size_t i = 0; i < num_outputs; i++) {
      all_bitstring_probabilities_.at(i) = all_bitstring_counts_.at(i) / (1.0 * sn_this_process);
    }

    // Construct the Jacobian
    std::vector<std::vector<double>>& jacobian = all_bitstring_probability_gradients_;
    for (size_t i = 0; i < 2*num_params; i += 2) {
      // Get the counts for the shifted values of the parameter
      const std::vector<int>& counts_plus = gradient_sessions.at(i).all_bitstring_counts();
      const std::vector<int>& counts_minus = gradient_sessions.at(i+1).all_bitstring_counts();
      // Compute the entry in the Jacobian
      for (size_t j = 0; j < num_outputs; j++) {
        jacobian[i/2][j] = 0.5 * (counts_plus[j] - counts_minus[j]) / (1.0 * sn_this_process);
      }
    }
  }

  /// Util method to compile input source string into IR
  /// This method is thread-safe, thus can be used to compile multiple source strings in parallel.
  std::shared_ptr<xacc::CompositeInstruction> session::compile_input(const std::string& in_source_string, int in_num_qubits,
                                                                     circuit_language in_source_type)
  {
    // Retrieve the compiler instance for the QASM dialect
    auto compiler = [&]() -> std::shared_ptr<xacc::Compiler>
    {
      switch (in_source_type)
      {
        case circuit_language::OpenQASM:
          return xacc::getCompiler("staq");
        case circuit_language::XASM:
          return xacc::getCompiler("xasm");
        case circuit_language::Quil:
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
    if (in_source_type == circuit_language::OpenQASM && in_source_string.find("__qpu__") != std::string::npos)
    {
      std::string tmp            = "";
      const auto first_brace_pos = in_source_string.find_first_of("{");
      const auto last_brace_pos  = in_source_string.find_last_of("}");
      const auto sub             = in_source_string.substr(first_brace_pos + 1, last_brace_pos - first_brace_pos - 1);
      auto lines                 = xacc::split(sub, '\n');
      // Flag to insert register declaration only once after the first include.
      bool added_reg_decl = false;
      for (auto& l : lines) {
        trim(l);
        tmp += l + "\n";
        if (l.find("include") != std::string::npos) {
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

  // Terminate the job if still running.
  void session::cancel_run() {
    if (qpu_) qpu_->cancel();
  }

  std::shared_ptr<async_job_handle> session::run()
  {
    // Validate run configuration
    validate();

    // Set the xacc verbose flag according to the value of the debug flag
    xacc::set_verbose(debug);

    // Determine input circuit providence
    auto input_origin = deduce_circuit_origin();

    // Set the number of shots available to be drawn after the run completes
    shots_remaining_ = sn;

    // Work out how many shots this process should actually run
    #ifdef USE_MPI
      int32_t num_mpi_processes = mpi_manager_.get_total_processes();
      int32_t current_process_mpi_id = mpi_manager_.get_process_id();
      sn_this_process = mpi::shots_for_mpi_process(num_mpi_processes, sn, current_process_mpi_id);
    #else
      sn_this_process = sn;
    #endif

    if (input_origin == circuit_origin::CUDAQ) {
      #ifdef WITH_CUDAQ
        // Execute CUDAQ kernel input
        run_cudaq();
        return nullptr;
      #else
        throw std::runtime_error("CUDAQ is not supported. Please build qristal::core with CUDAQ.");
      #endif
    }

    if (debug) {
      std::stringstream debug_msg;
      switch (input_language) {
      case circuit_language::XASM:
        debug_msg << "# XASM compiler: xasm" << std::endl;
        break;
      case circuit_language::Quil:
        debug_msg << "# Quil v1 compiler: quil" << std::endl;
        break;
      case circuit_language::OpenQASM:
        debug_msg << "# OpenQASM compiler: staq" << std::endl;
      }
      debug_msg << "# Qubits: " << qn << std::endl
                << "# Shots total: " << sn << std::endl
                << "# Shots this process: " << sn_this_process << std::endl;
      if (output_oqm_enabled) {
        debug_msg << "# Output transpiled circuit: enabled" << std::endl;
      } else {
        debug_msg << "# Output transpiled circuit: disabled" << std::endl;
      }
      std::cout << debug_msg.str() << std::endl;
    }

    // Has the user asked for a hardware backend?  Check that the backend is in the remote backend database, but not AWS.
    const bool exec_on_hardware = acc != "aws-braket" and remote_backend_database_[acc];

    // Collect all the simulator options
    const xacc::HeterogeneousMap mqbacc = configure_backend(remote_backend_database_);

    // ==============================================
    // Construct/initialize the Accelerator instance
    // ==============================================
    qpu_ = get_sim_qpu(exec_on_hardware);
    auto backend_instance = std::make_shared<qristal::backend>();
    qpu_->updateConfiguration(mqbacc);
    backend_instance->updateConfiguration(mqbacc);

    auto buffer_b = std::make_shared<xacc::AcceleratorBuffer>(qn);
    std::shared_ptr<xacc::CompositeInstruction> citarget;

    // ==============================================
    // ----------------- Compilation ----------------
    // ==============================================
    if (input_origin == circuit_origin::IR) {
      // Direct IR input (e.g., circuit builder)
      std::shared_ptr<xacc::CompositeInstruction> in_circ = irtarget;
      if (in_circ->nVariables() > 0) {
        in_circ = in_circ->operator()(circuit_parameters);
      }
      citarget = in_circ;
    } else {
      // String input -> compile
      const std::string target_circuit = get_target_circuit_qasm_string(input_origin);
      // Note: compile_input may not be thread-safe, e.g., XACC's staq
      // compiler plugin was not defined as Clonable, hence only one instance
      // is available from the service registry.
      citarget = compile_input(target_circuit, qn, input_language);
    }

    // ==============================================
    // -----------------  Placement  ----------------
    // ==============================================
    // Transform the target to account for QB topology: XACC
    if (!noplacement) {
      if (debug) std::cout << "# Quantum Brilliance topological placement: enabled" << std::endl;
      xacc::HeterogeneousMap m;
      if (acc == "aws-braket") m.merge(qpu_->getProperties());
      // Disable QASM inlining during placement.
      // e.g., we don't want to map gates to the IBM gateset (defined in
      // qelib1.inc) during placement.
      m.insert("no-inline", true);
      auto A = xacc::getIRTransformation(placement);
      A->apply(citarget, backend_instance, m);
    }

    // ==============================================
    // ----------  Circuit Optimization  ------------
    // ==============================================
    // Perform circuit optimisation: XACC "circuit-optimizer"
    if (!nooptimise) {
      std::stringstream debug_msg;
      if (debug) std::cout << "# Quantum Brilliance circuit optimiser: enabled" << std::endl;
      for (const auto &pass : circuit_opts) {
        if (debug) std::cout << "# Apply optimization pass: " << pass->get_name() << std::endl;

        // Wrap the composite IR (citarget) as a CircuitBuilder to send on
        // to the optimization pass. Set copy_nodes to false to keep the root
        // node (citarget) intact.
        CircuitBuilder ir_as_circuit(citarget, /*copy_nodes*/ false);

        // Check if the circuit contains control-unitary (C-U) gates, which
        // XACC's CircuitOptimizer is not able to correctly optimise.
        std::shared_ptr<xacc::CompositeInstruction> circ = ir_as_circuit.get();
        xacc::InstructionIterator iter(circ);
        while (iter.hasNext()) {
          xacc::InstPtr next = iter.next();
          if (next->name() == "C-U") {
            std::cout << "This circuit contains a control-unitary gate or a gate constructed from a control-unitary gate, " <<
                         "e.g. a multi-control or generalised multi-control gate. " <<
                         "Such a gate cannot be optimised by the circuit optimiser.\n";
            throw std::runtime_error("Gate error");
          }
        }

        pass->apply(ir_as_circuit);
      }
    }

    // ==============================================
    // ----------  Execution  ------------
    // ==============================================
    buffer_b->resetBuffer();

    xacc::ScopeTimer timer_for_qpu(
        "Walltime, in ms, for simulator to execute quantum circuit", false);

    if (exec_on_hardware) {
      // Hardware execution
      std::shared_ptr<xacc::quantum::qb_qpu> hardware_device = std::make_shared<xacc::quantum::qb_qpu>(acc, debug);
      hardware_device->updateConfiguration(mqbacc);
      if (debug) std::cout << "# " << backend_instance << " accelerator: initialised" << std::endl;

      // Execute (and polling wait)
      execute_on_qb_hardware(hardware_device, buffer_b, citarget, execute_circuit, debug);

      // Store the JSON sent to the QB hardware for the user to inspect
      qbjson_ = hardware_device->get_qbjson();

    } else if (execute_circuit) {
      try {
        if (debug) std::cout << "# Prior to qpu_->execute..." << std::endl;
        if (qpu_->name() == "aws-braket" && std::dynamic_pointer_cast<qristal::remote_accelerator>(qpu_)) {
          // Asynchronously offload the circuit to AWS Braket
          auto as_remote_acc = std::dynamic_pointer_cast<qristal::remote_accelerator>(qpu_);
          auto aws_job_handle = as_remote_acc->async_execute(citarget);
          aws_job_handle->add_done_callback([=](auto &handle) {
            auto buffer_temp = std::make_shared<xacc::AcceleratorBuffer>(buffer_b->size());
            handle.load_result(buffer_temp);
            auto qb_transpiler = std::make_shared<qristal::backend>();
            this->process_run_result(citarget, qpu_, mqbacc, buffer_temp, timer_for_qpu.getDurationMs(), qb_transpiler);
          });
          return aws_job_handle;
        } else {
          // Blocking (synchronous) execution of a local simulator instance
          execute_on_simulator(qpu_, buffer_b, citarget);
        }
      } catch (...) {
        throw std::invalid_argument("The simulation of your input circuit failed");
      }
    }

    // ==============================================
    // ------------  Post processing  ---------------
    // ==============================================

    /// Post-processing results with local backend, i.e., execution occurs on this thread.
    process_run_result(citarget, qpu_, mqbacc, buffer_b, timer_for_qpu.getDurationMs(), backend_instance);

    return nullptr;
  }

  void session::process_run_result(
      std::shared_ptr<xacc::CompositeInstruction> ir_target,
      std::shared_ptr<xacc::Accelerator> sim_qpu,
      const xacc::HeterogeneousMap& sim_qpu_config,
      std::shared_ptr<xacc::AcceleratorBuffer> buffer_b,
      double xacc_scope_timer_qpu_ms,
      std::shared_ptr<qristal::backend> qb_transpiler) {
    if (debug) {
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
    acc_outputs_qbit0_left_ = (sim_qpu->getBitOrder() == 0) or sim_qpu->name() == "tnqvm";

    // Keep the qobj so that a user can call Aer standalone later.
    if (sim_qpu->name() == "aer") qobj_ = sim_qpu->getNativeCode(ir_target, sim_qpu_config);

    // Get Z operator expectation:
    if (execute_circuit) {
      if (buffer_b->hasExtraInfoKey("ro-fixed-exp-val-z") ||
          buffer_b->hasExtraInfoKey("exp-val-z") ||
          (!buffer_b->getMeasurementCounts().empty())) {
        z_op_expectation_ = getExpectationValueZ(buffer_b);
        if (debug) std::cout << "* Z-operator expectation value: " << z_op_expectation_ << std::endl;
      } else {
        xacc::warning("No Z operator expectation available");
      }
    }

    // Get the state vector from qpp or AER
    if (calc_state_vec and
       (sim_qpu->name() == "qpp" || (sim_qpu->name() == "aer" && aer_sim_type == "statevector"))) {
      state_vec_ = sim_qpu->getExecutionInfo<xacc::ExecutionInfo::WaveFuncPtrType>(
                        xacc::ExecutionInfo::WaveFuncKey);

      if (all_bitstring_counts_ordered_by_MSB_) {
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
    if (execute_circuit) {
      // Save the counts to results_
      populate_measure_counts_data(counts_map);
      // If required to calculate gradients, do it now
      if (calc_gradients) run_gradients();
    }

    // Transpile to QB native gates (acc)
    if (output_oqm_enabled) {
      auto buffer_qb = std::make_shared<xacc::AcceleratorBuffer>(qn);
      try {
        qb_transpiler->execute(buffer_qb, ir_target);
      } catch (...) {
        throw std::invalid_argument(
            "Transpiling to QB native gates for your input circuit failed");
      }

      // Save the transpiled circuit string
      transpiled_circuit_ = qb_transpiler->getTranspiledResult();

      // Invoke the Profiler
      Profiler timing_profile(qb_transpiler->getTranspiledResult(), qn, debug);

      // Save single qubit gate qtys to std::map<int,int>
      one_qubit_gate_depths_ = timing_profile.get_count_1q_gates_on_q();

      // Save two-qubit gate qtys to std::map<int,int>
      two_qubit_gate_depths_ = timing_profile.get_count_2q_gates_on_q();

      // Save timing results to std::map<int,double>
      timing_estimates_ = timing_profile.get_total_initialisation_maxgate_readout_time_ms(xacc_scope_timer_qpu_ms, sn);
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
      results_ = apply_SPAM_correction(results_, SPAM_correction_matrix);
    }

    #ifdef USE_MPI
      // Sync data across all processes now that all post processing has been done
      if (mpi_acceleration_enabled && mpi_manager_.get_total_processes() > 1) {

        std::optional<std::reference_wrapper<mpi::ResultsMap>> results_native_local;
        std::optional<std::span<mpi::Count>> all_bitstring_counts_local;
        std::optional<std::span<mpi::Probability>> all_bitstring_probabilities_local;
        std::optional<std::reference_wrapper<mpi::OutProbabilityGradients>> all_bitstring_probability_gradients_local;

        if (perform_SPAM_correction_) results_native_local = results_native_;
        if (calc_all_bitstring_counts) all_bitstring_counts_local = all_bitstring_counts_;
        if (calc_gradients) {
          all_bitstring_probabilities_local = all_bitstring_probabilities_;
          all_bitstring_probability_gradients_local = all_bitstring_probability_gradients_;
        }

        auto current_process_mpi_id = mpi_manager_.get_process_id();
        if (current_process_mpi_id == 0) {
          // This is the supervisor process. Receive, unpack and combine
          // the results from the other worker processes with the results
          // from this process
          mpi::collect_results_from_mpi_processes(
              mpi_manager_, sn, sn_this_process, results_, results_native_local, all_bitstring_counts_local,
              all_bitstring_probabilities_local, all_bitstring_probability_gradients_local);
        } else {
          // This is a worker process. Pack the results and send to the
          // supervisor process
          mpi::send_results_to_supervisor(mpi_manager_, results_, results_native_local, all_bitstring_counts_local,
              all_bitstring_probabilities_local, all_bitstring_probability_gradients_local);
        }
      }
    #endif

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
    if (all_bitstring_counts_ordered_by_MSB_) {
      for (uint i = 0; i < bitvec.size(); i++) if (bitvec[i]) result += 1<<(bitvec.size()-i-1);
    } else {
      for (uint i = 0; i < bitvec.size(); i++) if (bitvec[i]) result += 1<<i;
    }
    return result;
  }

  // Randomly draw (and remove) a single shot from the results map
  std::vector<bool> session::draw_shot() {

    if (shots_remaining_ == 0) throw std::out_of_range("Unable to draw shot as no shots remain.");

    // RNG
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<> gen_shot_index(0, shots_remaining_-1);

    // Randomly generate an integer representing the index of the shot to draw
    const size_t shot_index = gen_shot_index(rng);

    // Iterate through the results map entries until the drawn shot has been reached
    size_t sum = 0;
    std::vector<bool> bitvec;
    for (auto entry = results_.begin(); entry != results_.end(); ++entry) {
      assert(entry->second > 0);
      sum += entry->second;
      // If the drawn shot has been reached, save the bitvector and break the loop
      if (sum >= shot_index) {
        bitvec = entry->first;
        // If this was the last shot for this bitvector, remove it
        if (--(entry->second) == 0) results_.erase(entry);
        break;
      }
    }

    shots_remaining_--;
    return bitvec;
  }


  // Forward declarations of all backend option-setting functions
  void add_qb_hardware_options(xacc::HeterogeneousMap&, YAML::Node& be_info, size_t num_qubits);
  void add_aws_braket_options(xacc::HeterogeneousMap&, YAML::Node& be_info, size_t num_qubits, size_t num_shots);

  // Combine all backend options into a dict (xacc::HeterogeneousMap)
  // Note: this dict is a 'kitchen sink' of all configurations.
  // The xacc::Accelerator may or may not use these configurations.
  xacc::HeterogeneousMap session::configure_backend(const YAML::Node& rbdb) {

    xacc::HeterogeneousMap m;

    // Generic options. XACC backends need qn and sn as ints, not size_ts.
    m.insert("n_qubits", static_cast<int>(qn));
    m.insert("output_oqm_enabled", output_oqm_enabled);
    if (noise) {
      m.insert("noise-model", noise_model->to_json());
      m.insert("noise-model-name", noise_model->name);
      m.insert("m_connectivity", noise_model->get_connectivity());
    }
    // If sn_this_process=0, then execute_circuit = False and we will only do compilation. Tell XACC shots=1 in that case, to avoid it crashing.
    m.insert("shots", static_cast<int>(sn_this_process == 0 ? 1 : sn_this_process));

    // Random seed
    m.insert("seed", static_cast<int>(seed));

    // Attempt to get the entry from the remote backend yaml file corresponding to the user's chosen backend
    YAML::Node be_info = rbdb[acc];

    // If successful, use it to populate the remote backend settings
    if (be_info)
    {
      // Backend-specific options
      if (acc == "aws-braket") {
        add_aws_braket_options(m, be_info, qn, sn);
      }
      else {
        add_qb_hardware_options(m, be_info, qn);
      }
    }

    return m;
  }

}

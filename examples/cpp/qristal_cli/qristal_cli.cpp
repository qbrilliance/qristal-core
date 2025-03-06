// Copyright (c) Quantum Brilliance Pty Ltd
#include "xacc.hpp"
#include "qristal/core/session.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include "qristal/core/profiler.hpp"
#include "qristal/core/pretranspiler.hpp"
#include <string>
#include <fstream>

using namespace qristal;

// Quickstart
void print_quickstart() {
  std::cout << "*" << std::endl;
  std::cout << "* Quickstart:" << std::endl;
  std::cout << "*" << std::endl;
  std::cout << "* To run with your own input circuit, see help instructions:"
            << std::endl;
  std::cout << "*    qristal_cli --help" << std::endl;
  std::cout << "*" << std::endl
            << "* Examples:" << std::endl
            << "*" << std::endl
            << "* Bernstein-Vazirani:" << std::endl
            << "*   qristal_cli "
               "/path/to/02_BernsteinVazirani/"
               "BernsteinVazirani-secret110-openqasm.inc"
            << std::endl
            << "*" << std::endl
            << "* Deutsch-Jozsa:" << std::endl
            << "*   qristal_cli "
               "/path/to/02_DeutschJozsa/"
               "DeutschJozsa-Balanced-string101-openqasm.inc"
            << std::endl
            << "*" << std::endl
            << std::endl;
}

// JSON conversion routines
namespace std {
void from_json(const nlohmann::json &js, std::complex<double> &elem) {
  if (js.is_array()) {
    throw std::invalid_argument(std::string("JSON: invalid complex element."));
  } else {
    std::complex<double> ret{js["r"].get<double>(), js["i"].get<double>()};
    std::cout << ret << std::endl;
    elem = ret;
  }
}

void to_json(nlohmann::json &j, const std::complex<double> &elem) {
  j = nlohmann::json{{"r", elem.real()}, {"i", elem.imag()}};
}

void from_json(const nlohmann::json &js, std::vector<std::complex<double>> &vec) {
  std::vector<std::complex<double>> ret;
  if (js.is_array()) {
    for (auto &elt : js)
      ret.push_back(elt);
    vec = ret;
  } else {
    throw std::invalid_argument(std::string("JSON: invalid complex vector."));
  }
}
} // namespace std

// Print classical wall-time + distribution of shot counts
void print_classical(const qristal::session &s) {
  for (int i = 0; i < s.results().size(); i++) {
    for (int j = 0; j < s.results().at(i).size(); j++) {
      std::cout << "Run " << i << ", " << j << std::endl;
      int qubits = s.results().at(i).at(j).begin()->first.size();
      std::cout << "State";
      if (qubits > 1) std::cout << " (bit " << qubits-1 << " .. bit 0)";
      std::cout << ":  " << "Counts" << std::endl << s.results().at(i).at(j);
    }
  }
  Table2d<std::map<int,double>> ctimes = s.get_out_total_init_maxgate_readout_times();
  double classical_ms = 0.0;
  qristal::Profiler dummypr = qristal::Profiler("__qpu__ void qristal_circuit(qreg q) {\nOPENQASM 2.0;\ninclude \"qelib1.inc\";\ncreg c0[1];\nmeasure q[0] -> c0[0];\n}\n", 1);
  for (auto &it : ctimes) {
    for (auto &itel : it) {
      auto ndelem = itel.find(dummypr.KEY_SIMULATION_TOTAL_TIME);
      if (ndelem != itel.end()) {
        classical_ms = ndelem->second;
      }
    }
  }
  std::cout << std::endl;
  std::cout << "* Classical actual walltime: " << classical_ms << " ms (time elapsed for the simulator to perform the requested number of shots of the quantum circuit)" <<  std::endl;
  std::cout << std::endl;

}

// Print quantum estimated wall-time
void print_quantum(const qristal::session &s, const double gate_1q_time_ms,
                   const double gate_2q_time_ms,
                   const double q_initialisation_time_ms,
                   const double q_readout_time_ms,
                   const double pc_send_to_control_time_ms,
                   const bool debug_session = false, const bool verbose = false) {
  Table2d<std::map<int,double>> qtimes = s.get_out_total_init_maxgate_readout_times();
  double quantum_ms = 0.0;

  Table2d<std::string> out_transps = s.get_out_transpiled_circuits();
  std::vector<std::string> out_transps_0 = out_transps.at(0);
  std::string out_transps_0_0 = out_transps_0.at(0);
  if (verbose) {
    std::cout << "* Transpiled circuit: " << std::endl
              << out_transps_0_0 << std::endl;
  }

  Table2d<size_t> qns = s.get_qns();
  std::vector<size_t> qns_0 = qns.at(0);
  int qn = qns_0.at(0);
  qristal::Profiler tpr = qristal::Profiler(out_transps_0_0, qn);

  for (auto &it : qtimes) {
    for (auto &itel : it) {
      auto ndelem = itel.find(tpr.KEY_TOTAL_TIME);
      if (ndelem != itel.end()) {
        quantum_ms += ndelem->second;
      }
      auto ndelem_fixed = itel.find(tpr.KEY_PC_SEND_TO_CONTROL_TIME);
      if (ndelem_fixed != itel.end()) {
        quantum_ms += ndelem_fixed->second;
      }
    }
  }

  std::cout << std::endl;
  std::cout << "* Quantum (estimated) walltime: " << quantum_ms << " ms"
            << std::endl;
  std::cout << std::endl;
}

// Test Jensen-Shannon divergence and print results
int test_jensen_shannon(qristal::session &s, const double jenshan_threshold) {
  s.get_jensen_shannon();
  double jenshan_value = 0.0;
  Table2d<std::map<int,double>> jsv = s.get_out_divergences();
  for (auto &it : jsv) {
    for (auto &itel : it) {
      auto ndelem = itel.find(0);
      if (ndelem != itel.end()) {
        jenshan_value += ndelem->second;
      }
    }
  }
  std::cout << "* Jensen-Shannon divergence: " << jenshan_value << std::endl;

  if (std::abs(jenshan_value) > jenshan_threshold) {
    std::cerr << "Qristal warning: The Jensen-Shannon divergence exceeds the "
                 "threshold of "
              << jenshan_threshold << std::endl;
    return 1; // Return non-zero because we exceeded the set threshold
  } else {
    return 0;
  }
}

int main(int argc, char **argv) {
  int ret_qb = 0;
  std::srand(time(0));

  // Added code here to handle arguments
  args::ArgumentParser parser("qristal_cli - Circuit simulation with the timing, "
                              "noise and topology parameters of QB hardware.  "
                              "This tool is a component of Qristal.  The "
                              "configuration of this tool is set "
                              "in a JSON file named \"sdk_cfg.json\". Note: "
                              "command-line options specified here will "
                              "override that of the configuration file.",
                              "\n Qristal version: " SDK_VERSION);
  args::Positional<std::string> inputcircuit(
      parser, "input-circuit-file", "Name of file containing a circuit");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Flag verbose(
      parser, "verbose",
      "Display additional placement and circuit optimisation info",
      {'v', "verbose"});
  args::Group general(parser, "* General options");
  args::Group switches(parser, "* Switches");
  args::Group random_circuit_opts(parser, "* Random circuit options");
  args::Group test_opts(parser, "* Test threshold options");
  args::Group backend_opts(parser, "* Developer/test use only");
  args::ValueFlag<std::string> arg_gtest_output(
      backend_opts, "gtest_output",
      "--gtest_output='xml:report.xml' creates a JUnit report for GitLab",
      {"gtest_output"});
  args::ValueFlag<int> arg_q(
      general, "#qubits",
      "-q10 accepts up to 10 qubits, default: 12", {'q'});
  args::ValueFlag<int> arg_shots(general, "#shots",
                                 "-s128 gives 128 shots, default: 1024", {'s'});
  args::ValueFlag<double> arg_svd_cutoff(
      backend_opts, "SVD cutoff",
      "--svd-cutoff=1.0e-12 sets the cutoff for exatn-mps to 1.0e-12, default: "
      "1.0e-8",
      {"svd-cutoff"});
  args::ValueFlag<double> arg_rel_svd_cutoff(
      backend_opts, "Relative SVD cutoff",
      "--rel-svd-cutoff=1.0e-12 sets the relative cutoff for exatn-mps to 1.0e-12, default: "
      "1.0e-8",
      {"rel-svd-cutoff"});
  args::ValueFlag<int> arg_max_bond_dimension(
      backend_opts, "max bond dimension",
      "--max-bond-dimension=2000 sets the maximum bond dimenson for exatn-mps "
      "to 2000, default: 256",
      {"max-bond-dimension"});
  args::ValueFlag<int> arg_random_circ(
      random_circuit_opts, "#random_circuit_depth",
      "--random=20 will sample and analyse quantum random circuits of "
      "[#qubits] and depth 20 at each repetition",
      {"random"});
  args::ValueFlag<double> arg_jen_shan_threshold(
      test_opts, "cutoff",
      "--threshold=0.15 sets 0.15 as critical value for Jensen-Shannon "
      "divergence, default: 0.05",
      {"threshold"});
  args::ValueFlag<std::string> arg_acc(
      backend_opts, "acc",
      "--acc='aer' or --acc='qpp' to select back-end simulators, default: qpp",
      {"acc"});
  args::Flag arg_noisy(
      switches, "",
      "Enable noise modelling, a simulation of noise sources within quantum "
      "hardware and their effect on results. The noise has three main sources, "
      "internal thermal and magnetic fluctuations, and also fluctuations in "
      "the control mechanism. The inputs for the noise-model are already "
      "hard-coded with realistic parameters. Currently, the noise-model can "
      "only work alongside \"--acc=aer\" option",
      {'n', "noise"});
  args::Flag arg_xasm(
      backend_opts, "",
      "Interpret input in XASM format, default input is OpenQASM",
      {'x', "xasm"});
  args::Flag arg_quil(backend_opts, "", "Interpret input in QUIL 1.0 format",
                      {"quil1"});
  args::Flag arg_disable_placement(switches, "", "Disable placement mapping",
                                   {"noplacement"});
  args::Flag arg_enable_optimiser(switches, "", "Enable circuit optimiser",
                                  {"optimise"});
  args::Flag arg_disable_simulator(switches, "", "Perform actual execution of circuit", {"execute_circuit"});

  try {
    parser.ParseCLI(argc, argv);
  } catch (const args::Help &) {
    std::cout << parser;
    return 0;
  } catch (const args::ValidationError &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  } catch (const args::ParseError &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  //
  // Constants for QB hardware characteristics as at Aug 2021
  //
  const double gate_2q_time_ms = 0.001;
  const double gate_1q_time_ms = 0.001;
  const double q_initialisation_time_ms = 30;
  const double q_readout_time_ms = 10;
  const double pc_send_time_ms = 10000;

  xacc::HeterogeneousMap mqbacc; // Used for passing key-value pairs to Accelerators
  nlohmann::json output_to_js;  // Used for storing the configuration key-value pairs coming from a JSON format file

  //
  // Input configuration file (default: sdk_cfg.json)
  // overrides to the default values
  //
  std::string input_cfg = "sdk_cfg.json";

  //
  // Output file name (JSON format, default: QBINFO.json)
  //
  bool redirect_screen = true;
  std::string redirect_screen_to_file_name = "QBINFO.json";

  std::ifstream config_f(input_cfg);
  if (verbose) {
    std::cout << std::endl << "## 0.0 Configuration:" << std::endl << std::endl;
  }

  if (config_f.is_open()) {
    std::string config_buf(std::istreambuf_iterator<char>(config_f), {});
    output_to_js = qristal::get_session_cfg(config_buf);
  }
  else {
    if (verbose) {
        std::cout << "Qristal notice: No configuration file (" << input_cfg
              << ") provided...using defaults unless overriden by command options." << std::endl << std::endl;
    }
  }

  //
  // Start a session
  //
  qristal::session s;
  s.init(); // setup defaults = 12 qubits, 1024 shots, tnqvm-exatn-mps back-end

  //
  // targetCircuit: contains the quantum circuit that will be processed/executed
  //
  std::string targetCircuit;

  //
  // User specified overrides to defaults/config file values:
  //

  // shots: number of shots in one cycle
  int shots = 1024;
  shots = qristal::get_arg_or_cfg(shots, arg_shots, output_to_js, "shots");
  mqbacc.insert("shots", shots);
  if (shots==0) {
      std::cout << std::endl << "Qristal warning: Nothing to do here; no. of shots is set to zero." << std::endl << std::endl;
      return 0;
  }
  s.set_sn(shots);

  // n_qubits: number of physical qubits
  int n_qubits = 12;
  n_qubits = qristal::get_arg_or_cfg(n_qubits, arg_q, output_to_js, "n_qubits");
  mqbacc.insert("n_qubits", n_qubits);
  s.set_qn(n_qubits);

  // depth_rndcct: depth of random circuit to be generated when using --random option
  int depth_rndcct = 0;
  depth_rndcct = qristal::get_arg_or_cfg(depth_rndcct, arg_random_circ, output_to_js, "depth_rndcct");
  mqbacc.insert("depth_rndcct", depth_rndcct);

  // noise model for QB
  NoiseModel noiseModel;
  mqbacc.insert("noise-model", noiseModel.to_json());
  mqbacc.insert("m_connectivity", noiseModel.get_connectivity());

  // expected_amplitude: theoretical output amplitude after the circuit is run
  std::map<std::vector<bool>, std::complex<double>> expected_amplitudes;
  if (!output_to_js["expected_amplitudes"].empty()) {
    std::cout << "* expected_amplitudes has been specified:" << std::endl;
    s.set_expected_amplitudes(expected_amplitudes);
  }

  /* Accelerator selection:
   * Default: tnqvm-exatn-mps
   * If detected -n (noise) option, then force selection to: aer
   * If detected --acc in sdk_cfg.json, then force the config file selection
   * If detected --acc as command option, then override all other selections
   *                   and use the accelerator specified at the command line
  */
  std::string acc_op = "tnqvm-exatn-mps";
  double svd_cutoff = 1.0e-8;
  svd_cutoff =
      qristal::get_arg_or_cfg(svd_cutoff, arg_svd_cutoff, output_to_js, "svd_cutoff");
  std::map<int,double> u_svd_cutoff{{0, svd_cutoff}};
  s.set_svd_cutoff(u_svd_cutoff);

  double rel_svd_cutoff = 1.0e-8;
  rel_svd_cutoff =
      qristal::get_arg_or_cfg(rel_svd_cutoff, arg_rel_svd_cutoff, output_to_js, "rel_svd_cutoff");
  std::map<int,double> u_rel_svd_cutoff{{0, rel_svd_cutoff}};
  s.set_rel_svd_cutoff(u_rel_svd_cutoff);

  int max_bond_dimension = 256;
  max_bond_dimension =
      qristal::get_arg_or_cfg(max_bond_dimension, arg_max_bond_dimension, output_to_js,
                     "max_bond_dimension");
  s.set_max_bond_dimension(max_bond_dimension);

  acc_op = qristal::get_arg_or_cfg(acc_op, arg_acc, output_to_js, "acc");
  std::string u_acc = "tnqvm";
  if (arg_noisy) {
    u_acc = "aer";
  }
  if (arg_acc) {
    if (!acc_op.compare("tnqvm-exatn")) {
            u_acc = "exatn";
    } else {
      u_acc = acc_op;
    }
    std::cout << std::endl
              << "* Using --acc override. Accelerator backend is set to: "
              << u_acc << std::endl
              << std::endl;
  } else {
    std::cout << std::endl
              << "* Selected accelerator backend: " << u_acc << std::endl
              << std::endl;
  }
  s.set_acc(u_acc);
  if (arg_noisy) {
      s.set_noise(true);
  } else {
      s.set_noise(false);
  }

  //
  // Test limits for comparing the sampling distribution against theoretical distribution
  // Note: requires "expected_amplitudes" to be specified via JSON config file
  //
  double jenshan_threshold = 0.05;
  if (arg_jen_shan_threshold) {
      jenshan_threshold = args::get(arg_jen_shan_threshold);
  }

  // More configurations get added here

  if (verbose) {
          std::cout << std::endl << "* Set n_qubits: " << n_qubits << std::endl;
          std::cout << "* Set shots: " << shots << std::endl;
          std::cout << "* Set SVD cutoff: " << svd_cutoff << std::endl;
          std::cout << "* Set relative SVD cutoff: " << rel_svd_cutoff << std::endl;
          std::cout << "* Set maximum bond dimension: " << max_bond_dimension << std::endl;
          std::cout << "* Set accelerator: " << u_acc  << std::endl;
          std::cout << "* Set random circuit depth: " << depth_rndcct << std::endl;
  }

  // Check if an input circuit has been specified on the command line...
  // If one is specified, then read the contents into a string...
  // Also handle the case of random circuits here
  if (argc > 1) {
    std::ifstream ins(argv[1]);
    if (ins.is_open()) {
      std::string outbuf(std::istreambuf_iterator<char>(ins), {});
      targetCircuit = outbuf;
      if (verbose) {
        std::cout << "* Source quantum circuit:" << std::endl << std::endl;
        std::cout << targetCircuit << std::endl << std::endl;
      }
    } else {
        if (!arg_random_circ) {
            std::cerr << "Qristal error: Input file not found: " << argv[1] << std::endl;
            exit(10);
        }
    }

    // Now execute the circuit...
    if (arg_random_circ) { // Use a random circuit
      s.set_random(depth_rndcct);
    } else { // or use a circuit contained in the file specified by the user
      s.set_instring(targetCircuit);
      //
      // Before we run the simulator, check for overrides...
      //

      // OpenQASM is the default, here we check if this is overriden by the user
      if (arg_xasm) {
          s.set_xasm(true);
      } else if (arg_quil) {
          s.set_quil1(true);
      }

      // Circuit placement subject to QB topology constraints is
      // performed by default...here we check if the user has requested this
      // to be skipped.
      if (arg_disable_placement) {
          s.set_noplacement(true);
      }

      // Circuit optimisation is not performed by default...
      // Here we check if the user wants circuit optimisation enabled.
      if (arg_enable_optimiser) {
          s.set_nooptimise(false);
      }

      // A back-end simulator is enabled by default...
      // Here we check if the user wants to bypass simulation (eg. user only
      // wants circuit transpiling and estimates of quantum timing)
      if (arg_disable_simulator) {
          s.set_execute_circuit(false);
      }
    }
    //
    s.run();
    //
    print_classical(s);
    //
    if (verbose) {
      print_quantum(s, gate_1q_time_ms, gate_2q_time_ms, q_initialisation_time_ms, q_readout_time_ms, pc_send_time_ms, false, true);
    } else {
      print_quantum(s, gate_1q_time_ms, gate_2q_time_ms, q_initialisation_time_ms, q_readout_time_ms, pc_send_time_ms, false, false);
    }

    // Test output against provided theoretical amplitudes
    if ((expected_amplitudes.size() > 0) && !arg_random_circ) {
      if (expected_amplitudes.size() < (1 << n_qubits)) {
        std::cout << std::endl
                  << "Qristal warning: size of expected_amplitudes provided in your "
                     "configuration file does not equal 2^n_qubits"
                  << std::endl;
      }
      ret_qb = test_jensen_shannon(s, jenshan_threshold);
    }
  } else if (argc == 1) {
    print_quickstart();
  }

  xacc::Finalize();
  return ret_qb;
}

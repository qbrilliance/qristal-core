// (c) 2021 Quantum Brilliance Pty Ltd
#include <ostream>

TEST(QuantumBrillianceNoiseModelTester, test48QubitQuantumError) {
  std::string qbnoise_model;
  int n_qubits = 48;
  std::shared_ptr<xacc::quantum::QuantumBrillianceNoiseModel> noiseModel =
      std::make_shared<xacc::quantum::QuantumBrillianceNoiseModel>();

  noiseModel->setup_48_qubits();
  noiseModel->set_m_nbQubits(
      n_qubits); // do this first before set_qb_connectivity()
  json j;
  j["n_qubits"] = 48;
  j["shots"] = 1024;
  j["device"] = "GPU";
  j["noise_model"] = json::parse(noiseModel->toJson());
  // std::cout << j.dump(2) << std::endl;
}

TEST(SDKTester, test1_qft4) {
  std::cout << "* qft4: Execute 4-qubit Quantum Fourier Transform, noiseless, "
               "ExaTN-MPS"
            << std::endl;

  // Start a QB SDK session.
  auto s = qb::session(true);

  s.qb12(); // setup defaults = 12 qubits, 1024 shots, tnqvm-exatn-mps
              // back-end

  // Override defaults
  auto n_qubits = 4;
  auto n_shots = 1024;
  s.set_qn(n_qubits);      // We only need 4 qubits here
  s.set_sn(n_shots);       // Explicitly use 1024 shots
  s.set_xasm(true);        // Use XASM circuit format to access XACC's qft()
  s.set_seed(23);
  // targetCircuit: contains the quantum circuit that will be processed/executed
  auto targetCircuit = R"(
    __qpu__ void QBCIRCUIT(qbit q) {
          qft(q, {{"nq",4}});
          Measure(q[3]);
          Measure(q[2]);
          Measure(q[1]);
          Measure(q[0]);
    }
  )";
  s.set_instring(targetCircuit);
  std::cout << "* [DEBUG] About to do s.run()..." << std::endl;

  // Run the circuit on the back-end
  s.run();

  // Get the Z-operator expectation value
  VectorMapND nd_expectation = s.get_out_z_op_expects();
  auto expectation_v = 0.0;
  for (auto &it : nd_expectation) {
    for (auto &itel : it) {
      ND::iterator ndelem = itel.find(0);
      if (ndelem != itel.end()) {
        expectation_v = ndelem->second;
      }
    }
  }

  // Test the value against assertions
  std::cout << "* Using " << n_shots
            << " shots, Z-operator expectation value: " << expectation_v
            << std::endl;
  ASSERT_NEAR(0.0, expectation_v, 0.2);
}

TEST(SDKTester, test_circuit_builder_1) {
  std::cout << "* QB SDK Circuit Builder simple test, noiseless, ExaTN-MPS"
            << std::endl;

  // Start a QB SDK session.
  auto s = qb::session(true);

  s.qb12(); // setup defaults = 12 qubits, 1024 shots, tnqvm-exatn-mps
              // back-end

  // Override defaults
  auto n_qubits = 4;
  auto n_shots = 1024;
  s.set_qn(n_qubits); // We only need 4 qubits here
  s.set_sn(n_shots);  // Explicitly use 1024 shots
  // s.set_xasm(true);   // Use XASM circuit format to access XACC's qft()

  constexpr double p = 0.2;
  const double theta_p = 2 * std::asin(std::sqrt(p));
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  // A circuit
  auto state_prep = gateRegistry->createComposite("A");
  state_prep->addInstruction(
      gateRegistry->createInstruction("Ry", {3}, {theta_p}));
  // Q circuit
  auto grover_op = gateRegistry->createComposite("Q");
  grover_op->addInstruction(
      gateRegistry->createInstruction("Ry", {3}, {2.0 * theta_p}));
  const int bits_precision = 3;
  auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
  const bool expand_ok = ae->expand({{"state_preparation_circuit", state_prep},
                                     {"grover_op_circuit", grover_op},
                                     {"num_evaluation_qubits", bits_precision},
                                     {"num_state_qubits", 1},
                                     {"num_trial_qubits", 1}});
  EXPECT_TRUE(expand_ok);

  // Simulation test:
  auto circuit = gateRegistry->createComposite("sim_ae");
  // Add amplitude estimation:
  circuit->addInstructions(ae->getInstructions());
  // Measure evaluation qubits:
  for (int i = 0; i < bits_precision; ++i) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", i));
  }
  // std::cout << "HOWDY: Amplitude estimation circuit:\n";
  std::cout << circuit->toString() << '\n';

  s.set_irtarget_ms({{circuit}, {circuit}});
  s.run();
}

/*
// This test is disabled by default:
// It requires the 'loopback' to be active
// To activate the loopback, run session_if_model.py

TEST(SDKTester, test_simple_loopback) {
  std::cout << "* loopback: execute Ry(0.5*pi)" << std::endl;

  // Start a QB SDK session.
  auto s = qb::session();
  s.qb12(); // setup defaults = 12 qubits, 1024 shots, tnqvm-exatn-mps
back-end

  // Override defaults
  auto n_qubits = 1;
  auto n_shots = 1024;
  s.set_qn(n_qubits);      // We only need 4 qubits here
  s.set_sn(n_shots);       // Explicitly use 1024 shots
  s.set_xasm(true);        // Use XASM circuit format to access XACC's qft()

  // targetCircuit: contains the quantum circuit that will be processed/executed
  auto targetCircuit = R"(
    __qpu__ void QBCIRCUIT(qbit q) {
        Rx(q[0], 0.125*pi);
        Ry(q[0], 0.25*pi);
        Rz(q[0], 0.5*pi);
        Measure(q[0]);
    }
  )";
  s.set_instring(targetCircuit);
  s.set_acc("loopback");

  // Run the circuit on the back-end
  s.run();

}
*/

TEST(SDKTester, test_readout_error_mitigation) {
  std::cout << "* Test simple readout error mitigation *" << std::endl;

  // Start a QB SDK session.
  auto s = qb::session(true);
  s.qb12();

  // Override defaults
  const int n_qubits = 1;
  const int n_shots = 1024;
  s.set_qn(n_qubits);
  s.set_sn(n_shots);
  s.set_xasm(true);
  s.set_noise(true);
  s.set_nooptimise(true);
  s.set_noplacement(true);
  s.set_noise_mitigation("ro-error");
  s.set_acc("aer");
  auto targetCircuit = R"(
    __qpu__ void QBCIRCUIT(qbit q) {
        X(q[0]);
        Measure(q[0]);
    }
  )";
  s.set_instring(targetCircuit);
  // Run the circuit on the back-end
  s.run();
  const auto iter = s.get_out_z_op_expects()[0][0].find(0);
  EXPECT_TRUE(iter != s.get_out_z_op_expects()[0][0].end());
  const double exp_val = iter->second;
  double raw_exp_val = 0.0;
  for (const auto &[bitString, count] : s.get_out_bitstrings()[0][0]) {
    if (bitString == "0") {
      raw_exp_val += (static_cast<double>(count) / 1024);
    } else {
      EXPECT_EQ(bitString, "1");
      raw_exp_val += (-static_cast<double>(count) / 1024);
    }
  }
  std::cout << "Error mitigated exp-val = " << exp_val
            << " vs. raw exp-val = " << raw_exp_val << "\n";
  // Ideal result is -1.0 (|1> state)
  const double delta_mitigated = -1.0 - exp_val;
  const double delta_raw = -1.0 - raw_exp_val;
  // Improved accuracy
  EXPECT_TRUE(std::abs(delta_mitigated) <= std::abs(delta_raw));
}

TEST(SDKTester, test_richardson_error_mitigation) {
  std::cout << "* Test simple Richardson error mitigation *" << std::endl;
  // Start a QB SDK session.
  auto s = qb::session(true);
  s.qb12();

  // Override defaults
  const int n_qubits = 10;
  const int n_shots = 8192;
  s.set_qn(n_qubits);
  s.set_sn(n_shots);
  s.set_xasm(true);
  s.set_noise(true);
  s.set_nooptimise(true);
  s.set_noplacement(true);
  s.set_noise_mitigation("rich-extrap");
  s.set_acc("aer");
  auto targetCircuit = R"(
    __qpu__ void QBCIRCUIT(qbit q) {
        H(q[0]);
        CNOT(q[0],q[1]);
        CNOT(q[1],q[2]);
        CNOT(q[2],q[3]);
        CNOT(q[3],q[4]);
        CNOT(q[4],q[5]);
        CNOT(q[5],q[6]);
        CNOT(q[6],q[7]);
        CNOT(q[7],q[8]);
        CNOT(q[8],q[9]);
        Measure(q[0]);
        Measure(q[1]);
        Measure(q[2]);
        Measure(q[3]);
        Measure(q[4]);
        Measure(q[5]);
        Measure(q[6]);
        Measure(q[7]);
        Measure(q[8]);
        Measure(q[9]);
    }
  )";
  s.set_instring(targetCircuit);
  // Run the circuit on the back-end
  s.run();
  const auto iter = s.get_out_z_op_expects()[0][0].find(0);
  EXPECT_TRUE(iter != s.get_out_z_op_expects()[0][0].end());
  const double exp_val = iter->second;
  std::cout << "Richardson extrapolated exp-val-z = " << exp_val << "\n";
}

TEST(SDKTester, test_assignment_kernel_error_mitigation) {
  std::cout << "* Test simple readout assignment kernel error mitigation *"
            << std::endl;

  // Start a QB SDK session.
  auto s = qb::session(true);
  s.qb12();

  // Override defaults
  const int n_qubits = 2;
  const int n_shots = 8192;
  s.set_qn(n_qubits);
  s.set_sn(n_shots);
  s.set_xasm(true);
  s.set_noise(true);
  s.set_nooptimise(true);
  s.set_noplacement(true);
  s.set_noise_mitigation("assignment-error-kernel");
  s.set_acc("aer");
  auto targetCircuit = R"(
    __qpu__ void QBCIRCUIT(qbit q) {
        H(q[0]);
        CNOT(q[0],q[1]);
        Measure(q[0]);
        Measure(q[1]);
    }
  )";
  s.set_instring(targetCircuit);
  // Run the circuit on the back-end
  s.run();
}

TEST(InitRepeatFlag_1, checkSimple) {
  std::cout << "InitRepeatFlagTester1:\n";
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  const std::vector<int> qubits_string = {0, 1, 2, 3};
  const std::vector<int> qubits_init_repeat = {4, 5};
  const std::vector<int> qubits_next_letter = {6, 7};
  int max_key = 7; // qubits_next_letter.end();
  // int *max_key = std::max_element(std::begin(key), std::end(key));
  // const int max_key = key.max();
  // const int min_key = min(key);
  // const int offset_key = max_key + min_key;

  auto init_repeat_flag = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("InitRepeatFlag"));
  std::cout << "expand\n";
  // const bool expand_ok =
  xacc::HeterogeneousMap map = {{"iteration", 1},
                                {"qubits_init_repeat", qubits_init_repeat},
                                {"qubits_string", qubits_string},
                                {"qubits_next_letter", qubits_next_letter}};
  init_repeat_flag->expand(map);
  // EXPECT_TRUE(expand_ok);
  //  Simulation test:
  //  Construct the full circuit, include state prep (eigen state of |1>);
  std::cout << "init_repeat_flag_test\n";
  auto init_flag_test = gateRegistry->createComposite("init_repeat_flag");

  for (int qindex = 0; qindex < std::size(qubits_string); qindex++) {
    init_flag_test->addInstruction(
        gateRegistry->createInstruction("H", qubits_string[qindex]));
  }
  init_flag_test->addInstruction(
      gateRegistry->createInstruction("X", qubits_next_letter[0]));

  // Add init rep flag
  init_flag_test->addInstructions(init_repeat_flag->getInstructions());

  // Measure
  for (int i = 0; i <= max_key; ++i) {
    init_flag_test->addInstruction(
        gateRegistry->createInstruction("Measure", i));
  }
  // std::cout << "HOWDY: InitRepeatFlag circuit:\n";
  // std::cout << init_flag_test->toString() << '\n';

  // Start a QB SDK session.
  auto s = qb::session();
  s.qb12();

  // Override defaults
  const int n_qubits = 8;
  const int n_shots = 1024;
  s.set_qn(n_qubits);
  s.set_sn(n_shots);
  s.set_nooptimise(true);
  s.set_noplacement(true);
  s.set_notiming(true);
  s.set_output_oqm_enabled(false);
  s.set_acc("qpp");

  // Sim:
  s.set_irtarget_m(init_flag_test);
  s.run();

  int nq_next_letter = qubits_next_letter.size();
  int string_integer;
  auto out_count = s.get_out_bitstrings()[0][0];

  auto a1 = out_count["01000000"]; //64
  auto a2 = out_count["01100001"]; //97
  auto a3 = out_count["01000010"]; //66
  auto a4 = out_count["01000011"]; //67
  auto a5 = out_count["01000100"]; //68
  auto a6 = out_count["01100101"]; //101
  auto a7 = out_count["01000110"]; //70
  auto a8 = out_count["01000111"]; //71
  auto a9 = out_count["01001000"]; //72
  auto a10 = out_count["01101001"]; //105
  auto a11 = out_count["01001010"]; //74
  auto a12 = out_count["01001011"]; //75
  auto a13 = out_count["01001011"]; //76
  auto a14 = out_count["01101101"]; //109
  auto a15 = out_count["01001110"]; //78
  auto a16 = out_count["01001111"]; //79

  EXPECT_GT(a1, 0);
  EXPECT_GT(a2, 0);
  EXPECT_GT(a3, 0);
  EXPECT_GT(a4, 0);
  EXPECT_GT(a5, 0);
  EXPECT_GT(a6, 0);
  EXPECT_GT(a7, 0);
  EXPECT_GT(a8, 0);
  EXPECT_GT(a9, 0);
  EXPECT_GT(a10, 0);
  EXPECT_GT(a11, 0);
  EXPECT_GT(a12, 0);
  EXPECT_GT(a13, 0);
  EXPECT_GT(a14, 0);
  EXPECT_GT(a15, 0);
  EXPECT_GT(a16, 0);
  EXPECT_EQ(a1+a2+a3+a4+a5+a6+a7+a8+a9+a10+a11+a12+a13+a14+a15+a16, 1024);

  EXPECT_EQ(out_count.size(), 16);
  // buffer->print();
}

TEST(QDBeamStatePrepCircuitTester, simple) {

  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  // Define the circuit we want to run
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("circuit");

  // Prepare the module

  std::vector<std::vector<float>> probability_table = {{0.5, 0.5}, {0.5, 0.5}};
  std::vector<int> qubits_string = {0, 1};
  std::vector<int> qubits_iteration = {2, 3, 4, 5};
  std::vector<int> qubits_metric = {6, 7};
  std::vector<int> qubits_next_letter = {8};
  std::vector<int> qubits_next_metric = {9};
  std::vector<int> qubits_is_occupied = {10, 11};
  int qubit_is_null = 12;
  int qubit_is_repetition = 13;
  int qubit_is_used = 14;
  std::vector<int> qubits_current_iteration = {15, 16};
  std::vector<int> qubits_ancilla_state_prep = {17, 18, 19, 20};
  std::vector<int> qubits_null = {21, 22};

  auto sp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("BeamStatePrep"));
  const bool expand_ok =
      sp->expand({{"qubits_string", qubits_string},
                  {"qubits_metric", qubits_metric},
                  {"qubits_next_letter", qubits_next_letter},
                  {"qubits_next_metric", qubits_next_metric},
                  {"probability_table", probability_table},
                  {"qubits_iteration", qubits_iteration},
                  {"qubits_is_occupied", qubits_is_occupied},
                  {"qubit_is_null", qubit_is_null},
                  {"qubit_is_repetition", qubit_is_repetition},
                  {"qubit_is_used", qubit_is_used},
                  {"qubits_null", qubits_null},
                  {"qubits_current_iteration", qubits_current_iteration},
                  {"qubits_ancilla_state_prep", qubits_ancilla_state_prep}});
  EXPECT_TRUE(expand_ok);

  // Add the module to the circuit
  circuit->addInstructions(sp->getInstructions());

  // Add measurements
  for (int q : qubits_string) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", q));
  }
  for (int q : qubits_iteration) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", q));
  }
  for (int q : qubits_metric) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", q));
  }
  for (int q : qubits_null) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", q));
  }

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////

  auto s = qb::session(true);
  s.qb12();

  // Override defaults
  const int n_qubits = 23;
  const int n_shots = 1024;
  s.set_qn(n_qubits);
  s.set_sn(n_shots);
  s.set_nooptimise(true);
  s.set_noplacement(true);
  s.set_notiming(true);
  s.set_output_oqm_enabled(false);
  s.set_acc("qsim");

  // Sim:
  s.set_irtarget_m(circuit);
  s.run();

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////

  // The expected outputs are:
  // Beam --, metric 11, first iteration placed at the end second iteration at the start, both marked as null
  // --->>> |0001101111>
  // Beam a-, three components equally weighted from strings aa, a-, -a. All metrics 11. Iterations and nulls marked differently
  // --->>> |1010011100>, |1010011101>, |1001101101>

  auto out_count = s.get_out_bitstrings()[0][0];

  auto a = out_count["1111011000"]; //984
  auto b = out_count["001110010"]; //229
  auto c = out_count["1011011001"]; //741
  auto d = out_count["1011100101"]; //729

  EXPECT_GT(a, 0);
  EXPECT_GT(b, 0);
  EXPECT_GT(c, 0);
  EXPECT_GT(d, 0);
  EXPECT_EQ(a+b+c+d, 1024);

}

TEST(SuperpositionAdderCircuitTester, check1) {
  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto ae_state_prep_circ = gateRegistry->createComposite("state_prep");

  int q0 = 0;
  int q1 = 1;
  int q2 = 2;
  std::vector<int> qubits_string = {3};
  std::vector<int> qubits_metric = {4};
  std::vector<int> qubits_superfluous_flags = {5};
  std::vector<int> qubits_beam_metric = {6,7};
  std::vector<int> qubits_ancilla = {8,9,10,11,12,13,14,15,16,17,18,19};

  // Let's define the state
  // |string>|metric>|flags> = |00>|11>|11> + |10>|10>|01> + |10>|11>|01> + |11>|01>|01>

  // state prep
  ae_state_prep_circ->addInstruction(gateRegistry->createInstruction("H", qubits_string[0]));

  auto mcx00b = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx00b->expand(
      {{"target", qubits_metric[0]}, {"controls_on", qubits_string}});
  ae_state_prep_circ->addInstruction(mcx00b);

  auto circ = gateRegistry->createComposite("circ");
  circ->addInstructions(ae_state_prep_circ->getInstructions());

  auto ae_adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("SuperpositionAdder"));
  const bool expand_ok_ae = ae_adder->expand({
      {"q0", q0}, {"q1", q1}, {"q2", q2},
      {"qubits_flags", qubits_superfluous_flags},
      {"qubits_string", qubits_string},
      {"qubits_metric", qubits_metric},
      {"ae_state_prep_circ", ae_state_prep_circ},
      {"qubits_ancilla", qubits_ancilla},
      {"qubits_beam_metric", qubits_beam_metric}});
  EXPECT_TRUE(expand_ok_ae);
  circ->addInstructions(ae_adder->getInstructions());

  // Measure
  for (int i = 0; i < qubits_beam_metric.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_beam_metric[i]));
  for (int i = 0; i < qubits_string.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_string[i]));
  for (int i = 0; i < qubits_metric.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_metric[i]));
  for (int i = 0; i < qubits_superfluous_flags.size(); i++)
    circ->addInstruction(gateRegistry->createInstruction("Measure", qubits_superfluous_flags[i]));

  //////////////////////////////////////
  // Run circuit
  //////////////////////////////////////
  auto s = qb::session(true);
  s.qb12();

  // Override defaults
  const int n_qubits = 20;
  const int n_shots = 1024;
  s.set_qn(n_qubits);
  s.set_sn(n_shots);
  s.set_nooptimise(true);
  s.set_noplacement(true);
  s.set_notiming(true);
  s.set_output_oqm_enabled(false);
  s.set_acc("qsim");

  // Sim:
  s.set_irtarget_m(circ);
  s.run();

  //////////////////////////////////////
  // Check results
  //////////////////////////////////////

  // The expected outputs are:

  auto out_count = s.get_out_bitstrings()[0][0];
  EXPECT_EQ((int)out_count.size(), 2);
}

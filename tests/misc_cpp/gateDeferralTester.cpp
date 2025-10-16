// Copyright (c) Quantum Brilliance Pty Ltd

// Gtest
#include <gtest/gtest.h>

// Qristal
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/passes/circuit_opt_passes.hpp>
#include <qristal/core/passes/gate_deferral_pass.hpp>
#include <qristal/core/session.hpp>
#include <qristal/core/utils.hpp>

// XACC
#include <xacc.hpp>
#include <xacc_service.hpp>

// STL
#include <random>

TEST(GateDeferralTester, canonicalCircuit) {
  // The canonical example of gate deferral from sparse-sim is the following:
  // H(0), H(1), CX(0, 1) -> CX(1, 0), H(0), H(1)
  size_t m_num_qubits = 2;
  // Create circuit
  std::shared_ptr<xacc::IRProvider> provider = xacc::getIRProvider("quantum");
  std::shared_ptr<xacc::CompositeInstruction> program = provider->createComposite("circuit");
  program->addInstruction(std::make_shared<xacc::quantum::Hadamard>(0));
  program->addInstruction(std::make_shared<xacc::quantum::Hadamard>(1));
  program->addInstruction(std::make_shared<xacc::quantum::CNOT>(0, 1));
  // Measure
  for (size_t i = 0; i < m_num_qubits; i++) {
    program->addInstruction(std::make_shared<xacc::quantum::Measure>(i));
  }

  // Create circuit with deferred gates using the initial circuit
  qristal::gate_deferral_pass defer(program);
  std::string openQasmCircuit = defer.toOpenQasm();
  std::cout<<"openQasmCircuit:\n"<<openQasmCircuit<<"\n";
  std::shared_ptr<xacc::CompositeInstruction> program_deferred_gates = defer.toXasm();

  // Check gate-deferred circuit
  std::vector<std::string> expected_gate = {"CNOT", "H", "H", "Measure", "Measure"};
  std::vector<std::vector<size_t>> expected_qubits = {{1, 0}, {0}, {1}, {0}, {1}};
  std::vector<xacc::InstPtr> inst = program_deferred_gates->getInstructions();
  for (size_t i = 0; i < inst.size(); i++) {
    // Check gate name
    EXPECT_EQ(inst[i]->name(), expected_gate[i]);

    // Check qubits
    std::vector<size_t> bits = inst[i]->bits();
    EXPECT_EQ(bits.size(), expected_qubits[i].size());
    for (size_t j = 0; j < bits.size(); j++) {
      EXPECT_EQ(bits[j], expected_qubits[i][j]);
    }
  }
}

TEST(GateDeferralTester, ghz) {
  size_t m_num_qubits = 4;
  // Create GHZ circuit
  std::shared_ptr<xacc::IRProvider> provider = xacc::getIRProvider("quantum");
  std::shared_ptr<xacc::CompositeInstruction> program = provider->createComposite("circuit");
  program->addInstruction(std::make_shared<xacc::quantum::Hadamard>(0));
  for (size_t i = 0; i < m_num_qubits - 1; i++) {
    program->addInstruction(std::make_shared<xacc::quantum::CNOT>(i, i + 1));
  }
  // Measure
  for (size_t i = 0; i < m_num_qubits; i++) {
    program->addInstruction(std::make_shared<xacc::quantum::Measure>(i));
  }

  // Create circuit with deferred gates using the initial circuit
  qristal::gate_deferral_pass defer(program);
  std::string openQasmCircuit = defer.toOpenQasm();
  std::cout<<"openQasmCircuit:\n"<<openQasmCircuit<<"\n";
  std::shared_ptr<xacc::CompositeInstruction> program_deferred_gates = defer.toXasm();

  // The GHZ circuit with gate deferral should be identical to the original circuit
  std::vector<xacc::InstPtr> inst = program->getInstructions();
  std::vector<xacc::InstPtr> inst_deferred_gates = program_deferred_gates->getInstructions();
  EXPECT_EQ(inst.size(), inst_deferred_gates.size());
  for (size_t i = 0; i < inst.size(); i++) {
    // Check gate name
    EXPECT_EQ(inst[i]->name(), inst_deferred_gates[i]->name());

    // Check qubits
    std::vector<size_t> bits = inst[i]->bits();
    std::vector<size_t> bits_deferred_gates = inst_deferred_gates[i]->bits();
    EXPECT_EQ(bits.size(), bits_deferred_gates.size());
    for (size_t j = 0; j < bits.size(); j++) {
      EXPECT_EQ(bits[j], bits_deferred_gates[j]);
    }
  }
}

TEST(GateDeferralTester, qft_iqft) {
  size_t m_num_qubits = 3;
  // Create initial state: 100 + 111
  std::shared_ptr<xacc::IRProvider> provider = xacc::getIRProvider("quantum");
  std::shared_ptr<xacc::CompositeInstruction> program = provider->createComposite("circuit");
  program->addInstruction(std::make_shared<xacc::quantum::X>(0));
  program->addInstruction(std::make_shared<xacc::quantum::Hadamard>(1));
  program->addInstruction(std::make_shared<xacc::quantum::CNOT>(1, 2));

  // Set up and add QFT & IQFT circuits
  auto qft = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("qft"));
  auto iqft = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("iqft"));
  qft->expand({{"nq", static_cast<int>(m_num_qubits)}});
  iqft->expand({{"nq", static_cast<int>(m_num_qubits)}});
  program->addInstructions(qft->getInstructions());
  program->addInstructions(iqft->getInstructions());
  // Measure
  for (size_t i = 0; i < m_num_qubits; i++) {
    program->addInstruction(std::make_shared<xacc::quantum::Measure>(i));
  }

  // Execute circuit
  std::shared_ptr<xacc::Accelerator> acc = xacc::getAccelerator("aer", {{"shots", 1024}});
  auto buffer = xacc::qalloc(m_num_qubits);
  acc->execute(buffer, program);
  buffer->print();

  // Create circuit with deferred gates using the initial circuit
  qristal::gate_deferral_pass defer(program);
  std::string openQasmCircuit = defer.toOpenQasm();
  std::cout<<"openQasmCircuit:\n"<<openQasmCircuit<<"\n";
  std::shared_ptr<xacc::CompositeInstruction> program_deferred_gates = defer.toXasm();

  // Execute the circuit created using the deferred gate technique
  auto buffer_deferred_gates = xacc::qalloc(m_num_qubits);
  acc->execute(buffer_deferred_gates, program_deferred_gates);
  buffer_deferred_gates->print();

  // Check that both distributions are close
  EXPECT_NEAR(buffer->getMeasurementCounts()["100"], buffer_deferred_gates->getMeasurementCounts()["100"], 70);
  EXPECT_NEAR(buffer->getMeasurementCounts()["111"], buffer_deferred_gates->getMeasurementCounts()["111"], 70);
}

TEST(GateDeferralTester, qft_iqft_session) {
  qristal::session s;
  s.qn = 3;
  s.sn = 1024;
  s.acc = "aer";

  // Create initial state: 100 + 111
  qristal::CircuitBuilder circ;
  circ.X(0);
  circ.H(1);
  circ.CNOT(1, 2);
  std::vector<int> qft_qubits(s.qn);
  // Fill the qubit list with 0, 1, ..., n-1
  // i.e., the qubits that we want to apply the QFT circuit to.
  std::iota(std::begin(qft_qubits), std::end(qft_qubits), 0);
  // Apply QFT
  circ.QFT(qft_qubits);
  // Apply inverse QFT
  circ.IQFT(qft_qubits);
  // Measure qubits
  circ.MeasureAll(s.qn);

  // Execute circuit
  s.irtarget = circ.get();
  s.run();
  std::map<std::vector<bool>, int> results = s.results();
  std::cout << "Results:\n" << results << "\n";

  // Create circuit with deferred gates using the initial circuit
  qristal::gate_deferral_pass defer(s.irtarget);
  std::string openQasmCircuit = defer.toOpenQasm();
  std::cout<<"openQasmCircuit:\n"<<openQasmCircuit<<"\n";
  std::shared_ptr<xacc::CompositeInstruction> program_deferred_gates = defer.toXasm();

  // Execute the circuit created using the deferred gate technique
  qristal::session s_deferred_gate;
  s_deferred_gate.qn = s.qn;
  s_deferred_gate.sn = s.sn;
  s_deferred_gate.acc = s.acc;
  s_deferred_gate.irtarget = program_deferred_gates;
  s_deferred_gate.run();
  std::map<std::vector<bool>, int> results_gate_deferred = s_deferred_gate.results();
  std::cout << "Deferred gate results:\n" << results_gate_deferred << "\n";

  // Check that both distributions are close
  for (const auto &[bitstring, count] : results) {
    EXPECT_NEAR(count, results_gate_deferred[bitstring], 70);
  }
}

std::string generate_random_circuit(size_t num_qubits, size_t depth) {
  std::vector<std::string> one_q_ops = {"id", "u1", "x", "y", "z", "h", "s", "sdg", "t", "tdg", "rx", "ry", "rz"};
  std::vector<std::string> two_q_ops = {"cx", "cy", "cz", "swap", "cu1", "crz", "cu3", "ch"};
  std::vector<std::string> one_params = {"u1", "rx", "ry", "rz", "crz", "cu1"};
  std::vector<std::string> three_params = {"cu3"};

  // Random number generation setup
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> gate_type_dist(1, 2); // select 1- or 2-qubit gates
  std::uniform_int_distribution<> gate1_dist(0, one_q_ops.size() - 1); // 1-qubit gates
  std::uniform_int_distribution<> gate2_dist(0, two_q_ops.size() - 1); // 2-qubit gates
  std::uniform_int_distribution<> qubit_dist(0, num_qubits - 1); // Select random qubits
  std::uniform_real_distribution<double> angle_dist(-M_PI, M_PI); // Random angles

  std::stringstream circuit;
  circuit << "  OPENQASM 2.0;" << std::endl
          << "  include \"qelib1.inc\";" << std::endl;
  circuit << "  qreg q[" << num_qubits << "];" << std::endl;
  circuit << "  creg c[" << num_qubits << "];" << std::endl;

  for (size_t level = 0; level < depth; level++) {
    size_t gate_type = gate_type_dist(gen);
    size_t qubit = qubit_dist(gen);

    size_t gate_idx = 0;
    if (gate_type == 1) { // 1-qubit gate
      gate_idx = gate1_dist(gen);
      std::string gate = one_q_ops[gate_idx];
      circuit << "  " << gate;

      // Check if gate is a rotation gate. Create random rotation angle if it is.
      auto it = std::find(one_params.begin(), one_params.end(), gate);
      if (it != one_params.end()) {
        double angle = angle_dist(gen);
        circuit << "(" << std::to_string(angle) << ")";
      }
      circuit << " q[" << std::to_string(qubit) << "]" << ";" << std::endl;

    } else if (gate_type == 2) { // 2-qubit gate
      gate_idx = gate2_dist(gen);
      std::string gate = two_q_ops[gate_idx];
      circuit << "  " << gate;

      // Create random target qubit distinct from control qubit
      size_t target_qubit;
      do {
        target_qubit = qubit_dist(gen);
      } while (target_qubit == qubit); // Ensure different qubits.

      // Check if gate is a 1-parameter rotation gate. Create a random rotation angle if it is.
      auto it = std::find(one_params.begin(), one_params.end(), gate);
      if (it != one_params.end()) {
        double angle = angle_dist(gen);
        circuit << "(" << std::to_string(angle) << ")";
      }

      // Check if gate is a 3-parameter rotation gate. Create 3 random rotation angles if it is.
      it = std::find(three_params.begin(), three_params.end(), gate);
      if (it != three_params.end()) {
        double angle1 = angle_dist(gen);
        double angle2 = angle_dist(gen);
        double angle3 = angle_dist(gen);
        circuit << "(" << std::to_string(angle1) << "," << std::to_string(angle2) << "," << std::to_string(angle3) << ")";
      }

      circuit << " q[" << std::to_string(qubit) << "],q[" << std::to_string(target_qubit) << "];" << std::endl;
    }
  }

  // Measurement gates
  for (size_t i = 0; i < num_qubits; i++) {
    circuit << "  measure q[" << i << "] -> c[" << i << "];" << std::endl;
  }

  return circuit.str();
}

TEST(GateDeferralTester, randomCircuit) {
  size_t num_qubits = 4;
  // Create random circuit
  size_t circuit_depth = num_qubits * num_qubits;
  std::string randomCircuit = generate_random_circuit(num_qubits, circuit_depth);
  std::cout << "Circuit:\n" << randomCircuit << "\n";

  // Execute circuit
  std::shared_ptr<xacc::Compiler> openQasmCompiler = xacc::getCompiler("staq"); // Circuit is in openQasm, so we need to use the staq compiler
  std::shared_ptr<xacc::Accelerator> acc = xacc::getAccelerator("aer", {{"shots", 1024}});
  std::shared_ptr<xacc::CompositeInstruction> program =
      openQasmCompiler->compile(randomCircuit, nullptr)->getComposites()[0];
  auto buffer = xacc::qalloc(num_qubits);
  acc->execute(buffer, program);
  buffer->print();

  // Create circuit with deferred gates using the initial circuit
  qristal::gate_deferral_pass defer(program);
  std::string openQasmCircuit = defer.toOpenQasm();
  std::cout<<"openQasmCircuit:\n"<<openQasmCircuit<<"\n";
  std::shared_ptr<xacc::CompositeInstruction> program_deferred_gates = defer.toXasm();

  // Execute the circuit created using the deferred gate technique
  auto buffer_deferred_gates = xacc::qalloc(num_qubits);
  acc->execute(buffer_deferred_gates, program_deferred_gates);
  buffer_deferred_gates->print();

  // Check that both distributions are close
  std::map<std::string, int> m;
  std::map<std::string, int> m_compare;
  if (buffer->getMeasurementCounts().size() >= buffer_deferred_gates->getMeasurementCounts().size()) {
    m = buffer->getMeasurementCounts();
    m_compare = buffer_deferred_gates->getMeasurementCounts();
  } else {
    m = buffer_deferred_gates->getMeasurementCounts();
    m_compare = buffer->getMeasurementCounts();
  }

  for (const auto &[bitstring, count] : m) {
    int count_compare = m_compare[bitstring];
    double percentage_diff = (count > count_compare) ? (double)std::abs(count - count_compare) / count * 100
                                                     : (double)std::abs(count_compare - count) / count_compare * 100;;
    std::cout << "bitstring:" << bitstring << " : " << "count:" << count <<
        ", count_compare:" << count_compare << ", % diff:" << percentage_diff << "\n";
    if (count > 20 && count_compare > 20) {
      EXPECT_LT(percentage_diff, 50);
    }
  }
}

TEST(GateDeferralTester, testControlUnitaryGate) {
  // Create a control unitary circuit (a mcx gate in this example) that acts on the target qubit, condition
  // on the control qubit. The control qubit is prepared as the equal superposition |0> + |1>, hence acting
  // the unitary gate will flip the target qubit from |0> -> |1> with equal weight, i.e.
  // initial state: |00> + |10>, final state: |00> + |11>.
  const auto num_qubits = 2;
  const std::vector<int> control_qubit = {0};
  const int target_qubit = 1;

  // Create unitary circuit (a mcx gate in this case)
  std::shared_ptr<xacc::IRProvider> gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto x_gate_on_target_qubit = std::make_shared<xacc::quantum::X>(target_qubit);
  std::shared_ptr<xacc::CompositeInstruction> unitary_gate = gateRegistry->createComposite("__UNITARY_GATE__");
  unitary_gate->addInstruction(x_gate_on_target_qubit);
  auto unitary_circuit = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("C-U"));
  unitary_circuit->expand({{"U", unitary_gate}, {"control-idx", control_qubit}});

  // Create circuit
  auto circuit = gateRegistry->createComposite("__CIRCUIT__");
  // State prep - control qubit in |0> + |1> state
  circuit->addInstruction(gateRegistry->createInstruction("H", control_qubit[0]));
  // Add unitary circuit
  circuit->addInstruction(unitary_circuit);

  // Measure qubits
  for (size_t i = 0; i < num_qubits; i++) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", {i}));
  }

  // Execute circuit
  auto buffer = xacc::qalloc(num_qubits);
  auto acc = xacc::getAccelerator("aer", {{"shots", 1024}});
  acc->execute(buffer, circuit);
  buffer->print();

  // Create circuit with deferred gates using the initial circuit
  qristal::gate_deferral_pass defer(circuit);
  std::string openQasmCircuit = defer.toOpenQasm();
  std::cout<<"openQasmCircuit:\n"<<openQasmCircuit<<"\n";
  std::shared_ptr<xacc::CompositeInstruction> program_deferred_gates = defer.toXasm();

  // Execute the circuit created using the deferred gate technique
  auto buffer_deferred_gates = xacc::qalloc(num_qubits);
  acc->execute(buffer_deferred_gates, program_deferred_gates);
  buffer_deferred_gates->print();

  // Check that both distributions are close
  EXPECT_NEAR(buffer->getMeasurementCounts()["00"], buffer_deferred_gates->getMeasurementCounts()["00"], 50);
  EXPECT_NEAR(buffer->getMeasurementCounts()["11"], buffer_deferred_gates->getMeasurementCounts()["11"], 50);
}

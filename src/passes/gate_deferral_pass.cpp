// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/passes/gate_deferral_pass.hpp>
#include <qristal/core/circuit_builder.hpp>

namespace qristal {
gate_deferral_pass::gate_deferral_pass(const std::shared_ptr<xacc::CompositeInstruction> &circuit) :
    num_qubits(circuit->nPhysicalBits()),
    queue_Rx(num_qubits, false), queue_Ry(num_qubits, false), queue_H(num_qubits, false),
    angles_Rx(num_qubits, 0.0), angles_Ry(num_qubits, 0.0) {
  apply_gate_deferral(circuit);
}

std::string gate_deferral_pass::toOpenQasm() {
  std::string openQasmCircuit = R"(__qpu__ void circuit(qbit q) {)";
  openQasmCircuit += "\n";
  for (size_t i = 0; i < modified_circuit.size(); i++) {
    openQasmCircuit += "  " + modified_circuit[i] + "\n";
  }
  openQasmCircuit += "})\"";
  return openQasmCircuit;
}

std::shared_ptr<xacc::CompositeInstruction> gate_deferral_pass::toXasm() {
  std::string openQasmCircuit = toOpenQasm();
  std::shared_ptr<xacc::Compiler> xasmCompiler = xacc::getCompiler("xasm");
  std::shared_ptr<xacc::CompositeInstruction> program =
      xasmCompiler->compile(openQasmCircuit, nullptr)->getComposites()[0];

  return program;
}

void gate_deferral_pass::apply_gate_deferral(const std::shared_ptr<xacc::CompositeInstruction> &compositeInstruction) {
  // Walk the IR tree, and visit each node
  xacc::InstructionIterator it(compositeInstruction);
  while (it.hasNext()) {
    xacc::InstPtr nextInst = it.next();
    std::string gate_name = nextInst->name();
    std::vector<size_t> qubits = nextInst->bits();

    // Check if gate is supported
    std::string unsupported_gate_error_message = "Gate deferral technique does not support ";
    if (gate_name == "iSwap" || gate_name == "fSim" || gate_name == "IfStmt") {
      std::cout<< unsupported_gate_error_message << gate_name << "\n";
      throw std::runtime_error("Unsupported gate");
    }

    if (gate_op_map.find(gate_name) != gate_op_map.end()) {
      if (nextInst->isParameterized()) {
        std::vector<xacc::InstructionParameter> parameters = nextInst->getParameters();
        std::vector<double> angles(parameters.size());
        for (size_t i = 0; i < parameters.size(); i++) {
          angles[i] = xacc::InstructionParameterToDouble(parameters[i]);
        }
        apply_gate(gate_op_map.at(gate_name), qubits, angles);
      } else {
        apply_gate(gate_op_map.at(gate_name), qubits);
      }
    }
  }

  // Add back queued gates
  for (size_t i = 0; i < num_qubits; i++) {
    if (queue_H[i]) {
      queued_gates.emplace_back("H(q[" + std::to_string(i) + "]);");
    }
    if (queue_Rx[i]) {
      queued_gates.emplace_back("Rx(q[" + std::to_string(i) + "], " + std::to_string(angles_Rx[i]) + ");");
    }
    if (queue_Ry[i]) {
      queued_gates.emplace_back("Ry(q[" + std::to_string(i) + "], " + std::to_string(angles_Ry[i]) + ");");
    }
  }

  execute_phase_and_permute();
  // Add measure gates if supplied
  if (!measure.empty()) {
    for (size_t i = 0; i < measure.size(); i++) {
      modified_circuit.emplace_back(measure[i]);
    }
  }
}

// Strings for the names of the various gate types
void gate_deferral_pass::apply_gate(const OP &gate, std::vector<size_t> &qubits,
        std::optional<std::vector<double>> angles) {
  switch (gate) {
    case OP::I:
      I(qubits[0]);
      break;
    case OP::H:
      H(qubits[0]);
      break;
    case OP::X:
      X(qubits[0]);
      break;
    case OP::Y:
      Y(qubits[0]);
      break;
    case OP::Z:
      Z(qubits[0]);
      break;
    case OP::S:
      S(qubits[0]);
      break;
    case OP::Sdg:
      AdjS(qubits[0]);
      break;
    case OP::T:
      T(qubits[0]);
      break;
    case OP::Tdg:
      AdjT(qubits[0]);
      break;
    case OP::Measure:
      Measure(qubits[0]);
      break;
    case OP::Rx:
      R("Rx", qubits[0], angles.value()[0]);
      break;
    case OP::Ry:
      R("Ry", qubits[0], angles.value()[0]);
      break;
    case OP::Rz:
      R("Rz", qubits[0], angles.value()[0]);
      break;
    case OP::U:
      U(qubits[0], angles.value()[0], angles.value()[1], angles.value()[2]);
      break;
    case OP::CH:
      CH(qubits[0], qubits[1]);
      break;
    case OP::CNOT:
      CNOT(qubits[0], qubits[1]);
      break;
    case OP::CY:
      CY(qubits[0], qubits[1]);
      break;
    case OP::CZ:
      CZ(qubits[0], qubits[1]);
      break;
    case OP::SWAP:
      SWAP(qubits[0], qubits[1]);
      break;
    case OP::CPhase:
      CPhase(qubits[0], qubits[1], angles.value()[0]);
      break;
    case OP::CRX:
      CR("CRx", qubits[0], qubits[1], angles.value()[0]);
      break;
    case OP::CRY:
      CR("CRy", qubits[0], qubits[1], angles.value()[0]);
      break;
    case OP::CRZ:
      CR("CRz", qubits[0], qubits[1], angles.value()[0]);
      break;
  }
}

void gate_deferral_pass::execute_RyRxH_single_qubit(size_t &qubit) {
  execute_RxH_single_qubit(qubit);
  if (queue_Ry[qubit]) {
    modified_circuit.emplace_back("Ry(q[" + std::to_string(qubit) + "], " + std::to_string(angles_Ry[qubit]) + ");");
    angles_Ry[qubit] = 0.0;
    queue_Ry[qubit] = false;
  }
}

void gate_deferral_pass::execute_RxH_single_qubit(size_t &qubit) {
  execute_H_single_qubit(qubit);
  if (queue_Rx[qubit]) {
    modified_circuit.emplace_back("Rx(q[" + std::to_string(qubit) + "], " + std::to_string(angles_Rx[qubit]) + ");");
    angles_Rx[qubit] = 0.0;
    queue_Rx[qubit] = false;
  }
}

void gate_deferral_pass::execute_H_single_qubit(size_t &qubit) {
  if (queue_H[qubit]) {
    modified_circuit.emplace_back("H(q[" + std::to_string(qubit) + "]);");
    queue_H[qubit] = false;
  }
}

// Executes all phase and permutation operations, followed by any H, Rx, or Ry gates queued on the qubit
// index, up to the level specified (where H < Rx < Ry).
void gate_deferral_pass::execute_queued_ops(size_t &qubit, std::string Op) {
  // Use simple strategy with queues of four types of gates: Phase/permutation gates, H, Rx, and Ry.
  // For a gate G, we check for any Ry, Rx, then H gates queued on the requested qubit, and attempt
  // to commute G through each gate. If a simple commutation relation exists, G is modified and
  // continued. If the commutation relation is difficult to compute, we execute all queued gates
  // on those qubits, including any phase/permutation gates.
  execute_phase_and_permute();
  if (Op == "Ry") {
    execute_RyRxH_single_qubit(qubit);
  } else if (Op == "Rx") {
    execute_RxH_single_qubit(qubit);
  } else if (Op == "H") {
    execute_H_single_qubit(qubit);
  }
}

void gate_deferral_pass::execute_phase_and_permute() {
  if (!queued_gates.empty()) {
    for (size_t i = 0; i < queued_gates.size(); i++) {
      modified_circuit.emplace_back(queued_gates[i]);
    }
    queued_gates.clear();
  }
}

// Executes if there is anything already queued on the qubits in controls. Used when queuing gates
// that do not commute well.
void gate_deferral_pass::execute_if(size_t &qubit) {
  if (queue_Ry[qubit] || queue_Rx[qubit] || queue_H[qubit]) {
    execute_queued_ops(qubit, "Ry");
  }
}


//-------------------------------------------------------------------------------------------------
//----------------------------------------- 1-qubit gates -----------------------------------------
//-------------------------------------------------------------------------------------------------
void gate_deferral_pass::I(size_t &qubit) {}

void gate_deferral_pass::H(size_t &qubit) {
  // YH = -HY
  angles_Ry[qubit] *= (queue_Ry[qubit] ? -1.0 : 1.0);

  // Commuting with Rx creates a phase, but on the wrong side. So we execute any Rx immediately.
  if (queue_Rx[qubit]) {
    execute_queued_ops(qubit, "Rx");
  }
  queue_H[qubit] = !queue_H[qubit];
}

void gate_deferral_pass::X(size_t &qubit) {
  // XY = - YX
  if (queue_Ry[qubit]) {
    angles_Ry[qubit] *= -1.0;
  }

  // Rx trivially commutes, so do nothing when it is encountered.

  // Since XH = HZ, we can add a Z gate to the phase/permutation queue and keep the H queue.
  if (queue_H[qubit]) {
    queued_gates.emplace_back("Z(q[" + std::to_string(qubit) + "]);");
    return;
  }
  queued_gates.emplace_back("X(q[" + std::to_string(qubit) + "]);");
}

void gate_deferral_pass::Y(size_t &qubit) {
  // XY = -YX
  if (queue_Rx[qubit]) {
    angles_Rx[qubit] *= -1.0;
  }
  // commutes with H up to phase, so we ignore the H queue
  queued_gates.emplace_back("Y(q[" + std::to_string(qubit) + "]);");
}

void gate_deferral_pass::Z(size_t &qubit) {
  // ZY = -YZ
  if (queue_Ry[qubit]) {
    angles_Ry[qubit] *= -1;
  }
  // XZ = -ZX
  if (queue_Rx[qubit]) {
    angles_Rx[qubit] *= -1;
  }
  // HZ = XH
  if (queue_H[qubit]) {
    queued_gates.emplace_back("X(q[" + std::to_string(qubit) + "]);");
    return;
  }
  queued_gates.emplace_back("Z(q[" + std::to_string(qubit) + "]);");
}

void gate_deferral_pass::S(size_t &qubit) {
  double angle = M_PI / 2.0;
  Phase(qubit, angle);
}

void gate_deferral_pass::AdjS(size_t &qubit) {
  double angle = -M_PI / 2.0;
  Phase(qubit, angle);
}

void gate_deferral_pass::T(size_t &qubit) {
  double angle = M_PI / 4.0;
  Phase(qubit, angle);
}

void gate_deferral_pass::AdjT(size_t &qubit) {
  double angle = -M_PI / 4.0;
  Phase(qubit, angle);
}

void gate_deferral_pass::Measure(size_t &qubit) {
  measure.emplace_back("Measure(q[" + std::to_string(qubit) + "]);");
}

//-------------------------------------------------------------------------------------------------
//------------------------------------- 1-qubit rotation gates ------------------------------------
//-------------------------------------------------------------------------------------------------
void gate_deferral_pass::R(std::string gate, size_t &qubit, double &angle) {
  if (gate == "Ry") {
    queue_Ry[qubit] = true;
    angles_Ry[qubit] += angle;
    return;
  } else if (queue_Ry[qubit]) {
    execute_queued_ops(qubit, "Ry");
  }

  if (gate == "Rx") {
    queue_Rx[qubit] = true;
    angles_Rx[qubit] += angle;
    return;
  } else if (queue_Rx[qubit]) {
    execute_queued_ops(qubit, "Rz");
  }

	// A Rz is just a phase.
  if (gate == "Rz") {
    // HRz = RxH, but that's the wrong order for this structure, thus we must execute the H queue
    if (queue_H[qubit]) {
      execute_queued_ops(qubit, "H");
    }
    // Rz(phi) = RI(phase)*R1(-2*phase) -> global phase from RI is ignored
    R1(qubit, angle);
  }
}

void gate_deferral_pass::R1(size_t &qubit, double &angle) {
  Phase(qubit, angle);
}

void gate_deferral_pass::Phase(size_t &qubit, double &angle) {
  // Rx, Ry, and H do not commute well with arbitrary angle gates
  if (queue_Ry[qubit] || queue_Rx[qubit] || queue_H[qubit]) {
    execute_queued_ops(qubit, "Ry");
  }
  queued_gates.emplace_back("U1(q[" + std::to_string(qubit) + "], " + std::to_string(angle) + ");");
}

void gate_deferral_pass::U(size_t &qubit, double &theta, double &phi, double &lambda) {
  // Rz(lambda)
  R("Rz", qubit, lambda);
  // Ry(theta)
  R("Ry", qubit, theta);
  // Rz(phi)
  R("Rz", qubit, phi);
}

//-------------------------------------------------------------------------------------------------
//----------------------------------------- 2-qubit gates -----------------------------------------
//-------------------------------------------------------------------------------------------------
void gate_deferral_pass::CH(size_t &control_qubit, size_t &target_qubit) {
  // No commutation on controls
  execute_if(control_qubit);
  // No Ry or Rx commutation on target
  if (queue_Ry[target_qubit] || queue_Rx[target_qubit]) {
    execute_queued_ops(target_qubit, "Ry");
  }
  // Commutes through H gates on the target, so it does not check
  execute_phase_and_permute();
  modified_circuit.emplace_back("CH(q[" + std::to_string(control_qubit) + "], " +
                                   "q[" + std::to_string(target_qubit)  + "]);");
}

void gate_deferral_pass::CNOT(size_t &control_qubit, size_t &target_qubit) {
  // A H on the control but not the target forces execution
  if (queue_Ry[control_qubit] || queue_Rx[control_qubit] || (queue_H[control_qubit] && !queue_H[target_qubit])) {
    execute_queued_ops(control_qubit, "Ry");
  }

  // Rx on the target trivially commutes.
  // Ry on the target causes issues, so execute it immediately
  if (queue_Ry[target_qubit]) {
    execute_queued_ops(target_qubit, "Ry");
  }

  // A H on the target flips the operation
  if (queue_H[target_qubit]) {
    // If it is a CNOT and there is also a H on the control, swap control and target qubits
    if (queue_H[control_qubit]) {
      queued_gates.emplace_back("CNOT(q[" + std::to_string(target_qubit) + "], " +
                                     "q[" + std::to_string(control_qubit)  + "]);");
    } else {
      queued_gates.emplace_back("CZ(q[" + std::to_string(control_qubit) + "], " +
                                   "q[" + std::to_string(target_qubit)  + "]);");
    }
    return;
  }
  queued_gates.emplace_back("CNOT(q[" + std::to_string(control_qubit) + "], " +
                                 "q[" + std::to_string(target_qubit)  + "]);");
}

void gate_deferral_pass::CY(size_t &control_qubit, size_t &target_qubit) {
  execute_if(control_qubit);
  // Commutes with Ry on the target, not Rx
  if (queue_Rx[target_qubit]) {
    execute_queued_ops(target_qubit, "Rx");
  }

  // HY = -YH, so we add a phase to track this
  if (queue_H[target_qubit]) {
    // The phase added does not depend on the target, thus we use one of the controls as a target
    queued_gates.emplace_back("Z(q[" + std::to_string(control_qubit) + "]);");
  }
  queued_gates.emplace_back("CY(q[" + std::to_string(control_qubit) + "], " +
                               "q[" + std::to_string(target_qubit)  + "]);");
}

void gate_deferral_pass::CZ(size_t &control_qubit, size_t &target_qubit) {
  // If the only thing on the controls is one H, we can switch this to an MCX. Any Rx or Ry, or more than 1 H,
  // means we must execute.
  size_t count = 0;
  if (queue_Ry[control_qubit] || queue_Rx[control_qubit]) count += 2;
  if (queue_H[control_qubit]) count++;
  if (queue_Ry[target_qubit] || queue_Rx[target_qubit]) count +=2;
  if (queue_H[target_qubit]) count++;

  if (count > 1) {
    execute_queued_ops(control_qubit, "Ry");
    execute_queued_ops(target_qubit, "Ry");
  } else if (count == 1) {
    // Transform to an MCX, but we need to swap one of the controls with the target if the Hadamard is on one
    // of the control qubits.
    size_t new_control_qubit(control_qubit);
    if (queue_H[new_control_qubit]) {
      std::swap(new_control_qubit, target_qubit);
    }
    queued_gates.emplace_back("CNOT(q[" + std::to_string(new_control_qubit) + "], " +
                                   "q[" + std::to_string(target_qubit)  + "]);");
    return;
  }
  queued_gates.emplace_back("CZ(q[" + std::to_string(control_qubit) + "], " +
                               "q[" + std::to_string(target_qubit)  + "]);");
}

void gate_deferral_pass::SWAP(size_t &qubit1, size_t &qubit2) {
  if (qubit1 > qubit2) {
    std::swap(qubit2, qubit1);
  }

  // Everything commutes nicely with a swap
  queue_Ry.swap(queue_Ry[qubit1], queue_Ry[qubit2]);
  std::swap(angles_Ry[qubit1], angles_Ry[qubit2]);
  queue_Rx.swap(queue_Rx[qubit1], queue_Rx[qubit2]);
  std::swap(angles_Rx[qubit1], angles_Rx[qubit2]);
  queue_H.swap(queue_H[qubit1], queue_H[qubit2]);
  queued_gates.emplace_back("Swap(q[" + std::to_string(qubit1) + "], " +
                                 "q[" + std::to_string(qubit2) + "]);");
}

//-------------------------------------------------------------------------------------------------
//------------------------------------- 2-qubit rotation gates ------------------------------------
//-------------------------------------------------------------------------------------------------
void gate_deferral_pass::CR(std::string gate, size_t &control_qubit, size_t &target_qubit, double &angle) {
  execute_if(control_qubit);
  if (queue_Ry[target_qubit] && gate != "CRy") {
    execute_queued_ops(target_qubit, "Ry");
  }
  if (queue_Rx[target_qubit] && gate != "CRx") {
    execute_queued_ops(target_qubit, "Rx");
  }
  if (queue_H[target_qubit]) {
    execute_queued_ops(target_qubit, "H");
  }

  // Execute any phase and permutation gates. These are not indexed by qubit so it does not matter what
  // the qubit argument is.
  size_t qubit = 0;
  execute_queued_ops(qubit, "PermuteLarge");
  modified_circuit.emplace_back(gate + "(q[" + std::to_string(control_qubit) + "], " +
                                       "q[" + std::to_string(target_qubit)  + "], " + 
                                       std::to_string(angle) + ");");
}

void gate_deferral_pass::CPhase(size_t &control_qubit, size_t &target_qubit, double &angle) {
  execute_if(control_qubit);
  execute_if(target_qubit);
  queued_gates.emplace_back("CPhase(q[" + std::to_string(control_qubit) + "], " +
                                   "q[" + std::to_string(target_qubit)  + "], " + 
                                   std::to_string(angle) + ");");
}
} // namespace qristal

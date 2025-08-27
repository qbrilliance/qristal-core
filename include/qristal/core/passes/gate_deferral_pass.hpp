// Copyright (c) Quantum Brilliance Pty Ltd

// Gate deferral technique used in Microsoft Quantum's sparse simulator backend,
// see https://quantum.microsoft.com/en-us/insights/blogs/qsharp/testing-large-quantum-algorithms-using-sparse-simulation
// This technique defers gates that generate superpositions in order to maintain the sparsity of the state for
// as long as possible.
#pragma once

// XACC
#include <xacc.hpp>
#include <AllGateVisitor.hpp>

// STL
#include <optional>

namespace qristal {
// Gates
enum class OP {
  I,
  H,
  X,
  Y,
  Z,
  S,
  Sdg,
  T,
  Tdg,
  Measure,
  Rx,
  Ry,
  Rz,
  U,
  CH,
  CNOT,
  CY,
  CZ,
  SWAP,
  CRX,
  CRY,
  CRZ,
  CPhase
};

// Map containing gate string to type OP
const std::map<const std::string, const OP> gate_op_map =
  {{"I", OP::I},
   {"H", OP::H},
   {"X", OP::X},
   {"Y", OP::Y},
   {"Z", OP::Z},
   {"S", OP::S},
   {"Sdg", OP::Sdg},
   {"T", OP::T},
   {"Tdg", OP::Tdg},
   {"Measure", OP::Measure},
   {"Rx", OP::Rx},
   {"Ry", OP::Ry},
   {"Rz", OP::Rz},
   {"U", OP::U},
   {"CH", OP::CH},
   {"CNOT", OP::CNOT},
   {"CY", OP::CY},
   {"CZ", OP::CZ},
   {"Swap", OP::SWAP},
   {"CRX", OP::CRX},
   {"CRY", OP::CRY},
   {"CRZ", OP::CRZ},
   {"CPhase", OP::CPhase}};

class gate_deferral_pass {
 public:
  /**
   * @brief Construct a new gate deferral pass object
   * 
   * @param circuit Input circuit
   */
  gate_deferral_pass(const std::shared_ptr<xacc::CompositeInstruction> &circuit);

  /**
   * @brief Apply gate deferral to circuit
   * 
   * @param compositeInstruction Input circuit
   */
  void apply_gate_deferral(const std::shared_ptr<xacc::CompositeInstruction> &compositeInstruction);

  /**
   * @brief Apply gate
   * 
   * @param gate Gate name
   * @param qubits Qubits the gate acts on
   * @param angles Optional angles for rotation gate
   */
  void apply_gate(const OP &gate, std::vector<size_t> &qubits, 
          std::optional<std::vector<double>> angles = std::nullopt);

  /**
   * @brief Convert circuit to openQASM format
   * 
   * @return Converted circuit as a string
   */
  std::string toOpenQasm();

  /**
   * @brief Convert circuit to XASM format
   * 
   * @return Converted circuit as a Xacc composite instruction 
   */
  std::shared_ptr<xacc::CompositeInstruction> toXasm();

  /**
   * @brief Execute queued single qubit gates Rx, Ry and H
   * 
   * @param qubit Qubit the gate acts on
   */
  void execute_RyRxH_single_qubit(size_t &qubit);

  /**
   * @brief Execute queued single qubit gates Rx and H
   * 
   * @param qubit Qubit the gate acts on
   */
  void execute_RxH_single_qubit(size_t &qubit);

  /**
   * @brief Execute queued single qubit gate H
   * 
   * @param qubit Qubit the gate acts on
   */
  void execute_H_single_qubit(size_t &qubit);

  /**
   * @brief Execute queued gates
   * 
   * @param qubit Qubit the gate acts on
   * @param Op Gate operator
   */
  void execute_queued_ops(size_t &qubit, std::string Op);

  /**
   * @brief Adds all gates queued gates to circuit
   * 
   */
  void execute_phase_and_permute();

  /**
   * @brief Execute queued gates before a control qubit
   * 
   * @param qubit Qubit the queued gate acts on
   */
  void execute_if(size_t &qubit);

  // 1-qubit gates
  void I(size_t &qubit);
  void H(size_t &qubit);
  void X(size_t &qubit);
  void Y(size_t &qubit);
  void Z(size_t &qubit);
  void S(size_t &qubit);
  void AdjS(size_t &qubit);
  void T(size_t &qubit);
  void AdjT(size_t &qubit);
  void Measure(size_t &qubit);

  // 1-qubit rotation gates
  void R(std::string gate, size_t &qubit, double &angle);
  void R1(size_t &qubit, double &angle);
  void Phase(size_t &qubit, double &angle);
  void U(size_t &qubit, double &theta, double &phi, double &lambda);

  // 2-qubit gates
  void CH(size_t &control_qubit, size_t &target_qubit);
  void CNOT(size_t &control_qubit, size_t &target_qubit);
  void CY(size_t &control_qubit, size_t &target_qubit);
  void CZ(size_t &control_qubit, size_t &target_qubit);
  void SWAP(size_t &qubit1, size_t &qubit2);

  // 2-qubit rotation gates
  void CR(std::string gate, size_t &control_qubit, size_t &target_qubit, double &angle);
  void CPhase(size_t &control_qubit, size_t &target_qubit, double &angle);

 protected:
  std::vector<std::string> modified_circuit = {};
  std::vector<std::string> queued_gates = {};

 private:
  size_t num_qubits = 0;
	// These indicate whether there are any H, Rx, or Ry gates that have yet to be applied to the wavefunction.
	// Since HH = I and Rx(theta_1)Rx(theta_2) = Rx(theta_1 + theta_2), it only needs a boolean to track them.
	std::vector<bool> queue_H = {};
	std::vector<bool> queue_Rx = {};
	std::vector<bool> queue_Ry = {};
	std::vector<double> angles_Rx = {};
	std::vector<double> angles_Ry = {};
  std::vector<std::string> measure = {};
  std::vector<std::reference_wrapper<xacc::quantum::Circuit>> m_controlledBlocks;
};
} // namespace qristal

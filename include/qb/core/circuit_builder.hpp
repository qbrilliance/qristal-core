// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#pragma once

#include "CompositeInstruction.hpp"
#include "IRProvider.hpp"
#include "InstructionIterator.hpp"
//#include <assert.h>
//#include <iostream>
#include "Accelerator.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <assert.h>
#include "Circuit.hpp"
#include "GateModifier.hpp"
namespace qb {
using StatePrepFuncCType =
    std::function<std::shared_ptr<xacc::CompositeInstruction>(
        std::vector<int>, std::vector<int>,
        std::vector<int>, std::vector<int>, std::vector<int>)>;
using OracleFuncCType =
    std::function<std::shared_ptr<xacc::CompositeInstruction>(int)>;

inline std::set<std::size_t> uniqueBitsQD(std::shared_ptr<xacc::CompositeInstruction> &circ) {
    std::set<std::size_t> uniqueBits;
    xacc::InstructionIterator iter(circ);
    while (iter.hasNext()) {
      auto next = iter.next();
      if (!next->isComposite()) {
        for (auto &b : next->bits()) {
          uniqueBits.insert(b);
        }
      } else if (next->isComposite() && next->name() == "C-U") {
        auto *asControlledBlock =
            dynamic_cast<xacc::quantum::ControlModifier *>(next.get());
        if (asControlledBlock) {
          const auto controlQubits = asControlledBlock->getControlQubits();
          auto baseCircuit = asControlledBlock->getBaseInstruction();
          assert(baseCircuit->isComposite());
          auto asComp = xacc::ir::asComposite(baseCircuit);
          auto targetUniqueBits = asComp->uniqueBits();
          for (const auto &[reg, qIdx] : controlQubits) {
              uniqueBits.insert(qIdx);
          }
          for (const auto &qIdx : targetUniqueBits) {
              uniqueBits.insert(qIdx);
          }
        }
      }
    }
    return uniqueBits;
  }

/**
* @brief This class is used to build quantum circuits for execution.
*
* This class is used to construct quantum circuits from elementary gates,
* such as X, Y, Z, Hadamard and CNOT.
*
* We also provide high-level methods to construct quantum circuits
* for commonly-used quantum algorithms, such as QFT and amplitude amplification
*/
class CircuitBuilder {
private:
  std::shared_ptr<xacc::IRProvider> gate_provider_;
  std::shared_ptr<xacc::CompositeInstruction> circuit_;

public:

/// @private
static const char *help_execute_;


  /**
  * @brief Constructor
  *
  * A constructor for the CircuitBuilder class.
  * Creates an empty circuit.
  */
  CircuitBuilder()
      : gate_provider_(xacc::getService<xacc::IRProvider>("quantum")) {
    circuit_ = gate_provider_->createComposite("QBSDK_circuit");
  }

  /**
  * @brief Constructor
  *
  * A constructor for the CircuitBuilder class.
  * Creates a circuit from a specified list of instructions.
  *
  * @param composite A pointer to a xacc::CompositeInstruction object [shared ptr]
  */
  CircuitBuilder(std::shared_ptr<xacc::CompositeInstruction> &composite) : gate_provider_(xacc::getService<xacc::IRProvider>("quantum")) {
      circuit_ = gate_provider_->createComposite("QBSDK_circuit");
      circuit_->addInstructions(composite->getInstructions());
  }

  /**
  * @brief return the list of instructions comprising the circuit
  *
  * @return A pointer to the xacc::CompositeInstruction (list of instructions) that defines the circuit.
  */
  std::shared_ptr<xacc::CompositeInstruction> get() { return circuit_; }

  /**
  * @brief print the list of instructions comprising the circuit
  */
  void print() { std::cout << circuit_->toString() << std::endl; }

  /**
  * @brief append another CircuitBuilder object to this one
  *
  * @param other the circuit to be appended [CircuitBuilder]
  */
  void append(CircuitBuilder &other) {
    xacc::InstructionIterator it(other.circuit_);
    while (it.hasNext()) {
      auto nextInst = it.next();
      if (nextInst->isEnabled() && !nextInst->isComposite()) {
        circuit_->addInstruction(nextInst->clone());
      }
    }
  }

  // Gates:
  /**
  * @brief Hadamard gate
  *
  * This method adds a Hadamard (H) gate to the circuit.
  *
  * The H gate is defined by its action on the basis states
  *
  * H|0> -> |+>
  *
  * H|1> -> |->
  *
  * @param idx the index of the qubit being acted on [size_t]
  */
  void H(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("H", idx));
  }

    /**
  * @brief Pauli-X gate
  *
  * This method adds a Pauli-X (X) gate to the circuit.
  *
  * The X gate is defined by its action on the basis states
  *
  * X|0> -> |1>
  *
  * X|1> -> |0>
  *
  * @param idx the index of the qubit being acted on [size_t]
  */
  void X(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("X", idx));
  }

    /**
  * @brief Pauli-Y gate
  *
  * This method adds a Pauli-Y (Y) gate to the circuit.
  *
  * The Y gate is defined by its action on the basis states
  *
  * Y|0> -> -i|1>
  *
  * Y|1> -> i |0>
  *
  * @param idx the index of the qubit being acted on [size_t]
  */
  void Y(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Y", idx));
  }

    /**
  * @brief Pauli-Z gate
  *
  * This method adds a Pauli-Z (Z) gate to the circuit.
  *
  * The Z gate is defined by its action on the basis states
  *
  * Z|0> -> |0>
  *
  * Z|1> -> -|1>
  *
  * @param idx the index of the qubit being acted on [size_t]
  */
  void Z(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Z", idx));
  }

    /**
  * @brief T gate
  *
  * This method adds a T gate to the circuit.
  *
  * The T gate is defined by its action on the basis states
  *
  * T|0> -> |0>
  *
  * T|1> -> e^{i\pi/4}|1>
  *
  * @param idx the index of the qubit being acted on [size_t]
  */
  void T(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("T", idx));
  }

    /**
  * @brief S gate
  *
  * This method adds an S gate to the circuit.
  *
  * The S gate is defined by its action on the basis states
  *
  * S|0> -> |0>
  *
  * S|1> -> i|1>
  *
  * @param idx the index of the qubit being acted on [size_t]
  */
  void S(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("S", idx));
  }

    /**
  * @brief Tdg gate
  *
  * This method adds an inverse of the T gate (Tdg) to the circuit.
  *
  * The Tdg gate is defined by its action on the basis states
  *
  * Tdg|0> -> |0>
  *
  * Tdg|1> -> e^{-i\pi/4}|1>
  *
  * @param idx the index of the qubit being acted on [size_t]
  */
  void Tdg(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Tdg", idx));
  }

    /**
  * @brief Sdg gate
  *
  * This method adds an inverse of the S gate (Sdg) to the circuit.

  * The Sdg gate is defined by its action on the basis states
  *
  * Sdg|0> -> |0>
  *
  * Sdg|1> -> i|1>
  *
  * @param idx the index of the qubit being acted on [size_t]
  */
  void Sdg(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Sdg", idx));
  }

    /**
  * @brief RX gate
  *
  * This method adds an x-axis rotation (RX) gate to the circuit.
  *
  * The RX gate is defined by its action on the basis states
  *
  * RX(theta)|0> -> cos(theta/2)|0> - i*sin(theta/2)|1>
  *
  * RX(theta)|1> -> -i*sin(theta/2)|0> + cos(theta/2)|1>
  *
  * @param idx the index of the qubit being acted on [size_t]
  * @param theta the angle of rotation about the x-axis [double]
  */
  void RX(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Rx", {idx}, {theta}));
  }

      /**
  * @brief RY gate
  *
  * This method adds a y-axis rotation (RY) gate to the circuit.
  *
  * The RY gate is defined by its action on the basis states
  *
  * RY(theta)|0> -> cos(theta/2)|0> + sin(theta/2)|1>
  *
  * RY(theta)|1> -> -sin(theta/2)|0> + cos(theta/2)|>
  *
  * @param idx the index of the qubit being acted on [size_t]
  * @param theta the angle of rotation about the y-axis [double]
  */
  void RY(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Ry", {idx}, {theta}));
  }

      /**
  * @brief RZ gate
  *
  * This method adds a z-axis rotation (RZ) gate to the circuit.
  *
  * The RZ gate is defined by its action on the basis states
  *
  * RZ(theta)|0> -> e^{-itheta/2}|0>
  *
  * RZ(theta)|1> -> e^{itheta/2}|1>
  *
  * @param idx the index of the qubit being acted on [size_t]
  * @param theta the angle of rotation about the z-axis [double]
  */
  void RZ(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Rz", {idx}, {theta}));
  }

        /**
  * @brief U1 gate
  *
  * This method adds a phase (U1) gate to the circuit.
  *
  * The U1 gate is defined by its action on the basis states
  *
  * U1(theta)|0> -> |0>
  *
  * U1(theta)|1> -> e^{itheta}|1>
  *
  * @param idx the index of the qubit being acted on [size_t]
  * @param theta the value of the phase [double]
  */
  void U1(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("U1", {idx}, {theta}));
  }

        /**
  * @brief U3 gate
  *
  * This method adds an arbitrary single qubit gate to the circuit.
  *
  * The U3 gate is defined by its action on the basis states
  *
  * U3(theta,phi,lambda)|0> -> cos(theta/2)|0> + e^{iphi}sin(theta/2)|1>
  *
  * U3(theta,phi,lambda)|1> -> -e^{ilambda}sin(theta/2)|0> + e^{i(phi+lambda)}cos(theta/2)|1>
  *
  * @param idx the index of the qubit being acted on [size_t]
  * @param theta the angle of rotation about the z-axis [double]
  */
  void U3(size_t idx, double theta, double phi, double lambda) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("U", {idx}, {theta, phi, lambda}));
  }

      /**
  * @brief CNOT gate
  *
  * This method adds a controlled-X (CNOT) gate to the circuit.
  *
  * The CNOT gate performs an X gate on the target qubit
  * conditional on the control qubit being in the |1> state.
  *
  * @param ctrl_idx the index of the control qubit [size_t]
  * @param target_idx the index of the target qubit [size_t]
  */
  void CNOT(size_t ctrl_idx, size_t target_idx) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("CNOT", {ctrl_idx, target_idx}));
  }

        /**
  * @brief MCX gate
  *
  * This method adds a multi-controlled X (MCX) gate to the circuit.
  *
  * The MCX gate performs an X gate on the target qubit
  * conditional on all control qubits being in the |1> state.
  *
  * @param ctrl_inds the indices of the control qubits [vector of int]
  * @param target_idx the index of the target qubit [size_t]
  */
  void MCX(const std::vector<int> &ctrl_inds, size_t target_idx) {
    auto x_gate = gate_provider_->createComposite("temp_X");
    auto temp_gate = gate_provider_->createInstruction("X", target_idx);
    temp_gate->setBufferNames({"q"});
    x_gate->addInstruction(temp_gate);
    auto controlled_U = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    controlled_U->expand({{"U", x_gate}, {"control-idx", ctrl_inds}});
    circuit_->addInstruction(controlled_U);
  }

  /**
  * @brief CU gate
  *
  * This method adds a controlled version of an arbitrary unitary (CU) to the circuit.
  *
  * The CU gate implements the U gate on the target qubits
  * conditional on all control qubits being in the |1> state.
  *
  * @param circ the circuit for the unitary operation U [CircuitBuilder]
  * @param ctrl_inds the indices of the control qubits [vector of int]
  */
  void CU(CircuitBuilder &circ, std::vector<int> ctrl_inds) {
    auto controlled_U = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    controlled_U->expand({{"U", circ.circuit_}, {"control-idx", ctrl_inds}});
    circuit_->addInstruction(controlled_U);
  }

        /**
  * @brief CZ gate
  *
  * This method adds a controlled-Z (CZ) gate to the circuit.
  *
  * The CZ gate performs a Z gate on the target qubit
  * conditional on the control qubit being in the |1> state.
  *
  * @param ctrl_idx the index of the control qubit [size_t]
  * @param target_idx the index of the target qubit [size_t]
  */
  void CZ(size_t ctrl_idx, size_t target_idx) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("CZ", {ctrl_idx, target_idx}));
  }

        /**
  * @brief CH gate
  *
  * This method adds a controlled-H (CH) gate to the circuit.
  *
  * The CH gate performs an H gate on the target qubit
  * conditional on the control qubit being in the |1> state.
  *
  * @param ctrl_idx the index of the control qubit [size_t]
  * @param target_idx the index of the target qubit [size_t]
  */
  void CH(size_t ctrl_idx, size_t target_idx) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("CH", {ctrl_idx, target_idx}));
  }

  // CPhase == CU1
        /**
  * @brief CPhase gate
  *
  * This method adds a controlled-U1 (CPhase) gate to the circuit.
  *
  * The CPHase gate performs a U1(theta) gate on the target qubit
  * conditional on the control qubit being in the |1> state.
  *
  * @param ctrl_idx the index of the control qubit [size_t]
  * @param target_idx the index of the target qubit [size_t]
  * @param theta the value of the phase [double]
  */
  void CPhase(size_t ctrl_idx, size_t target_idx, double theta) {
    circuit_->addInstruction(gate_provider_->createInstruction(
        "CPhase", {ctrl_idx, target_idx}, {theta}));
  }

        /**
  * @brief SWAP gate
  *
  * This method adds a SWAP gate to the circuit.
  *
  * The SWAP gate is used to swap the quantum state of two qubits.
  * That is, it acts as:
  *
  * SWAP|psi> |phi> -> |phi> |psi>
  *
  * @param q1 the index of the first qubit [size_t]
  * @param q2 the index of the second qubit [size_t]
  */
  void SWAP(size_t q1, size_t q2) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Swap", {q1, q2}));
  }

  /**
  * @brief Measurement
  *
  * This method is used to indicate a qubit in the circuit should be measured.
  *
  * @param idx the index of the qubit to be measured [size_t]
  */
  void Measure(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Measure", idx));
  }

  /**
  * @brief Measure all qubits
  *
  * This method adds a measurement for all qubits involved in the circuit.
  *
  * @param NUM_QUBITS the number of qubits in the circuit [int] [optional]
  */
  void MeasureAll(int NUM_QUBITS) {
    int nbQubits;
    if (NUM_QUBITS < 0) {
        auto qubits_set = uniqueBitsQD(circuit_);
        nbQubits = 0;
        for (int q: qubits_set) {
            if (q+1 > nbQubits) {
                nbQubits = q+1;
            }
        }
    } else {
        nbQubits = NUM_QUBITS;
    }
    // std::cout << "nbQubits " << nbQubits << "\n";
    for (int idx = 0; idx < nbQubits; ++idx) {
      circuit_->addInstruction(
          gate_provider_->createInstruction("Measure", (size_t)idx));
    }
  }

  /**
  * @brief Quantum Fourier Transform
  *
  * This method adds the Quantum Fourier Transform (QFT) to the circuit.
  * This is a quantum analogue of the discrete Fourier Transform.
  *
  * @param qubit_idxs the indices of the target qubits [vector of int]
  */
  void QFT(const std::vector<int> &qubit_idxs) {
    auto qft = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("qft"));
    qft->expand({{"nq", static_cast<int>(qubit_idxs.size())}});

    // Need to remap qubit operands of these instruction to the
    // evaluation_qubits register range
    for (auto &inst : qft->getInstructions()) {
      const auto bits = inst->bits();
      std::vector<size_t> new_bits;
      for (const auto &bit : bits) {
        new_bits.emplace_back(qubit_idxs[bit]);
      }
      auto new_inst = inst->clone();
      new_inst->setBits(new_bits);
      circuit_->addInstruction(new_inst);
    }
  }

  /**
  * @brief Inverse Quantum Fourier Transform
  *
  * This method adds the inverse of the Quantum Fourier Transform (IQFT) to the circuit.
  *
  * @param qubit_idxs the indices of the target qubits [vector of int]
  */
  void IQFT(const std::vector<int> &qubit_idxs) {
    auto qft = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("iqft"));
    qft->expand({{"nq", static_cast<int>(qubit_idxs.size())}});

    // Need to remap qubit operands of these instruction to the
    // evaluation_qubits register range
    for (auto &inst : qft->getInstructions()) {
      const auto bits = inst->bits();
      std::vector<size_t> new_bits;
      for (const auto &bit : bits) {
        new_bits.emplace_back(qubit_idxs[bit]);
      }
      auto new_inst = inst->clone();
      new_inst->setBits(new_bits);
      circuit_->addInstruction(new_inst);
    }
  }

  /**
  * @brief Quantum Phase Estimation
  *
  * This method adds the Quantum Phase Estimation (QPE) sub-routine to the circuit.
  *
  * Given some unitary operator $U$ and and eigenvector |psi> of U we can write
  *
  * U|psi> = e^{2\pi i theta}|psi>
  *
  * for some value of theta. QPE is used to provide a k-bit approximation to theta
  * storing the result in an evaluation register whilst leaving |psi> unchanged.
  *
  * @param oracle The unitary operator U involved in the QPE routine [CircuitBuilder]
  * @param num_evaluation_qubits The number of bits k used to approximate the phase [int]
  * @param trial_qubits The indices of the qubits encoding the eigenvector of the unitary [vector of int]
  * @param evaluation_qubits The indices of the qubits that will be used to store the approximate phase [vector of int]
  */
  void QPE(CircuitBuilder &oracle, int num_evaluation_qubits,
           std::vector<int> trial_qubits, std::vector<int> evaluation_qubits) {
    auto qpe = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("PhaseEstimation"));
    assert(qpe);
    const bool expand_ok =
        qpe->expand({{"unitary", oracle.circuit_},
                     {"num_evaluation_qubits", num_evaluation_qubits},
                     {"trial_qubits", trial_qubits},
                     {"evaluation_qubits", evaluation_qubits}});
    circuit_->addInstructions(qpe->getInstructions());
  }

  /**
  * @brief Canonical Amplitude Estimation
  *
  * This method adds the canonical version of Quantum Amplitude Estimation (QAE) to the circuit.
  *
  * Given a quantum state split into a good subspace and a bad subspace
  *
  * |psi> = a|good> + b|bad> = A|0>
  *
  * the QAE sub-routine provides a k-bit approximation to the amplitude of the good subspace, a.
  *
  * QAE works by using the Grovers operator Q, which amplifies the amplitude of the
  * good subspace, as the unitary input to a Quantum Phase Estimation routine.
  *
  * @param state_prep The circuit A used to prepare the input state [CircuitBuilder]
  * @param grover_op The circuit for the Grovers operator Q for the good subspace [CircuitBuilder]
  * @param num_evaluation_qubits The number of bits k used to approximate the amplitude [int]
  * @param num_state_qubits The number of qubits acted on by the state_prep circuit A [int]
  * @param num_trial_qubits The number of qubits acted on by the grover_op circuit Q [int]
  * @param trial_qubits The indices of the qubits acted on by the grover_op circuit Q [vector of int]
  * @param evaluation_qubits The indices of the qubits used to store the approximate amplitude [vector of int]
  * @param no_state_prep If true, assumes the state |psi> is already prepared in the appropriate register [bool]
  */
  void CanonicalAmplitudeEstimation(CircuitBuilder &state_prep,
                                    CircuitBuilder &grover_op,
                                    int num_evaluation_qubits,
                                    int num_state_qubits, int num_trial_qubits,
                                    std::vector<int> trial_qubits,
                                    std::vector<int> evaluation_qubits, bool no_state_prep) {
    auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
    assert(ae);
    const bool expand_ok =
        ae->expand({{"state_preparation_circuit", state_prep.circuit_},
                    {"grover_op_circuit", grover_op.circuit_},
                    {"num_evaluation_qubits", num_evaluation_qubits},
                    {"num_state_qubits", num_state_qubits},
                    {"trial_qubits", trial_qubits},
                    {"evaluation_qubits", evaluation_qubits},
                    {"num_trial_qubits", num_trial_qubits},
                    {"no_state_prep", no_state_prep}});
    circuit_->addInstructions(ae->getInstructions());
  }

  /**
  * @brief Multi Controlled Unitary With Ancilla
  *
  * This method decomposes a multi-controlled unitary into Toffoli gates
  * and the unitary itself, with the use of ancilla qubits. With N control qubits
  * there should be N-1 ancilla. The resulting instructions are added to the circuit (AMCU gate).
  *
  * @param U The unitary operation [CircuitBuilder]
  * @param qubits_control The indices of the control qubits [vector of int]
  * @param qubits_ancilla The indices of the ancilla qubits [vector of int]
  */
  void MultiControlledUWithAncilla(CircuitBuilder &U,
                                   std::vector<int> qubits_control,
                                   std::vector<int> qubits_ancilla) {
    auto amcu = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("MultiControlledUWithAncilla"));
    assert(amcu);
    const bool expand_ok = amcu->expand({{"U", U.circuit_},
                                         {"qubits_control", qubits_control},
                                         {"qubits_ancilla", qubits_ancilla}});
    assert(expand_ok);
    circuit_->addInstructions(amcu->getInstructions());
  }

  /**
  * @brief Run Canonical Amplitude Estimation
  *
  * This method sets up and executes an instance of the canonical amplitude estimation circuit.
  *
  * @param state_prep The circuit A used to prepare the input state [CircuitBuilder]
  * @param grover_op The circuit for the Grovers operator Q for the good subspace [CircuitBuilder]
  * @param num_evaluation_qubits The number of bits k used to approximate the amplitude [int]
  * @param num_state_qubits The number of qubits acted on by the state_prep circuit A [int]
  * @param num_trial_qubits The number of qubits acted on by the grover_op circuit Q [int]
  * @param trial_qubits The indices of the qubits acted on by the grover_op circuit Q [vector of int]
  * @param evaluation_qubits The indices of the qubits used to store the approximate amplitude [vector of int]
  * @param acc_name The name of the accelerator used to execute the circuit [string]
  *
  * @return The output buffer of the execution
  */
  std::string RunCanonicalAmplitudeEstimation(
      CircuitBuilder &state_prep, CircuitBuilder &grover_op,
      int num_evaluation_qubits, int num_state_qubits, int num_trial_qubits,
      std::vector<int> trial_qubits, std::vector<int> evaluation_qubits,
      std::string acc_name) {
    auto acc = xacc::getAccelerator(acc_name);
    auto buffer = xacc::qalloc(num_evaluation_qubits + num_trial_qubits);
    auto ae_algo = xacc::getAlgorithm(
        "canonical-ae", {{"state_preparation_circuit", state_prep.circuit_},
                         {"grover_op_circuit", grover_op.circuit_},
                         {"num_evaluation_qubits", num_evaluation_qubits},
                         {"num_state_qubits", num_state_qubits},
                         {"trial_qubits", trial_qubits},
                         {"evaluation_qubits", evaluation_qubits},
                         {"num_trial_qubits", num_trial_qubits},
                         {"qpu", acc}});
    ae_algo->execute(buffer);
    return buffer->toString();
  }

  /**
  * @brief Run Canonical Amplitude Estimation with Oracle
  *
  * This method sets up and executes an instance of the canonical amplitude estimation circuit,
  * but instead of providing the grovers_op Q, we provide the oracle circuit $O$ which acts as
  *
  * O(|psi>) = O(a|good> + b|bad>) = -a|good> + b|bad>
  *
  * The Grovvers operator Q is then constructed within the method from O and the state_prep circuit A as
  *
  * Q = Adg*S0*A*O
  *
  * where S0 is an easily implementable rotation about the all 0 state |00...0>.
  *
  * @param state_prep The circuit A used to prepare the input state [CircuitBuilder]
  * @param oracle The oracle circuit O that marks the good subspace [CircuitBuilder]
  * @param num_evaluation_qubits The number of bits k used to approximate the amplitude [int]
  * @param num_state_qubits The number of qubits acted on by the state_prep circuit A [int]
  * @param num_trial_qubits The number of qubits acted on by the grover_op circuit Q [int]
  * @param evaluation_qubits The indices of the qubits used to store the approximate amplitude [vector of int]
  * @param trial_qubits The indices of the qubits acted on by the grover_op circuit Q [vector of int]
  * @param acc_name The name of the accelerator used to execute the circuit [string]
  *
  * @return The output buffer of the execution
  */
  std::string RunCanonicalAmplitudeEstimationWithOracle(
      CircuitBuilder &state_prep, CircuitBuilder &oracle,
      int num_evaluation_qubits, int num_state_qubits, int num_trial_qubits,
      std::vector<int> evaluation_qubits, std::vector<int> trial_qubits,
      std::string acc_name) {
    auto acc = xacc::getAccelerator(acc_name);
    auto buffer = xacc::qalloc(num_evaluation_qubits + num_trial_qubits);
    auto ae_algo = xacc::getAlgorithm(
        "canonical-ae", {{"state_preparation_circuit", state_prep.circuit_},
                         {"oracle", oracle.circuit_},
                         {"num_evaluation_qubits", num_evaluation_qubits},
                         {"num_state_qubits", num_state_qubits},
                         {"trial_qubits", trial_qubits},
                         {"evaluation_qubits", evaluation_qubits},
                         {"num_trial_qubits", num_trial_qubits},
                         {"qpu", acc}});

    ae_algo->execute(buffer);
    return buffer->toString();
  }

   /**
  * @brief Run Maximum-Likelihood Amplitude Estimation
  *
  * This method sets up and executes an instance of the maximum-likelihood amplitude estimation circuit.
  *
  * Given the state
  *
  * |psi> = a|good> + b|bad>
  *
  * MLQAE is an alternative to canonical QAE to find an estimate for the
  * amplitude of the good subspace, a. It works by performing several runs of amplitude
  * amplification with various iterations and recording the number of |good>
  * shots measured. Given this data, it finds the value of a that maximises the
  * likelihood function.
  *
  * @param state_prep The circuit A used to prepare the input state [CircuitBuilder]
  * @param oracle The oracle circuit O that marks the good subspace [CircuitBuilder]
  * @param is_in_good_subspace A function that, given a measured bitstring and potentially some other input value, returns a 1 if the measurement is in the good subspace and a 0 otherwise. [func(str, int) -> int]
  * @param score_qubits The indices of the qubits that determine whether the state is in the good or bad subspace [vector of int]
  * @param total_num_qubits The total number of qubits in the circuit [int]
  * @param num_runs The number of runs of amplitude amplification (~4-6 is usually sufficient)
  * @param shots The number of shots in each run [int]
  * @param acc_name The name of the accelerator used to execute the circuit [string]
  *
  * @return The output buffer of the execution
  */
  std::string RunMLAmplitudeEstimation(
      CircuitBuilder &state_prep, CircuitBuilder &oracle,
      std::function<int(std::string, int)> is_in_good_subspace,
      std::vector<int> score_qubits, int total_num_qubits, int num_runs,
      int shots, std::string acc_name) {
    auto buffer = xacc::qalloc(total_num_qubits);
    auto acc = xacc::getAccelerator(acc_name);
    auto ae_algo = xacc::getAlgorithm(
        "ML-ae", {{"state_preparation_circuit", state_prep.circuit_},
                  {"oracle_circuit", oracle.circuit_},
                  {"is_in_good_subspace", is_in_good_subspace},
                  {"score_qubits", score_qubits},
                  {"num_runs", num_runs},
                  {"shots", shots},
                  {"qpu", acc}});

    ae_algo->execute(buffer);
    return buffer->toString();
  }

  /**
  * @brief Amplitude Amplification
  *
  * This method adds a number of Grovers operators to the circuit.
  *
  * Grovers operators are used to amplify the amplitude of some desired
  * subspace of your quantum state. Given a state
  *
  * |psi> = a|good> + b|bad> = A|0>
  *
  * and an oracle unitary O that acts as
  *
  * O|psi> = -a|good> + b|bad>
  *
  * the Grovers operator that can be used to amplify the amplitude of the good state is
  *
  * Q = Adg*S0*A*O
  *
  * where S0 is an easily implemented reflection about the all zero state.
  *
  * @param oracle The oracle circuit O that marks the good subspace [CircuitBuilder]
  * @param state_prep The circuit A used to prepare the input state [CircuitBuilder]
  * @param power The number of Grovers operators to append to the circuit [int]
  */
  void AmplitudeAmplification(CircuitBuilder &oracle,
                              CircuitBuilder &state_prep, int power) {
    auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("AmplitudeAmplification"));
    assert(ae);
    const bool expand_ok =
        ae->expand({{"oracle", oracle.circuit_},
                    {"state_preparation", state_prep.circuit_},
                    {"power", power}});
    assert(expand_ok);
    circuit_->addInstructions(ae->getInstructions());
  }

  /**
  * @brief Q' Unitary
  *
  * This method adds a Q' unitary to the circuit.
  *
  * Q' is a unitary required for the quantum decoder algorithm.
  */
  void QPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
              int &nb_qubits_next_letter_probabilities,
              int &nb_qubits_next_letter) {
    auto qprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("QPrime"));
    // assert(qprime);
    // const bool expand_ok =
    qprime->expand({{"nb_qubits_ancilla_metric", nb_qubits_ancilla_metric},
                    {"nb_qubits_ancilla_letter", nb_qubits_ancilla_letter},
                    {"nb_qubits_next_letter_probabilities",
                     nb_qubits_next_letter_probabilities},
                    {"nb_qubits_next_letter", nb_qubits_next_letter}});
    // assert(expand_ok);
    circuit_->addInstructions(qprime->getInstructions());
  }

  /**
  * @brief U' Unitary
  *
  * This method adds a U' unitary to the circuit.
  *
  * U' is a unitary required for the quantum decoder algorithm.
  */
  void UPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
              int &nb_qubits_next_letter_probabilities,
              int &nb_qubits_next_letter) {
    auto uprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("UPrime"));
    // assert(qprime);
    // const bool expand_ok =
    uprime->expand({{"nb_qubits_ancilla_metric", nb_qubits_ancilla_metric},
                    {"nb_qubits_ancilla_letter", nb_qubits_ancilla_letter},
                    {"nb_qubits_next_letter_probabilities",
                     nb_qubits_next_letter_probabilities},
                    {"nb_qubits_next_letter", nb_qubits_next_letter}});
    // assert(expand_ok);
    circuit_->addInstructions(uprime->getInstructions());
  }

  /**
  * @brief W' Unitary
  *
  * This method adds a W' unitary to the circuit.
  *
  * W' is a unitary required for the quantum decoder algorithm.
  */
  void WPrime(int iteration, std::vector<int> qubits_next_metric,
              std::vector<int> qubits_next_letter, std::vector<std::vector<float>> probability_table,
              std::vector<int> qubits_init_null, int null_integer, bool use_ancilla,
              std::vector<int> qubits_ancilla) {
    auto wprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("WPrime"));
    // assert(wprime);
    // const bool expand_ok =
    wprime->expand({{"probability_table", probability_table},
                    {"iteration", iteration},
                    {"qubits_next_metric", qubits_next_metric},
                    {"qubits_next_letter", qubits_next_letter},
                    {"qubits_init_null", qubits_init_null},
                    {"null_integer", null_integer},
                    {"use_ancilla", use_ancilla},
                    {"ancilla_qubits", qubits_ancilla}});
    // assert(expand_ok);
    // std::cout << wprime->toString() << "circuit_builder\n";
    circuit_->addInstructions(wprime->getInstructions());
  }

  /**
  * @brief UQ' Unitary
  *
  * This method adds a UQ' unitary to the circuit.
  *
  * UQ' is a unitary required for the quantum decoder algorithm.
  */
  void UQPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
               int &nb_qubits_next_letter_probabilities,
               int &nb_qubits_next_letter) {
    auto uqprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("UQPrime"));
    // assert(uqprime);
    // const bool expand_ok =
    uqprime->expand({{"nb_qubits_ancilla_metric", nb_qubits_ancilla_metric},
                     {"nb_qubits_ancilla_letter", nb_qubits_ancilla_letter},
                     {"nb_qubits_next_letter_probabilities",
                      nb_qubits_next_letter_probabilities},
                     {"nb_qubits_next_letter", nb_qubits_next_letter}});
    // assert(expand_ok);
    circuit_->addInstructions(uqprime->getInstructions());
  }

  /**
  * @brief Ripple Carry Adder
  *
  * This method adds a ripple carry adder to the circuit.
  *
  * The ripple carry adder is an efficient in-line addition operation
  * with a carry-in bit:
  *
  * RippleAdd|c_in>|a>|b> -> |a> |a+b+c_in>
  *
  * @param a The qubit indices of the first register in the addition [vector of int]
  * @param b The qubit indices of the second register in the addition [vector of int]
  * @param carry_bit The index of the carry-in bit [int]
  */
  void RippleAdd(const std::vector<int> &a, const std::vector<int> &b,
                 int carry_bit) {
    auto adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("RippleCarryAdder"));
    adder->expand({{"adder_bits", a}, {"sum_bits", b}, {"c_in", carry_bit}});
    circuit_->addInstructions(adder->getInstructions());
  }

  /**
  * @brief Comparator as Oracle
  *
  * This method adds a quantum bit string comparator oracle to the circuit.
  *
  * The quantum bit string comparator is used to add a negative phase to any
  * trial state whose bit string value is greater than the state being compared to.
  * In this way it can be used as an oracle in a Grovers operator that amplifies
  * higher scoring strings. This may be useful in many search problems.
  *
  * @param BestScore The score we are comparing strings to [int]
  * @param num_scoring_qubits The number of qubits used to encode the scores [int]
  * @param trial_score_qubits The indices of the qubits encoding the trial states [vector of int]
  * @param flag_qubit The index of the flag qubit which acquires a negative phase whenever trial score > BestScore [int]
  * @param best_score_qubits The indices of the qubits encoding the BestScore value [vector of int]
  * @param ancilla_qubits The indices of the ancilla qubits required for the comparator circuit, if num_scoring_qubits = N we need 3N-1 ancilla [vector of int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  * @param controls_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = |1>) [vector of int]
  * @param controls_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = |0>) [vector of int]
  */
  void Comparator_as_Oracle(int BestScore, int num_scoring_qubits,
                            std::vector<int> trial_score_qubits, int flag_qubit,
                            std::vector<int> best_score_qubits,
                            std::vector<int> ancilla_qubits, bool is_LSB,
                            std::vector<int> controls_on, std::vector<int> controls_off) {
    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Comparator"));
    assert(comp);
    const bool expand_ok =
        comp->expand({{"BestScore", BestScore},
                      {"num_scoring_qubits", num_scoring_qubits},
                      {"trial_score_qubits", trial_score_qubits},
                      {"flag_qubit", flag_qubit},
                      {"best_score_qubits", best_score_qubits},
                      {"ancilla_qubits", ancilla_qubits},
                      {"as_oracle", true},
                      {"is_LSB", is_LSB},
                      {"controls_on", controls_on},
                      {"controls_off", controls_off}});
    circuit_->addInstructions(comp->getInstructions());
  }

  /**
  * @brief Comparator
  *
  * This method adds a quantum bit string comparator to the circuit.
  *
  * The quantum bit string comparator is used to compare the values of two bit string.
  * If the trial score is greater than the best score, the flag qubit is flipped |0>->|1>
  *
  * @param BestScore The score we are comparing strings to [int]
  * @param num_scoring_qubits The number of qubits used to encode the scores [int]
  * @param trial_score_qubits The indices of the qubits encoding the trial states [vector of int]
  * @param flag_qubit The index of the flag qubit which is flipped whenever trial score > BestScore [int]
  * @param best_score_qubits The indices of the qubits encoding the BestScore value [vector of int]
  * @param ancilla_qubits The indices of the ancilla qubits required for the comparator circuit, if num_scoring_qubits = N we need 3N-1 ancilla [vector of int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  * @param controls_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = |1>) [vector of int]
  * @param controls_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = |0>) [vector of int]
  */
  void Comparator(int BestScore, int num_scoring_qubits,
                  std::vector<int> trial_score_qubits, int flag_qubit,
                  std::vector<int> best_score_qubits,
                  std::vector<int> ancilla_qubits, bool is_LSB,
                  std::vector<int> controls_on, std::vector<int> controls_off) {
    auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Comparator"));
    assert(comp);
    const bool expand_ok =
        comp->expand({{"BestScore", BestScore},
                      {"num_scoring_qubits", num_scoring_qubits},
                      {"trial_score_qubits", trial_score_qubits},
                      {"flag_qubit", flag_qubit},
                      {"best_score_qubits", best_score_qubits},
                      {"ancilla_qubits", ancilla_qubits},
                      {"is_LSB", is_LSB},
                      {"controls_on", controls_on},
                      {"controls_off", controls_off}});
    circuit_->addInstructions(comp->getInstructions());
  }

  /**
  * @brief Efficient Encoding
  *
  * This method adds an efficient encoding routine to the circuit.
  *
  * Given a lookup function f that assigns a score to each binary string,
  * we encode the state
  *
  * |psi> = |x> |f(x)> + |y> |f(y)> + ...
  *
  * where the superposition is over all possible bitstrings. Rather than encoding states in order |000>, |001>, |010>...etc. we cut down
  * on the amount of X gates required by instead following the Gray code ordering of states.
  *
  * This module can optionally also flag strings of a certain value.
  *
  * @param scoring_function A function that inputs the integer value of a binary string and outputs its score [func(int) -> int]
  * @param num_state_qubits The number of qubits encoding the strings [int]
  * @param num_scoring_qubits The number of qubits encoding the scores [int]
  * @param state_qubits The indices of the qubits encoding the strings [vector of int]
  * @param scoring_qubits The indices of the qubits encoding the scores [vector of int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  * @param use_ancilla Indicates that ancilla qubits can be used to decompose MCX gates [bool]
  * @param qubits_init_flag The indices of any flag qubits [vector of int]
  * @param flag_integer The integer value of binary strings that should be flagged [int]
  */
  void EfficientEncoding(std::function<int(int)> scoring_function,
                         int num_state_qubits, int num_scoring_qubits,
                         std::vector<int> state_qubits,
                         std::vector<int> scoring_qubits, bool is_LSB, bool use_ancilla,
                         std::vector<int> qubits_init_flag, int flag_integer) {
    auto ee = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("EfficientEncoding"));
    assert(ee);
    const bool expand_ok =
        ee->expand({{"scoring_function", scoring_function},
                    {"num_state_qubits", num_state_qubits},
                    {"num_scoring_qubits", num_scoring_qubits},
                    {"state_qubits", state_qubits},
                    {"scoring_qubits", scoring_qubits},
                    {"is_LSB", is_LSB},
                    {"use_ancilla", use_ancilla},
                    {"qubits_init_flag", qubits_init_flag},
                    {"flag_integer", flag_integer}});
    circuit_->addInstructions(ee->getInstructions());
  }

  /**
  * @brief Equality Checker
  *
  * This method adds an equality checker to the circuit.
  *
  * Given two input bitstrings |a> and |b> the equality checker is
  * used to flip a flag qubit |0> \to |1> whenever a=b.
  *
  * @param qubits_a the indices of the qubits encoding a [vector of int]
  * @param qubits_b the indices of the qubits encoding b [vector of int]
  * @param flag the index of the flag qubit that gets flipped whenever a=b [int]
  * @param use_ancilla Indicates that ancilla qubits can be used to decompose MCX gates [bool]
  * @param qubits_ancilla The indices of the qubits to be used as ancilla qubits if use_ancilla=true [vector of int]
  * @param controls_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = |1>) [vector of int]
  * @param controls_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = |0>) [vector of int]
  */
  void EqualityChecker(std::vector<int> qubits_a, std::vector<int> qubits_b,
                       int flag, bool use_ancilla,
                       std::vector<int> qubits_ancilla,
                       std::vector<int> controls_on,
                       std::vector<int> controls_off) {
    auto ec = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("EqualityChecker"));
    assert(ec);
    const bool expand_ok = ec->expand({{"qubits_a", qubits_a},
                                       {"qubits_b", qubits_b},
                                       {"flag", flag},
                                       {"use_ancilla", use_ancilla},
                                       {"qubits_ancilla", qubits_ancilla},
                                       {"controls_on", controls_on},
                                       {"controls_off", controls_off}});
    circuit_->addInstructions(ec->getInstructions());
  }

  /**
  * @brief Controlled SWAP
  *
  * This method adds a controlled SWAP to the circuit.
  *
  * Performs a SWAP operation on |a> and |b> if an only if the controls are satisfied.
  *
  * @param qubits_a the indices of the qubits encoding a [vector of int]
  * @param qubits_b the indices of the qubits encoding b [vector of int]
  * @param flags_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = |1>) [vector of int]
  * @param flags_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = |0>) [vector of int]
  */
  void ControlledSwap(std::vector<int> qubits_a, std::vector<int> qubits_b,
                       std::vector<int> flags_on, std::vector<int> flags_off) {
    auto cs = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ControlledSwap"));
    assert(cs);
    const bool expand_ok = cs->expand({{"qubits_a", qubits_a},
                                       {"qubits_b", qubits_b},
                                       {"flags_on", flags_on},
                                       {"flags_off", flags_off}});
    circuit_->addInstructions(cs->getInstructions());
  }

  /**
  * @brief Controlled Addition
  *
  * This method adds a controlled ripple carry adder to the circuit.
  *
  * Performs a RippleAdd operation on |adder_bits> and |sum_bits> if an only if the controls are satisfied.
  *
  * @param qubits_adder the indices of the qubits encoding adder_bits [vector of int]
  * @param qubits_sum the indices of the qubits encoding sum_bits [vector of int]
  * @param c_in the index of the carry-in bit [int]
  * @param flags_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = |1>) [vector of int]
  * @param flags_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = |0>) [vector of int]
  * @param no_overflow Indicates that the sum |adder+sum> can be encoded on the same number of qubits as |sum> without overflowing [bool]
  */
  void ControlledAddition(std::vector<int> qubits_adder, std::vector<int> qubits_sum, int c_in,
                       std::vector<int> flags_on, std::vector<int> flags_off, bool no_overflow) {
    auto ca = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ControlledAddition"));
    assert(ca);
    const bool expand_ok = ca->expand({{"qubits_adder", qubits_adder},
                                       {"qubits_sum", qubits_sum},
                                       {"c_in", c_in},
                                       {"flags_on", flags_on},
                                       {"flags_off", flags_off},
                                       {"no_overflow", no_overflow}});
    circuit_->addInstructions(ca->getInstructions());
  }

  /**
  * @brief Generalised MCX
  *
  * This method adds a generalised MCX gate to the circuit.
  *
  * By generalised MCX we mean that we allow the control qubits to be
  * conditional on being off or conditional on being on.
  *
  * @param target The index of the target qubit [int]
  * @param controls_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = |1>) [vector of int]
  * @param controls_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = |0>) [vector of int]
  */
  void GeneralisedMCX(int target, std::vector<int> controls_on,
                       std::vector<int> controls_off) {
    auto gmcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("GeneralisedMCX"));
    assert(gmcx);
    const bool expand_ok = gmcx->expand({{"target", target},
                                        {"controls_on", controls_on},
                                        {"controls_off", controls_off}});
    circuit_->addInstructions(gmcx->getInstructions());
  }

  /**
  * @brief Compare Beam Oracle
  *
  * This method adds a compare beam oracle to the circuit.
  *
  * This method is required for the quantum decoder algorithm.
  */
  void CompareBeamOracle(int q0, int q1, int q2, std::vector<int> FA,
                       std::vector<int> FB, std::vector<int> SA, std::vector<int> SB, bool simplified) {
    auto cbo = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("CompareBeamOracle"));
    assert(cbo);
    const bool expand_ok = cbo->expand({{"q0", q0},
                                        {"q1", q1},
                                        {"q2", q2},
                                        {"FA", FA},
                                        {"FB", FB},
                                        {"SA", SA},
                                        {"SB", SB},
                                        {"simplified", simplified}});
    circuit_->addInstructions(cbo->getInstructions());
  }

  void SuperpositionAdder(int q0, int q1, int q2,
                          std::vector<int> qubits_flags, std::vector<int> qubits_string,
                          std::vector<int> qubits_metric,
                          CircuitBuilder &ae_state_prep_circ,
                          std::vector<int> qubits_ancilla,
                          std::vector<int> qubits_beam_metric) {
    auto sa = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("SuperpositionAdder"));
    assert(sa);
    const bool expand_ok = sa->expand({
      {"q0", q0}, {"q1", q1}, {"q2", q2},
      {"qubits_flags", qubits_flags},
      {"qubits_string", qubits_string},
      {"qubits_metric", qubits_metric},
      {"ae_state_prep_circ", ae_state_prep_circ.circuit_},
      {"qubits_ancilla", qubits_ancilla},
      {"qubits_beam_metric", qubits_beam_metric}});
    circuit_->addInstructions(sa->getInstructions());
  }

  /**
  * @brief Inverse Circuit
  *
  * This method adds the inverse of a circuit to the current circuit.
  *
  * Given some collection of unitary operations,
  *
  * U = U_N*U_{N-1}...U_2*U_1
  *
  * this method appends the inverse to the circuit:
  *
  * U^{-1} = U_1dg U_2dg...U_{N-1}dg U_Ndg
  *
  * This may be useful for un-computing ancilla or for constructing Grovers operators.
  *
  * @param circ The circuit whose inverse we want to add to the current circuit [CircuitBuilder]
  */
  void InverseCircuit(CircuitBuilder &circ) {
    auto is = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("InverseCircuit"));
    const bool expand_ok = is->expand({{"circ", circ.circuit_}});
    circuit_->addInstructions(is->getInstructions());
  }

  /**
  * @brief Subtraction
  *
  * This method adds a subtraction to the circuit.
  *
  * Performs the mapping:
  *
  * |a>|b> -> |a-b>|b>
  *
  * assuming a>b.
  *
  * @param qubits_larger the indices of the qubits encoding the larger value [vector of int]
  * @param qubits_smaller the indices of the qubits encoding the smaller value [vector of int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  * @param qubit_ancilla the index of the required ancilla [vector of int]
  */
  void Subtraction(std::vector<int> qubits_larger,
                   std::vector<int> qubits_smaller, bool is_LSB, int qubit_ancilla) {
    auto s = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Subtraction"));
    const bool expand_ok = s->expand({{"qubits_larger", qubits_larger},
                                      {"qubits_smaller", qubits_smaller},
                                      {"qubit_ancilla", qubit_ancilla},
                                      {"is_LSB", is_LSB}});
    circuit_->addInstructions(s->getInstructions());
  }
  /**
  * @brief Controlled Subtraction
  *
  * This method adds a controlled subtraction to the circuit.
  *
  * Performs a subtraction operation on |a> and |b> if an only if the controls are satisfied.
  *
  * @param qubits_larger the indices of the qubits encoding the larger value [vector of int]
  * @param qubits_smaller the indices of the qubits encoding the smaller value [vector of int]
  * @param controls_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = |1>) [vector of int]
  * @param controls_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = |0>) [vector of int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  * @param qubit_ancilla the index of the required ancilla [vector of int]
  */
  void ControlledSubtraction(std::vector<int> qubits_larger,
                             std::vector<int> qubits_smaller,
                             std::vector<int> controls_on,
                             std::vector<int> controls_off, bool is_LSB, int qubit_ancilla) {
    auto cs = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ControlledSubtraction"));
    const bool expand_ok = cs->expand({{"qubits_larger", qubits_larger},
                                       {"qubits_smaller", qubits_smaller},
                                       {"is_LSB", is_LSB},
                                       {"qubit_ancilla", qubit_ancilla},
                                       {"controls_on", controls_on},
                                       {"controls_off", controls_off}});
    circuit_->addInstructions(cs->getInstructions());
  }

  /**
  * @brief Proper Fraction Division
  *
  * This method adds a proper fraction division to the circuit.
  *
  * Performs the mapping:
  *
  * |num>|denom>|0> -> |num>|denom>|num/denom>
  *
  * assuming denom > num
  *
  * @param qubits_numerator the indices of the qubits encoding the numerator [vector of int]
  * @param qubits_denominator the indices of the qubits encoding the denominator [vector of int]
  * @param qubits_fraction the indices of the qubits that will ecode the division result [vector of int]
  * @param qubit_ancilla the index of the required ancilla [vector of int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  */
  void ProperFractionDivision(std::vector<int> qubits_numerator,
                              std::vector<int> qubits_denominator,
                              std::vector<int> qubits_fraction,
                              std::vector<int> qubits_ancilla, bool is_LSB) {
    auto PFD = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ProperFractionDivision"));
    const bool expand_ok =
        PFD->expand({{"qubits_numerator", qubits_numerator},
                     {"qubits_denominator", qubits_denominator},
                     {"qubits_fraction", qubits_fraction},
                     {"qubits_ancilla", qubits_ancilla},
                     {"is_LSB", is_LSB}});
    circuit_->addInstructions(PFD->getInstructions());
  }

  /**
  * @brief Controlled Proper Fraction Division
  *
  * This method adds a controlled proper fraction division to the circuit.
  *
  * Performs a PFD operation on |a> and |b> if an only if the controls are satisfied.
  *
  * @param qubits_numerator the indices of the qubits encoding the numerator [vector of int]
  * @param qubits_denominator the indices of the qubits encoding the denominator [vector of int]
  * @param qubits_fraction the indices of the qubits that will ecode the division result [vector of int]
  * @param qubit_ancilla the index of the required ancilla [vector of int]
  * @param controls_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = |1>) [vector of int]
  * @param controls_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = |0>) [vector of int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  */
  void ControlledProperFractionDivision(std::vector<int> qubits_numerator,
                                        std::vector<int> qubits_denominator,
                                        std::vector<int> qubits_fraction,
                                        std::vector<int> qubits_ancilla,
                                        std::vector<int> controls_on,
                                        std::vector<int> controls_off,
                                        bool is_LSB) {
    auto cPFD = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>(
            "ControlledProperFractionDivision"));
    const bool expand_ok =
        cPFD->expand({{"qubits_numerator", qubits_numerator},
                      {"qubits_denominator", qubits_denominator},
                      {"qubits_fraction", qubits_fraction},
                      {"qubits_ancilla", qubits_ancilla},
                      {"controls_on", controls_on},
                      {"controls_off", controls_off},
                      {"is_LSB", is_LSB}});
    circuit_->addInstructions(cPFD->getInstructions());
  }

  /**
  * @brief Compare Greater Than
  *
  * This method adds a > comparator to the circuit.
  *
  * Given two binary strings |a> and |b>, this comparator flips a flag qubit whenever a>b.
  * This method uses far less ancilla than the more general comparator method provided.
  *
  * @param qubits_a The indices of the qubits encoding a [vector of int]
  * @param qubits_b The indices of the qubits encoding b [vector of int]
  * @param qubit_flag The index of the flag qubit that is flipped whenever a>b [int]
  * @param qubit_ancilla The index of the single ancilla qubit required [int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  */
  void CompareGT(std::vector<int> qubits_a,
                              std::vector<int> qubits_b,
                              int qubit_flag,
                              int qubit_ancilla, bool is_LSB) {
    auto cgt = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("CompareGT"));
    const bool expand_ok =
        cgt->expand({{"qubits_a", qubits_a},
                     {"qubits_b", qubits_b},
                     {"qubit_flag", qubit_flag},
                     {"qubit_ancilla", qubit_ancilla},
                     {"is_LSB", is_LSB}});
    circuit_->addInstructions(cgt->getInstructions());
  }

  /**
  * @brief Multiplication
  *
  * This method adds a Multiplication to the circuit.
  *
  * Performs the mapping:
  *
  * |a>|b>|0> -> |a>|b>|ab>
  *
  * @param qubits_a the indices of the qubits encoding a [vector of int]
  * @param qubits_b the indices of the qubits encoding b [vector of int]
  * @param qubits_result the indices of the qubits that will ecode the multiplication result [vector of int]
  * @param qubit_ancilla the index of the single required ancilla [int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  */
  void Multiplication(std::vector<int> qubits_a, std::vector<int> qubits_b,
                      std::vector<int> qubits_result, int qubit_ancilla, bool is_LSB) {
    auto multiplication = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Multiplication"));
    assert(multiplication);
    const bool expand_ok = multiplication->expand({
        {"qubit_ancilla", qubit_ancilla},
        {"qubits_a", qubits_a},
        {"qubits_b", qubits_b},
        {"qubits_result", qubits_result},
        {"is_LSB", is_LSB}});
    circuit_->addInstructions(multiplication->getInstructions());
  }

  /**
  * @brief Controlled Multiplication
  *
  * This method adds a controlled Multiplication to the circuit.
  *
  * Performs a Multiplication operation on |a> and |b> if an only if the controls are satisfied.
  *
  * @param qubits_a the indices of the qubits encoding a [vector of int]
  * @param qubits_b the indices of the qubits encoding b [vector of int]
  * @param qubits_result the indices of the qubits that will ecode the multiplication result [vector of int]
  * @param qubit_ancilla the index of the single required ancilla [int]
  * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
  * @param controls_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = |1>) [vector of int]
  * @param controls_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = |0>) [vector of int]
  */
  void ControlledMultiplication(std::vector<int> qubits_a, std::vector<int> qubits_b,
                       std::vector<int> qubits_result, int qubit_ancilla, bool is_LSB, std::vector<int> controls_on, std::vector<int> controls_off) {
    auto multiplication = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ControlledMultiplication"));
    assert(multiplication);
    const bool expand_ok = multiplication->expand({
        {"qubit_ancilla", qubit_ancilla},
        {"qubits_a", qubits_a},
        {"qubits_b", qubits_b},
        {"qubits_result", qubits_result},
        {"is_LSB", is_LSB},
        {"controls_on", controls_on},
        {"controls_off", controls_off}});
    circuit_->addInstructions(multiplication->getInstructions());
  }

  /**
  * @brief Exponential Search
  *
  * This method sets up and executes the exponential search routine.
  *
  * Exponential search is a way to perform amplitude estimation when the size
  * of the "good" subspace is unknown (so the number of Grovers operators to use is unknown).
  *
  * We implement three variants:
  * - canonical exponential search is a specific "guess and check" method
  * - MLQAE exponential search uses MLQAE to first estimate the size of the good subspace then perform regular amplitude estimation with the appropriate number of Grovers operators
  * - CQAE exponential search uses canonical QAE to first estimate the size of the good subspace then perform regular amplitude estimation with the appropriate number of Grovers operators
  *
  * Exponential search here has been designed for use in the quantum decoder algorithm, so many inputs may not
  * be required for general use.
  *
  * @param method indicates which method to use. Options are "canonical", "MLQAE", "CQAE" [string]
  * @param oracle_gen a function which produces the oracle circuit that marks the good subspace [OracleFuncCType]
  * @param state_prep_gen a function which produces the state prep circuit [StatePrepFuncCType]
  * @param f_score a function that returns a 1 if the input binary string has value greater than the current best score and 0 otherwise [func(int)->int]
  * @param best_score the current best score [int]
  * @param qubits_string the indices of the qubits encoding the strings [vector of int]
  * @param qubits_metric the indices of the qubits encoding the string scores [vector of int]
  * @param qubits_next_letter the indices of some ancilla qubits labelled for the quantum decoder [vector of int]
  * @param qubits_next_metric the indices of some ancilla qubits labelled for the quantum decoder [vector of int]
  * @param qubit_flag the index of the qubit that is flagged by the oracle whenever a trial score is greater than the current best score [int]
  * @param qubits_best_score the indices of the qubits used to encode the current best score [vector of int]
  * @param qubits_ancilla_oracle the indices of any ancilla qubits required by the oracle [vector of int]
  * @param qubits_ancilla_adder the indices of ancilla qubits required by the ripple carry adder (required by decoder) [vector of int]
  * @param total_metric the indices of the qubits encoding the string scores after any required pre-processing of qubits_metric (required by decoder) [vector of int]
  * @param CQAE_num_evaluation_qubits if using CQAE method, specifies the number of evaluation qubits used in the QAE routine [int]
  * @param MLQAE_is_in_good_subspace if using MLQAE method, the function that indicates whether a measurement is in the good subspace [func(str,int)->int]
  * @param MLQAE_num_runs if using MLQAE method, the number of runs [int]
  * @param MLQAE_num_shots if using MLQAE method, the number of shots [int]
  * @param acc_name the name of the accelerator used to execute the algorithm [string]
  *
  * @return a better score if found, otherwise returns the current best score
  */
  int ExponentialSearch(
      std::string method, StatePrepFuncCType state_prep_circ, OracleFuncCType oracle_func,
      int best_score, std::function<int(int)> f_score, int total_num_qubits,
      std::vector<int> qubits_string, std::vector<int> total_metric,
      std::string acc_name) {
    auto acc = xacc::getAccelerator(acc_name);
    auto exp_search_algo = xacc::getAlgorithm(
      "exponential-search", {{"method", method},
                             {"state_preparation_circuit", state_prep_circ},
                             {"oracle_circuit", oracle_func},
                             {"best_score", best_score},
                             {"f_score", f_score},
                             {"total_num_qubits", total_num_qubits},
                             {"qubits_string", qubits_string},
                             {"total_metric", total_metric},
                             {"qpu", acc}});
    auto buffer = xacc::qalloc(total_num_qubits);
    exp_search_algo->execute(buffer);
    auto info = buffer->getInformation();
    if (info.find("best-score") != info.end()) {
      return info.at("best-score").as<int>();
    }
    return best_score;
  }
};
} // namespace qb

// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

#include <CompositeInstruction.hpp>
#include <IRProvider.hpp>
#include <InstructionIterator.hpp>
#include <Accelerator.hpp>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <Circuit.hpp>
#include <GateModifier.hpp>
#include <assert.h>
#include <string>

namespace qristal {

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
  * It can build circuits that are executed using the session object.
  *
  * We also provide high-level methods to construct quantum circuits
  * for commonly-used quantum algorithms, such as QFT and amplitude amplification
  */
  class CircuitBuilder {

    protected:
      /// The registry used to fetch quantum gates from XACC
      std::shared_ptr<xacc::IRProvider> gate_provider_;
      /// The circuit that is built using the gate API
      std::shared_ptr<xacc::CompositeInstruction> circuit_;
      /// The number of qubits in the circuit
      size_t num_qubits_;
      /// The free parameters in the circuit
      std::vector<std::string> free_params_;
      /// Whether the circuit is parametrized
      bool is_parametrized_{false};
      /// Process a gate and its parameter names, then add the gate to the circuit
      virtual void add_gate_with_free_parameters(std::string gate_name, std::vector<size_t> qubits,
                             std::vector<std::string> param_names);
      /// Add the parameters of an instruction to the list of circuit params
      void add_instruction_params_to_list(std::shared_ptr<xacc::Instruction> inst);

    public:

      /// @private
      static const char *help_execute_;

      /**
      * @brief Constructor
      *
      * A constructor for the CircuitBuilder class.
      * Creates an empty circuit.
      */
      CircuitBuilder();

      /**
      * @brief Constructor
      *
      * A constructor for the CircuitBuilder class.
      * Creates a circuit from a specified list of instructions.
      *
      * @param composite A pointer to a xacc::CompositeInstruction object [shared ptr]
      * @param copy_nodes If true, child nodes (instructions) of the CompositeInstruction will be copied over.
      * Otherwise, the input composite will become the root node of this CircuitBuilder.
      * Default to true.
      */
      CircuitBuilder(std::shared_ptr<xacc::CompositeInstruction> &composite,
                     bool copy_nodes = true);

      /// @brief Return the number of qubits in the circuit.
      const std::size_t num_qubits() {
        num_qubits_ = circuit_->nPhysicalBits();
        return num_qubits_;
      }

      /**
       * @brief Copy constructor
       *
       * Creates a deep copy of another CircuitBuilder, including its circuit,
       * variables, and configuration.
       *
       * @param other The CircuitBuilder to copy.
       */
      CircuitBuilder(const CircuitBuilder& other);

      /**
       * @brief Create a deep copy of this CircuitBuilder.
       *
       * @return A new CircuitBuilder instance with copied data.
       */
      CircuitBuilder copy() const {
        return CircuitBuilder(*this);
      }

      /**
       * @brief Get the names of the free parameters.
       *
       * @return Names of the free parameters as a vector of strings
       */
      const std::vector<std::string> &get_free_params() const { return free_params_; }

      /**
       * @brief Get the number of free parameters.
       *
       * @return The number of free parameters in this circuit.
       */
      const std::size_t num_free_params() const { return free_params_.size(); }

      /**
       * @brief Helper function to create a vector of parameters from a map for input to the `session` object.
       *
       * @param param_map the map of parameters (e.g. {"alpha": 0.5, "beta": 0.3})
       * @return A vector of parameters values ordered by first appearance of the parameter in the circuit.
       */
      const std::vector<double> param_map_to_vec(const std::map<std::string, double> &param_map) const;

      /**
       * @brief Get the parametrization flag for this circuit.
       *
       * @return Whether or not the circuit is parametrized (i.e. has >0 free parameters)
       */
      const bool is_parametrized() const { return is_parametrized_; }

      /**
      * @brief return the list of instructions comprising the circuit
      *
      * @return A pointer to the xacc::CompositeInstruction (list of instructions) that defines the circuit.
      */
      std::shared_ptr<xacc::CompositeInstruction> get() const { return circuit_; }

      /**
      * @brief print the list of instructions comprising the circuit
      */
      void print() const { std::cout << circuit_->toString() << std::endl; }

      /**
      * @brief append another CircuitBuilder object to this one
      *
      * @param other the circuit to be appended [CircuitBuilder]
      */
      void append(const CircuitBuilder &other);

      // Gates:
      /**
      * @brief Hadamard gate
      *
      * This method adds a Hadamard (H) gate to the circuit.
      *
      * The H gate is defined by its action on the basis states
      *
      * \f[
      * H\ket{0} \rightarrow \ket{+}
      * \\
      * H\ket{1} \rightarrow \ket{-}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      */
      void H(size_t idx);

      /**
      * @brief Pauli-X gate
      *
      * This method adds a Pauli-X (X) gate to the circuit.
      *
      * The X gate is defined by its action on the basis states
      *
      * \f[
      * X\ket{0} \rightarrow \ket{1}
      * \\
      * X\ket{1} \rightarrow \ket{0}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      */
      void X(size_t idx);

      /**
      * @brief Pauli-Y gate
      *
      * This method adds a Pauli-Y (Y) gate to the circuit.
      *
      * The Y gate is defined by its action on the basis states
      *
      * \f[
      * Y\ket{0} \rightarrow -i\ket{1}
      * \\
      * Y\ket{1} \rightarrow i\ket{0}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      */
      void Y(size_t idx);

      /**
      * @brief Pauli-Z gate
      *
      * This method adds a Pauli-Z (Z) gate to the circuit.
      *
      * The Z gate is defined by its action on the basis states
      *
      * \f[
      * Z\ket{0} \rightarrow \ket{0}
      * \\
      * Z\ket{1} \rightarrow -\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      */
      void Z(size_t idx);

      /**
      * @brief T gate
      *
      * This method adds a T gate to the circuit.
      *
      * The T gate is defined by its action on the basis states
      *
      * \f[
      * T\ket{0} \rightarrow \ket{0}
      * \\
      * T\ket{1} \rightarrow e^{i\pi/4}\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      */
      void T(size_t idx);

      /**
      * @brief S gate
      *
      * This method adds an S gate to the circuit.
      *
      * The S gate is defined by its action on the basis states
      *
      * \f[
      * S\ket{0} \rightarrow \ket{0}
      * \\
      * S\ket{1} \rightarrow i\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      */
      void S(size_t idx);

      /**
      * @brief Tdg gate
      *
      * This method adds an inverse of the T gate (Tdg) to the circuit.
      *
      * The Tdg gate is defined by its action on the basis states
      *
      * \f[
      * Tdg\ket{0} \rightarrow \ket{0}
      * \\
      * Tdg\ket{1} \rightarrow e^{-i\pi/4}\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      */
      void Tdg(size_t idx);

      /**
      * @brief Sdg gate
      *
      * This method adds an inverse of the S gate (Sdg) to the circuit.

      * The Sdg gate is defined by its action on the basis states
      *
      * \f[
      * Sdg\ket{0} \rightarrow \ket{0}
      * \\
      * Sdg\ket{1} \rightarrow -i\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      */
      void Sdg(size_t idx);

      /**
      * @brief RX gate
      *
      * This method adds an x-axis rotation (RX) gate to the circuit.
      *
      * The RX gate is defined by its action on the basis states
      *
      * \f[
      * RX(\theta)\ket{0} \rightarrow \cos(\theta/2)\ket{0} - i\sin(\theta/2)\ket{1}
      * \\
      * RX(\theta)\ket{1} \rightarrow -i\sin(\theta/2)\ket{0} + \cos(\theta/2)\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      * @param theta the angle of rotation about the x-axis [double]
      */
      void RX(size_t idx, double theta);

      /**
      * @overload
      *
      * This overloaded method adds an x-axis rotation (RX) gate with a free
      * parameter to the circuit.
      *
      * @param idx  the index of the qubit being acted on [size_t]
      * @param param_name the name of the free parameter [std::string]
      */
      void RX(size_t idx, std::string param_name);

      /**
      * @brief RY gate
      *
      * This method adds a y-axis rotation (RY) gate to the circuit.
      *
      * The RY gate is defined by its action on the basis states
      *
      * \f[
      * RY(\theta)\ket{0} \rightarrow \cos(\theta/2)\ket{0} + \sin(\theta/2)\ket{1}
      * \\
      * RY(\theta)\ket{1} \rightarrow -\sin(\theta/2)\ket{0} + \cos(\theta/2)\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      * @param theta the angle of rotation about the y-axis [double]
      */
      void RY(size_t idx, double theta);
      /**
      * @overload
      *
      * This overloaded method adds an y-axis rotation (RY) gate with a free
      * parameter to the circuit.
      *
      * @param idx  the index of the qubit being acted on [size_t]
      * @param param_name the name of the free parameter [std::string]
      */
      void RY(size_t idx, std::string param_name);

      /**
      * @brief RZ gate
      *
      * This method adds a z-axis rotation (RZ) gate to the circuit.
      *
      * The RZ gate is defined by its action on the basis states
      *
      * \f[
      * RZ(\theta)\ket{0} \rightarrow e^{-i\theta/2}\ket{0}
      * \\
      * RZ(\theta)\ket{1} \rightarrow e^{i\theta/2}\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      * @param theta the angle of rotation about the z-axis [double]
      */
      void RZ(size_t idx, double theta);

      /**
      * @overload
      *
      * This overloaded method adds an z-axis rotation (RZ) gate with a free
      * parameter to the circuit.
      * The name of the free parameter must be passed to the method.
      *
      * @param idx  the index of the qubit being acted on [size_t]
      * @param param_name the name of the free parameter [std::string]
      */
      void RZ(size_t idx, std::string param_name);

      /**
      * @brief U1 gate
      *
      * This method adds a phase (U1) gate to the circuit.
      *
      * The U1 gate is defined by its action on the basis states
      *
      * \f[
      * U1(\theta)\ket{0} \rightarrow \ket{0}
      * \\
      * U1(\theta)\ket{1} \rightarrow e^{i\theta}\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      * @param theta the value of the phase [double]
      */
      void U1(size_t idx, double theta);

      /**
      * @overload
      *
      * This overloaded method adds a parametrized phase (U1) gate with a free
      * parameter to the circuit.
      * The name of the free parameter must be passed to the method.
      *
      * @param idx  the index of the qubit being acted on [size_t]
      * @param param_name the name of the free parameter [std::string]
      */
      void U1(size_t idx, std::string param_name);

      /**
      * @brief U3 gate
      *
      * This method adds an arbitrary single qubit gate to the circuit.
      *
      * The U3 gate is defined by its action on the basis states
      *
      * \f[
      * U3(\theta,\phi,\lambda)\ket{0} \rightarrow \cos(\theta/2)\ket{0} + e^{i\phi}\sin(\theta/2)\ket{1}
      * \\
      * U3(\theta,\phi,\lambda)\ket{1} \rightarrow -e^{i\lambda}\sin(\theta/2)\ket{0} + e^{i(\phi+\lambda)}\cos(\theta/2)\ket{1}
      * \f]
      *
      * @param idx the index of the qubit being acted on [size_t]
      * @param theta the first rotation parameter [double]
      * @param phi the second rotation parameter [double]
      * @param lambda the third rotation parameter [double]
      */
      void U3(size_t idx, double theta, double phi, double lambda);

      /**
      * @overload
      *
      * This overloaded method adds a multi-parametrized 1-qubit unitary (U) gate
      * with 3 free parameters to the circuit.
      * The names of the free parameters must be passed to the method.
      *
      * @param idx the index of the qubit being acted on [size_t]
      * @param param1_name the name of the first free parameter [std::string]
      * @param param2_name the name of the second free parameter [std::string]
      * @param param3_name the name of the third free parameter [std::string]
      */
      void U3(size_t idx, std::string param1_name, std::string param2_name, std::string param3_name);

      /**
      * @brief CNOT gate
      *
      * This method adds a controlled-X (CNOT) gate to the circuit.
      *
      * The CNOT gate performs an X gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * CNOT\ket{ab} \rightarrow \ket{a}X^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx the index of the control qubit [size_t]
      * @param target_idx the index of the target qubit [size_t]
      */
      void CNOT(size_t ctrl_idx, size_t target_idx);

      /**
      * @brief MCX gate
      *
      * This method adds a multi-controlled X (MCX) gate to the circuit.
      *
      * The MCX gate performs an X gate on the target qubit
      * conditional on all control qubits being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * MCX\ket{c_1...c_Nt} \rightarrow \ket{c_1...c_N} X^{c_1...c_N} \ket{t}
      *
      * \f]
      *
      * @param ctrl_inds the indices of the control qubits [vector of int]
      * @param target_idx the index of the target qubit [size_t]
      */
      void MCX(const std::vector<int> &ctrl_inds, size_t target_idx);

      /**
      * @brief CU gate
      *
      * This method adds a controlled version of an arbitrary unitary (CU) to the circuit.
      *
      * The CU gate implements the U gate on the target qubits
      * conditional on all control qubits being in the \f$\ket{1}\f$ state.
      *
      * \f[
      *
      * CU\ket{c_1...c_Nt_1...t_M} \rightarrow \ket{c_1...c_N} U^{c_1...c_N} \ket{t_1...t_M}
      *
      * \f]
      *
      * @param circ the circuit for the unitary operation U [CircuitBuilder]
      * @param ctrl_inds the indices of the control qubits [vector of int]
      */
      void CU(CircuitBuilder &circ, std::vector<int> ctrl_inds);

      /**
      * @brief CZ gate
      *
      * This method adds a controlled-Z (CZ) gate to the circuit.
      *
      * The CZ gate performs a Z gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is
      *
      * \f[
      *
      * CZ\ket{ab} \rightarrow \ket{a}Z^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx the index of the control qubit [size_t]
      * @param target_idx the index of the target qubit [size_t]
      */
      void CZ(size_t ctrl_idx, size_t target_idx);

      /**
      * @brief ACZ gate
      *
      * This method adds an anti-controlled-Z (ACZ) gate to the circuit.
      *
      * The ACZ gate performs a Z gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{0}\f$ state. That is
      *
      * \f[
      *
      * CZ\ket{ab} \rightarrow \ket{a}Z^{a+1} \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx the index of the control qubit [size_t]
      * @param target_idx the index of the target qubit [size_t]
      */
      void ACZ(size_t ctrl_idx, size_t target_idx);

      /**
      * @brief CH gate
      *
      * This method adds a controlled-H (CH) gate to the circuit.
      *
      * The CH gate performs an H gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * CH\ket{ab} \rightarrow \ket{a}H^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx the index of the control qubit [size_t]
      * @param target_idx the index of the target qubit [size_t]
      */
      void CH(size_t ctrl_idx, size_t target_idx);

      // CPhase == CU1
      /**
      * @brief CPhase gate
      *
      * This method adds a controlled-U1 (CPhase) gate to the circuit.
      *
      * The CPHase gate performs a U1 gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * CPhase(\theta)\ket{ab} \rightarrow \ket{a}U1(\theta)^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx the index of the control qubit [size_t]
      * @param target_idx the index of the target qubit [size_t]
      * @param theta the value of the phase [double]
      */
      void CPhase(size_t ctrl_idx, size_t target_idx, double theta);

      /**
      * @overload
      *
      * This overloaded method adds a parametrized phase (U1) gate with a free
      * parameter to the circuit.
      * The name of the free parameter must be passed to the method.
      *
      *
      *
      * @param ctrl_idx the index of the control qubit [size_t]
      * @param target_idx the index of the target qubit [size_t]
      * @param param_name the name of the input parameter [std::string]
      */
      void CPhase(size_t ctrl_idx, size_t target_idx, std::string param_name);

      /**
      * @brief CRZ gate
      *
      * This method adds a controlled-RZ gate to the circuit.
      *
      * The CRZ gate performs a RZ gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * CRZ(\theta)\ket{ab} \rightarrow \ket{a}RZ(\theta)^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx the index of the control qubit [size_t]
      * @param target_idx the index of the target qubit [size_t]
      * @param theta the value of the phase [double]
      */
      void CRZ(size_t ctrl_idx, size_t target_idx, double theta);

      /**
      * This method adds an z-axis rotation (CRZ) gate with a free
      * parameter to the circuit.
      * It is equivalent to the parameterized CPhase gate.
      *
      * The name of the free parameter must be passed to the method.
      *
      * The CRZ gate performs a RZ gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * CRZ(\theta)\ket{ab} \rightarrow \ket{a}RZ(\theta)^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx  the index of the control qubit [size_t]
      * @param target_idx  the index of the qubit being acted on [size_t]
      * @param param_name the name of the free parameter [std::string]
      */
      void CRZ(size_t ctrl_idx, size_t target_idx, std::string param_name);

      /**
      * @brief CRX gate
      *
      * This method adds a controlled-RX gate to the circuit.
      *
      * The CRX gate performs a RX gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * CRX(\theta)\ket{ab} \rightarrow \ket{a}RX(\theta)^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx the index of the control qubit [size_t]
      * @param target_idx the index of the target qubit [size_t]
      * @param theta the value of the phase [double]
      */
      void CRX(size_t ctrl_idx, size_t target_idx, double theta);

     /**
      * This method adds an x-axis rotation (CRX) gate with a free
      * parameter to the circuit.
      * The name of the free parameter must be passed to the method.
      *
      * The CRX gate performs a RX gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * CRX(\theta)\ket{ab} \rightarrow \ket{a}RX(\theta)^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx  the index of the control qubit [size_t]
      * @param target_idx  the index of the qubit being acted on [size_t]
      * @param param_name the name of the free parameter [std::string]
      */
      void CRX(size_t ctrl_idx, size_t target_idx, std::string param_name);

      /**
      * @brief CRY gate
      *
      * This method adds a controlled-RY gate to the circuit.
      *
      * The CRY gate performs a RY gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * CRY(\theta)\ket{ab} \rightarrow \ket{a}RY(\theta)^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx the index of the control qubit [size_t]
      * @param target_idx the index of the target qubit [size_t]
      * @param theta the value of the phase [double]
      */
      void CRY(size_t ctrl_idx, size_t target_idx, double theta);

      /**
      * This method adds an y-axis rotation (CRY) gate with a free
      * parameter to the circuit.
      * The name of the free parameter must be passed to the method.
      *
      * The CRY gate performs a RY gate on the target qubit
      * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
      *
      * \f[
      *
      * CRY(\theta)\ket{ab} \rightarrow \ket{a}RY(\theta)^a \ket{b}
      *
      * \f]
      *
      * @param ctrl_idx  the index of the control qubit [size_t]
      * @param target_idx  the index of the qubit being acted on [size_t]
      * @param param_name the name of the free parameter [std::string]
      */
      void CRY(size_t ctrl_idx, size_t target_idx, std::string param_name);

      /**
      * @brief SWAP gate
      *
      * This method adds a SWAP gate to the circuit.
      *
      * The SWAP gate is used to swap the quantum state of two qubits.
      * That is, it acts as:
      *
      * \f[
      *
      * SWAP\ket{\psi}\ket{\phi} \rightarrow \ket{\phi}\ket{\psi}
      *
      * \f]
      *
      * @param q1 the index of the first qubit [size_t]
      * @param q2 the index of the second qubit [size_t]
      */
      void SWAP(size_t q1, size_t q2);

      /**
      * @brief Measurement
      *
      * This method is used to indicate a qubit in the circuit should be measured.
      *
      * @param idx the index of the qubit to be measured [size_t]
      */
      void Measure(size_t idx);

      /**
      * @brief Measure all qubits
      *
      * This method adds a measurement for all qubits involved in the circuit.
      *
      * @param NUM_QUBITS the number of qubits in the circuit [int] [optional]
      */
      void MeasureAll(int NUM_QUBITS);

      /**
      * @brief Quantum Fourier Transform
      *
      * This method adds the Quantum Fourier Transform (QFT) to the circuit.
      * This is a quantum analogue of the discrete Fourier Transform.
      * It can be described by its action on basis states:
      *
      * \f[
      *
      * QFT\ket{x} \rightarrow \frac{1}{\sqrt{N}} \sum_{k=0}^{N-1} \omega_N^{xk} \ket{k}
      *
      * \f]
      *
      * @param qubit_idxs the indices of the target qubits [vector of int]
      */
      void QFT(const std::vector<int> &qubit_idxs);

      /**
      * @brief Inverse Quantum Fourier Transform
      *
      * This method adds the inverse of the Quantum Fourier Transform (IQFT) to the circuit.
      *
      * @param qubit_idxs the indices of the target qubits [vector of int]
      */
      void IQFT(const std::vector<int> &qubit_idxs);

      /**
      * @brief Quantum Phase Estimation
      *
      * This method adds the Quantum Phase Estimation (QPE) sub-routine to the circuit.
      *
      * Given some unitary operator U and and eigenvector \f$\ket{\psi}\f$ of U we can write
      *
      * \f[
      *
      * U\ket{\psi} = e^{2\pi i \theta}\ket{\psi}
      *
      * \f]
      *
      * for some value of \f$ \theta \f$. QPE is used to provide a k-bit approximation to \f$ \theta \f$
      * storing the result in an evaluation register whilst leaving \f$ \ket{\psi} \f$ unchanged.
      *
      * @param oracle The unitary operator U involved in the QPE routine [CircuitBuilder]
      * @param num_evaluation_qubits The number of bits k used to approximate the phase [int]
      * @param trial_qubits The indices of the qubits encoding the eigenvector of the unitary [vector of int]
      * @param evaluation_qubits The indices of the qubits that will be used to store the approximate phase [vector of int]
      */
      void QPE(CircuitBuilder &oracle, int num_evaluation_qubits,
               std::vector<int> trial_qubits, std::vector<int> evaluation_qubits);

      /**
      * @brief Canonical Amplitude Estimation
      *
      * This method adds the canonical version of Quantum Amplitude Estimation (QAE) to the circuit.
      *
      * Given a quantum state split into a good subspace and a bad subspace
      *
      * \f[
      *
      * \ket{\psi} = a\ket{good} + b\ket{bad} = A\ket{0}
      *
      * \f]
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
                                        std::vector<int> evaluation_qubits, bool no_state_prep);

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
                                       std::vector<int> qubits_ancilla);

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
          std::string acc_name);

      /**
      * @brief Run Canonical Amplitude Estimation with Oracle
      *
      * This method sets up and executes an instance of the canonical amplitude estimation circuit,
      * but instead of providing the grovers_op Q, we provide the oracle circuit O which acts as
      *
      * \f[
      *
      * O\ket{\psi} = O(a\ket{good} + b\ket{bad}) = -a\ket{good} + b\ket{bad}
      *
      * \f]
      *
      * The Grover operator Q is then constructed within the method from O and the state_prep circuit A as
      *
      * \f[
      *
      * Q = A^\dagger S_0 A O
      *
      * \f]
      *
      * where \f$ S_0 \f$ is an easily implementable rotation about the all 0 state \f$ \ket{000....0} \f$.
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
          std::string acc_name);

      /**
      * @brief Run Maximum-Likelihood Amplitude Estimation
      *
      * This method sets up and executes an instance of the maximum-likelihood amplitude estimation circuit.
      *
      * Given the state
      *
      * \f[
      *
      * \ket{\psi} = a\ket{good} + b\ket{bad}
      *
      * \f]
      *
      * MLQAE is an alternative to canonical QAE to find an estimate for the
      * amplitude of the good subspace, a. It works by performing several runs of amplitude
      * amplification with various iterations and recording the number of \f$ \ket{good} \f$
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
          int shots, std::string acc_name);

      /**
      * @brief Amplitude Amplification
      *
      * This method adds a number of Grovers operators to the circuit.
      *
      * Grovers operators are used to amplify the amplitude of some desired
      * subspace of your quantum state. Given a state
      *
      * \f[
      *
      * \ket{\psi} = a\ket{good} + b\ket{bad} = A\ket{0}
      *
      * \f]
      *
      * and an oracle unitary O that acts as
      *
      * \f[
      *
      * O\ket{\psi} = -a\ket{good} + b\ket{bad}
      *
      * \f]
      *
      * the Grovers operator that can be used to amplify the amplitude of the good state is
      *
      * \f[
      *
      * Q = A^\dagger S_0 A O
      *
      * \f]
      *
      * where \f$ S_0 \f$ is an easily implemented reflection about the all zero state.
      *
      * @param oracle The oracle circuit O that marks the good subspace [CircuitBuilder]
      * @param state_prep The circuit A used to prepare the input state [CircuitBuilder]
      * @param power The number of Grovers operators to append to the circuit [int]
      */
      void AmplitudeAmplification(CircuitBuilder &oracle,
                                  CircuitBuilder &state_prep, int power);

      /**
      * @brief Q' Unitary
      *
      * This method adds a Q' unitary to the circuit.
      *
      * Q' is a unitary required for the quantum decoder algorithm.
      */
      void QPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
                  int &nb_qubits_next_letter_probabilities,
                  int &nb_qubits_next_letter);

      /**
      * @brief U' Unitary
      *
      * This method adds a U' unitary to the circuit.
      *
      * U' is a unitary required for the quantum decoder algorithm.
      */
      void UPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
                  int &nb_qubits_next_letter_probabilities,
                  int &nb_qubits_next_letter);

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
                  std::vector<int> qubits_ancilla);

      /**
      * @brief UQ' Unitary
      *
      * This method adds a UQ' unitary to the circuit.
      *
      * UQ' is a unitary required for the quantum decoder algorithm.
      */
      void UQPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
                   int &nb_qubits_next_letter_probabilities,
                   int &nb_qubits_next_letter);

      /**
      * @brief Ripple Carry Adder
      *
      * This method adds a ripple carry adder to the circuit.
      *
      * The ripple carry adder is an efficient in-line addition operation
      * with a carry-in bit:
      *
      * \f[
      *
      * RippleAdd\ket{c_{in}}\ket{a}\ket{b} \rightarrow \ket{a} \ket{a+b+c_in}
      *
      * \f]
      *
      * @param a The qubit indices of the first register in the addition [vector of int]
      * @param b The qubit indices of the second register in the addition [vector of int]
      * @param carry_bit The index of the carry-in bit [int]
      */
      void RippleAdd(const std::vector<int> &a, const std::vector<int> &b,
                     int carry_bit);

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
                                std::vector<int> controls_on, std::vector<int> controls_off);

      /**
      * @brief Comparator
      *
      * This method adds a quantum bit string comparator to the circuit.
      *
      * The quantum bit string comparator is used to compare the values of two bit string.
      * If the trial score is greater than the best score, the flag qubit is flipped from 0 to 1.
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
                      std::vector<int> controls_on, std::vector<int> controls_off);

      /**
      * @brief Efficient Encoding
      *
      * This method adds an efficient encoding routine to the circuit.
      *
      * Given a lookup function f that assigns a score to each binary string,
      * we encode the state
      *
      * \f[
      *
      * \ket{\psi} = \ket{x} \ket{f(x)} + \ket{y} \ket{f(y)} + ...
      *
      * \f]
      *
      * where the superposition is over all possible bitstrings. Rather than encoding states in order \f$ \ket{000}, \ket{001}, \ket{010} \f$...etc. we cut down
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
                             std::vector<int> qubits_init_flag, int flag_integer);

      /**
      * @brief Equality Checker
      *
      * This method adds an equality checker to the circuit.
      *
      * Given two input bitstrings \f$ \ket{a} \f$ and \f$ \ket{b} \f$ the equality checker is
      * used to flip a flag qubit \f$ \ket{0} \f$ \to \f$ \ket{1} \f$ whenever a=b.
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
                           std::vector<int> controls_off);

      /**
      * @brief Controlled SWAP
      *
      * This method adds a controlled SWAP to the circuit.
      *
      * Performs a SWAP operation on \f$ \ket{a} \f$ and \f$ \ket{b} \f$ if an only if the controls are satisfied.
      *
      * @param qubits_a the indices of the qubits encoding a [vector of int]
      * @param qubits_b the indices of the qubits encoding b [vector of int]
      * @param flags_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = \f$ \ket{1} \f$) [vector of int]
      * @param flags_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = \f$ \ket{0} \f$) [vector of int]
      */
      void ControlledSwap(std::vector<int> qubits_a, std::vector<int> qubits_b,
                           std::vector<int> flags_on, std::vector<int> flags_off);

      /**
      * @brief Controlled Addition
      *
      * This method adds a controlled ripple carry adder to the circuit.
      *
      * Performs a RippleAdd operation on adder_bits and sum_bits if an only if the controls are satisfied.
      *
      * @param qubits_adder the indices of the qubits encoding adder_bits [vector of int]
      * @param qubits_sum the indices of the qubits encoding sum_bits [vector of int]
      * @param c_in the index of the carry-in bit [int]
      * @param flags_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = \f$ \ket{1} \f$) [vector of int]
      * @param flags_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = \f$ \ket{0} \f$) [vector of int]
      * @param no_overflow Indicates that the sum adder_bits + sum_bits can be encoded on the same number of qubits as |sum> without overflowing [bool]
      */
      void ControlledAddition(std::vector<int> qubits_adder, std::vector<int> qubits_sum, int c_in,
                           std::vector<int> flags_on, std::vector<int> flags_off, bool no_overflow);

      /**
      * @brief Generalised MCX
      *
      * This method adds a generalised MCX gate to the circuit.
      *
      * By generalised MCX we mean that we allow the control qubits to be
      * conditional on being off or conditional on being on.
      *
      * @param target The index of the target qubit [int]
      * @param controls_on The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = \f$ \ket{1} \f$) [vector of int]
      * @param controls_off The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = \f$ \ket{0} \f$) [vector of int]
      */
      void GeneralisedMCX(int target, std::vector<int> controls_on,
                           std::vector<int> controls_off);

      /**
      * @brief Compare Beam Oracle
      *
      * This method adds a compare beam oracle to the circuit.
      *
      * This method is required for the quantum decoder algorithm.
      */
      void CompareBeamOracle(int q0, int q1, int q2, std::vector<int> FA,
                           std::vector<int> FB, std::vector<int> SA, std::vector<int> SB, bool simplified);

      /**
       * @brief Superposition Adder
       *
       * This method adds a superposition adder to the current circuit.
       *
       * Given a superposition state
       *
       * \f[
       *
       * \ket{\psi} = \sum a_1\ket{\phi_1} + a_2\ket{\phi_2} + ... + a_n\ket{\phi_n},
       *
       * \f]
       *
       * this function finds the mean of the of the amplitudes, i.e.
       *
       * \f[
       *
       * (|a_1|^2 + |a_2|^2 + |a_n|^2) / N.
       *
       * \f]
       *
       * @param q0 the index of the single required ancilla [int]
       * @param q1 the index of the single required ancilla [int]
       * @param q2 the index of the single required ancilla [int]
       * @param qubits_flags the indices of the flag qubits [vector of int]
       * @param qubits_string the indices of the qubits encoding the string [vector of int]
       * @param qubits_metric the indices of the qubits encoding the metric value corresponding to the string [vector of int]
       * @param ae_state_prep_circ The circuit A used to prepare the input state [CircuitBuilder]
       * @param qubits_ancilla the indices of the required ancilla qubits [vector of int]
       * @param qubits_beam_metric the indices of the qubits encoding class' metric [vector of int]
       */
      void SuperpositionAdder(int q0, int q1, int q2,
                              std::vector<int> qubits_flags, std::vector<int> qubits_string,
                              std::vector<int> qubits_metric,
                              CircuitBuilder &ae_state_prep_circ,
                              std::vector<int> qubits_ancilla,
                              std::vector<int> qubits_beam_metric);

      /**
      * @brief Inverse Circuit
      *
      * This method adds the inverse of a circuit to the current circuit.
      *
      * Given some collection of unitary operations,
      *
      * \f[
      *
      * U = U_N*U_{N-1}...U_2*U_1
      *
      * \f]
      *
      * this method appends the inverse to the circuit:
      *
      * \f[
      *
      * U^{-1} = U_1^\dagger U_2^\dagger...U_{N-1}^\dagger U_N^\dagger
      *
      * \f]
      *
      * This may be useful for un-computing ancilla or for constructing Grovers operators.
      *
      * @param circ The circuit whose inverse we want to add to the current circuit [CircuitBuilder]
      */
      void InverseCircuit(CircuitBuilder &circ);

      /**
      * @brief Subtraction
      *
      * This method adds a subtraction to the circuit.
      *
      * Performs the mapping:
      *
      * \f[
      *
      * \ket{a}\ket{b} \rightarrow \ket{a-b}\ket{b}
      *
      * \f]
      *
      * assuming a>b.
      *
      * @param qubits_larger the indices of the qubits encoding the larger value [vector of int]
      * @param qubits_smaller the indices of the qubits encoding the smaller value [vector of int]
      * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
      * @param qubit_ancilla the index of the required ancilla [vector of int]
      */
      void Subtraction(std::vector<int> qubits_larger,
                       std::vector<int> qubits_smaller, bool is_LSB, int qubit_ancilla);

      /**
      * @brief Controlled Subtraction
      *
      * This method adds a controlled subtraction to the circuit.
      *
      * Performs a subtraction operation on a and b if an only if the controls are satisfied.
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
                                 std::vector<int> controls_off, bool is_LSB, int qubit_ancilla);

      /**
      * @brief Proper Fraction Division
      *
      * This method adds a proper fraction division to the circuit.
      *
      * Performs the mapping:
      *
      * \f[
      *
      * \ket{num}\ket{denom}\ket{0} \rightarrow \ket{num}\ket{denom}\ket{num/denom}
      *
      * \f]
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
                                  std::vector<int> qubits_ancilla, bool is_LSB);

      /**
      * @brief Controlled Proper Fraction Division
      *
      * This method adds a controlled proper fraction division to the circuit.
      *
      * Performs a PFD operation on a and b if an only if the controls are satisfied.
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
                                            bool is_LSB);

      /**
      * @brief Compare Greater Than
      *
      * This method adds a > comparator to the circuit.
      *
      * Given two binary strings \f$ \ket{a} \f$ and \f$ \ket{b} \f$, this comparator flips a flag qubit whenever a>b.
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
                                  int qubit_ancilla, bool is_LSB);

      /**
      * @brief Multiplication
      *
      * This method adds a Multiplication to the circuit.
      *
      * Performs the mapping:
      *
      * \f[
      *
      * \ket{a}\ket{b}\ket{0} \rightarrow \ket{a}\ket{b}\ket{ab}
      *
      * \f]
      *
      * @param qubits_a the indices of the qubits encoding a [vector of int]
      * @param qubits_b the indices of the qubits encoding b [vector of int]
      * @param qubits_result the indices of the qubits that will ecode the multiplication result [vector of int]
      * @param qubit_ancilla the index of the single required ancilla [int]
      * @param is_LSB Indicates that the trial scores are encoded with LSB ordering [bool]
      */
      void Multiplication(std::vector<int> qubits_a, std::vector<int> qubits_b,
                          std::vector<int> qubits_result, int qubit_ancilla, bool is_LSB);

      /**

      * @brief Controlled Multiplication
      *
      * This method adds a controlled Multiplication to the circuit.
      *
      * Performs a Multiplication operation on a and b if an only if the controls are satisfied.
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
                           std::vector<int> qubits_result, int qubit_ancilla, bool is_LSB, std::vector<int> controls_on, std::vector<int> controls_off);

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
       * @param state_prep_circ a function which produces the state prep circuit [StatePrepFuncCType]
       * @param oracle_func a function which produces the oracle circuit that marks the good subspace [OracleFuncCType]
       * @param best_score the current best score [int]
       * @param f_score a function that returns a 1 if the input binary string has value greater than the current best score and 0 otherwise [func(int)->int]
       * @param total_num_qubits the total number of qubits, i.e. number of string qubits + number of metric qubits [int]
       * @param qubits_string the indices of the qubits encoding the strings [vector of int]
       * @param total_metric the indices of the qubits encoding the string scores [vector of int]
       * @param acc_name the name of the accelerator used to execute the algorithm [string]
       *
       * @return a better score if found, otherwise returns the current best score
       */
      int ExponentialSearch(
          std::string method, StatePrepFuncCType state_prep_circ, OracleFuncCType oracle_func,
          int best_score, std::function<int(int)> f_score, int total_num_qubits,
          std::vector<int> qubits_string, std::vector<int> total_metric,
          std::string acc_name);

  };

}

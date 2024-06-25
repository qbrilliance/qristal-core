// Copyright (c) Quantum Brilliance Pty Ltd

#include "py_circuit_builder.hpp"
#include "py_stl_containers.hpp"
#include "qb/core/circuit_builder.hpp"
#include "qb/core/circuit_builders/exponent.hpp"
namespace qb {
void bind_circuit_builder(pybind11::module &m) {
  namespace py = pybind11;
  using OracleFuncPyType = std::function<qb::CircuitBuilder(int)>;
  using StatePrepFuncPyType = std::function<qb::CircuitBuilder(
      py::array_t<int>, py::array_t<int>, py::array_t<int>, py::array_t<int>,
      py::array_t<int>)>;
  py::class_<qb::CircuitBuilder>(m, "Circuit")
      .def(py::init())
      .def("print", &qb::CircuitBuilder::print,
           "Print the quantum circuit that has been built")
      .def(
          "openqasm",
          [&](qb::CircuitBuilder &this_) {
            if (this_.is_parametrized()) 
            {
              throw std::runtime_error("Cannot convert parametrized circuit to OpenQASM!");
            }
            auto staq = xacc::getCompiler("staq");
            const auto openQASMSrc = staq->translate(this_.get());
            return openQASMSrc;
          },
          "Get the OpenQASM representation of the (non-parametrized) circuit.")
      .def(
          "append",
          [&](qb::CircuitBuilder &this_, qb::CircuitBuilder other) {
            this_.append(other);
          },
          py::arg("other"),
          "Append the 'other' quantum circuit to this circuit.")
      // Temporary interface to execute the circuit.
      // TODO: using s `run` once the QE-382 is implemented
      .def(
          "execute",
          [&](qb::CircuitBuilder &this_, const std::string &QPU, int NUM_SHOTS,
              int NUM_QUBITS) {
            auto acc = xacc::getAccelerator(QPU, {{"shots", NUM_SHOTS}});
            if (NUM_QUBITS < 0) {
              NUM_QUBITS = this_.get()->nPhysicalBits();
            }
            auto buffer = xacc::qalloc(NUM_QUBITS);
            acc->execute(buffer, this_.get());
            return buffer->toString();
          },
          py::arg("QPU") = "qpp", py::arg("NUM_SHOTS") = 1024,
          py::arg("NUM_QUBITS") = -1, R"(
              Run the circuit.

              This method is used to pass the circuit to an accelerator backend
              for execution.

              The optional parameters are:

              - **QPU** The accelerator name [string]
              - **NUM_SHOTS** The number of shots to use [int]
              - **NUM_QUBITS** The number of qubits required for the circuit [int]

          )")
      .def(
          "num_qubits", [&](qb::CircuitBuilder &builder) {builder.num_qubits();}, R"(
      Returns the number of (physical) qubits in the circuit.
    )")
      .def(
          "num_free_params", [&](qb::CircuitBuilder &builder) {builder.num_free_params();}, R"(
      Returns the number of free parameters in the (parametrized) circuit.
    )")
      .def(
          "param_dict_to_list", 
          [&](qb::CircuitBuilder &builder, std::map<std::string, double> param_dict) {
            return builder.param_map_to_vec(param_dict);
          }, R"(
      Convert a dictionary that defines parameter assignments to a vector for 
      input to the session object. The vector will be ordered according to 
      the definition of the free parameter in the circuit; for example, if 
      a gate is defined with the free parameter "alpha" in an empty circuit, 
      its mapped parameter will be at index 0 in the vector. If another gate 
      exists in this circuit with the parameter "beta", the value for this 
      mapped parameter will be at index 1, and so on.

      Parameters:

      - **param_dict** the dictionary 

      Returns:

      A vector containing the ordered parameter values.

    )")
      .def(
          "h", [&](qb::CircuitBuilder &builder, int idx) { builder.H(idx); },
          py::arg("idx"), R"(
    Hadamard gate
  
   This method adds a Hadamard (H) gate to the circuit. 
   
   Parameters:

   - **idx** the index of the qubit being acted on [int]

  )")
      .def(
          "x", [&](qb::CircuitBuilder &builder, int idx) { builder.X(idx); },
          py::arg("idx"), R"(
      Pauli-X gate

     This method adds a Pauli-X (X) gate to the circuit.

     The X gate is defined by its action on the basis states

    Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
      .def(
          "y", [&](qb::CircuitBuilder &builder, int idx) { builder.Y(idx); },
          py::arg("idx"), R"(
      Pauli-Y gate

     This method adds a Pauli-Y (Y) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
      .def(
          "z", [&](qb::CircuitBuilder &builder, int idx) { builder.Z(idx); },
          py::arg("idx"), R"(
      Pauli-Z gate

     This method adds a Pauli-Z (Z) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
      .def(
          "t", [&](qb::CircuitBuilder &builder, int idx) { builder.T(idx); },
          py::arg("idx"), R"(
      T gate

     This method adds a T gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
      .def(
          "tdg",
          [&](qb::CircuitBuilder &builder, int idx) { builder.Tdg(idx); },
          py::arg("idx"), R"(
      Tdg gate

     This method adds an inverse of the T gate (Tdg) to the circuit.

     The Tdg gate is defined by its action on the basis states

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
      .def(
          "s", [&](qb::CircuitBuilder &builder, int idx) { builder.S(idx); },
          py::arg("idx"), R"(
      S gate

     This method adds an S gate to the circuit.

     The S gate is defined by its action on the basis states

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
      .def(
          "sdg",
          [&](qb::CircuitBuilder &builder, int idx) { builder.Sdg(idx); },
          py::arg("idx"), R"(
      Sdg gate

     This method adds an inverse of the S gate (Sdg) to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]

    )")
      .def(
          "rx",
          [&](qb::CircuitBuilder &builder, int idx, double theta) {
            builder.RX(idx, theta);
          },
          py::arg("idx"), py::arg("theta"), R"(
      RX gate

     This method adds an x-axis rotation (RX) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** the angle of rotation about the x-axis [double]

    )")
      .def(
          "rx",
          [&](qb::CircuitBuilder &builder, int idx, std::string param_name) {
            builder.RX(idx, param_name);
          },
          py::arg("idx"), py::arg("name"), R"(
      RX gate

     This method adds an x-axis rotation (RX) gate with a free parameter "
     "to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **name** the name of the free parameter [string]

    )")
      .def(
          "ry",
          [&](qb::CircuitBuilder &builder, int idx, double theta) {
            builder.RY(idx, theta);
          },
          py::arg("idx"), py::arg("theta"), R"(
      RY gate

     This method adds a y-axis rotation (RY) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** the angle of rotation about the y-axis [double]

    )")
      .def(
          "ry",
          [&](qb::CircuitBuilder &builder, int idx, std::string param_name) {
            builder.RY(idx, param_name);
          },
          py::arg("idx"), py::arg("param_name"), R"(
      RY gate

     This method adds a y-axis rotation (RY) gate with a free parameter "
     "to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **name** the name of the free parameter [string]

    )")
      .def(
          "rz",
          [&](qb::CircuitBuilder &builder, int idx, double theta) {
            builder.RZ(idx, theta);
          },
          py::arg("idx"), py::arg("theta"), R"(
      RZ gate

     This method adds a z-axis rotation (RZ) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** the angle of rotation about the z-axis [double]

    )")
      .def(
          "rz",
          [&](qb::CircuitBuilder &builder, int idx, std::string param_name) {
            builder.RZ(idx, param_name);
          },
          py::arg("idx"), py::arg("param_name"), R"(
      RZ gate

     This method adds a z-axis rotation (RZ) gate with a free parameter "
     "to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **name** the name of the free parameter [string]

    )")
      .def(
          "cnot",
          [&](qb::CircuitBuilder &builder, int ctrl_idx, int target_idx) {
            builder.CNOT(ctrl_idx, target_idx);
          },
          py::arg("ctrl_idx"), py::arg("target_idx"), R"(
      CNOT gate

     This method adds a controlled-X (CNOT) gate to the circuit.

     The CNOT gate performs an X gate on the target qubit
     conditional on the control qubit being in the 1 state.

     Parameters:

     - **ctrl_idx** the index of the control qubit [int]
     - **target_idx** the index of the target qubit [int]

    )")
      .def(
          "mcx",
          [&](qb::CircuitBuilder &builder, py::array_t<int> ctrl_inds,
              int target_idx) {
            builder.MCX(py_array_to_std_vec(ctrl_inds), target_idx);
          },
          py::arg("ctrl_inds"), py::arg("target_idx"),
          R"(
      MCX gate

     This method adds a multi-controlled X (MCX) gate to the circuit.

     The MCX gate performs an X gate on the target qubit
     conditional on all control qubits being in the 1 state.

     Parameters:

     - **ctrl_inds** the indices of the control qubits [list of int]
     - **target_idx** the index of the target qubit [int]

    )")
      .def(
          "ccx",
          [&](qb::CircuitBuilder &builder, int ctrl_idx1, int ctrl_idx2,
              int target_idx) {
            builder.MCX({ctrl_idx1, ctrl_idx2}, target_idx);
          },
          py::arg("ctrl_idx1"), py::arg("ctrl_idx2"), py::arg("target_idx"),
          R"(
      Toffoli gate

     This method adds a Toffoli gate (CCX) to the circuit.

     The CCX gate performs an X gate on the target qubit
     conditional on the two control qubits being in the 1 state.

     Parameters:

     - **ctrl_idx1** the index of the first control qubit [int]
     - **ctrl_idx2** the index of the second control qubit [int]
     - **target_idx** the index of the target qubit [int]

    )")
      .def(
          "swap",
          [&](qb::CircuitBuilder &builder, int q1, int q2) {
            builder.SWAP(q1, q2);
          },
          py::arg("q1"), py::arg("q2"), R"(
      SWAP gate

     This method adds a SWAP gate to the circuit. The SWAP gate is used to swap the quantum state of two qubits.

     Parameters:

     - **q1** the index of the first qubit [int]
     - **q2** the index of the second qubit [int]

    )")
      .def(
          "cphase",
          [&](qb::CircuitBuilder &builder, int ctrl_idx, int target_idx,
              double theta) { builder.CPhase(ctrl_idx, target_idx, theta); },
          py::arg("ctrl_idx"), py::arg("target_idx"), py::arg("theta"),
          R"(
      CPhase gate

     This method adds a controlled-U1 (CPhase) gate to the circuit.

     The CPHase gate performs a U1(theta) gate on the target qubit
     conditional on the control qubit being in the 1 state.

     Parameters:

     - **ctrl_idx** the index of the control qubit [int]
     - **target_idx** the index of the target qubit [int]
     - **theta** the value of the phase [double]

    )")
      .def(
          "cphase",
          [&](qb::CircuitBuilder &builder, int ctrl_idx, int target_idx,
              std::string param_name) { builder.CPhase(ctrl_idx, target_idx, param_name); },
          py::arg("ctrl_idx"), py::arg("target_idx"), py::arg("param_name"),
          R"(
      CPhase gate

     This method adds a controlled-U1 (CPhase) gate with a free parameter "
     "to the circuit.

     The CPHase gate performs a U1(theta) gate on the target qubit
     conditional on the control qubit being in the 1 state.

     Parameters:

     - **ctrl_idx** the index of the control qubit [int]
     - **target_idx** the index of the target qubit [int]
     - **name** the name of the free parameter [string]

    )")
      .def(
          "cz",
          [&](qb::CircuitBuilder &builder, int ctrl_idx, int target_idx) {
            builder.CZ(ctrl_idx, target_idx);
          },
          py::arg("ctrl_idx"), py::arg("target_idx"), R"(
      CZ gate

     This method adds a controlled-Z (CZ) gate to the circuit.

     The CZ gate performs a Z gate on the target qubit
     conditional on the control qubit being in the 1 state.

     Parameters:

     - **ctrl_idx** the index of the control qubit [int]
     - **target_idx** the index of the target qubit [int]

    )")
      .def(
          "ch",
          [&](qb::CircuitBuilder &builder, int ctrl_idx, int target_idx) {
            builder.CH(ctrl_idx, target_idx);
          },
          py::arg("ctrl_idx"), py::arg("target_idx"), R"(
      CH gate

     This method adds a controlled-H (CH) gate to the circuit.

     The CH gate performs an H gate on the target qubit
     conditional on the control qubit being in the 1 state.

     - **ctrl_idx** the index of the control qubit [int]
     - **target_idx** the index of the target qubit [int]

    )")
      .def(
          "u1",
          [&](qb::CircuitBuilder &builder, int idx, double theta) {
            builder.U1(idx, theta);
          },
          py::arg("idx"), py::arg("theta"), R"(
      U1 gate

     This method adds a phase (U1) gate to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** the value of the phase [double]

    )")
      .def(
          "u1",
          [&](qb::CircuitBuilder &builder, int idx, std::string param_name) {
            builder.U1(idx, param_name);
          },
          py::arg("idx"), py::arg("param_name"), R"(
     U1 gate

     This method adds a phase (U1) gate with a free parameter "
     "to the circuit.

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **name** the name of the free parameter [string]

    )")
      .def(
          "u3",
          [&](qb::CircuitBuilder &builder, int idx, std::string param_1, std::string param_2,
              std::string param_3) { builder.U3(idx, param_1, param_2, param_3); },
          py::arg("idx"), py::arg("param_1"), py::arg("param_2"), py::arg("param_3"),
          R"(
     Parameterized U3 gate
     U3 gate

     This method adds an arbitrary parametrized single qubit gate (U3) to the circuit, shown as U at https://qristal.readthedocs.io/en/latest/rst/quantum_gates.html
     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **param_1** the name of the 1st free parameter (theta) [string] 
     - **param_2** the name of the 2nd free parameter (phi) [string]
     - **param_3** the name of the 3rd free parameter (lambda) [string]

      )")
      .def(
          "u3",
          [&](qb::CircuitBuilder &builder, int idx, double theta, double phi,
              double lambda) { builder.U3(idx, theta, phi, lambda); },
          py::arg("idx"), py::arg("theta"), py::arg("phi"), py::arg("lambda"),
          R"(
     U3 gate
     
     This method adds an arbitrary single qubit gate (U3) to the circuit, shown as U at https://qristal.readthedocs.io/en/latest/rst/quantum_gates.html

     Parameters:

     - **idx** the index of the qubit being acted on [int]
     - **theta** [double]
     - **phi** [double]
     - **lambda** [double]

    )")
      .def(
          "measure",
          [&](qb::CircuitBuilder &builder, int idx) { builder.Measure(idx); },
          py::arg("idx"), R"(
      Measurement

     This method is used to indicate a qubit in the circuit should be
     measured.

     Parameters:

     - **idx** the index of the qubit to be measured [int]

    )")
      .def(
          "measure_all",
          [&](qb::CircuitBuilder &builder, int NUM_QUBITS) {
            builder.MeasureAll(NUM_QUBITS);
          },
          py::arg("NUM_QUBITS") = -1,
          R"(
      Measure all qubits

     This method adds a measurement for all qubits involved in the circuit.

     Parameters:

     - **NUM_QUBITS** the number of qubits in the circuit [int] [optional, the default value of -1 becomes the output of the XACC nPhysicalBits method.]

    )")
      .def(
          "qft",
          [&](qb::CircuitBuilder &builder, py::array_t<int> inds) {
            builder.QFT(py_array_to_std_vec(inds));
          },
          py::arg("qubits"), R"(
      Quantum Fourier Transform

     This method adds the Quantum Fourier Transform (QFT) to the circuit.
     This is a quantum analogue of the discrete Fourier Transform.

     Parameters:

     - **qubits** the indices of the target qubits [list of int]

    )")
      .def(
          "iqft",
          [&](qb::CircuitBuilder &builder, py::array_t<int> inds) {
            builder.IQFT(py_array_to_std_vec(inds));
          },
          py::arg("qubits"), R"(
      Inverse Quantum Fourier Transform

     This method adds the inverse of the Quantum Fourier Transform (IQFT) to
     the circuit.

     Parameters:

     - **qubits** the indices of the target qubits [list of int]

    )")
      .def(
          "exponent",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_log,
              py::array_t<int> qubits_exponent, py::array_t<int> qubits_ancilla,
              int min_significance, bool is_LSB) {
            qb::Exponent build_exp;
            xacc::HeterogeneousMap map = {
                {"qubits_log", py_array_to_std_vec(qubits_log)},
                {"min_significance", min_significance},
                {"is_LSB", is_LSB}};
            if (qubits_exponent.size() > 0) {
              map.insert("qubits_exponent", qubits_exponent);
            }
            if (qubits_ancilla.size() > 0) {
              map.insert("qubits_ancilla", qubits_ancilla);
            }
            bool expand_ok = build_exp.expand(map);
            builder.append(build_exp);
            return expand_ok;
          },
          py::arg("qubits_log") = py::array_t<int>(),
          py::arg("qubits_exponent") = py::array_t<int>(),
          py::arg("qubits_ancilla") = py::array_t<int>(),
          py::arg("min_significance") = 1, py::arg("is_LSB") = true,
          R"(
                Exponent Base 2

                This method adds an exponent to the circuit. This is used to replace some value
                by its exponent base 2. 

                Parameters:

                - **qubits_log** the indices of the qubits encoding the original value [list of int]
                - **qubits_exponent** the indices of the qubits used to store the result [list of int]
                - **qubits_ancilla** the indices of the required ancilla qubits [list of int]
                - **min_significance** the accuracy cutoff [int]
                - **is_LSB** indicates LSB ordering is used [bool]

            )")
      .def(
          "qpe",
          [&](qb::CircuitBuilder &builder, py::object &oracle, int precision,
              py::array_t<int> trial_qubits,
              py::array_t<int> evaluation_qubits) {
            qb::CircuitBuilder *casted = oracle.cast<qb::CircuitBuilder *>();
            assert(casted);
            builder.QPE(*casted, precision, py_array_to_std_vec(trial_qubits),
                        py_array_to_std_vec(evaluation_qubits));
          },
          py::arg("oracle"), py::arg("precision"),
          py::arg("trial_qubits") = py::array_t<int>(),
          py::arg("precision_qubits") = py::array_t<int>(),
          R"(
      Quantum Phase Estimation

     This method adds the Quantum Phase Estimation (QPE) sub-routine to the
     circuit.

     Given some unitary operator U and and eigenvector v of U, QPE is used to provide a k-bit approximation to
     the corresponding eigenvalue's phase, storing the result in an evaluation register whilst leaving the eigenvector
     unchanged.

     Parameters:

     - **oracle** The unitary operator U involved in the QPE routine [CircuitBuilder]
     - **precision** The number of bits k used to approximate the phase [int]
     - **trial_qubits** The indices of the qubits encoding the eigenvector of the unitary [list of int]
     - **precision_qubits** The indices of the qubits that will be used to store the approximate phase [list of int]

    )")
      .def(
          "canonical_ae",
          [&](qb::CircuitBuilder &builder, py::object &state_prep,
              py::object &grover_op, int precision, int num_state_prep_qubits,
              int num_trial_qubits, py::array_t<int> precision_qubits,
              py::array_t<int> trial_qubits, bool no_state_prep) {
            qb::CircuitBuilder *casted_state_prep =
                state_prep.cast<qb::CircuitBuilder *>();
            assert(state_prep);

            qb::CircuitBuilder *casted_grover_op =
                grover_op.cast<qb::CircuitBuilder *>();
            assert(casted_grover_op);
            builder.CanonicalAmplitudeEstimation(
                *casted_state_prep, *casted_grover_op, precision,
                num_state_prep_qubits, num_trial_qubits,
                py_array_to_std_vec(precision_qubits),
                py_array_to_std_vec(trial_qubits), no_state_prep);
          },
          py::arg("state_prep"), py::arg("grover_op"), py::arg("precision"),
          py::arg("num_state_prep_qubits"), py::arg("num_trial_qubits"),
          py::arg("precision_qubits") = py::array_t<int>(),
          py::arg("trial_qubits") = py::array_t<int>(),
          py::arg("no_state_prep") = false, R"(
      Canonical Amplitude Estimation

     This method adds the canonical version of Quantum Amplitude Estimation
     (QAE) to the circuit.

     Given a quantum state split into a good subspace and a bad subspace, 
     the QAE sub-routine provides a k-bit approximation to the amplitude of
     the good subspace, a.

     QAE works by using the Grovers operator Q, which amplifies the amplitude
     of the good subspace, as the unitary input to a Quantum Phase Estimation
     routine.

     Parameters:

     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **grover_op** The circuit for the Grovers operator Q for the good subspace [CircuitBuilder]
     - **precision** The number of bits k used to approximate the amplitude [int]
     - **num_state_prep_qubits** The number of qubits acted on by the state_prep circuit A [int]
     - **num_trial_qubits** The number of qubits acted on by the grover_op circuit Q [int]
     - **trial_qubits** The indices of the qubits acted on by the grover_op circuit Q [list of int]
     - **precision_qubits** The indices of the qubits used to store the approximate amplitude [list of int]
     - **no_state_prep** If true, assumes the state is already prepared in the appropriate register [bool]

    )")
      .def(
          "run_canonical_ae",
          [&](qb::CircuitBuilder &builder, py::object &state_prep,
              py::object &grover_op, int precision, int num_state_prep_qubits,
              int num_trial_qubits, py::array_t<int> precision_qubits,
              py::array_t<int> trial_qubits, py::str acc_name) {
            qb::CircuitBuilder *casted_state_prep =
                state_prep.cast<qb::CircuitBuilder *>();
            assert(state_prep);

            qb::CircuitBuilder *casted_grover_op =
                grover_op.cast<qb::CircuitBuilder *>();
            assert(casted_grover_op);
            return builder.RunCanonicalAmplitudeEstimation(
                *casted_state_prep, *casted_grover_op, precision,
                num_state_prep_qubits, num_trial_qubits,
                py_array_to_std_vec(precision_qubits),
                py_array_to_std_vec(trial_qubits), acc_name);
          },
          py::arg("state_prep"), py::arg("grover_op"), py::arg("precision"),
          py::arg("num_state_prep_qubits"), py::arg("num_trial_qubits"),
          py::arg("precision_qubits") = py::array_t<int>(),
          py::arg("trial_qubits") = py::array_t<int>(), py::arg("qpu") = "qpp",
          R"(
      Run Canonical Amplitude Estimation

     This method sets up and executes an instance of the canonical amplitude
     estimation circuit.

     Parameters:

     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **grover_op** The circuit for the Grovers operator Q for the good subspace [CircuitBuilder]
     - **precision** The number of bits k used to approximate the amplitude [int]
     - **num_state_prep_qubits** The number of qubits acted on by the state_prep circuit A [int]
     - **num_trial_qubits** The number of qubits acted on by the grover_op circuit Q [int]
     - **trial_qubits** The indices of the qubits acted on by the grover_op circuit Q [list of int]
     - **precision_qubits** The indices of the qubits used to store the approximate amplitude [list of int]
     - **qpu** The name of the accelerator used to execute the circuit [string]

     Returns: The output buffer of the execution
    )")
      .def(
          "amcu",
          [&](qb::CircuitBuilder &builder, py::object &U,
              py::array_t<int> qubits_control,
              py::array_t<int> qubits_ancilla) {
            qb::CircuitBuilder *casted_U = U.cast<qb::CircuitBuilder *>();

            return builder.MultiControlledUWithAncilla(
                *casted_U, py_array_to_std_vec(qubits_control),
                py_array_to_std_vec(qubits_ancilla));
          },
          py::arg("U"), py::arg("qubits_control"), py::arg("qubits_ancilla"),
          R"(
      Multi Controlled Unitary With Ancilla

     This method decomposes a multi-controlled unitary into Toffoli gates
     and the unitary itself, with the use of ancilla qubits. With N control
     qubits there should be N-1 ancilla. The resulting instructions are added
     to the circuit (AMCU gate).

     Parameters:

     - **U** The unitary operation [CircuitBuilder]
     - **qubits_control** The indices of the control qubits [list of int]
     - **qubits_ancilla** The indices of the ancilla qubits [list of int]

    )")
      .def(
          "run_canonical_ae_with_oracle",
          [&](qb::CircuitBuilder &builder, py::object &state_prep,
              py::object &oracle, int precision, int num_state_prep_qubits,
              int num_trial_qubits, py::array_t<int> precision_qubits,
              py::array_t<int> trial_qubits, py::str acc_name) {
            qb::CircuitBuilder *casted_state_prep =
                state_prep.cast<qb::CircuitBuilder *>();
            assert(state_prep);

            qb::CircuitBuilder *casted_oracle =
                oracle.cast<qb::CircuitBuilder *>();
            assert(casted_oracle);
            return builder.RunCanonicalAmplitudeEstimationWithOracle(
                *casted_state_prep, *casted_oracle, precision,
                num_state_prep_qubits, num_trial_qubits,
                py_array_to_std_vec(precision_qubits),
                py_array_to_std_vec(trial_qubits), acc_name);
          },
          py::arg("state_prep"), py::arg("oracle"), py::arg("precision"),
          py::arg("num_state_prep_qubits"), py::arg("num_trial_qubits"),
          py::arg("precision_qubits") = py::array_t<int>(),
          py::arg("trial_qubits") = py::array_t<int>(), py::arg("qpu") = "qpp",
          R"(
      Run Canonical Amplitude Estimation with Oracle

     This method sets up and executes an instance of the canonical amplitude
     estimation circuit, but instead of providing the grovers_op Q, we
     provide the oracle circuit O which marks the good elements.

     The Grovvers operator Q is then constructed within the method from O and
     the state_prep circuit A.

     Parameters:

     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **oracle** The oracle circuit O that marks the good subspace [CircuitBuilder]
     - **precision** The number of bits k used to approximate the amplitude [int]
     - **num_state_prep_qubits** The number of qubits acted on by the state_prep circuit A [int]
     - **num_trial_qubits** The number of qubits acted on by the grover_op circuit Q [int]
     - **precision_qubits** The indices of the qubits used to store the approximate amplitude [list of int]
     - **trial_qubits** The indices of the qubits acted on by the grover_op circuit Q [list of int]
     - **qpu** The name of the accelerator used to execute the circuit [string]

     Returns: The output buffer of the execution
    )")
      .def(
          "run_MLQAE",
          [&](qb::CircuitBuilder &builder, py::object &state_prep,
              py::object &oracle,
              std::function<int(std::string, int)> is_in_good_subspace,
              py::array_t<int> score_qubits, int total_num_qubits, int num_runs,
              int shots, py::str acc_name) {
            qb::CircuitBuilder *casted_state_prep =
                state_prep.cast<qb::CircuitBuilder *>();
            assert(state_prep);

            qb::CircuitBuilder *casted_oracle =
                oracle.cast<qb::CircuitBuilder *>();
            assert(casted_oracle);
            return builder.RunMLAmplitudeEstimation(
                *casted_state_prep, *casted_oracle, is_in_good_subspace,
                py_array_to_std_vec(score_qubits), total_num_qubits, num_runs,
                shots, acc_name);
          },
          py::arg("state_prep"), py::arg("oracle"),
          py::arg("is_in_good_subspace"), py::arg("score_qubits"),
          py::arg("total_num_qubits"), py::arg("num_runs") = 4,
          py::arg("shots") = 100, py::arg("qpu") = "qpp",
          R"(
      Run Maximum-Likelihood Amplitude Estimation

     This method sets up and executes an instance of the maximum-likelihood
     amplitude estimation circuit.

     Given a state split into a good subspace and a bad subspace, MLQAE is an alternative to canonical QAE to find an estimate for the
     amplitude of the good subspace, a. It works by performing several runs
     of amplitude amplification with various iterations and recording the
     number of good shots measured. Given this data, it finds the value of
     a that maximises the likelihood function.

     Parameters:

     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **oracle** The oracle circuit O that marks the good subspace [CircuitBuilder]
     - **is_in_good_subspace** A function that, given a measured bitstring and potentially some other input value, returns a 1 if the measurement is in the good subspace and a 0 otherwise. [func(str, int) -> int]
     - **score_qubits** The indices of the qubits that determine whether the state is in the good or bad subspace [list of int]
     - **total_num_qubits** The total number of qubits in the circuit [int]
     - **num_runs** The number of runs of amplitude amplification (~4-6 is usually sufficient)
     - **shots** The number of shots in each run [int]
     - **qpu** The name of the accelerator used to execute the circuit [string]

     Returns: The output buffer of the execution
    )")
      .def(
          "amplitude_amplification",
          [&](qb::CircuitBuilder &builder, py::object &oracle,
              py::object &state_prep, int power) {
            qb::CircuitBuilder *oracle_casted =
                oracle.cast<qb::CircuitBuilder *>();
            assert(oracle_casted);
            qb::CircuitBuilder *state_prep_casted =
                state_prep.cast<qb::CircuitBuilder *>();
            assert(state_prep_casted);
            builder.AmplitudeAmplification(*oracle_casted, *state_prep_casted,
                                           power);
          },
          py::arg("oracle"), py::arg("state_prep"), py::arg("power") = 1,
          R"(
      Amplitude Amplification

     This method adds a number of Grovers operators to the circuit.

     Grovers operators are used to amplify the amplitude of some desired
     subspace of your quantum state. 

     Parameters:

     - **oracle** The oracle circuit O that marks the good subspace [CircuitBuilder]
     - **state_prep** The circuit A used to prepare the input state [CircuitBuilder]
     - **power** The number of Grovers operators to append to the circuit [int]

     )")
      .def(
          "ripple_add",
          [&](qb::CircuitBuilder &builder, py::array_t<int> a,
              py::array_t<int> b, int c_in) {
            builder.RippleAdd(py_array_to_std_vec(a), py_array_to_std_vec(b),
                              c_in);
          },
          py::arg("a"), py::arg("b"), py::arg("carry_bit"), R"(
      Ripple Carry Adder

     This method adds a ripple carry adder to the circuit.

     The ripple carry adder is an efficient in-line addition operation
     with a carry-in bit. 

     Parameters:

     - **a** The qubit indices of the first register in the addition [list of int]
     - **b** The qubit indices of the second register in the addition. This is where the result of a+b will be stored [list of int]
     - **carry_bit** The index of the carry-in bit [int]

    )")
      .def(
          "comparator",
          [&](qb::CircuitBuilder &builder, int BestScore,
              int num_scoring_qubits, py::array_t<int> trial_score_qubits,
              int flag_qubit, py::array_t<int> best_score_qubits,
              py::array_t<int> ancilla_qubits, bool is_LSB,
              py::array_t<int> controls_on, py::array_t<int> controls_off) {
            builder.Comparator(BestScore, num_scoring_qubits,
                               py_array_to_std_vec(trial_score_qubits),
                               flag_qubit,
                               py_array_to_std_vec(best_score_qubits),
                               py_array_to_std_vec(ancilla_qubits), is_LSB,
                               py_array_to_std_vec(controls_on),
                               py_array_to_std_vec(controls_off));
          },
          py::arg("best_score"), py::arg("num_scoring_qubits"),
          py::arg("trial_score_qubits") = py::array_t<int>(),
          py::arg("flag_qubit") = -1,
          py::arg("best_score_qubits") = py::array_t<int>(),
          py::arg("ancilla_qubits") = py::array_t<int>(),
          py::arg("is_LSB") = true, py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(),
          R"(
      Comparator

     This method adds a quantum bit string comparator to the circuit.

     The quantum bit string comparator is used to compare the values of two
     bit string. If the trial score is greater than the best score, the flag
     qubit is flipped.

     Parameters:

     - **best_score** The score we are comparing strings to [int]
     - **num_scoring_qubits** The number of qubits used to encode the scores [int]
     - **trial_score_qubits** The indices of the qubits encoding the trial states [list of int]
     - **flag_qubit** The index of the flag qubit which is flipped whenever trial score > BestScore [int]
     - **best_score_qubits** The indices of the qubits encoding the BestScore value [list of int]
     - **ancilla_qubits** The indices of the ancilla qubits required for the comparator circuit, if num_scoring_qubits = N we need 3N-1 ancilla [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
      .def(
          "efficient_encoding",
          [&](qb::CircuitBuilder &builder,
              std::function<int(int)> scoring_function, int num_state_qubits,
              int num_scoring_qubits, py::array_t<int> state_qubits,
              py::array_t<int> scoring_qubits, bool is_LSB, bool use_ancilla,
              py::array_t<int> qubits_init_flags, int flag_integer) {
            builder.EfficientEncoding(
                scoring_function, num_state_qubits, num_scoring_qubits,
                py_array_to_std_vec(state_qubits),
                py_array_to_std_vec(scoring_qubits), is_LSB, use_ancilla,
                py_array_to_std_vec(qubits_init_flags), flag_integer);
          },
          py::arg("scoring_function"), py::arg("num_state_qubits"),
          py::arg("num_scoring_qubits"),
          py::arg("state_qubits") = py::array_t<int>(),
          py::arg("scoring_qubits") = py::array_t<int>(),
          py::arg("is_LSB") = true, py::arg("use_ancilla") = false,
          py::arg("qubits_init_flags") = py::array_t<int>(),
          py::arg("flag_integer") = 0, R"(
      Efficient Encoding

     This method adds an efficient encoding routine to the circuit.

     Given a lookup function f that assigns a score to each binary string,
     we entangle each string to its score. Rather than
     encoding states sequentially we cut down on the
     amount of X gates required by instead following the Gray code ordering
     of states.

     This module can optionally also flag strings of a certain value.

     Parameters:

     - **scoring_function** A function that inputs the integer value of a binary string and outputs its score [func(int) -> int]
     - **num_state_qubits** The number of qubits encoding the strings [int]
     - **num_scoring_qubits** The number of qubits encoding the scores [int]
     - **state_qubits** The indices of the qubits encoding the strings [list of int]
     - **scoring_qubits** The indices of the qubits encoding the scores [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **use_ancilla** Indicates that ancilla qubits can be used to decompose MCX gates [bool]
     - **qubits_init_flag** The indices of any flag qubits [list of int]
     - **flag_integer** The integer value of binary strings that should be flagged [int]

    )")
      .def(
          "equality_checker",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_a,
              py::array_t<int> qubits_b, int flag, bool use_ancilla,
              py::array_t<int> qubits_ancilla, py::array_t<int> controls_on,
              py::array_t<int> controls_off) {
            builder.EqualityChecker(
                py_array_to_std_vec(qubits_a), py_array_to_std_vec(qubits_b),
                flag, use_ancilla, py_array_to_std_vec(qubits_ancilla),
                py_array_to_std_vec(controls_on),
                py_array_to_std_vec(controls_off));
          },
          py::arg("qubits_a"), py::arg("qubits_b"), py::arg("flag"),
          py::arg("use_ancilla") = false,
          py::arg("qubits_ancilla") = py::array_t<int>(),
          py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(),
          R"(
      Equality Checker

     This method adds an equality checker to the circuit.

     Given two input bitstrings a and b the equality checker is
     used to flip a flag qubit whenever a=b.

     Parameters:

     - **qubits_a** the indices of the qubits encoding a [list of int]
     - **qubits_b** the indices of the qubits encoding b [list of int]
     - **flag** the index of the flag qubit that gets flipped whenever a=b [int]
     - **use_ancilla** Indicates that ancilla qubits can be used to decompose MCX gates [bool]
     - **qubits_ancilla** The indices of the qubits to be used as ancilla qubits if use_ancilla=true [list of int]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
      .def(
          "controlled_swap",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_a,
              py::array_t<int> qubits_b, py::array_t<int> flags_on,
              py::array_t<int> flags_off) {
            builder.ControlledSwap(
                py_array_to_std_vec(qubits_a), py_array_to_std_vec(qubits_b),
                py_array_to_std_vec(flags_on), py_array_to_std_vec(flags_off));
          },
          py::arg("qubits_a"), py::arg("qubits_b"),
          py::arg("flags_on") = py::array_t<int>(),
          py::arg("flags_off") = py::array_t<int>(),
          R"(
      Controlled SWAP

     This method adds a controlled SWAP to the circuit.

     Performs a SWAP operation on a and b if an only if the controls are
     satisfied.

     Parameters:

     - **qubits_a** the indices of the qubits encoding a [list of int]
     - **qubits_b** the indices of the qubits encoding b [list of int]
     - **flags_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **flags_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
      .def(
          "controlled_ripple_carry_adder",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_adder,
              py::array_t<int> qubits_sum, int c_in, py::array_t<int> flags_on,
              py::array_t<int> flags_off, bool no_overflow) {
            builder.ControlledAddition(py_array_to_std_vec(qubits_adder),
                                       py_array_to_std_vec(qubits_sum), c_in,
                                       py_array_to_std_vec(flags_on),
                                       py_array_to_std_vec(flags_off),
                                       no_overflow);
          },
          py::arg("qubits_adder"), py::arg("qubits_sum"), py::arg("c_in"),
          py::arg("flags_on") = py::array_t<int>(),
          py::arg("flags_off") = py::array_t<int>(),
          py::arg("no_overflow") = false, R"(
      Controlled Addition

     This method adds a controlled ripple carry adder to the circuit.

     Performs a RippleAdd operation on adder_bits and sum_bits if and only
     if the controls are satisfied.

     Parameters:

     - **qubits_adder** the indices of the qubits encoding adder_bits [list of int]
     - **qubits_sum** the indices of the qubits encoding sum_bits [list of int]
     - **c_in** the index of the carry-in bit [int]
     - **flags_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **flags_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]
     - **no_overflow** Indicates that the total of the addition can be encoded on the same number of qubits as sum_bits without overflowing [bool]

    )")
      .def(
          "generalised_mcx",
          [&](qb::CircuitBuilder &builder, int target,
              py::array_t<int> controls_on, py::array_t<int> controls_off) {
            builder.GeneralisedMCX(target, py_array_to_std_vec(controls_on),
                                   py_array_to_std_vec(controls_off));
          },
          py::arg("target"), py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(),
          R"(
    Generalised MCX

   This method adds a generalised MCX gate to the circuit.

   By generalised MCX we mean that we allow the control qubits to be
   conditional on being off or conditional on being on.

   Parameters:

   - **target** The index of the target qubit [int]
   - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
   - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

  )")
      .def(
          "compare_beam_oracle",
          [&](qb::CircuitBuilder &builder, int q0, int q1, int q2,
              py::array_t<int> FA, py::array_t<int> FB, py::array_t<int> SA,
              py::array_t<int> SB, bool simplified) {
            builder.CompareBeamOracle(
                q0, q1, q2, py_array_to_std_vec(FA), py_array_to_std_vec(FB),
                py_array_to_std_vec(SA), py_array_to_std_vec(SB), simplified);
          },
          py::arg("q0"), py::arg("q1"), py::arg("q2"), py::arg("FA"),
          py::arg("FB"), py::arg("SA"), py::arg("SB") = py::array_t<int>(),
          py::arg("simplified") = true,
          R"(
      Compare Beam Oracle

     This method adds a compare beam oracle to the circuit.

     This method is required for the quantum decoder algorithm.
    )")
      .def(
          "inverse_circuit",
          [&](qb::CircuitBuilder &builder, py::object &circ) {
            qb::CircuitBuilder *casted_circ = circ.cast<qb::CircuitBuilder *>();
            builder.InverseCircuit(*casted_circ);
          },
          py::arg("circ"),
          R"(
    Inverse Circuit

   This method adds the inverse of a circuit to the current circuit.

   Given some collection of unitary operations,

   U = U_NU_{N-1}...U_2U_1

   this method appends the inverse to the circuit:

   U^{-1} = U_1dg U_2dg...U_{N-1}dg U_Ndg

   This may be useful for un-computing ancilla or for constructing Grovers
   operators.

   Parameters:

   - **circ** The circuit whose inverse we want to add to the current circuit [CircuitBuilder]

  )")
      .def(
          "comparator_as_oracle",
          [&](qb::CircuitBuilder &builder, int BestScore,
              int num_scoring_qubits, py::array_t<int> trial_score_qubits,
              int flag_qubit, py::array_t<int> best_score_qubits,
              py::array_t<int> ancilla_qubits, bool is_LSB,
              py::array_t<int> controls_on, py::array_t<int> controls_off) {
            builder.Comparator_as_Oracle(
                BestScore, num_scoring_qubits,
                py_array_to_std_vec(trial_score_qubits), flag_qubit,
                py_array_to_std_vec(best_score_qubits),
                py_array_to_std_vec(ancilla_qubits), is_LSB,
                py_array_to_std_vec(controls_on),
                py_array_to_std_vec(controls_off));
          },
          py::arg("best_score"), py::arg("num_scoring_qubits"),
          py::arg("trial_score_qubits") = py::array_t<int>(),
          py::arg("flag_qubit") = -1,
          py::arg("best_score_qubits") = py::array_t<int>(),
          py::arg("ancilla_qubits") = py::array_t<int>(),
          py::arg("is_LSB") = true, py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(),
          R"(
      Comparator as Oracle

     This method adds a quantum bit string comparator oracle to the circuit.

     The quantum bit string comparator is used to add a negative phase to any
     trial state whose bit string value is greater than the state being
     compared to. In this way it can be used as an oracle in a Grovers
     operator that amplifies higher scoring strings. This may be useful in
     many search problems.

     Parameters:

     - **best_score** The score we are comparing strings to [int]
     - **num_scoring_qubits** The number of qubits used to encode the scores [int]
     - **trial_score_qubits** The indices of the qubits encoding the trial states [list of int]
     - **flag_qubit** The index of the flag qubit which acquires a negative phase whenever trial score > BestScore [int]
     - **best_score_qubits** The indices of the qubits encoding the BestScore value [list of int]
     - **ancilla_qubits** The indices of the ancilla qubits required for the comparator circuit, if num_scoring_qubits = N we need 3N-1 ancilla [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
      .def(
          "multiplication",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_a,
              py::array_t<int> qubits_b, py::array_t<int> qubits_result,
              int qubit_ancilla, bool is_LSB) {
            builder.Multiplication(
                py_array_to_std_vec(qubits_a), py_array_to_std_vec(qubits_b),
                py_array_to_std_vec(qubits_result), qubit_ancilla, is_LSB);
          },
          py::arg("qubit_ancilla"), py::arg("qubits_a"), py::arg("qubits_b"),
          py::arg("qubits_result"), py::arg("is_LSB") = true,
          R"(
      Multiplication

     This method adds a Multiplication to the circuit.

     Given two inputs a and b, computes the product a*b and stores the result on a new register.

     Parameters:

     - **qubits_a** the indices of the qubits encoding a [list of int]
     - **qubits_b** the indices of the qubits encoding b [list of int]
     - **qubits_result** the indices of the qubits that will ecode the multiplication result [list of int]
     - **qubits_ancilla** the index of the single required ancilla [int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]

    )")
      .def(
          "controlled_multiplication",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_a,
              py::array_t<int> qubits_b, py::array_t<int> qubits_result,
              int qubit_ancilla, bool is_LSB, py::array_t<int> controls_on,
              py::array_t<int> controls_off) {
            builder.ControlledMultiplication(
                py_array_to_std_vec(qubits_a), py_array_to_std_vec(qubits_b),
                py_array_to_std_vec(qubits_result), qubit_ancilla, is_LSB,
                py_array_to_std_vec(controls_on),
                py_array_to_std_vec(controls_off));
          },
          py::arg("qubit_ancilla"), py::arg("qubits_a"), py::arg("qubits_b"),
          py::arg("qubits_result"), py::arg("is_LSB") = true,
          py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(), R"(
      Controlled Multiplication

     This method adds a controlled Multiplication to the circuit.

     Performs a Multiplication operation on a and b if an only if the
     controls are satisfied.

     Parameters:

     - **qubits_a** the indices of the qubits encoding a [list of int]
     - **qubits_b** the indices of the qubits encoding b [list of int]
     - **qubits_result** the indices of the qubits that will ecode the multiplication result [list of int]
     - **qubits_ancilla** the index of the single required ancilla [int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]

    )")
      .def(
          "superposition_adder",
          [&](qb::CircuitBuilder &builder, int q0, int q1, int q2,
              py::array_t<int> qubits_flags, py::array_t<int> qubits_string,
              py::array_t<int> qubits_metric, py::object &ae_state_prep_circ,
              py::array_t<int> qubits_ancilla,
              py::array_t<int> qubits_beam_metric) {
            qb::CircuitBuilder *casted_ae_state_prep_circ =
                ae_state_prep_circ.cast<qb::CircuitBuilder *>();
            assert(casted_ae_state_prep_circ);

            builder.SuperpositionAdder(
                q0, q1, q2, py_array_to_std_vec(qubits_flags),
                py_array_to_std_vec(qubits_string),
                py_array_to_std_vec(qubits_metric), *casted_ae_state_prep_circ,
                py_array_to_std_vec(qubits_ancilla),
                py_array_to_std_vec(qubits_beam_metric));
          },
          py::arg("q0"), py::arg("q1"), py::arg("q2"),
          py::arg("qubits_flags") = py::array_t<int>(),
          py::arg("qubits_string") = py::array_t<int>(),
          py::arg("qubits_metric") = py::array_t<int>(),
          py::arg("ae_state_prep_circ"),
          py::arg("qubits_ancilla") = py::array_t<int>(),
          py::arg("qubits_beam_metric") = py::array_t<int>(), R"(
      Superposition adder

     This method adds a Superposition Adder to the circuit.

     Given a superposition state, this circuit computes the mean of the amplitudes of the superposition components.

     Parameters:

     - **q0** the index of the single required ancilla [int]
     - **q1** the index of the single required ancilla [int]
     - **q2** the index of the single required ancilla [int]
     - **qubits_flags** the indices of the flag qubits [list of int]
     - **qubits_string** the indices of the qubits encoding the string [list of int]
     - **qubits_metric** the indices of the qubits encoding the metric value corresponding to the string [list of int]
     - **ae_state_prep_circ** The circuit A used to prepare the input state [CircuitBuilder]
     - **qubits_ancilla** the indices of the required ancilla qubits [list of int]
     - **qubits_beam_metric** the indices of the qubits encoding class' metric [list of int]

    )")
      .def(
          "exponential_search",
          [&](qb::CircuitBuilder &builder, py::str method,
              py::object state_prep, OracleFuncPyType oracle_func,
              int best_score, const std::function<int(int)> f_score,
              int total_num_qubits, py::array_t<int> qubits_string,
              py::array_t<int> total_metric, py::str acc_name) {
            qb::OracleFuncCType oracle_converted = [&](int best_score) {
              // Do conversion
              auto conv = oracle_func(best_score);
              return conv.get();
            };

            std::shared_ptr<xacc::CompositeInstruction> static_state_prep_circ;
            StatePrepFuncPyType state_prep_casted;
            qb::StatePrepFuncCType state_prep_func;
            try {
              qb::CircuitBuilder state_prep_casted =
                  state_prep.cast<qb::CircuitBuilder>();
              static_state_prep_circ = state_prep_casted.get();
              state_prep_func = [&](std::vector<int> a, std::vector<int> b,
                                    std::vector<int> c, std::vector<int> d,
                                    std::vector<int> e) {
                return static_state_prep_circ;
              };
            } catch (...) {
              state_prep_casted = state_prep.cast<StatePrepFuncPyType>();
              state_prep_func = [&](std::vector<int> qubits_string,
                                    std::vector<int> qubits_metric,
                                    std::vector<int> qubits_next_letter,
                                    std::vector<int> qubits_next_metric,
                                    std::vector<int> qubits_ancilla_adder) {
                // Do conversion
                auto conv = state_prep_casted(
                    std_vec_to_py_array(qubits_string),
                    std_vec_to_py_array(qubits_metric),
                    std_vec_to_py_array(qubits_next_letter),
                    std_vec_to_py_array(qubits_next_metric),
                    std_vec_to_py_array(qubits_ancilla_adder));
                return conv.get();
              };
            }

            return builder.ExponentialSearch(
                method, state_prep_func, oracle_converted, best_score, f_score,
                total_num_qubits, py_array_to_std_vec(qubits_string),
                py_array_to_std_vec(total_metric), acc_name);
          },
          py::arg("method"), py::arg("state_prep"), py::arg("oracle"),
          py::arg("best_score"), py::arg("f_score"),
          py::arg("total_num_qubits"), py::arg("qubits_string"),
          py::arg("total_metric"), py::arg("qpu") = "qpp",
          R"(
      Exponential Search

     This method sets up and executes the exponential search routine.

     Exponential search is a way to perform amplitude estimation when the
     size of the "good" subspace is unknown (so the number of Grovers
     operators to use is unknown).

     We implement three variants:
     - canonical exponential search is a specific "guess and check" method
     - MLQAE exponential search uses MLQAE to first estimate the size of the good subspace then perform regular amplitude estimation with the appropriate number of Grovers operators
     - CQAE exponential search uses canonical QAE to first estimate the size of the good subspace then perform regular amplitude estimation with the appropriate number of Grovers operators

     Parameters:

     - **method** indicates which method to use. Options are "canonical", "MLQAE", "CQAE" [string]
     - **state_prep** a function which produces the state prep circuit [StatePrepFuncCType]
     - **oracle** a function which produces the oracle circuit that marks the good subspace [OracleFuncCType]
     - **best_score** the current best score [int]
     - **f_score** a function that returns a 1 if the input binary string has value greater than the current best score and 0 otherwise [func(int)->int]
     - **total_num_qubits** total number of qubits [int]
     - **qubits_string** the indices of the qubits encoding the strings [list of int]
     - **total_metric** the indices of the qubits encoding the string scores after any required pre-processing of qubits_metric (required by decoder) [list of int]
     - **qpu** the name of the accelerator used to execute the algorithm [string]

     Returns: a better score if found, otherwise returns the current best
     score
    )")
      .def(
          "q_prime_unitary",
          [&](qb::CircuitBuilder &builder, int nb_qubits_ancilla_metric,
              int nb_qubits_ancilla_letter,
              int nb_qubits_next_letter_probabilities,
              int nb_qubits_next_letter) {
            builder.QPrime(nb_qubits_ancilla_metric, nb_qubits_ancilla_letter,
                           nb_qubits_next_letter_probabilities,
                           nb_qubits_next_letter);
          },
          py::arg("nb_qubits_ancilla_metric"),
          py::arg("nb_qubits_ancilla_letter"),
          py::arg("nb_qubits_next_letter_probabilities"),
          py::arg("nb_qubits_next_letter"), R"(
      Q' Unitary

     This method adds a Q' unitary to the circuit.

     Q' is a unitary required for the quantum decoder algorithm.
    )")
      .def(
          "subtraction",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_larger,
              py::array_t<int> qubits_smaller, bool is_LSB, int qubit_ancilla) {
            builder.Subtraction(py_array_to_std_vec(qubits_larger),
                                py_array_to_std_vec(qubits_smaller), is_LSB,
                                qubit_ancilla);
          },
          py::arg("qubits_larger"), py::arg("qubits_smaller"),
          py::arg("is_LSB") = true, py::arg("qubit_ancilla") = -1,
          R"(
      Subtraction

     This method adds a subtraction to the circuit.

     Given two inputs a and b, leaves b unchanged but maps a to the difference a-b, assuming a>b.

     Parameters:

     - **qubits_larger** the indices of the qubits encoding the larger value [list of int]
     - **qubits_smaller** the indices of the qubits encoding the smaller value [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **qubit_ancilla** the index of the required ancilla [list of int]

    )")
      .def(
          "controlled_subtraction",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_larger,
              py::array_t<int> qubits_smaller, py::array_t<int> controls_on,
              py::array_t<int> controls_off, bool is_LSB, int qubit_ancilla) {
            builder.ControlledSubtraction(py_array_to_std_vec(qubits_larger),
                                          py_array_to_std_vec(qubits_smaller),
                                          py_array_to_std_vec(controls_on),
                                          py_array_to_std_vec(controls_off),
                                          is_LSB, qubit_ancilla);
          },
          py::arg("qubits_larger"), py::arg("qubits_smaller"),
          py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(),
          py::arg("is_LSB") = true, py::arg("qubit_ancilla") = -1,
          R"(
      Controlled Subtraction

     This method adds a controlled subtraction to the circuit.

     Performs a subtraction operation on a and b if an only if the
     controls are satisfied.

     Parameters:

     - **qubits_larger** the indices of the qubits encoding the larger value [list of int]
     - **qubits_smaller** the indices of the qubits encoding the smaller value [list of int]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
     - **qubit_ancilla** the index of the required ancilla [list of int]

    )")
      .def(
          "proper_fraction_division",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_numerator,
              py::array_t<int> qubits_denominator,
              py::array_t<int> qubits_fraction, py::array_t<int> qubits_ancilla,
              bool is_LSB) {
            builder.ProperFractionDivision(
                py_array_to_std_vec(qubits_numerator),
                py_array_to_std_vec(qubits_denominator),
                py_array_to_std_vec(qubits_fraction),
                py_array_to_std_vec(qubits_ancilla), is_LSB);
          },
          py::arg("qubits_numerator"), py::arg("qubits_denominator"),
          py::arg("qubits_fraction"), py::arg("qubits_ancilla"),
          py::arg("is_LSB") = true,
          R"(
      Proper Fraction Division

     This method adds a proper fraction division to the circuit.

     Given two inputs num and denom, calculates num/denom and stores the result in a new register, assuming denom > num

     Parameters:

     - **qubits_numerator** the indices of the qubits encoding the numerator [list of int]
     - **qubits_denominator** the indices of the qubits encoding the denominator [list of int]
     - **qubits_fraction** the indices of the qubits that will ecode the division result [list of int]
     - **qubit_ancilla** the index of the required ancilla [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]

    )")
      .def(
          "controlled_proper_fraction_division",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_numerator,
              py::array_t<int> qubits_denominator,
              py::array_t<int> qubits_fraction, py::array_t<int> qubits_ancilla,
              py::array_t<int> controls_on, py::array_t<int> controls_off,
              bool is_LSB) {
            builder.ControlledProperFractionDivision(
                py_array_to_std_vec(qubits_numerator),
                py_array_to_std_vec(qubits_denominator),
                py_array_to_std_vec(qubits_fraction),
                py_array_to_std_vec(qubits_ancilla),
                py_array_to_std_vec(controls_on),
                py_array_to_std_vec(controls_off), is_LSB);
          },
          py::arg("qubits_numerator"), py::arg("qubits_denominator"),
          py::arg("qubits_fraction"), py::arg("qubits_ancilla"),
          py::arg("controls_on") = py::array_t<int>(),
          py::arg("controls_off") = py::array_t<int>(),
          py::arg("is_LSB") = true,
          R"(
      Controlled Proper Fraction Division

     This method adds a controlled proper fraction division to the circuit.

     Performs a PFD operation on a and b if an only if the controls are
     satisfied.

     Parameters:

     - **qubits_numerator** the indices of the qubits encoding the numerator [list of int]
     - **qubits_denominator** the indices of the qubits encoding the denominator [list of int]
     - **qubits_fraction** the indices of the qubits that will ecode the division result [list of int]
     - **qubit_ancilla** the index of the required ancilla [list of int]
     - **controls_on** The indices of any qubits that should be "on" controls (i.e. circuit executed if qubit = 1) [list of int]
     - **controls_off** The indices of any qubits that should be "off" controls (i.e. circuit executed if qubit = 0) [list of int]
     - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]

    )")
      .def(
          "compare_gt",
          [&](qb::CircuitBuilder &builder, py::array_t<int> qubits_a,
              py::array_t<int> qubits_b, int qubit_flag, int qubit_ancilla,
              bool is_LSB) {
            builder.CompareGT(py_array_to_std_vec(qubits_a),
                              py_array_to_std_vec(qubits_b), qubit_flag,
                              qubit_ancilla, is_LSB);
          },
          py::arg("qubits_a"), py::arg("qubits_b"), py::arg("qubit_flag"),
          py::arg("qubit_ancilla"), py::arg("is_LSB") = true,
          R"(
    Compare Greater Than

   This method adds a greater-than comparator to the circuit.

   Given two binary strings a and b, this comparator flips a flag qubit
   whenever a>b. This method uses far less ancilla than the more general
   comparator method provided.

   Parameters:

   - **qubits_a** The indices of the qubits encoding a [list of int]
   - **qubits_b** The indices of the qubits encoding b [list of int]
   - **qubit_flag** The index of the flag qubit that is flipped whenever a>b [int]
   - **qubit_ancilla** The index of the single ancilla qubit required [int]
   - **is_LSB** Indicates that the trial scores are encoded with LSB ordering [bool]
  )");
  m.def(
      "run_canonical_ae",
      [&](py::object &state_prep, py::object &grover_op, int precision,
          int num_state_prep_qubits, int num_trial_qubits,
          py::array_t<int> precision_qubits, py::array_t<int> trial_qubits,
          py::str acc_name) {
        qb::CircuitBuilder builder;

        qb::CircuitBuilder *casted_state_prep =
            state_prep.cast<qb::CircuitBuilder *>();
        assert(state_prep);

        qb::CircuitBuilder *casted_grover_op =
            grover_op.cast<qb::CircuitBuilder *>();
        assert(casted_grover_op);

        return builder.RunCanonicalAmplitudeEstimation(
            *casted_state_prep, *casted_grover_op, precision,
            num_state_prep_qubits, num_trial_qubits,
            py_array_to_std_vec(precision_qubits),
            py_array_to_std_vec(trial_qubits), acc_name);
      },
      py::arg("state_prep"), py::arg("grover_op"), py::arg("precision"),
      py::arg("num_state_prep_qubits"), py::arg("num_trial_qubits"),
      py::arg("precision_qubits") = py::array_t<int>(),
      py::arg("trial_qubits") = py::array_t<int>(), py::arg("qpu") = "qpp",
      "Execute Canonical Quantum Amplitude Estimation Procedure with "
      "pre-constructed Grover operator circuit, including "
      "post-processing.");
  m.def(
      "run_canonical_ae_with_oracle",
      [&](py::object &state_prep, py::object &oracle, int precision,
          int num_state_prep_qubits, int num_trial_qubits,
          py::array_t<int> precision_qubits, py::array_t<int> trial_qubits,
          py::str acc_name) {
        qb::CircuitBuilder builder;
        qb::CircuitBuilder *casted_state_prep =
            state_prep.cast<qb::CircuitBuilder *>();
        assert(state_prep);

        qb::CircuitBuilder *casted_oracle = oracle.cast<qb::CircuitBuilder *>();
        assert(casted_oracle);
        return builder.RunCanonicalAmplitudeEstimationWithOracle(
            *casted_state_prep, *casted_oracle, precision,
            num_state_prep_qubits, num_trial_qubits,
            py_array_to_std_vec(precision_qubits),
            py_array_to_std_vec(trial_qubits), acc_name);
      },
      py::arg("state_prep"), py::arg("grover_op"), py::arg("precision"),
      py::arg("num_state_prep_qubits"), py::arg("num_trial_qubits"),
      py::arg("precision_qubits") = py::array_t<int>(),
      py::arg("trial_qubits") = py::array_t<int>(), py::arg("qpu") = "qpp",
      "Execute Canonical Quantum Amplitude Estimation procedure for the "
      "oracle "
      "including "
      "post-processing.");
  m.def(
      "run_MLQAE",
      [&](py::object &state_prep, py::object &oracle,
          std::function<int(std::string, int)> is_in_good_subspace,
          py::array_t<int> score_qubits, int total_num_qubits, int num_runs,
          int shots, py::str acc_name) {
        qb::CircuitBuilder builder;
        qb::CircuitBuilder *casted_state_prep =
            state_prep.cast<qb::CircuitBuilder *>();
        assert(state_prep);

        qb::CircuitBuilder *casted_oracle = oracle.cast<qb::CircuitBuilder *>();
        assert(casted_oracle);
        return builder.RunMLAmplitudeEstimation(
            *casted_state_prep, *casted_oracle, is_in_good_subspace,
            py_array_to_std_vec(score_qubits), total_num_qubits, num_runs,
            shots, acc_name);
      },
      py::arg("state_prep"), py::arg("oracle"), py::arg("is_in_good_subspace"),
      py::arg("score_qubits"), py::arg("total_num_qubits"),
      py::arg("num_runs") = 4, py::arg("shots") = 100, py::arg("qpu") = "qpp",
      "MLQAE");
  m.def(
      "exponential_search",
      [&](py::str method, py::object state_prep, OracleFuncPyType oracle_func,
          int best_score, const std::function<int(int)> f_score,
          int total_num_qubits, py::array_t<int> qubits_string,
          py::array_t<int> total_metric, py::str acc_name) {
        qb::CircuitBuilder builder;
        qb::OracleFuncCType oracle_converted = [&](int best_score) {
          // Do conversion
          auto conv = oracle_func(best_score);
          return conv.get();
        };

        std::shared_ptr<xacc::CompositeInstruction> static_state_prep_circ;
        StatePrepFuncPyType state_prep_casted;
        qb::StatePrepFuncCType state_prep_func;
        try {
          qb::CircuitBuilder state_prep_casted =
              state_prep.cast<qb::CircuitBuilder>();
          static_state_prep_circ = state_prep_casted.get();
          state_prep_func = [&](std::vector<int> a, std::vector<int> b,
                                std::vector<int> c, std::vector<int> d,
                                std::vector<int> e) {
            return static_state_prep_circ;
          };
        } catch (...) {
          state_prep_casted = state_prep.cast<StatePrepFuncPyType>();
          state_prep_func = [&](std::vector<int> qubits_string,
                                std::vector<int> qubits_metric,
                                std::vector<int> qubits_next_letter,
                                std::vector<int> qubits_next_metric,
                                std::vector<int> qubits_ancilla_adder) {
            // Do conversion
            auto conv =
                state_prep_casted(std_vec_to_py_array(qubits_string),
                                  std_vec_to_py_array(qubits_metric),
                                  std_vec_to_py_array(qubits_next_letter),
                                  std_vec_to_py_array(qubits_next_metric),
                                  std_vec_to_py_array(qubits_ancilla_adder));
            return conv.get();
          };
        }
        return builder.ExponentialSearch(
            method, state_prep_func, oracle_converted, best_score, f_score,
            total_num_qubits, py_array_to_std_vec(qubits_string),
            py_array_to_std_vec(total_metric), acc_name);
      },
      py::arg("method"), py::arg("state_prep"), py::arg("oracle"),
      py::arg("best_score"), py::arg("f_score"), py::arg("total_num_qubits"),
      py::arg("qubits_string"), py::arg("total_metric"), py::arg("qpu") = "qpp",
      "Exp Search");
}
} // namespace qb
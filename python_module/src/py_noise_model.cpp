// Copyright (c) Quantum Brilliance Pty Ltd

#include "py_noise_model.hpp"
#include "qb/core/noise_model/noise_model.hpp"
#include "py_stl_containers.hpp"

namespace qb {
void bind_noise_model(pybind11::module &m) {
  namespace py = pybind11;
  py::class_<qb::KrausOperator>(m, "KrausOperator")
      .def(py::init<>())
      .def_readwrite("matrix", &qb::KrausOperator::matrix, R"(Kraus matrix)")
      .def_readwrite("qubits", &qb::KrausOperator::qubits,
                     R"(Qubits that this Kraus operator acts on.)");

  py::class_<qb::AmplitudeDampingChannel>(
      m, "AmplitudeDampingChannel", R"(Amplitude damping channel factory)")
      .def(py::init<>())
      .def_readonly_static("name", &qb::AmplitudeDampingChannel::name)
      .def("Create", &qb::AmplitudeDampingChannel::Create);

  py::class_<qb::PhaseDampingChannel>(m, "PhaseDampingChannel",
                                      R"(Phase damping channel factory)")
      .def(py::init<>())
      .def_readonly_static("name", &qb::PhaseDampingChannel::name)
      .def("Create", &qb::PhaseDampingChannel::Create);

  py::class_<qb::DepolarizingChannel>(m, "DepolarizingChannel",
                                      R"(Depolarizing channel factory)")
      .def(py::init<>())
      .def_readonly_static("name", &qb::DepolarizingChannel::name)
      .def("Create",
           py::overload_cast<size_t, double>(&qb::DepolarizingChannel::Create),
           R"(

      Create single-qubit depolarizing channel (balanced/symmetric)

      Parameters: 

      - *q* Qubit index
      - *p* Total depolarizing probability

      )")
      .def("Create",
           py::overload_cast<size_t, size_t, double>(
               &qb::DepolarizingChannel::Create),
           R"(

      Create two-qubit depolarizing channel (balanced/symmetric)

      Parameters: 

      - *q1* First qubit
      - *q2* Second qubit
      - *p* Total depolarizing probability

      )");

  py::class_<qb::GeneralizedPhaseAmplitudeDampingChannel>(
      m, "GeneralizedPhaseAmplitudeDampingChannel",
      R"(Generalized Single-qubit combined phase and amplitude damping quantum error channel)")
      .def(py::init<>())
      .def_readonly_static("name",
                           &qb::GeneralizedPhaseAmplitudeDampingChannel::name)
      .def("Create", &qb::GeneralizedPhaseAmplitudeDampingChannel::Create, R"(

      Create a generalized amplitude and phase damping channel

      Parameters:

      - *q* Qubit
      - *excited_state_population* Excited state population
      - *param_amp* Amplitude damping parameter
      - *param_phase* Phase damping parameter
    
    )");

  py::class_<qb::GeneralizedAmplitudeDampingChannel>(
      m, "GeneralizedAmplitudeDampingChannel",
      R"(Generalized amplitude damping quantum error channel)")
      .def(py::init<>())
      .def_readonly_static("name",
                           &qb::GeneralizedAmplitudeDampingChannel::name)
      .def("Create", &qb::GeneralizedAmplitudeDampingChannel::Create, R"(

      Create a generalized amplitude damping channel

      Parameters:

      - *q* Qubit
      - *excited_state_population* Excited state population
      - *param_amp* Amplitude damping parameter
    
    )");

  py::class_<qb::NoiseProperties>(m, "NoiseProperties", R"(

    Use NoiseProperties to accept user input parameters for custom noise models.  There are 3 types of inputs used for constructing a custom noise model:
      - Qubit topology
      - Time duration of quantum gate operations
      - Parameters for quantum noise channels and classical errors
    
    )")
      .def(py::init<>())
      .def_readwrite("t1_us", &qb::NoiseProperties::t1_us, R"(
        :math:`T_1` is the *qubit relaxation time*.

        For a qubit register, with individual qubits zero-indexed by `i`; `t1_us` is a map from qubit[i] -> T1[i].

        Unit: microseconds
        
        Code example: 4 qubits all with T1 = 1.5us::
          
          # Initialize an empty NoiseProperties
          t_qbnp = NoiseProperties()
          # Set T1 of qubits (all with 1.5 us)
          for i in range(4):
            t_qbnp.t1_us[i] =  1.5 
      )")
      .def_readwrite("t2_us", &qb::NoiseProperties::t2_us, R"(
        :math:`T_2` is the *qubit dephasing time*.

        For a qubit register, with individual qubits zero-indexed by `i`; `t2_us` is a map from qubit[i] -> T2[i].

        Unit: microseconds
        
        Code example: 4 qubits all with T2 = 0.15us::
          
          # Initialize an empty NoiseProperties
          t_qbnp = NoiseProperties()
          # Set T2 of qubits (all with 0.15 us)
          for i in range(4):
            t_qbnp.t2_us[i] =  0.15 
      )")
      .def_readwrite("readout_errors", &qb::NoiseProperties::readout_errors, R"(
        `readout_errors` is the *classical readout error* (off-diagonal elements of the confusion matrix).  
        
        For a qubit register, with individual qubits zero-indexed by i, `readout_errors` is a map from qubit[i] -> `ReadoutError[i]`.

        Unit: none (quantities are probabilities).
        
        Code example: 4-qubit device: 2 qubits (Q0 and Q1) with p(0|1) = p(1|0) = 0.05, 
        2 qubits (Q2 and Q3) with p(0|1) = 0.1 and p(1|0) = 0.08::
          
          # Initialize an empty NoiseProperties
          t_qbnp = NoiseProperties()
          t_qbnpro_balanced = ReadoutError()
          t_qbnpro_balanced.p_01 = 0.05
          t_qbnpro_balanced.p_10 = 0.05
          t_qbnpro_asym = ReadoutError()
          t_qbnpro_asym.p_01 = 0.10
          t_qbnpro_asym.p_10 = 0.08
          # Q0 and Q1 readout errors
          t_qbnp.readout_errors[0] = t_qbnpro_balanced
          t_qbnp.readout_errors[1] = t_qbnpro_balanced
          # Q2 and Q3 readout errors
          t_qbnp.readout_errors[2] = t_qbnpro_asym
          t_qbnp.readout_errors[3] = t_qbnpro_asym
      )")
      .def_readwrite("gate_time_us", &qb::NoiseProperties::gate_time_us, R"(
        `gate_time_us` is the duration for a quantum gate operation when applied at a target set of qubits.
        
        Unit: microseconds
        
        Code example: 4 qubits: "u3" single-qubit gate, uniform duration of 5.2us; "cx" between neighboring qubits (on a line), uniform duration of 20us::
          
          # Initialize an empty NoiseProperties
          t_qbnp = NoiseProperties()
          t_qbnp.gate_time_us["u3"] = {}
          t_qbnp.gate_time_us["cx"] = {}
          num_qubits = 4
          for i in range(num_qubits):
            t_qbnp.gate_time_us["u3"][[i]] = 5.2
            # Qubits on a line:
            if i != num_qubits - 1:
              t_qbnp.gate_time_us["cx"][[i, i + 1]] = 20.0

          # Print out the gate time map:
          print(t_qbnp.gate_time_us)

      )")
      .def_readwrite("gate_pauli_errors",
                     &qb::NoiseProperties::gate_pauli_errors, R"(
        `gate_pauli_errors` is the parameter for gate error derived from randomized benchmarking of a quantum gate operation that is applied at a target set of qubits.

        Unit: none (range: [0.0, 1.0])
        
        Code example: 4 qubits: "u3" single-qubit gate, uniform gate error parameter = 0.03 (3%); "cx" between neighboring qubits (on a line), gate error parameter = 0.1 (10%)::
          
          # Initialize an empty NoiseProperties
          t_qbnp = NoiseProperties()
          t_qbnp.gate_pauli_errors["u3"] = {}
          t_qbnp.gate_pauli_errors["cx"] = {}
          num_qubits = 4
          for i in range(num_qubits):
            t_qbnp.gate_pauli_errors["u3"][[i]] = 0.03
            # Qubits on a line:
            if i != num_qubits - 1:
              t_qbnp.gate_pauli_errors["cx"][[i, i + 1]] = 0.1

          # Print out the gate error map:
          print(t_qbnp.gate_pauli_errors)
      )")
      .def_readwrite("qubit_topology", &qb::NoiseProperties::qubit_topology, R"(
        `qubit_topology` is a graph comprised of directed edges {control qubit, target qubit} with control qubit as the source of the edge -> target qubit as the destination of the edge.
        
        Code example: "cx" symmetrical two-qubit gate with 4 qubits in the topology below::
          
          # Topology 
          #    q0 <--cx--> q1
          #     ^           ^
          #     |           |
          #     cx          cx
          #     |           |
          #     v           v
          #    q3 <--cx--> q2
          # Initialize an empty NoiseProperties
          t_qbnp = NoiseProperties()
          t_qbnp.qubit_topology = [[0, 1], [1, 2], [2, 3], [3, 0]]
      )");

  py::class_<qb::ReadoutError>(
      m, "ReadoutError",
      R"(Probabilities of reading out a value for a qubit that does not reflect its true state.)")
      .def(py::init<>())
      .def_readwrite(
          "p_01", &qb::ReadoutError::p_01,
          R"(Classical probability of detecting 0 whereas the true state was :math:`|1\rangle`)")
      .def_readwrite(
          "p_10", &qb::ReadoutError::p_10,
          R"(Classical probability of detecting 1 whereas the true state was :math:`|0\rangle`)");

  py::class_<qb::NoiseModel> nm(
      m, "NoiseModel",
      R"(Noise model class allowing specification of noise parameters and connectivity for each gate.)");
  nm.def(py::init<>())
      //.def(py::init<const nlohmann::json&>())
      .def(py::init<const NoiseProperties &>())
      .def(py::init<const std::string &, size_t,
                    std::optional<qb::NoiseModel::QubitConnectivity>,
                    std::optional<std::reference_wrapper<
                        const std::vector<std::pair<size_t, size_t>>>>>(),
           py::arg("name"), py::arg("nb_qubits"),
           py::arg("connectivity") = std::nullopt,
           py::arg("connected_pairs") = std::nullopt)
      .def("to_json", &qb::NoiseModel::to_json,
           R"(Convert noise model to json string)")
      .def("add_gate_error", &qb::NoiseModel::add_gate_error, R"(
  
        Add a gate error channel for a gate operation
  
        Parameters: 
        
        - *noise_channel* Noise channel to be associated with the gate [List(KraussOperator)]
        - *gate_name* Name of the gates [String]
        - *qubits* Qubit indices of the gate. [qb.core.N]
  
        )")
      // Note that this can be removed when the silly aliases in typedefs.hpp
      // are deleted.
      .def(
          "add_gate_error",
          [](qb::NoiseModel *model, const NoiseChannel &noise_channel,
             const std::string &gate_name, const py::list qubits) {
            std::vector<size_t> v;
            for (auto &x : qubits)
              v.emplace_back(py::cast<size_t>(x));
            model->add_gate_error(noise_channel, gate_name, v);
          },
          R"(        Overload of add_gate_error that takes *qubits* directly as List(Integer).)")
      .def("set_qubit_readout_error", &qb::NoiseModel::set_qubit_readout_error,
           R"(
  
        Set the qubit readout error 
  
        Parameters: 
        
        - *qubitIdx* Qubit to set [Integer]
        - *ro_error* Readout error [ReadoutError]
  
        )")
      .def("add_qubit_connectivity", &qb::NoiseModel::add_qubit_connectivity,
           R"(
  
        Add a connected qubit pair to the topology model
  
        Parameters: 
        
        - *q1* First qubit index [Integer]
        - *q2* Second qubit index [Integer]
  
        )")
      .def_property_readonly(
          "connectivity", &qb::NoiseModel::get_connectivity,
          R"(Get connectivity as a list of connected qubit pairs)")
      .def_property(
          "qobj_compiler", &qb::NoiseModel::get_qobj_compiler,
          &qb::NoiseModel::set_qobj_compiler,
          R"(The name of the QObj compiler to use with the AER simulator. Valid options: 'xacc-qobj' | 'qristal-qobj'.)")
      .def_property_readonly(
          "qobj_basis_gates", &qb::NoiseModel::get_qobj_basis_gates,
          R"(The list of basis gates that the AER QObj will be referring to.)")
      .def_readwrite("name", &qb::NoiseModel::name,
                     R"(The colloquial name of the noise model)");

  py::enum_<qb::NoiseModel::QubitConnectivity>(nm, "QubitConnectivity",
                                               R"(Type of qubit connectivity)")
      .value("AllToAll", qb::NoiseModel::QubitConnectivity::AllToAll)
      .value("Custom", qb::NoiseModel::QubitConnectivity::Custom)
      .export_values();
}
} // namespace qb
#include <qristal/core/python/py_benchmark_workflows.hpp>
#include <qristal/core/python/py_help_strings_benchmark_workflows.hpp>
#include <qristal/core/python/py_stl_containers.hpp>

//workflows
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp>
#include <qristal/core/benchmark/workflows/QuantumStateTomography.hpp>
#include <qristal/core/benchmark/workflows/QuantumProcessTomography.hpp>

#include <pybind11/eigen.h>

using namespace qristal::benchmark;
namespace py = pybind11;

namespace qristal {
  void bind_SPAMBenchmark(pybind11::module &m) {
    py::class_<SPAMBenchmark>(m, "SPAMBenchmark")
      .def(py::init<const std::set<size_t>&, qristal::session&>())
      .def(
        "execute", 
        &SPAMBenchmark::execute, 
        py::arg("tasks"), 
        qristal::help::benchmark::execute_
      )
      .def(
        "execute_all", 
        &SPAMBenchmark::execute_all,
        qristal::help::benchmark::execute_all_
      )
      .def_property_readonly(
        "qubits", 
        &SPAMBenchmark::get_qubits, 
        qristal::help::benchmark::qubits_
      )
      .def_property_readonly(
        "identifier",  
        &qristal::benchmark::SPAMBenchmark::get_identifier, 
        qristal::help::benchmark::identifier_
      )
      .def_property_readonly(
        "circuits", 
        &qristal::benchmark::SPAMBenchmark::get_circuits, 
        qristal::help::benchmark::circuits_
      )
      .def_property(
        "session", 
        &qristal::benchmark::SPAMBenchmark::get_session, 
        &qristal::benchmark::SPAMBenchmark::set_session, 
        qristal::help::benchmark::session_
      )
      .def(
        "calculate_confusion_matrix",
        [](const SPAMBenchmark& self, const std::vector<std::map<std::vector<bool>, int>>& counts) {
          return self.calculate_confusion_matrix(counts);
        },
        py::arg("counts"), 
        qristal::help::benchmark::SPAMBenchmark_::calculate_confusion_matrix_
      );
  }


  void bind_RotationSweep(pybind11::module &m) {
    py::class_<RotationSweep>(m, "RotationSweep")
     .def(py::init<const std::vector<char>&, const int&, const int&,const size_t&, qristal::session&>())
     .def(
        "execute", 
        &RotationSweep::execute, 
        qristal::help::benchmark::execute_
      )
     .def(
        "execute_all", 
        &RotationSweep::execute_all, 
        qristal::help::benchmark::execute_all_
      )
     .def_property_readonly(
        "identifier",  
        &RotationSweep::get_identifier,
        qristal::help::benchmark::identifier_
      )
     .def_property_readonly(
        "circuits", 
        &RotationSweep::get_circuits, 
        qristal::help::benchmark::circuits_
      )
      .def_property(
        "session", 
        &RotationSweep::get_session, 
        &RotationSweep::set_session, 
        qristal::help::benchmark::session_
      )
     .def(
        "start_rad", 
        &RotationSweep::start_rad, 
        qristal::help::benchmark::RotationSweep_::start_rad_
      )
     .def(
        "end_rad", 
        &RotationSweep::end_rad, 
        qristal::help::benchmark::RotationSweep_::end_rad_
      )
     .def(
        "step", 
        &RotationSweep::step ,
        qristal::help::benchmark::RotationSweep_::step_
      )
     .def(
        "get_rotations_per_qubit", 
        &RotationSweep::get_rotations_per_qubit, 
        qristal::help::benchmark::RotationSweep_::get_rotations_per_qubit_
      );    
  }


  void bind_SimpleCircuitExecution(pybind11::module &m) {
    py::class_<SimpleCircuitExecution>(m, "SimpleCircuitExecution")
      .def(py::init<const std::vector<qristal::CircuitBuilder>&, qristal::session&>())
      .def(py::init<const qristal::CircuitBuilder&, qristal::session&>())
      .def(
        "execute", 
        &SimpleCircuitExecution::execute, 
        qristal::help::benchmark::execute_
      )
      .def(
        "execute_all", 
        &SimpleCircuitExecution::execute_all, 
        qristal::help::benchmark::execute_all_
      )
      .def_property_readonly(
        "identifier",  
        &SimpleCircuitExecution::get_identifier,
        qristal::help::benchmark::identifier_
      )
     .def_property_readonly(
        "circuits", 
        &SimpleCircuitExecution::get_circuits, 
        qristal::help::benchmark::circuits_
      )
      .def_property(
        "session", 
        &SimpleCircuitExecution::get_session, 
        &SimpleCircuitExecution::set_session, 
        qristal::help::benchmark::session_
      );
  }

  
  void bind_PyGSTiBenchmark(pybind11::module &m) {
    py::class_<PyGSTiBenchmark>(m, "PyGSTiBenchmark")
     .def(py::init<const std::vector<std::string>&, qristal::session&>())
     .def(py::init<const std::string&, qristal::session&>())
     .def(
        "execute", 
        &PyGSTiBenchmark::execute, 
        qristal::help::benchmark::execute_
      )
      .def(
        "execute_all", 
        &PyGSTiBenchmark::execute_all, 
        qristal::help::benchmark::execute_all_
      )
      .def_property_readonly(
        "identifier",  
        &PyGSTiBenchmark::get_identifier,
        qristal::help::benchmark::identifier_
      )
     .def_property_readonly(
        "circuits", 
        &PyGSTiBenchmark::get_circuits, 
        qristal::help::benchmark::circuits_
      )
      .def_property(
        "session", 
        &PyGSTiBenchmark::get_session, 
        &PyGSTiBenchmark::set_session, 
        qristal::help::benchmark::session_
      )
     .def_property_readonly(
        "pyGSTi_circuit_strings", 
        &PyGSTiBenchmark::get_pyGSTi_circuit_strings,
        qristal::help::benchmark::PyGSTiBenchmark_::pyGSTi_circuit_strings_
      );
  }


  void bind_QuantumStateTomography(pybind11::module &m) {
    py::class_<QuantumStateTomographyPython>(m, "QuantumStateTomography")
      //specialized constructors
      .def(py::init<SPAMBenchmark&,
                    const std::set<size_t>&,
                    const bool, 
                    const std::vector<Pauli>&,
                    const Pauli&>(),
        py::arg("workflow"),
        py::arg("qubits"),
        py::arg("perform_maximum_likelihood_estimation") = false,
        py::arg("basis") = std::vector<Pauli>{Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z},
        py::arg("use_for_identity") = Pauli(Pauli::Symbol::Z)
      )
      .def(py::init<SPAMBenchmark&,
                    const bool,
                    const std::vector<Pauli>&,
                    const Pauli&>(),
        py::arg("workflow"),
        py::arg("perform_maximum_likelihood_estimation") = false,
        py::arg("basis") = std::vector<Pauli>({Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z}),
        py::arg("use_for_identity") = Pauli(Pauli::Symbol::Z)
      )
      .def(py::init<RotationSweep&,
                    const std::set<size_t>&,
                    const bool,
                    const std::vector<Pauli>&,
                    const Pauli&>(),
        py::arg("workflow"),
        py::arg("qubits"),
        py::arg("perform_maximum_likelihood_estimation") = false,
        py::arg("basis") = std::vector<Pauli>({Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z}),
        py::arg("use_for_identity") = Pauli(Pauli::Symbol::Z)
      )
      .def(py::init<RotationSweep&,
                    const bool,
                    const std::vector<Pauli>&,
                    const Pauli&>(),
        py::arg("workflow"),
        py::arg("perform_maximum_likelihood_estimation") = false,
        py::arg("basis") = std::vector<Pauli>({Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z}),
        py::arg("use_for_identity") = Pauli(Pauli::Symbol::Z)
      )
      .def(py::init<PyGSTiBenchmark&,
                    const std::set<size_t>&,
                    const bool,
                    const std::vector<Pauli>&,
                    const Pauli&>(),
        py::arg("workflow"),
        py::arg("qubits"),
        py::arg("perform_maximum_likelihood_estimation") = false,
        py::arg("basis") = std::vector<Pauli>({Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z}),
        py::arg("use_for_identity") = Pauli(Pauli::Symbol::Z)
      )
      .def(py::init<PyGSTiBenchmark&,
                    const bool,
                    const std::vector<Pauli>&,
                    const Pauli&>(),
        py::arg("workflow"),
        py::arg("perform_maximum_likelihood_estimation") = false,
        py::arg("basis") = std::vector<Pauli>({Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z}),
        py::arg("use_for_identity") = Pauli(Pauli::Symbol::Z)
      )
      .def(py::init<SimpleCircuitExecution&,
                    const std::set<size_t>&,
                    const bool,
                    const std::vector<Pauli>&,
                    const Pauli&>(),
        py::arg("workflow"),
        py::arg("qubits"),
        py::arg("perform_maximum_likelihood_estimation") = false,
        py::arg("basis") = std::vector<Pauli>({Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z}),
        py::arg("use_for_identity") = Pauli(Pauli::Symbol::Z)
      )
      .def(py::init<SimpleCircuitExecution&,
                    const bool,
                    const std::vector<Pauli>&,
                    const Pauli&>(),
        py::arg("workflow"),
        py::arg("perform_maximum_likelihood_estimation") = false,
        py::arg("basis") = std::vector<Pauli>({Pauli::Symbol::X, Pauli::Symbol::Y, Pauli::Symbol::Z}),
        py::arg("use_for_identity") = Pauli(Pauli::Symbol::Z)
      )
      //type-erased member functions
      .def(
        "execute", 
        &QuantumStateTomographyPython::execute, 
        py::arg("tasks"), 
        qristal::help::benchmark::execute_
      )
      .def(
        "execute_all", 
        &QuantumStateTomographyPython::execute_all,
        qristal::help::benchmark::execute_all_
      )
      .def_property_readonly(
        "qubits", 
        &QuantumStateTomographyPython::get_qubits, 
        qristal::help::benchmark::qubits_
      )
      .def_property_readonly(
        "identifier",  
        &QuantumStateTomographyPython::get_identifier, 
        qristal::help::benchmark::identifier_
      )
      .def_property_readonly(
        "get_basis", 
        &QuantumStateTomographyPython::get_basis<Pauli>, 
        qristal::help::benchmark::QuantumStateTomography_::get_basis_
      )
      .def(
          "set_maximum_likelihood_estimation", 
          [&](
            QuantumStateTomographyPython& self, 
            const size_t n_MLE_iterations, 
            const double MLE_conv_threshold, 
            const std::map<Pauli, std::vector<ComplexMatrix>>& mBasisSymbols_to_Projectors
          ) {
            return self.set_maximum_likelihood_estimation(n_MLE_iterations, MLE_conv_threshold, mBasisSymbols_to_Projectors);
          }, 
          py::arg("n_MLE_iterations") = 100,
          py::arg("MLE_conv_threshold") = 1e-3,
          py::arg("mBasisSymbols_to_Projectors") = std::map<Pauli, std::vector<ComplexMatrix>>{
            {Pauli::Symbol::X, std::vector<ComplexMatrix>{BlochSphereUnitState(BlochSphereUnitState::Symbol::Xp).get_matrix(), BlochSphereUnitState(BlochSphereUnitState::Symbol::Xm).get_matrix()}},
            {Pauli::Symbol::Y, std::vector<ComplexMatrix>{BlochSphereUnitState(BlochSphereUnitState::Symbol::Yp).get_matrix(), BlochSphereUnitState(BlochSphereUnitState::Symbol::Ym).get_matrix()}},
            {Pauli::Symbol::Z, std::vector<ComplexMatrix>{BlochSphereUnitState(BlochSphereUnitState::Symbol::Zp).get_matrix(), BlochSphereUnitState(BlochSphereUnitState::Symbol::Zm).get_matrix()}}
         }, 
         qristal::help::benchmark::QuantumStateTomography_::set_maximum_likelihood_estimation_
      )
      .def(
          "assemble_densities",  
          [&](
            const QuantumStateTomographyPython& self, 
            const std::vector<std::map<std::vector<bool>, int>>& counts
          ) {
            return self.assemble_densities(counts);
          },
          py::arg("counts"),
          qristal::help::benchmark::QuantumStateTomography_::assemble_densities_
      );
  }


  void bind_QuantumProcessTomography(pybind11::module &m) {
    py::class_<QuantumProcessTomographyPython>(m, "QuantumProcessTomography")
      .def(py::init<QuantumStateTomographyPython&, const std::vector<BlochSphereUnitState>&>(), 
        py::arg("qstworkflow"), 
        py::arg("states") = std::vector<BlochSphereUnitState>{BlochSphereUnitState::Symbol::Zp, BlochSphereUnitState::Symbol::Zm, BlochSphereUnitState::Symbol::Xp, BlochSphereUnitState::Symbol::Ym}
      )
      .def(
        "execute", 
        &QuantumProcessTomographyPython::execute, 
        py::arg("tasks"), 
        qristal::help::benchmark::execute_
      )
      .def(
        "execute_all", 
        &QuantumProcessTomographyPython::execute_all,
        qristal::help::benchmark::execute_all_
      )
      .def_property_readonly(
        "identifier",  
        &QuantumProcessTomographyPython::get_identifier, 
        qristal::help::benchmark::identifier_
      )
      .def(
          "assemble_processes",
          [&](
            QuantumProcessTomographyPython& self, 
            const std::vector<ComplexMatrix>& densities
          ) {
            return self.assemble_processes(densities);
          },
          py::arg("densities"),
          qristal::help::benchmark::QuantumProcessTomography_::assemble_processes_
      );
  }


}
#include <qristal/core/python/py_benchmark.hpp>
#include <qristal/core/python/py_help_strings_benchmark.hpp>

#include <qristal/core/benchmark/Task.hpp>
#include <qristal/core/primitives.hpp>

using namespace qristal::benchmark;
namespace py = pybind11;

namespace qristal {
  void bind_Task(pybind11::module &m) {
    py::enum_<Task>(m, "Task")
      .value("MeasureCounts", Task::MeasureCounts)
      .value("IdealCounts", Task::IdealCounts)
      .value("IdealDensity", Task::IdealDensity)
      .value("IdealProcess", Task::IdealProcess)
      .value("Session", Task::Session)
      .export_values();
      
    m.def(
      "get_identifier", 
      &get_identifier, 
      py::arg("task"),
      qristal::help::benchmark::Task_::get_identifier_
    );
  }

  void bind_Pauli(pybind11::module& m) {
    py::enum_<Pauli::Symbol>(m, "PauliSymbol")
      .value("I", Pauli::Symbol::I)
      .value("X", Pauli::Symbol::X)
      .value("Y", Pauli::Symbol::Y)
      .value("Z", Pauli::Symbol::Z)
      .export_values(); 

    py::class_<Pauli>(m, "Pauli")
      .def(py::init<const Pauli::Symbol &>(), py::arg("symbol"))
      .def(
        "get_matrix", 
        &Pauli::get_matrix, 
        qristal::help::benchmark::Pauli_::get_matrix_
      )
      .def_property_readonly(
        "symbol", 
        &Pauli::get_symbol,
        qristal::help::benchmark::Pauli_::symbol_
      )
      .def("__eq__", &Pauli::operator==, py::is_operator())
      .def("__lt__", &Pauli::operator<, py::is_operator())
      .def(
        "__repr__", 
        [](const Pauli &p) {
          std::ostringstream oss;
          oss << p; 
          return oss.str();
        }
      )
      .def(
        "__hash__", 
        [](const Pauli &p) {
          return static_cast<std::size_t>(p.get_symbol());
        }
      )
      .def(
        "append_circuit", 
        [](const Pauli& self, qristal::CircuitBuilder& cb, const size_t qubit) {
          return self.append_circuit(cb, qubit);
        },
        py::arg("circuit"), 
        py::arg("qubit"), 
        qristal::help::benchmark::Pauli_::append_circuit_
      );
  }

  void bind_BlochSphereUnitState(pybind11::module& m) {
    py::enum_<BlochSphereUnitState::Symbol>(m, "BlochSphereUnitStateSymbol")
      .value("Zp", BlochSphereUnitState::Symbol::Zp)
      .value("Zm", BlochSphereUnitState::Symbol::Zm)
      .value("Xp", BlochSphereUnitState::Symbol::Xp)
      .value("Xm", BlochSphereUnitState::Symbol::Xm)
      .value("Yp", BlochSphereUnitState::Symbol::Yp)
      .value("Ym", BlochSphereUnitState::Symbol::Ym)
      .export_values(); 

    py::class_<BlochSphereUnitState>(m, "BlochSphereUnitState")
      .def(py::init<const BlochSphereUnitState::Symbol &>(), py::arg("symbol"))
      .def(
        "get_matrix", 
        &BlochSphereUnitState::get_matrix, 
        qristal::help::benchmark::BlochSphereUnitState_::get_matrix_
      )
      .def_property_readonly(
        "symbol", 
        &BlochSphereUnitState::get_symbol,
        qristal::help::benchmark::BlochSphereUnitState_::symbol_
      )
      .def(
        "__repr__", 
        [](const BlochSphereUnitState &bus) {
          std::ostringstream oss;
          oss << bus; 
          return oss.str();
        }
      )
      .def(
        "append_circuit", 
        [](const BlochSphereUnitState& self, qristal::CircuitBuilder& cb, const size_t qubit) {
          return self.append_circuit(cb, qubit);
        },
        py::arg("circuit"), 
        py::arg("qubit"), 
        qristal::help::benchmark::BlochSphereUnitState_::append_circuit_
      );
  }
}
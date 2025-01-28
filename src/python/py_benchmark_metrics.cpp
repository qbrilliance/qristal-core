#include "qristal/core/python/py_benchmark_metrics.hpp"
#include "qristal/core/python/py_help_strings_benchmark_metrics.hpp"
#include "qristal/core/python/py_stl_containers.hpp"

//workflows (needed for constructor specialization)
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/workflows/RotationSweep.hpp"
#include "qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp"

//metrics 
#include "qristal/core/benchmark/metrics/CircuitFidelity.hpp"
#include "qristal/core/benchmark/metrics/PyGSTiResults.hpp"
#include "qristal/core/benchmark/metrics/ConfusionMatrix.hpp"
#include "qristal/core/benchmark/metrics/QuantumStateDensity.hpp"
#include "qristal/core/benchmark/metrics/QuantumProcessMatrix.hpp"
#include "qristal/core/benchmark/metrics/QuantumStateFidelity.hpp"
#include "qristal/core/benchmark/metrics/QuantumProcessFidelity.hpp"

#include <pybind11/eigen.h>

namespace py = pybind11;
using namespace qristal::benchmark;

namespace qristal {

  void bind_CircuitFidelity(pybind11::module &m) {
    py::class_<CircuitFidelityPython>(m, "CircuitFidelity")
      //specialized constructors
      .def(py::init<SPAMBenchmark&>())
      .def(py::init<RotationSweep&>())
      //type-erased member functions
      .def(
        "evaluate", 
        [&](const CircuitFidelityPython& self, const bool force_new) {
          return self.evaluate(force_new);
        }, 
        py::arg("force_new") = false,
        qristal::help::benchmark::evaluate_
      );
  }

  void bind_PyGSTiResults(pybind11::module &m) {
    py::class_<PyGSTiResultsPython>(m, "PyGSTiResults")
      //specialized constructors
      .def(py::init<PyGSTiBenchmark&>())
      //type-erased member functions
      .def(
        "evaluate", 
        [&](const PyGSTiResultsPython& self, const bool force_new, const bool verbose) {
          return self.evaluate(force_new, verbose);
        }, 
        py::arg("force_new") = false,
        py::arg("verbose") = false,
        qristal::help::benchmark::evaluate_
      );
  }

  void bind_ConfusionMatrix(pybind11::module &m) {
    py::class_<ConfusionMatrixPython>(m, "ConfusionMatrix")
      //specialized constructors
      .def(py::init<SPAMBenchmark&>())
      //type-erased member functions
      .def(
        "evaluate", 
        [&](const ConfusionMatrixPython& self, const bool force_new) {
          return self.evaluate(force_new);
        }, 
        py::arg("force_new") = false,
        qristal::help::benchmark::evaluate_
      );
  }

  void bind_QuantumStateDensity(pybind11::module &m) {
    py::class_<QuantumStateDensityPython>(m, "QuantumStateDensity")
      .def(py::init<QuantumStateTomographyPython&>(), py::arg("qstworkflow"))
      .def(
        "evaluate", 
        [&](const QuantumStateDensityPython& self, const bool force_new) {
          return self.evaluate(force_new);
        }, 
        py::arg("force_new") = false,
        qristal::help::benchmark::evaluate_
      );
  }

  void bind_QuantumProcessMatrix(pybind11::module &m) {
    py::class_<QuantumProcessMatrixPython>(m, "QuantumProcessMatrix")
      .def(py::init<QuantumProcessTomographyPython&>(), py::arg("qptworkflow"))
      .def(
        "evaluate", 
        [&](const QuantumProcessMatrixPython& self, const bool force_new) {
          return self.evaluate(force_new);
        }, 
        py::arg("force_new") = false,
        qristal::help::benchmark::evaluate_
      );
  }

  void bind_QuantumStateFidelity(pybind11::module &m) {
    py::class_<QuantumStateFidelityPython>(m, "QuantumStateFidelity")
      .def(py::init<QuantumStateTomographyPython&>(), py::arg("qstworkflow"))
      .def(
        "evaluate", 
        [&](const QuantumStateFidelityPython& self, const bool force_new) {
          return self.evaluate(force_new);
        }, 
        py::arg("force_new") = false,
        qristal::help::benchmark::evaluate_
      );
  }

  void bind_QuantumProcessFidelity(pybind11::module &m) {
    py::class_<QuantumProcessFidelityPython>(m, "QuantumProcessFidelity")
      .def(py::init<QuantumProcessTomographyPython&>(), py::arg("qptworkflow"))
      .def(
        "evaluate", 
        [&](const QuantumProcessFidelityPython& self, const bool force_new) {
          return self.evaluate(force_new);
        }, 
        py::arg("force_new") = false,
        qristal::help::benchmark::evaluate_
      );
  }

}

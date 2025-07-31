#pragma once
#include <pybind11/pybind11.h>

namespace qristal {
  // Bind qristal::benchmark::SPAMBenchmark class to Python API.
  void bind_SPAMBenchmark(pybind11::module &m);

  // Bind qristal::benchmark::RotationSweep class to Python API.
  void bind_RotationSweep(pybind11::module &m);

  // Bind qristal::benchmark::SimpleCircuitExecution class to Python API.
  void bind_SimpleCircuitExecution(pybind11::module &m);

  // Bind qristal::benchmark::PyGSTiBenchmark class to Python API.
  void bind_PyGSTiBenchmark(pybind11::module &m);

  // Bind qristal::benchmark::QuantumStateTomography class to Python API.
  void bind_QuantumStateTomography(pybind11::module &m);

  // Bind qristal::benchmark::QuantumProcessTomography class to Python API.
  void bind_QuantumProcessTomography(pybind11::module &m);

  // Bind qristal::benchmark::AddinFromIdealSimulation class to Python API. 
  void bind_AddinFromIdealSimulation(pybind11::module &m);
}

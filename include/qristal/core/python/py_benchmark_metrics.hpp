#pragma once
#include <pybind11/pybind11.h>

namespace qristal {
  // Bind qristal::benchmark::CircuitFidelity class to Python API.
  void bind_CircuitFidelity(pybind11::module &m);

  // Bind qristal::benchmark::ConfusionMatrix class to Python API.
  void bind_ConfusionMatrix(pybind11::module &m);

  // Bind qristal::benchmark::PyGSTiResults class to Python API.
  void bind_PyGSTiResults(pybind11::module &m);

  // Bind qristal::benchmark::QuantumProcessFidelity class to Python API.
  void bind_QuantumProcessFidelity(pybind11::module &m);

  // Bind qristal::benchmark::QuantumProcessMatrix class to Python API.
  void bind_QuantumProcessMatrix(pybind11::module &m);

  // Bind qristal::benchmark::QuantumStateDensity class to Python API.
  void bind_QuantumStateDensity(pybind11::module &m);

  // Bind qristal::benchmark::QuantumStateFidelity class to Python API.
  void bind_QuantumStateFidelity(pybind11::module &m);
}

#pragma once
#include <pybind11/pybind11.h>

#include <qristal/core/python/py_benchmark_workflows.hpp>
#include <qristal/core/python/py_benchmark_metrics.hpp>

namespace qristal {
  // Bind qristal::benchmark::Task class to Python API.
  void bind_Task(pybind11::module &m);

  // Bind qristal::Pauli class to Python API.
  void bind_Pauli(pybind11::module &m);

  // Bind qristal::BlochSphereUnitState class to Python API
  void bind_BlochSphereUnitState(pybind11::module &m);
}

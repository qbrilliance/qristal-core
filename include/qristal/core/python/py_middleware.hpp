// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace qristal {
  /// Bind circuit placement passes to Python API.
  void bind_placement_passes(pybind11::module &m);
  /// Bind circuit optimization passes to Python API.
  void bind_circuit_opt_passes(pybind11::module &m);
}

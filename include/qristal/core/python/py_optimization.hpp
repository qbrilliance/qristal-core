// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <pybind11/pybind11.h>

namespace qristal {
  /// Bind VQEE-related types/functions to the optimization sub-module.
  void bind_vqee(pybind11::module &opt_m);
  /// Bind the simple QAOA class to the optimization sub-module.
}

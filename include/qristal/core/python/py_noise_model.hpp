// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <pybind11/pybind11.h>

namespace qristal {
  /// Bind noise modelling-related types
  void bind_noise_model(pybind11::module &m);
}

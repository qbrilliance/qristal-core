// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <pybind11/pybind11.h>
#include <qristal/core/noise_model/readout_error.hpp>

PYBIND11_MAKE_OPAQUE(qristal::ReadoutError);
PYBIND11_MAKE_OPAQUE(std::unordered_map<size_t, qristal::ReadoutError>);

namespace qristal {
  /// Bind noise modelling-related types
  void bind_noise_model(pybind11::module &m);
}

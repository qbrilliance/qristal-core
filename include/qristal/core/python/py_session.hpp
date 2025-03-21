// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace qristal {
  /// Bind session class to the Python module
  void bind_session(pybind11::module &m);
}

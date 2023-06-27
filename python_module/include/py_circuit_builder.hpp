// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <pybind11/pybind11.h>

namespace qb {
/// Bind circuit builder class to Python API.
void bind_circuit_builder(pybind11::module &m);
} // namespace qb
// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <pybind11/pybind11.h>

namespace qb {
/// Bind VQEE-related types/functions to the optimization sub-module.
void bind_vqee(pybind11::module &opt_m);
/// Bind QML-related types to the optimization sub-module.
void bind_qml(pybind11::module &opt_m);
/// Bind the simple QAOA class to the optimization sub-module.
void bind_qaoa_simple(pybind11::module &opt_m);
/// Bind the recursive QAOA class to the optimization sub-module.
void bind_qaoa_recursive(pybind11::module &opt_m);
/// Bind the warm-start QAOA class to the optimization sub-module.
void bind_qaoa_warm_start(pybind11::module &opt_m);
} // namespace qb
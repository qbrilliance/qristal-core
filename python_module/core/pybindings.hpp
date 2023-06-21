// Copyright (c) Quantum Brilliance Pty Ltd

// This file declares sub-component bindings (e.g., for a particular C++ class) of the overall module
// so that we can break the module binding into multiple cpp files. All PYBIND11_MAKE_OPAQUE invocations
// should appear in this header in order to avoid violations of the One Definition Rule.

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>
#include "qb/core/passes/noise_aware_placement_config.hpp"

PYBIND11_MAKE_OPAQUE(std::vector<int>);
PYBIND11_MAKE_OPAQUE(std::vector<std::string>);
PYBIND11_MAKE_OPAQUE(std::vector<size_t>);
PYBIND11_MAKE_OPAQUE(std::vector<bool>);
PYBIND11_MAKE_OPAQUE(std::map<int, double>);
PYBIND11_MAKE_OPAQUE(std::map<int, std::complex<double>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::map<int, double>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::map<int, std::complex<double>>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<size_t>>);
PYBIND11_MAKE_OPAQUE(std::map<std::string, double>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<std::string>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<bool>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<std::map<int, std::complex<double>>>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<std::map<int, double>>>);
PYBIND11_MAKE_OPAQUE(qb::noise_aware_placement_config::device_topology_t);
PYBIND11_MAKE_OPAQUE(qb::noise_aware_placement_config::single_qubit_gate_errors_t);
PYBIND11_MAKE_OPAQUE(qb::noise_aware_placement_config::two_qubit_gate_errors_t);
PYBIND11_MAKE_OPAQUE(std::unordered_map<std::string, std::map<std::vector<size_t>, double>>);
PYBIND11_MAKE_OPAQUE(std::map<std::vector<size_t>, double>);

namespace qb {
/// Bind circuit placement passes to Python API.
void bind_placement_passes(pybind11::module &m);
/// Bind circuit optimization passes to Python API.
void bind_circuit_opt_passes(pybind11::module &m);
} // namespace qb

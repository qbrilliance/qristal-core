// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include "qb/core/passes/noise_aware_placement_config.hpp"
#include <map>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <vector>

PYBIND11_MAKE_OPAQUE(std::vector<std::string>);
PYBIND11_MAKE_OPAQUE(std::vector<size_t>);
PYBIND11_MAKE_OPAQUE(std::vector<bool>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<size_t>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<std::string>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<bool>>);
PYBIND11_MAKE_OPAQUE(std::map<int, double>);
PYBIND11_MAKE_OPAQUE(std::map<int, std::complex<double>>);
PYBIND11_MAKE_OPAQUE(std::map<std::vector<bool>, int>);
PYBIND11_MAKE_OPAQUE(std::vector<std::map<int, double>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::map<int, std::complex<double>>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::map<std::vector<bool>, int>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<std::map<int, std::complex<double>>>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<std::map<int, double>>>);
PYBIND11_MAKE_OPAQUE(std::vector<std::vector<std::map<std::vector<bool>, int>>>);
PYBIND11_MAKE_OPAQUE(std::map<std::vector<size_t>, double>);
PYBIND11_MAKE_OPAQUE(std::unordered_map<std::string, std::map<std::vector<size_t>, double>>);
PYBIND11_MAKE_OPAQUE(qb::noise_aware_placement_config::device_topology_t);
PYBIND11_MAKE_OPAQUE(qb::noise_aware_placement_config::single_qubit_gate_errors_t);
PYBIND11_MAKE_OPAQUE(qb::noise_aware_placement_config::two_qubit_gate_errors_t);

namespace qb {
/// Bind opaque STL containers to the Python API.
void bind_opaque_containers(pybind11::module &m);

/// Helper to convert from a Python array to a std::vector
template <typename T>
std::vector<T> py_array_to_std_vec(pybind11::array_t<T> input) {
  pybind11::buffer_info buf = input.request();
  if (buf.ndim != 1) {
    throw std::runtime_error("Number of dimensions must be one");
  }
  std::vector<T> result;
  result.reserve(buf.shape[0]);
  T *ptr = static_cast<T *>(buf.ptr);
  for (size_t idx = 0; idx < buf.shape[0]; ++idx) {
    result.emplace_back(ptr[idx]);
  }

  return result;
}

/// Helper to convert std::vector<int> to a Python array
pybind11::array_t<int> std_vec_to_py_array(const std::vector<int> &input);
} // namespace qb

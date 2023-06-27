// Copyright (c) Quantum Brilliance Pty Ltd
#include "py_middleware.hpp"
#include "qb/core/circuit_builder.hpp"
#include "qb/core/passes/noise_aware_placement_config.hpp"
#include "qb/core/passes/noise_aware_placement_pass.hpp"
#include "qb/core/passes/swap_placement_pass.hpp"
#include "py_stl_containers.hpp"


namespace qb {
void bind_placement_passes(pybind11::module &m) {
  pybind11::bind_vector<noise_aware_placement_config::device_topology_t>(
      m, "device_topology");
  pybind11::bind_map<noise_aware_placement_config::single_qubit_gate_errors_t>(
      m, "single_qubit_gate_error");
  pybind11::bind_map<noise_aware_placement_config::two_qubit_gate_errors_t>(
      m, "two_qubit_gate_error");
  pybind11::class_<noise_aware_placement_config>(
      m, "noise_aware_placement_config",
      "The noise_aware_placement_config class encapsulates generic backend "
      "information required by the noise-aware placement pass.")
      .def(pybind11::init<>())
      .def_readwrite("connectivity",
                     &noise_aware_placement_config::qubit_connectivity)
      .def_readwrite(
          "single_qubit_gate_errors",
          &noise_aware_placement_config::avg_single_qubit_gate_errors)
      .def_readwrite("two_qubit_gate_errors",
                     &noise_aware_placement_config::avg_two_qubit_gate_errors)
      .def_readwrite("readout_errors",
                     &noise_aware_placement_config::avg_qubit_readout_errors);

  pybind11::class_<noise_aware_placement_pass,
                   std::shared_ptr<noise_aware_placement_pass>>(
      m, "noise_aware_placement_pass",
      "The noise_aware_placement_pass class uses device connectivity, gate "
      "errors (1-q and 2-q) and readout errors to find the best placement map.")
      .def(pybind11::init<noise_aware_placement_config &>(),
           "Construct a noise_aware_placement_pass object.\n"
           "\nArgs:\n"
           "  noise_aware_placement_config: Placement configurations "
           "(connectivity, readout and gate errors)",
           pybind11::arg("noise_aware_placement_config"))
      .def("apply", &noise_aware_placement_pass::apply,
           "Apply noise-aware placement on the input circuit.\n"
           "\nArgs:\n"
           "  circuit: Circuit to be placed (map qubit indices and inject SWAP "
           "gates as necessary)",
           pybind11::arg("circuit"));

  pybind11::class_<swap_placement_pass, std::shared_ptr<swap_placement_pass>>(
      m, "swap_placement_pass",
      "Circuit placement pass based on injection of SWAP gates to satisfy "
      "device connectivity topology.")
      .def(pybind11::init<std::vector<std::pair<int, int>> &>(),
           "Construct a swap_placement_pass object.\n"
           "\nArgs:\n"
           "  connectivity: Device connectivity information as a list of "
           "pairs.\n"
           "  For example, [(0, 1), (1, 2), ...]",
           pybind11::arg("connectivity"))
      .def("apply", &swap_placement_pass::apply,
           "Apply SWAP-based placement on the input circuit.\n"
           "\nArgs:\n"
           "  circuit: Circuit to be placed (map qubit indices and inject SWAP "
           "gates as necessary)",
           pybind11::arg("circuit"));
}
} // namespace qb

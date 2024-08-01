// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/python/py_middleware.hpp"
#include "qristal/core/python/py_stl_containers.hpp"
#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/passes/circuit_opt_passes.hpp"

namespace qristal {
void bind_circuit_opt_passes(pybind11::module &m) {
  pybind11::class_<CircuitPass, std::shared_ptr<CircuitPass>>(
      m, "CircuitPass", "Base circuit IR transformation pass")
      .def("apply", &CircuitPass::apply,
           "Apply the circuit optimization pass on the input circuit.\n"
           "\nArgs:\n"
           "  circuit: Circuit to be optimized",
           pybind11::arg("circuit"));

  m.def("circuit_optimizer", &qristal::create_circuit_optimizer_pass,
        "Generic pattern-based circuit optimization pass.");
  m.def("redundancy_removal", &qristal::create_remove_redundancies_pass,
        "Circuit optimization pass that removes gate-inverse pairs, merges "
        "rotations and removes identity rotations.");
  m.def("two_qubit_squash", &qristal::create_two_qubit_squash_pass,
        "Circuit optimization pass that squashes together sequences of single- "
        "and two-qubit gates into minimal form.");
  m.def("peephole_optimisation", &qristal::create_peephole_pass,
        "Circuit optimization pass that performs peephole optimisation.");
  m.def(
      "simplify_initial", &qristal::create_initial_state_simplify_pass,
      "Circuit optimization pass that performs contextual circuit optimisation "
      "based on known input states.\nNote: The simplified circuit is input "
      "dependent, thus this pass should **only** be used on the entire circuit "
      "(i.e., the qubit register is at the all 0's state) and should **not** "
      "be used on sub-circuits.");
}
}

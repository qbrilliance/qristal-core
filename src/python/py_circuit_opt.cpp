// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/python/py_middleware.hpp>
#include <qristal/core/python/py_stl_containers.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/passes/circuit_opt_passes.hpp>

namespace qristal {
void bind_circuit_opt_passes(pybind11::module &m) {
  pybind11::class_<CircuitPass, std::shared_ptr<CircuitPass>>(
      m, "CircuitPass", "Base circuit IR transformation pass")
      .def("apply", &CircuitPass::apply,
           "Apply the circuit optimization pass on the input circuit.\n"
           "\nArgs:\n"
           "  circuit: Circuit to be optimized",
           pybind11::arg("circuit"));

  pybind11::class_<sequence_pass, std::shared_ptr<sequence_pass>>(
      m, "sequence_pass","Sequence optimization")
      .def(pybind11::init<const std::vector<std::string> &>(),
            "Construct a sequence pass object.\n",
            pybind11::arg("pass_list"))
      .def("apply", &sequence_pass::apply,
            "Apply sequence optimization placement on the input circuit.\n"
            "\nArgs:\n",
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
  m.def("decompose_swap", &qristal::create_decompose_swap_pass,
      "Decomposes all SWAP gates into triples of CX gates.");
  m.def("commute_through_multis", &qristal::create_commute_through_multis_pass,
      "Moves single-qubit gates forward past multi-qubit gates they commute with, "
      "simplifying the circuit.");
  m.def("optimise_post_routing", &qristal::create_optimise_post_routing_pass,
      "Optimises the circuit after qubit routing by removing redundant gates and "
      "simplifying sequences, preserving hardware connectivity.");
  m.def("rebase_to_rzrx", &qristal::create_decompose_ZX_pass,
        "Rebases single-qubit gates into equivalent sequences of Rz and Rx gates.");
  m.def("rebase_to_clifford", &qristal::create_rebase_to_clifford_pass,
        "Replaces single-qubit gates that are Clifford but not in the basic set "
        "{Z, X, S, V} with equivalent gate sequences only using those four.");
  m.def("optimise_cliffords", &qristal::create_optimise_cliffords_pass,
        "Optimizes Clifford gate sequences using rewrite rules to reduce circuit "
        "depth and size");
}
}

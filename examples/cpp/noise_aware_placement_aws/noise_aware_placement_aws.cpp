// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/circuit_builder.hpp"
#include "qb/core/session.hpp"
#include "qb/core/cmake_variables.hpp"
#include <iostream>
#include <string>
#include <pybind11/embed.h>

int main() {
  // Since this will call AWS Python SDK (from C++), make sure that we have an interpreter session.
  pybind11::scoped_interpreter guard{};
  // And we're off!
  std::cout << "Executing C++ placement demo..." << std::endl;

  // Make a QB SDK session
  auto my_sim = qb::session(false);

  // Set up sensible default parameters
  my_sim.qb12();

  // Choose a AWS backend, set device to "Rigetti" to use its hardware info for
  // noise-aware placement
  my_sim.set_acc("aws_acc");
  my_sim.set_remote_backend_database_path(SDK_DIR "/examples/cpp/noise_aware_placement_aws/aws_rigetti.yaml");
  // Don't submit the circuit to AWS for execution.
  // Note: there is no charge when querying backend information (still needs a
  // valid AWS Braket account). Actual circuit execution, on the other hand,
  // will incur a cost.
  my_sim.set_nosim(true);

  // Choose noise-aware placement strategy
  my_sim.set_placement("noise-aware");

  // Choose how many qubits to simulate
  my_sim.set_qn(2);

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(100);

  qb::CircuitBuilder my_circuit;
  // Create a simple Bell state circuit.
  my_circuit.H(0);
  my_circuit.CNOT(0, 1);
  my_circuit.MeasureAll(2);
  std::cout << "Original quantum circuit:" << std::endl;
  my_circuit.print();

  // Set the input circuit
  my_sim.set_irtarget_m(my_circuit.get());
  my_sim.run();
  std::cout << "Ran successfully!" << std::endl;
  std::cout << "Placed circuit (for Rigetti device):" << std::endl;
  my_circuit.print();
}

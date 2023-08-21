// Copyright (c) Quantum Brilliance Pty Ltd

#include "qb/core/session.hpp"
#include "qb/core/circuit_builder.hpp"

#include <string>
#include <iostream>

int main()
{

  // And we're off!
  std::cout << "Executing C++ demo..." << std::endl;

  // Make a QB SDK session
  auto my_sim = qb::session(false);

  // Set up sensible default parameters
  my_sim.qb12();

  // Choose a CUDAQ simulator backend, e.g., custatevec_fp32 (single-precision)
  my_sim.set_acc("cudaq:custatevec_fp32");

  // Choose how many qubits to simulate
  constexpr int num_qubits = 10;
  my_sim.set_qn(num_qubits);

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(1024);

  qb::CircuitBuilder circ;
  std::vector<int> qft_qubits(num_qubits);
  // Fill the qubit list with 0, 1, ..., n-1
  // i.e., the qubits that we want to apply the QFT circuit to.
  std::iota(std::begin(qft_qubits), std::end(qft_qubits), 0);
  // Apply QFT
  circ.QFT(qft_qubits);
  circ.MeasureAll(num_qubits);
  // Hand the CircuitBuilder over to the sim object
  my_sim.set_irtarget_m(circ.get());

  // Run the circuit 1024 times and count up the results in each of the classical registers
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();
  std::cout << "Ran successfully!" << std::endl;

  // Print the cumulative results in each of the classical registers
  std::cout << "Results:" << std::endl << my_sim.get_out_raws()[0][0] << std::endl;

}

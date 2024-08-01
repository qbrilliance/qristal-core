// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/session.hpp"
#include "qristal/core/circuit_builder.hpp"

#include <string>
#include <iostream>

int main()
{

  // And we're off!
  std::cout << "Executing C++ demo..." << std::endl;

  // Make a Qristal session
  auto my_sim = qristal::session(false);

  // Set up sensible default parameters
  my_sim.init();

  // Choose a CUDAQ simulator backend, e.g., custatevec_fp32 (single-precision)
  my_sim.set_acc("cudaq:custatevec_fp32");

  // Choose how many qubits to simulate
  constexpr int num_qubits = 10;
  my_sim.set_qn(num_qubits);

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(1024);

  qristal::CircuitBuilder circ;
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
  std::cout << "Results:" << std::endl << my_sim.results()[0][0] << std::endl;

}

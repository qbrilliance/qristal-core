// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/session.hpp>
#include <qristal/core/circuit_builder.hpp>

#include <string>
#include <iostream>

int main()
{

  // And we're off!
  std::cout << "Executing C++ demo..." << std::endl;

  // Make a Qristal session
  qristal::session my_sim;

  // Choose a CUDAQ simulator backend, e.g., custatevec_fp32 (single-precision)
  my_sim.acc = "cudaq:custatevec_fp32";
  my_sim.gpu_device_ids = {0};

  // Choose how many qubits to simulate
  constexpr int num_qubits = 10;
  my_sim.qn = num_qubits;

  // Choose how many 'shots' to run through the circuit
  my_sim.sn = 1024;

  qristal::CircuitBuilder circ;
  std::vector<int> qft_qubits(num_qubits);
  // Fill the qubit list with 0, 1, ..., n-1
  // i.e., the qubits that we want to apply the QFT circuit to.
  std::iota(std::begin(qft_qubits), std::end(qft_qubits), 0);
  // Apply QFT
  circ.QFT(qft_qubits);
  circ.MeasureAll(num_qubits);
  // Hand the CircuitBuilder over to the sim object
  my_sim.irtarget = circ.get();

  // Run the circuit 1024 times and count up the results in each of the classical registers
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();
  std::cout << "Ran successfully!" << std::endl;

  // Print the cumulative results in each of the classical registers
  std::cout << "Results:" << std::endl << my_sim.results() << std::endl;

}

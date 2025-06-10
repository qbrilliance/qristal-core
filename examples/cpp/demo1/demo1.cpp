// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/session.hpp>
#include <iostream>
#include <string>

int main()
{
  // And we're off!
  std::cout << "Executing C++ demo..." << std::endl;

  // Make a Qristal session
  auto my_sim = qristal::session();

  // Choose a simulator backend
  my_sim.acc = "qpp";

  // Choose how many qubits to simulate
  my_sim.qn = 2;

  // Choose how many 'shots' to run through the circuit
  my_sim.sn = 100;

  // Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit')
  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      x q[1];
      measure q[0] -> c[0];
      measure q[1] -> c[1];
    }
    )";

  // Hand the kernel over to the sim object
  my_sim.instring = targetCircuit;

  // Run the circuit 100 times and count up the results in each of the classical
  // registers
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();
  std::cout << "Ran successfully!" << std::endl;

  // Print the cumulative results in each of the classical registers
  std::cout << "Results:" << std::endl << my_sim.results() << std::endl;
}

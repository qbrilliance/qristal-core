// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/session.hpp>

#include <fmt/base.h>

#include <iostream>
#include <string>

int main()
{
    // Make a Qristal session
  auto my_sim = qristal::session(false);

  my_sim.supervisor_print("Executing Qristal MPI Demo...");

  // Choose a simulator backend
  my_sim.acc = "qpp";

  // Choose how many qubits to simulate
  my_sim.qn = 2;

  // Choose how many 'shots' to run through the circuit
  my_sim.sn = 1000000;

  // Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit') and hand the kernel over to the sim object
  my_sim.instring = R"(
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

  // Run the circuit and count up the results in each of the classical
  // registers
  my_sim.supervisor_print("About to run quantum program...\n");
  my_sim.run();
  my_sim.supervisor_print("Ran successfully!\n");

  // Print the cumulative results in each of the classical registers
  my_sim.supervisor_print("Results:\n{}\n", my_sim.results());
}

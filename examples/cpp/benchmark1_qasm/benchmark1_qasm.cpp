// Copyright (c) Quantum Brilliance Pty Ltd
#include "qb/core/session.hpp"
#include <string>
#include <iostream>

int main()
{

  // And we're off!
  std::cout << "Executing C++ demo..." << std::endl;

  // Make a QB SDK session
  auto my_sim = qb::session(false);

  // Set up sensible default parameters
  my_sim.init();

  // Choose a simulator backend
  my_sim.set_acc("qpp");

  // Choose how many qubits to simulate
  my_sim.set_qn(20);

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(20000);

  // Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit')
  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[20];
      h q[0];
      cx q[0],q[1];
      cx q[1],q[2];
      cx q[2],q[3];
      cx q[3],q[4];
      cx q[4],q[5];
      cx q[5],q[6];
      cx q[6],q[7];
      cx q[7],q[8];
      cx q[8],q[9];
      cx q[9],q[10];
      cx q[10],q[11];
      cx q[11],q[12];
      cx q[12],q[13];
      cx q[13],q[14];
      cx q[14],q[15];
      cx q[15],q[16];
      cx q[16],q[17];
      cx q[17],q[18];
      cx q[18],q[19];
      measure q[0] -> c[0];
      measure q[1] -> c[1];
      measure q[2] -> c[2];
      measure q[3] -> c[3];
      measure q[4] -> c[4];
      measure q[5] -> c[5];
      measure q[6] -> c[6];
      measure q[7] -> c[7];
      measure q[8] -> c[8];
      measure q[9] -> c[9];
      measure q[10] -> c[10];
      measure q[11] -> c[11];
      measure q[12] -> c[12];
      measure q[13] -> c[13];
      measure q[14] -> c[14];
      measure q[15] -> c[15];
      measure q[16] -> c[16];
      measure q[17] -> c[17];
      measure q[18] -> c[18];
      measure q[19] -> c[19];
    }
    )";

  // Hand the kernel over to the sim object
  my_sim.set_instring(targetCircuit);

  // Run the circuit 200 times and count up the results in each of the classical registers
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();
  std::cout << "Ran successfully!" << std::endl;

  // Print the cumulative results in each of the classical registers
  std::cout << "Results:" << std::endl << my_sim.results()[0][0] << std::endl;

}

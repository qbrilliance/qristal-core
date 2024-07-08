// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#include "qb/core/session.hpp"
#include <string>
#include <iostream>

int main(int argc, char **argv)
{
  std::cout << "h1qb : single Hadamard gate demo... \n\n"<<"* To run on hardware QPU add the option: --qdk" << std::endl;

  // Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit')
  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      measure q[1] -> c[1];
      measure q[0] -> c[0];
    }
    )";
  std::cout << "\n" << targetCircuit << "\n";

  // Process the input arguments
  std::vector<std::string> arguments(argv + 1, argv + argc);
  
  // Make a QB SDK session
  auto my_sim = qb::session(false);

  // Set up sensible default parameters
  my_sim.init();

  // Choose a simulator backend
  for (int i = 0; i < arguments.size(); i++) {
    if (arguments[i] == "--qdk") {  
      my_sim.set_acc("qdk_gen1");
    } else {
      my_sim.set_acc("qpp");
    }
  }

  // Choose how many qubits to simulate
  my_sim.set_qn(2);

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(32);


  // Hand the kernel over to the sim object
  my_sim.set_instring(targetCircuit);

  // std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  // Print the cumulative results in each of the classical registers
  std::cout << "Results:" << std::endl << my_sim.get_out_raws_json()[0][0] << std::endl;
  // Print the walltime used to execute the quantum circuit
  std::cout << "\n" << "* Time used for circuit execution, in ms: " << (my_sim.get_out_total_init_maxgate_readout_times()[0][0]).at(4)<< "\n";
}

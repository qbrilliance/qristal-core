// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/session.hpp"
#include <string>
#include <iostream>

int main()
{
  // Make a QB SDK session
  auto s = qb::session(false);

  // Set up default parameters
  s.qb12();

  // Set the number of qubits
  const int n_qubits = 2;
  s.set_qn(n_qubits);

  // Set the number of shots
  const int n_shots = 1000;
  s.set_sn(n_shots);

  // Use the MPS emulator backend.
  // You will need to have the Qristal emulator installed for this to work.
  s.set_acc("qb-mps");

  // Set backend parameters
  s.set_initial_bond_dimension(1);
  s.set_max_bond_dimension(256);
  std::map<int, double> scut{{0, 1.0e-6}};
  s.set_svd_cutoff(scut);
  std::map<int, double> rel_scut{{0, 1.0e-3}};
  s.set_rel_svd_cutoff(rel_scut);
  s.set_measure_sample_sequential("auto");

  // Set the noise model
  // Uncomment the following lines to introduce noise to the simulation.
  // You will need to have the Qristal emulator installed for this to work.
  // s.set_noise(true);
  // qb::NoiseModel nm("qb-nm1", n_qubits);
  // s.set_noise_model(nm);

  // Define the circuit
  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      cx q[0],q[1];
      measure q[0] -> c[0];
      measure q[1] -> c[1];
    }
    )";
  // Hand over the circuit to the sim object
  s.set_instring(targetCircuit);

  // Run the circuit
  s.run();

  // Print out results
  std::cout << "Results:" << std::endl << s.get_out_raws()[0][0] << std::endl;
}

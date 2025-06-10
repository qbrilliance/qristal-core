// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/session.hpp>
#include <string>
#include <iostream>

int main()
{
  // Make a Qristal session
  qristal::session s;

  // Set the number of qubits
  s.qn = 2;

  // Set the number of shots
  s.sn = 1000;

  // Use the MPS emulator backend.
  // You will need to have the Qristal emulator installed for this to work.
  s.acc = "qb-mps";

  // Set backend parameters
  s.initial_bond_dimension = 1;
  s.max_bond_dimension = 256;
  s.svd_cutoff = 1.0e-6;
  s.rel_svd_cutoff = 1.0e-3;
  s.measure_sample_method = "auto";
  s.gpu_device_ids = {0};

  // Set the noise model
  // Uncomment the following lines to introduce noise to the simulation.
  // s.noise = true;
  // s.noise_model = std::make_shared<qristal::NoiseModel>("qb-nm1", s.qn);

  // Define the circuit
  s.instring = R"(
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

  // CudaQ has no transpiler, so we need to transpile the circuit to QB's native gate set {rx, ry, cz} first.
  if (s.acc == "cudaq:qb_mps" && s.noise == true) {
    // To do this, we simply run the program without executing the circuit, i.e. by setting execute_circuit = false
    s.execute_circuit = false;
    s.run();

    // Now we can get the transpiled circuit
    std::cout << s.transpiled_circuit() << "\n";
    // The transpiled circuit is in openQasm, so we feed it back into session via "instring"
    s.instring = s.transpiled_circuit();

    // Execute the transpiled circuit by setting execute_circuit back to true
    s.execute_circuit = true;
  }

  // Run the circuit
  s.run();

  // Print out results
  std::cout << "Results:" << std::endl << s.results() << std::endl;
}

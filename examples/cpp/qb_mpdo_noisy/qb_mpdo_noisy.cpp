// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/session.hpp"
#include <string>
#include <iostream>

int main()
{
  // Make a Qristal session
  auto s = qristal::session(false);

  // Set up default parameters
  s.init();

  // Set the number of qubits
  const int n_qubits = 2;
  s.set_qn(n_qubits);

  // Set the number of shots
  const int n_shots = 1000;
  s.set_sn(n_shots);

  // Use the MPDO emulator backend.
  // You will need to have the Qristal emulator installed for this to work.
  s.set_acc("qb-mpdo");

  // Set backend parameters
  s.set_initial_bond_dimension(1);
  s.set_max_bond_dimension(256);
  std::map<int, double> scut{{0, 1.0e-15}};
  s.set_svd_cutoff(scut);
  std::map<int, double> rel_scut{{0, 1.0e-10}};
  s.set_rel_svd_cutoff(rel_scut);
  s.set_measure_sample_method("auto");

  // Set the noise model
  // Uncomment the following lines to introduce noise to the simulation.
  // You will need to have the Qristal emulator installed for this to work.
  // s.set_noise(true);
  // qristal::NoiseModel nm("qb-nm1", n_qubits);
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

  // CudaQ has no transpiler, so we need to transpile the circuit to QB's native gate set {rx, ry, cz} first.
  if (s.get_accs()[0][0] == "cudaq:qb_mpdo" && s.get_noises()[0][0] == true) {
    // To do this, we simply run the program without executing the circuit, i.e. by setting execute_circuit = false
    s.set_execute_circuit(false);
    s.run();

    // Now we can get the transpiled circuit using "out_transpiled_circuit"
    std::string circ_qasm = s.get_out_transpiled_circuits()[0][0];
    std::cout << circ_qasm << "\n";
    // The transpiled circuit is in openQasm, so we feed it back into session via "instring"
    s.set_instring(circ_qasm);

    // Execute the transpiled circuit by setting execute_circuit = true
    s.set_execute_circuit(true);
  }

  // Run the circuit
  s.run();

  // Print out results
  std::cout << "Results:" << std::endl << s.results()[0][0] << std::endl;
}

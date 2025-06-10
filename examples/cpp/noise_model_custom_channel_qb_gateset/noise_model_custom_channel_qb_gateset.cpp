// Copyright (c) Quantum Brilliance Pty Ltd
/**
 * This example shows how to make your own noise model using QB's gateset
 */

#include <qristal/core/session.hpp>
#include <qristal/core/noise_model/noise_model.hpp>

// Build and return a noise model for an n-qubit ring
qristal::NoiseModel ring_noise_model(size_t nb_qubits) {
  // Make an empty noise model
  qristal::NoiseModel noise_model;

  // Name the model whatever you like
  noise_model.name = "ring_noise_model";

  // Define the gate fidelities (errors are 1 - fidelity)
  // for gates in the set { rx, ry, cz}
  constexpr double rx_error = 1e-3;
  constexpr double ry_error = 1e-3;
  constexpr double cz_error = 1e-2;

  // Use the "qristal-qobj" generator, which transpiles the circuit into the {rx, ry, cz} basis.
  noise_model.set_qobj_compiler("qristal-qobj");

  // Print out the QObj basis gate set to confirm
  std::cout << "Basis gate set of " << noise_model.get_qobj_compiler() << " generator is: ";
  for (const auto &x : noise_model.get_qobj_basis_gates()) {
    std::cout << x << " ";
  }
  std::cout << "\n";

  // Define the readout errors
  // Assume no readout errors (0.0)
  qristal::ReadoutError ro_error;
  ro_error.p_01 = 0.0;
  ro_error.p_10 = 0.0;

  // Loop over the qubits
  for (size_t qId = 0; qId < nb_qubits; ++qId) {
    // Set the readout errors
    noise_model.set_qubit_readout_error(qId, ro_error);

    // Set the single-qubit gate fidelities
    noise_model.add_gate_error(qristal::DepolarizingChannel::Create(qId, rx_error), "rx", {qId});
    noise_model.add_gate_error(qristal::DepolarizingChannel::Create(qId, ry_error), "ry", {qId});

    // Set the qubit connections to form a ring
    const size_t qId2 = (qId != nb_qubits - 1 ? qId + 1 : 0);
    noise_model.add_qubit_connectivity(qId, qId2);

    // Set the corresponding two-qubit gate fidelities
    noise_model.add_gate_error(qristal::DepolarizingChannel::Create(qId, qId2, cz_error), "cz", {qId, qId2});
    noise_model.add_gate_error(qristal::DepolarizingChannel::Create(qId, qId2, cz_error), "cz", {qId2, qId});
  }

  return noise_model;
}


int main(int argc, char * argv[])
{
  qristal::session my_sim;

  // Set the number of qubits
  my_sim.qn = 2;

  // Set the number of shots
  my_sim.sn = 100;

  // Aer simulator selected
  my_sim.acc = "aer";

  // Set this to true to include noise
  my_sim.noise = true;

  // Create the noise model and hand it over to the session.
  my_sim.noise_model = std::make_shared<qristal::NoiseModel>(ring_noise_model(my_sim.qn));

  // Define the kernel
  my_sim.instring = R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[2];
    h q[0];
    cx q[0],q[1];
    measure q[1] -> c[1];
    measure q[0] -> c[0];
    )";

  // Execute circuit
  my_sim.run();

  // Print results
  std::cout << my_sim.results() << std::endl;

  // Exit
  return 0;
}

# Copyright (c) 2022 Quantum Brilliance Pty Ltd

# This example shows how to turn on noise in a simulation,
# and how to modify the default noise model used.

import sys
import qb.core


# Build and return a noise model for an n-qubit ring
def ring_noise_model(nb_qubits):

  # Make an empty noise model
  noise_model = qb.core.NoiseModel()

  # Name the model whatever you like
  noise_model.name = "qb_ring_noise_model"

  # Define the gate fidelities (errors are 1 - fidelity)
  # for gates in the set { rx, ry, cz}
  rx_error = 1e-3
  ry_error = 1e-3
  cz_error = 1e-2

  # Use the "qristal-qobj" generator, which transpiles the circuit into the {rx, ry, cz} basis.
  noise_model.qobj_compiler = "qristal-qobj"
  
  # Print out the QObj basis gate set to confirm
  print("Basis gate set of ", noise_model.qobj_compiler, " generator is:", noise_model.qobj_basis_gates)

  # Define the readout errors
  # assume no readout errors (0.0)
  ro_error = qb.core.ReadoutError()
  ro_error.p_01 = 0.0
  ro_error.p_10 = 0.0

  # Loop over the qubits
  for qId in range(nb_qubits):

    # Set the readout errors
    noise_model.set_qubit_readout_error(qId, ro_error)

    # Set the single-qubit gate fidelities
    noise_model.add_gate_error(qb.core.DepolarizingChannel.Create(qId, rx_error), "rx", [qId])
    noise_model.add_gate_error(qb.core.DepolarizingChannel.Create(qId, ry_error), "ry", [qId])

    # Set the qubit connections to form a ring
    qId2 = 0 if qId == nb_qubits - 1 else qId + 1
    noise_model.add_qubit_connectivity(qId, qId2)

    # Set the corresponding two-qubit gate fidelities
    noise_model.add_gate_error(qb.core.DepolarizingChannel.Create(qId, qId2, cz_error), "cz", [qId, qId2])
    noise_model.add_gate_error(qb.core.DepolarizingChannel.Create(qId, qId2, cz_error), "cz", [qId2, qId])
  
  return noise_model


def main(arguments):

  # Create a quantum computing session using the QB SDK
  my_sim = qb.core.session()

  # Set up meaningful defaults for session parameters
  my_sim.qb12()

  # 2 qubits
  my_sim.qn = 2

  # Aer simulator selected
  my_sim.acc = "aer"

  # Set this to true to include noise
  my_sim.noise = True

  # Hand over the noise model to the session.
  my_sim.noise_model = ring_noise_model(my_sim.qn[0][0])

  # Define the kernel
  my_sim.instring = '''
  __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
  {
     OPENQASM 2.0;
     include "qelib1.inc";
     creg c[2];
     h q[0];
     cx q[0],q[1];
     measure q[1] -> c[1];
     measure q[0] -> c[0];
  }
  '''

  # Hit it.
  my_sim.run()

  # Lookee
  print(my_sim.out_raw[0][0])

#Actual program launched on invocation
main(sys.argv)

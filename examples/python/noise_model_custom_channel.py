# Copyright (c) 2022 Quantum Brilliance Pty Ltd

# This example shows how to turn on noise in a simulation,
# and how to modify the default noise model used.

import sys
import qristal.core


# Build and return a noise model for an n-qubit ring
def ring_noise_model(nb_qubits):

  # Make an empty noise model
  noise_model = qristal.core.NoiseModel()

  # Name the model whatever you like
  noise_model.name = "ring_noise_model"

  # Define the gate fidelities (errors are 1 - fidelity)
  u1_error = 1e-4
  u2_error = 1e-3
  u3_error = 1e-3
  cx_error = 1e-2

  # Define the readout errors
  ro_error = qristal.core.ReadoutError()
  ro_error.p_01 = 1e-2
  ro_error.p_10 = 5e-3

  # Loop over the qubits
  for qId in range(nb_qubits):

    # Set the readout errors
    noise_model.set_qubit_readout_error(qId, ro_error)

    # Set the single-qubit gate fidelities
    noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, u1_error), "u1", [qId])
    noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, u2_error), "u2", [qId])
    noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, u3_error), "u3", [qId])

    # Set the qubit connections to form a ring
    qId2 = 0 if qId == nb_qubits - 1 else qId + 1
    noise_model.add_qubit_connectivity(qId, qId2)

    # Set the corresponding two-qubit gate fidelities
    noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, qId2, cx_error), "cx", [qId, qId2])
    noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, qId2, cx_error), "cx", [qId2, qId])

  return noise_model


def main(arguments):

  # Create a quantum computing session using Qristal
  my_sim = qristal.core.session()

  # Set up meaningful defaults for session parameters
  my_sim.init()

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
  print(my_sim.results[0][0])

#Actual program launched on invocation
main(sys.argv)

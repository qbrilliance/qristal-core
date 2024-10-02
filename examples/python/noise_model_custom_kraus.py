# Copyright (c) 2022 Quantum Brilliance Pty Ltd

# This example shows how to use your own Kraus operators.

import sys
import qristal.core
import numpy as np

def ring_noise_model(nb_qubits):
  # Make an empty noise model
  noise_model = qristal.core.NoiseModel()

  # Name the model whatever you like
  noise_model.name = "ring_noise_model"

  # Define the readout errors
  ro_error = qristal.core.ReadoutError()
  ro_error.p_01 = 1e-2
  ro_error.p_10 = 5e-3

  # Define the gate fidelities (errors are 1 - fidelity)
  u1_error = 1e-3
  u2_error = 1e-3
  u3_error = 1e-3
  cx_error = 1e-2

  # Create depolarizing channel Kraus operators for u1 gate
  p_u1_1 = np.sqrt(1.0 - u1_error)
  p_u1_2 = np.sqrt(u1_error / 3.0)
  mat_Id_u1 = np.array([[p_u1_1, 0.0], [0.0, p_u1_1]])
  mat_X_u1 = np.array([[0.0, p_u1_2], [p_u1_2, 0.0]])
  mat_Y_u1 = np.array([[0.0, complex(0.0, -p_u1_2)], [complex(0.0, p_u1_2), 0.0]])
  mat_Z_u1 = np.array([[p_u1_2, 0.0], [0.0, -p_u1_2]])
  single_qubit_kraus_u1 = [mat_Id_u1, mat_X_u1, mat_Y_u1, mat_Z_u1]

  # Create depolarizing channel Kraus operators for u2 gate
  p_u2_1 = np.sqrt(1.0 - u2_error)
  p_u2_2 = np.sqrt(u2_error / 3.0)
  mat_Id_u2 = np.array([[p_u2_1, 0.0], [0.0, p_u2_1]])
  mat_X_u2 = np.array([[0.0, p_u2_2], [p_u2_2, 0.0]])
  mat_Y_u2 = np.array([[0.0, complex(0.0, -p_u2_2)], [complex(0.0, p_u2_2), 0.0]])
  mat_Z_u2 = np.array([[p_u2_2, 0.0], [0.0, -p_u2_2]])
  single_qubit_kraus_u2 = [mat_Id_u2, mat_X_u2, mat_Y_u2, mat_Z_u2]

  # Create depolarizing channel Kraus operators for u3 gate
  p_u3_1 = np.sqrt(1.0 - u3_error)
  p_u3_2 = np.sqrt(u3_error / 3.0)
  mat_Id_u3 = np.array([[p_u3_1, 0.0], [0.0, p_u3_1]])
  mat_X_u3 = np.array([[0.0, p_u3_2], [p_u3_2, 0.0]])
  mat_Y_u3 = np.array([[0.0, complex(0.0, -p_u3_2)], [complex(0.0, p_u3_2), 0.0]])
  mat_Z_u3 = np.array([[p_u3_2, 0.0], [0.0, -p_u3_2]])
  single_qubit_kraus_u3 = [mat_Id_u3, mat_X_u3, mat_Y_u3, mat_Z_u3]

  # Create depolarizing channel Kraus operators for cx gate
  num_terms = 16
  max_param = num_terms / (num_terms - 1.0)
  coeff_iden = np.sqrt(1 - cx_error / max_param)
  coeff_pauli = np.sqrt(cx_error / num_terms)

  def pauli_op_map():
    Id = np.array([[1.0, 0.0], [0.0, 1.0]])
    X = np.array([[0.0, 1.0], [1.0, 0.0]])
    Y = np.array([[0.0, complex(0.0, -1.0)], [complex(0.0, 1.0), 0.0]])
    Z = np.array([[1.0, 0.0], [0.0, -1.0]])
    return {'I': Id, 'X': X, 'Y': Y, 'Z': Z}

  def build_kraus_op(pauli_str, coeff):
    first_mat = pauli_op_map().get(pauli_str[0])
    second_mat = pauli_op_map().get(pauli_str[1])
    kron_mat = coeff * np.kron(first_mat, second_mat)
    return kron_mat

  two_qubit_kraus = []
  pauli_kraus_ops = [('II', coeff_iden), ('IX', coeff_pauli), ('IY', coeff_pauli), ('IZ', coeff_pauli),
                     ('XI', coeff_pauli), ('XX', coeff_pauli), ('XY', coeff_pauli), ('XZ', coeff_pauli),
                     ('YI', coeff_pauli), ('YX', coeff_pauli), ('YY', coeff_pauli), ('YZ', coeff_pauli),
                     ('ZI', coeff_pauli), ('ZX', coeff_pauli), ('ZY', coeff_pauli), ('ZZ', coeff_pauli)]
  for i in range(len(pauli_kraus_ops)):
    op_label = pauli_kraus_ops[i][0]
    coeff = pauli_kraus_ops[i][1]
    two_qubit_kraus.append(build_kraus_op(op_label, coeff))

  # Loop over the qubits
  for qId in range(nb_qubits):
    # Set the readout errors
    noise_model.set_qubit_readout_error(qId, ro_error)

    # Note: To use the emulator backends, Kraus operators for native gate set {rx, ry, cz} must be supplied.
    # Set the single-qubit gate fidelities.
    # Kraus operators for 1-qubit gate depolarizing channel
    noise_model.add_gate_error(qristal.core.krausOpToChannel.Create([qId], single_qubit_kraus_u1, None), "u1", [qId])
    noise_model.add_gate_error(qristal.core.krausOpToChannel.Create([qId], single_qubit_kraus_u2, None), "u2", [qId])
    noise_model.add_gate_error(qristal.core.krausOpToChannel.Create([qId], single_qubit_kraus_u3, None), "u3", [qId])

    # Set the qubit connections to form a ring
    qId2 = 0 if qId == nb_qubits - 1 else qId + 1
    noise_model.add_qubit_connectivity(qId, qId2)

    # Set the corresponding two-qubit gate fidelities
    # Create 2-qubit Kraus matrices
    noise_model.add_gate_error(qristal.core.krausOpToChannel.Create([qId, qId2], two_qubit_kraus, None), "cx", [qId, qId2])
    noise_model.add_gate_error(qristal.core.krausOpToChannel.Create([qId, qId2], two_qubit_kraus, None), "cx", [qId2, qId])

  return noise_model


def main(arguments):
  # Create a quantum computing session using Qristal
  my_sim = qristal.core.session()

  # 4 qubits
  n = 4

  # Set up meaningful defaults
  my_sim.init()

  # Set the number of qubits
  my_sim.qn = n

  # Aer simulator selected
  my_sim.acc = "aer"

  # Set this to true to include noise
  my_sim.noise = True

  # Create the noise model and hand it over to the session.
  nm = ring_noise_model(n)
  my_sim.noise_model = nm

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

  # Execute circuit
  my_sim.run()

  # Print results
  print(my_sim.results[0][0])

#Actual program launched on invocation
main(sys.argv)

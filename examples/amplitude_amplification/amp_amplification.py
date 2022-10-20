# Adapt from:
# https://qiskit.org/documentation/tutorials/algorithms/06_grover.html#State-preparation

import qbos as qb
import numpy as np

qb.core()

NB_QUBITS = 3

# Oracle: 
oracle = qb.Circuit()
oracle.h(2)
oracle.ccx(0, 1, 2)
oracle.h(2)

theta = 2 * np.arccos(1 / np.sqrt(3))
state_prep = qb.Circuit()
state_prep.ry(0, theta)
state_prep.ch(0,1)
state_prep.x(1)
state_prep.h(2)



# Construct full amplitude amplification circuit:
full_circuit = qb.Circuit()
# Add amplitude amplification circuit for the above oracle and state preparation sub-circuits.
full_circuit.amplitude_amplification(oracle, state_prep)
# Add measurement:
full_circuit.measure_all()
# Print the circuit:
#print("OpenQASM:\n", full_circuit.openqasm())

# Compare with initial state
state_prep.measure_all()
result_prep = state_prep.execute()
measure_index = result_prep.find("Measure")
left_index = result_prep.find("{",measure_index) + 1
right_index = result_prep.find("}",left_index)
print("prepared input:", result_prep[left_index:right_index])
#print("Grover output:", result[left_index:right_index])print("Initial state:\n", result_prep)

# Run the full amplitude estimation procedure:
result = full_circuit.execute()
measure_index = result.find("Measure")
left_index = result.find("{",measure_index) + 1
right_index = result.find("}",left_index)
print("Grover output:", result[left_index:right_index])
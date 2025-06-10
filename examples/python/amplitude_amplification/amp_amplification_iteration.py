import qristal.core
import numpy as np

s = qristal.core.session()
s.sn = 1000

# This example demonstrates the concept of amplitude amplification iterations.
# Here, we want to amplify the amplitude of |1> state (so-called marked state)

# To mark such state, the oracle is a simple Z gate, since Z|1> = -|1>
# Oracle:
oracle = qristal.core.Circuit()
oracle.z(0)

# For demonstration purposes, we start with a very low population of |1> state:
# using Ry(epsilon) as the state preparation:
# Ry(epsilon) |0> ~ epsilon |1> + sqrt(1 - epsilon^2) |1>
# i.e., the initial population of the marked state is small (~ epsilon)
epsilon = 0.05
state_prep = qristal.core.Circuit()
state_prep.ry(0, epsilon)

good_state_ampls = []
print("iteration","\t probability","\t prediction")
# Testing a varying number of iteration
for i in range(1, 41, 1):
    # Construct full amplitude amplification circuit:
    full_circuit = qristal.core.Circuit()
    # Add amplitude amplification circuit for the above oracle and state preparation sub-circuits.
    full_circuit.amplitude_amplification(oracle, state_prep, i)
    # Add measurement:
    full_circuit.measure_all()

    # Run the full amplitude estimation procedure:
    s.irtarget = full_circuit
    s.qn = full_circuit.num_qubits()
    s.run()
    # Calculate the probability of the marked state
    good_count = int(s.results[[1]])
    good_state_ampls.append(good_count/1000)
    print(i, "\t\t", good_state_ampls[-1], "\t", np.sin((2*i+1)*epsilon/2)**2)

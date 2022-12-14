import qb.core
import numpy as np
import ast
import matplotlib.pyplot as plt
s = qb.core.session()
s.qb12()

NB_QUBITS = 1

# This example demonstrates the concept of amplitude amplification iterations.
# Here, we want to amplify the amplitude of |1> state (so-called marked state)

# To mark such state, the oracle is a simple Z gate, since Z|1> = -|1>
# Oracle:
oracle = qb.core.Circuit()
oracle.z(0)

# For demonstration purposes, we start with a very low population of |1> state:
# using Ry(epsilon) as the state preparation:
# Ry(epsilon) |0> ~ epsilon |1> + sqrt(1 - epsilon^2) |1>
# i.e., the initial population of the marked state is small (~ epsilon)
epsilon = 0.05
state_prep = qb.core.Circuit()
state_prep.ry(0, epsilon)

good_state_ampls = []
print("iteration","\tprobability")
# Testing a varying number of iteration
for i in range(1, 40, 1):
    # Construct full amplitude amplification circuit:
    full_circuit = qb.core.Circuit()
    # Add amplitude amplification circuit for the above oracle and state preparation sub-circuits.
    full_circuit.amplitude_amplification(oracle, state_prep, i)
    # Add measurement:
    full_circuit.measure_all()

    # Run the full amplitude estimation procedure:
    s.ir_target = full_circuit
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    # Calculate the probability of the marked state
    good_count = int(res["1"])
    good_state_ampls.append(good_count/1024)

# Plot the results
iterations = range(1, 40, 1)
# Get analytical result for comparison
probs_theory = [np.sin((2*mm+1)*epsilon/2)**2 for mm in iterations]

# Plot the results
plt.figure(figsize=(7,5))
plt.plot(iterations, good_state_ampls, 'o')
plt.plot(iterations, probs_theory)
plt.xlabel('Number of Iterations')
plt.ylabel('Probability of measuring flag qubit in |1>')
plt.savefig('qaa.png')

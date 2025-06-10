# Example demonstrating Grover-based amplitude amplification
print("Example demonstrating Grover-based amplitude amplification.")

print("First, import libraries...")
import qristal.core
import numpy as np
import timeit
print("Libraries imported successfully!")
s = qristal.core.session()
s.sn = 1000

print("In this example we want to use amplitude amplification to search for")
print("the state |psi> = |101> - |110> within the equal superposition state.")

print("Begin demonstration!")
# Target search state:
# |psi> = |101> - |110>
# Hence, the oracle is f = CZ q0, q2; CZ q1, q2,
# because:
# f |psi> = -|101> + |110> = -|psi>
NB_QUBITS = 3

print("First define the oracle. In this case the oracle is:\n")
print("CZ q0, q2")
print("CZ q1, q2\n")
print("since this maps |psi> to -|psi>.")
# Oracle: CZ q0, q2; CZ q1, q2 (see above)
oracle = qristal.core.Circuit()
oracle.cz(0, 2)
oracle.cz(1, 2)
print("Oracle defined! Printing oracle...\n")
print("OpenQASM:\n", oracle.openqasm())

print("Now to create the state preparation circuit that will produce")
print("an equal superposition.")
# State preparation:
# Use a standard all Hadamards (equal superposition)
state_prep = qristal.core.Circuit()
for i in range(NB_QUBITS):
  state_prep.h(i)
print("State prep defined! Printing state prep...\n")
print("OpenQASM:\n", state_prep.openqasm())

# Construct full amplitude amplification circuit:
print("Now we construct the circuit that we will run to solve the problem.")
full_circuit = qristal.core.Circuit()
print("Circuit initialised!")

# Add amplitude amplification circuit for the above oracle and state preparation sub-circuits.
full_circuit.amplitude_amplification(oracle, state_prep, 4)
print("Amplitude Amplification module added!")

# Add measurement:
full_circuit.measure_all()
print("Measurements added!")

# Print the circuit:
# print("OpenQASM:\n", full_circuit.openqasm())

# Execute:
print("Running circuit...")

start = timeit.default_timer()
s.irtarget = full_circuit
s.qn = full_circuit.num_qubits()
s.run()
end = timeit.default_timer()
print("Circuit executed!")


print("Now let's see the results. We want to measure the state |psi>,")
print("so we expect measurements of '101' and '110' to dominate.")

print("Result:")
print(s.results)

print("Success!")

print("The amplitude amplifcation module ran in ", end-start, " seconds!")

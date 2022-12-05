# Example demonstrating Grover-based amplitude amplification
print("Example demonstrating Grover-based amplitude amplification.")
input("")

print("First, import libraries...")
import qb.core
import numpy as np
import timeit
import ast
print("Libraries imported successfully!")
input("")
s = qb.core.session()
s.qb12()

print("In this example we want to use amplitude amplification to search for")
print("the state |psi> = |101> - |011> within the equal superposition state.")

print("Begin demonstration!")
input("")
# Target search state:
# |psi> = |101> - |011>
# Hence, the oracle is f = CZ q0, q2; CZ q1, q2,
# because:
# f |psi> = -|101> + |011> = -|psi>
NB_QUBITS = 3

print("First define the oracle. In this case the oracle is:\n")
print("CZ q0, q2")
print("CZ q1, q2\n")
print("since this maps |psi> to -|psi>.")
input("")
# Oracle: CZ q0, q2; CZ q1, q2 (see above)
oracle = qb.core.Circuit()
oracle.cz(0, 2)
oracle.cz(1, 2)
print("Oracle defined! Printing oracle...\n")
print("OpenQASM:\n", oracle.openqasm())
input("")

print("Now to create the state preparation circuit that will produce")
print("an equal superposition.")
input("")
# State preparation:
# Use a standard all Hadamards (equal superposition)
state_prep = qb.core.Circuit()
for i in range(NB_QUBITS):
  state_prep.h(i)
print("State prep defined! Printing state prep...\n")
print("OpenQASM:\n", state_prep.openqasm())
input("")

# Construct full amplitude amplification circuit:
print("Now we construct the circuit that we will run to solve the problem.")
input("")
full_circuit = qb.core.Circuit()
print("Circuit initialised!")
input("")
# Add amplitude amplification circuit for the above oracle and state preparation sub-circuits.
full_circuit.amplitude_amplification(oracle, state_prep, 4)
print("Amplitude Amplification module added!")
input("")
# Add measurement:
full_circuit.measure_all()
print("Measurements added!")
input("")
# Print the circuit:
# print("OpenQASM:\n", full_circuit.openqasm())

# Execute:
print("Running circuit...")
input("")
start = timeit.default_timer()
s.ir_target = full_circuit
s.run()
result = s.out_raw[0][0]
res = ast.literal_eval(result)
end = timeit.default_timer()
print("Circuit executed!")
input("")

print("Now let's see the results. We want to measure the state |psi>,")
print("so we expect measurements of '101' and '011' to dominate.")
input("")
print("Result:\n", res)
input("")
print("Success!")
input("")
print("The amplitude amplifcation module ran in ", end-start, " seconds!")

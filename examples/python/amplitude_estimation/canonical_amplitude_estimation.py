# Example demonstration amplitude estimation circuit
print("Example demonstrating phase estimation based (canonical) amplitude estimation (CQAE).")

# Test the example here:
# https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
# i.e., estimate the amplitude of the state:
# sqrt(1-p)|0> + sqrt(p)|1>
print("First, import libraries...")
import qristal.core
from qristal.core import run_canonical_ae
import numpy as np
import timeit
import ast
print("Libraries imported successfully!")

s = qristal.core.session()

print("In this example, we want to use CQAE to estimate the amplitude of the |1> state")
print("in the superposition state |psi> = sqrt(1-p)|0> + sqrt(p)|1>.")
print("Begin demonstration!")


p = float(input("Set a value for p between 0 and 1. p = "))
print("Okay! p = ", p, ", so that means the amplitude we want to estimate is sqrt(p) = ", np.sqrt(p))


print("First, we need to define a few inputs for the amplitude estimation module.")
bits_precision = int(input("How many precision qubits should we use? (8 would be good) "))
state_prep_qubits = 1
trial_qubits = 1

print("Next step is to create the state preparation circuit.")
print("We use a y-rotation gate to prepare the state |psi>.")

theta_p = 2 * np.arcsin(np.sqrt(p))
# State prep circuit: (preparing the state that we want to estimate the amplitude)
state_prep = qristal.core.Circuit()
state_prep.ry(bits_precision, theta_p)
print("State prep defined! Printing state prep...\n")
print("OpenQASM:\n", state_prep.openqasm())


print("In this example, we will provide the amplitude estimation module with the")
print("Grover's operator that it will use in the phase estimation.")

# Grover operator circuit:
grover_op = qristal.core.Circuit()
grover_op.ry(bits_precision, 2*theta_p)
print("Gover's operator defined! Printing Grover's operator...")
print("OpenQASM:\n", grover_op.openqasm())


# Construct full amplitude estimation circuit:
print("Now we construct the circuit that we will run to solve the problem.")

# Execute:
start = timeit.default_timer()
result = run_canonical_ae(state_prep, grover_op, bits_precision, state_prep_qubits, trial_qubits)
end = timeit.default_timer()
res = ast.literal_eval(result)
print("Circuit initialised and CQAE algorithm executed!")

print("Remember that the expected amplitude = " + str(np.sqrt(p)))
print("And CQAE gives us...")

print("Result:\n", res["AcceleratorBuffer"]["Information"]["amplitude-estimation"])

print("The CQAE algorithm ran in ", end-start, " seconds!")

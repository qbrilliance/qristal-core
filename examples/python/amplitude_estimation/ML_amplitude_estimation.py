# Example demonstration amplitude estimation circuit
print("Example demonstrating maximum likelihood amplitude estimation (MLQAE).")

# Test the example here:
# https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
# i.e., estimate the amplitude of the state:
# sqrt(1-p)|0> + sqrt(p)|1>
print("First, import libraries...")
import qristal.core
from qristal.core import run_MLQAE
import numpy as np
import ast
import timeit
print("Libraries imported successfully!")


print("In this example, we want to use CQAE to estimate the amplitude of the |1> state")
print("in the superposition state |psi> = sqrt(1-p)|0> + sqrt(p)|1>.")
print("Begin demonstration!")


p = float(input("Set a value for p between 0 and 1. p = "))
print("Okay! p = ", p, ", so that means the amplitude we want to estimate is sqrt(p) = ", np.sqrt(p))


print("First step is to create the state preparation circuit.")
print("We use a y-rotation gate to prepare the state |psi>.")
theta_p = 2 * np.arcsin(np.sqrt(p))
# State prep circuit: (preparing the state that we want to estimate the amplitude)
state_prep = qristal.core.Circuit()
state_prep.ry(0, theta_p)
print("State prep defined! Printing state prep...\n")
print("OpenQASM:\n", state_prep.openqasm())


# In this case, we don't construct the Grover operator by ourselves,
# instead, just provide the oracle to detect the marked state (|1>)
print("We now need to define the oracle circuit that marks the desired state (|1>).")

oracle = qristal.core.Circuit()
oracle.z(0)
print("OpenQASM:\n", oracle.openqasm())


print("Lastly, we need to define a few more inputs for the amplitude estimation module.")
num_runs = int(input("How many runs should we use? (6 would be good) "))
shots = int(input("HOw many shots should we use? (100 would be good) "))

def is_in_good_subspace(s,x):
    if int(s[0]) == 1:
        return 1
    else:
        return 0
total_num_qubits = 1
score_qubits = [0]

# Execute:
print("Now we construct the circuit that we will run to solve the problem.")

start = timeit.default_timer()
result = run_MLQAE(state_prep, oracle, is_in_good_subspace, score_qubits, total_num_qubits, num_runs, shots)
end = timeit.default_timer()
res = ast.literal_eval(result)
print("Circuit initialised and MLQAE algorithm executed!")


print("Remember that the expected amplitude = " + str(np.sqrt(p)))
print("And MLQAE gives us...")

print("Result:\n", res["AcceleratorBuffer"]["Information"]["amplitude-estimation"])

print("The MLQAE algorithm ran in ", end-start, " seconds!")

# Example demonstration amplitude estimation circuit

# Test the example here:
# https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
# i.e., estimate the amplitude of the state:
# sqrt(1-p)|0> + sqrt(p)|1>
import numpy as np
import qristal.core
from qristal.core import run_canonical_ae_with_oracle

s = qristal.core.session()
s.init()
p = 0.24
theta_p = 2 * np.arcsin(np.sqrt(p))

# State prep circuit: (preparing the state that we want to estimate the amplitude)
state_prep = qristal.core.Circuit()
state_prep.ry(8, theta_p)

# In this case, we don't construct the Grover operator by ourselves,
# instead, just provide the oracle to detect the marked state (|1>)
oracle = qristal.core.Circuit()
oracle.z(8)
bit_precision = 8
state_prep_qubits = 1
trial_qubits = 1

# Execute:
result = run_canonical_ae_with_oracle(state_prep, oracle, bit_precision, state_prep_qubits, trial_qubits)
print("Result:\n", result)

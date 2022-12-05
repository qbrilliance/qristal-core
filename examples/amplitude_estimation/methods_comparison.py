# Example demonstration amplitude estimation circuit

# Test the example here:
# https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
# i.e., estimate the probability p  of the state:
# sqrt(1-p)|0> + sqrt(p)|1>
import numpy as np
import qb.core
from qb.core import run_MLQAE
from qb.core import run_canonical_ae_with_oracle
import ast
import timeit
s = qb.core.session()
s.qb12()
p = 0.24
theta_p = 2 * np.arcsin(np.sqrt(p))

# State prep circuit: (preparing the state that we want to estimate the amplitude)
state_prep = qb.core.Circuit()
state_prep.ry(0, theta_p)

#Set up the inputs
oracle = qb.core.Circuit()
oracle.z(0)
num_runs = 6
shots = 100
best_score = 0
def is_in_good_subspace(s,x):
    if int(s[0]) == 1:
        return 1
    else:
        return 0
total_num_qubits = 1
score_qubits = [0]
state_prep_qubits = [0]
trial_qubits = [0]
num_evaluation_qubits = 10

# Execute:
start_MLQAE = timeit.default_timer()
result_MLQAE = run_MLQAE(state_prep, oracle, is_in_good_subspace, score_qubits, total_num_qubits, num_runs, shots)
end_MLQAE = timeit.default_timer()
start_CanonicalQAE = timeit.default_timer()
result_CanonicalQAE = run_canonical_ae_with_oracle(state_prep, oracle, num_evaluation_qubits, total_num_qubits, total_num_qubits, [1,2,3,4,5,6,7,8,9,10], [0])
end_CanonicalQAE = timeit.default_timer()

MLQAE_amp_est = ast.literal_eval(result_MLQAE)['AcceleratorBuffer']['Information']['amplitude-estimation']
CanonicalQAE_amp_est = ast.literal_eval(result_CanonicalQAE)['AcceleratorBuffer']['Information']['amplitude-estimation']
MLQAE_time = end_MLQAE - start_MLQAE
CanonicalQAE_time = end_CanonicalQAE - start_CanonicalQAE

print("MLQAE estimate = " + str(MLQAE_amp_est))
print("MLQAE runtime = " + str(MLQAE_time))
print("CanonicalQAE estimate = " + str(CanonicalQAE_amp_est))
print("CanonicalQAE runtime = " + str(CanonicalQAE_time))
print("MLQAE difference = " + str(MLQAE_amp_est - np.sqrt(p)))
print("CanonicalQAE difference = " + str(CanonicalQAE_amp_est - np.sqrt(p)))

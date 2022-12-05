import qb.core
import numpy as np
import math
import timeit

# This is an example of using quantum exponential search to find the
# max value of a data set.
dataset = [0,1,1,3,0,1,1,1,2,1,1,1,1,0,1,2]

N = len(dataset)
n_qubits = math.ceil(math.log2(N))

# Registers
trial_score_qubits = [0,1]
trial_qubits = [2,3,4,5]
flag_qubit = 6
best_score_qubits = [7,8]
ancilla_qubits = [9,10,11,12]
next_letter = []
next_metric = []

# Inputs for MLQAE variant
num_runs = 6
num_shots = 100
def is_in_good_subspace(s, best_score):
    val = 0
    for i in range(len(s)):
        val += int(s[i]) * 2**(len(s)-1-i)
    if val > best_score:
        return 1
    else:
        return 0

# Inputs for Canonical QAE variant
num_evaluation_qubits = 10

#state prep
def state_prep(trial_qubits, trial_score_qubits, a,b):
    circ = qb.core.Circuit()
    for i in range(0,len(trial_qubits)):
        circ.h(trial_qubits[i])
    for i in range(0,len(dataset)):
        pos_bin = bin(i)[2:].zfill(len(trial_qubits))
        score_bin = bin(dataset[i])[2:].zfill(len(best_score_qubits))
        for j in range(0,len(trial_qubits)):
            if pos_bin[j] == '0':
                circ.x(trial_qubits[j])
        for j in range(0,len(trial_score_qubits)):
            if score_bin[j] == '1':
                circ.mcx(trial_qubits, trial_score_qubits[j])
        for j in range(0,len(trial_qubits)):
            if pos_bin[j] == '0':
                circ.x(trial_qubits[j])
    return circ

# Oracle
def oracle(bestScore, num_scoring_qubits, trial_score_qubits, flag_qubit, best_score_qubits, ancilla_qubits):
    circ = qb.core.Circuit()
    circ.comparator_as_oracle(bestScore,num_scoring_qubits,trial_score_qubits,flag_qubit,best_score_qubits,ancilla_qubits,False)
    return circ

# Scoring function:
def get_score(x):
    return x

# Can start with any value
bestScore = 0

# Success of each run > 1/4 and the number of times we need to change the pivot ~ log(N)
start_canonical_exp_search = timeit.default_timer()
print("Start canonical exponential search!\n")
max_run = 4 *  math.ceil(math.log2(N))
count = 0
while count < max_run:
    result = qb.core.Circuit().exponential_search(method=str("canonical"), oracle=oracle, state_prep=state_prep, f_score=get_score, best_score=bestScore, qubits_string=trial_qubits, qubits_metric=trial_score_qubits, qubits_next_letter=[], qubits_next_metric=[], qubit_flag=flag_qubit, qubits_best_score=best_score_qubits, qubits_ancilla_oracle=ancilla_qubits)
    if result > bestScore:
        bestScore = result
        print("New best score:", bestScore)
        print("\n")
    if bestScore == max(dataset):
        break
    count = count + 1

print("Final score:", bestScore)
if bestScore == max(dataset):
    print("It took " + str(count+1) + " runs to find the best score!")
end_canonical_exp_search = timeit.default_timer()
print("Canonical exponential search finished in " + str(end_canonical_exp_search-start_canonical_exp_search) + "s\n")

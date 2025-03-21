import qristal.core
import numpy as np
import math
import timeit

print("Example demonstrating the exponential search algorithm.")

# This is an example of using quantum exponential search to find the
# max value of a data set.
print("In this example, exponential search will be used to find the maximum value in a dataset.")
dataset = [0,1,1,3,0,1,1,1,2,1,1,1,1,0,1,2]
print("The dataset is: ", dataset)
print("We expect the exponential search algorithm to return the number ", max(dataset))
N = len(dataset)
n_qubits = math.ceil(math.log2(N))

print("Qubit registers.")
# Registers
trial_score_qubits = [0,1]
trial_qubits = [2,3,4,5]
flag_qubit = 6
best_score_qubits = [7,8]
ancilla_qubits = [9,10]

total_num_qubits = 1 + len(trial_score_qubits) + len(trial_qubits) + len(best_score_qubits) + len(ancilla_qubits)

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

# State prep - encoding the dataset in a quantum register.
def state_prep(trial_qubits, trial_score_qubits, a, b, c):
    circ = qristal.core.Circuit()
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

# Oracle - marks all states with a higher value than the current best value.
def oracle(bestScore):
    qubit_flag = ancilla_qubits[0]
    c_in = ancilla_qubits[1]
    circ = qristal.core.Circuit()

    bestScore_binary_n = bin(bestScore)[2:].zfill(len(best_score_qubits))
    for i in range(0,len(best_score_qubits)):
        if bestScore_binary_n[i] == '1':
            circ.x(best_score_qubits[i])
            print("i:", i)

    circ.x(qubit_flag)
    circ.h(qubit_flag)
    circ.compare_gt(trial_qubits, best_score_qubits, flag_qubit, c_in, True)
    circ.h(qubit_flag)
    circ.x(qubit_flag)
    return circ

# Scoring function
def get_score(x):
    return x

# Initial best score value
bestScore = 0

# Success of each run > 1/4 and the number of times we need to change the pivot ~ log(N)
start_canonical_exp_search = timeit.default_timer()
print("Start canonical exponential search!\n")
max_run = 4 *  math.ceil(math.log2(N))
count = 0
while count < max_run:
    result = qristal.core.Circuit().exponential_search("canonical", state_prep, oracle, bestScore, get_score, total_num_qubits, trial_qubits, trial_score_qubits, qpu="qsim")
    if result > bestScore:
        bestScore = result
        print("New best score found:", bestScore)
        print("\n")
    if bestScore == max(dataset):
        break
    count = count + 1
print("Canonical exponential search finished!")
end_canonical_exp_search = timeit.default_timer()

print("Final score:", bestScore)
if bestScore == max(dataset):
    print("Success!")
    print("It took " + str(count+1) + " runs to find the best score!")
print("Canonical exponential search finished in " + str(end_canonical_exp_search-start_canonical_exp_search) + "s\n")

# # MLQAE
# start_exp_search_MLQAE = timeit.default_timer()
# print("Start exponential search with MLQAE!\n")
# max_run = 4 *  math.ceil(math.log2(N))
# count = 0
# while count < max_run:
#     result = qristal.core.Circuit().exponential_search("MLQAE",oracle, state_prep, get_score, bestScore, trial_qubits, trial_score_qubits, [],[],flag_qubit, best_score_qubits, ancilla_qubits, MLQAE_is_in_good_subspace=is_in_good_subspace, MLQAE_num_runs=num_runs, MLQAE_num_shots=num_shots, qpu = "qsim")
#     if result > bestScore:
#         bestScore = result
#         print("New best score:", bestScore)
#         print("\n")
#     if bestScore == max(dataset):
#         break
#     count = count + 1

# print("Final score:", bestScore)
# if bestScore == max(dataset):
#     print("It took " + str(count+1) + " runs to find the best score!")
# end_exp_search_MLQAE = timeit.default_timer()
# print("Exponential search with MLQAE finished in " + str(end_exp_search_MLQAE-start_exp_search_MLQAE) + "s\n")

# # Canonical QAE
# start_exp_search_CQAE = timeit.default_timer()
# print("Start eexponential search with canonical QAE!\n")
# max_run = 4 *  math.ceil(math.log2(N))
# count = 0
# while count < max_run:
#     result = qristal.core.Circuit().exponential_search("CQAE",oracle, state_prep, get_score, bestScore,trial_qubits, trial_score_qubits,[],[], flag_qubit, best_score_qubits, ancilla_qubits,CQAE_num_evaluation_qubits= num_evaluation_qubits, qpu = "qsim")
#     if result > bestScore:
#         bestScore = result
#         print("New best score:", bestScore)
#         print("\n")
#     if bestScore == max(dataset):
#         break
#     count = count + 1

# print("Final score:", bestScore)
# if bestScore == max(dataset):
#     print("It took " + str(count+1) + " runs to find the best score!")
# end_exp_search_CQAE = timeit.default_timer()
# print("Exponential search with canonical QAE finished in " + str(end_exp_search_CQAE-start_exp_search_CQAE) + "s\n")

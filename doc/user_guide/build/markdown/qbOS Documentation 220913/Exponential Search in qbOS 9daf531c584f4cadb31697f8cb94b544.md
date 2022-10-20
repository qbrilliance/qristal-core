# Exponential Search in qbOS

# Background

## Problem Description

The exponential search algorithm is a key building block of the decoder. Exponential search will update the current best score until the maximum is found or a maximum number of iterations is reached. The basic idea will be to use amplitude amplification to improve the probability of measuring a trial state whose score is greater than the current best score. The issue is that at each iteration we don’t know the number of states whose score is greater than the current best score, hence we don’t know the number of iterations to perform in amplitude amplification.

**Notes:** 

- Implied in the above description is the **oracle** needed for Grover’s search algorithm. The oracle is the quantum circuit that guides the algorithm towards the states having a better score. In the quantum decoder paper, that oracle circuit implements the $`(-1)^{cmp \circ F}`$  logic (as a reversible circuit). In other words, that oracle circuit helps identify those states that have better scores than the current known $`bestScore`$. See the comparator page for more information on using the comparator as an oracle.
- The exponential search module that is implemented is generic w.r.t. that comparator oracle. The oracle is provided as an input parameter to the algorithm.

## Approaches

### Canonical Exponential Search

The *exponential search* algorithm (classical computing) solves the problem of giving a data set and finding the element with the largest/smallest value. On a classical computer, to complete such a task requires a traversal of the whole data set, the complexity is $`**`\mathcal{O}(N)`**`$.

The *quantum exponential search* algorithm (QESA) is based on Grover's search algorithm. It was proposed by [Christoph Durr and Peter Hoyer](https://arxiv.org/pdf/quant-ph/9607014) in January 1999. [Ashish Ahuja and Sanjiv Kapoor](https://arxiv.org/abs/quant-ph/9911082v1) then made a better complexity analysis.

The complexity of the algorithm is $`\mathcal{O}(\sqrt{N})`$, which is **quadratic** acceleration compared to the classical algorithm. 

**Note:** the term *exponential search* here refers to the exponential search algorithm in classical computing. QESA can only provide **quadratic** speedup.

The main reference for QESA is [[Durr & Hoyer](https://arxiv.org/pdf/quant-ph/9607014.pdf)] and [[Boyer et al](https://arxiv.org/pdf/quant-ph/9605034.pdf)]. In the `QuantumSearchDecode` algorithm, we use QESA to find a string (encoded in a qubit register) that has a better `bestScore`.

**Note:** `QuantumSearchDecode` will perform a number of QESA iterations (denoted as `m` in the algorithm). That value (query count) is provided in Theorem 4.

The basic idea of the canonical quantum exponential search algorithm is as follows:

- For the search space $`A`$ (a data set), select one of the elements as the current maximum element randomly (current $`bestScore`$), and use the improved Grover’s search algorithm (see below, from [[Boyer et al](https://arxiv.org/pdf/quant-ph/9605034.pdf)]), that is, Grover’s algorithm that does not know the number of target elements, to search for a larger element in $`A`$.
- Then replace the maximum element with the new one (improve the $`bestScore`$ estimate). The expected value of the total replacing times to get the maximum element is no more than $`\mathcal{O}(logN)`$.
- After finding the maximum element, Grover’s algorithm cannot find a new element. Then the algorithm stops and outputs the maximum element.

The algorithm implementation proceeds as follows:

1. Initialize $`m = 1`$ and set $`λ = 6/5`$ (any value of $`λ`$ strictly between 1 and 4/3 would do)
2. Choose $`j`$ uniformly at random among the nonnegative integers smaller than $`m`$.
3. Apply $`j`$ iterations of Grover’s algorithm (amplitude amplification)
4. Observe the register, checking the metric score (this could be any objective functions that we want to minimize/maximize, e.g., minimum/maximum finding)
5. If the score is improved, done. Otherwise, set $`m`$ to $`min(λm, \sqrt{N})`$ and go back to step 2.

**Note:** Step 4 above is denoted as $`M_{score} |\psi\rangle`$ in [Bausch2020]. In particular, the qubit encoding scheme, in that case, has the metric score stored in a sub-register (the $`|p_q\rangle`$ sub-register in $`|h_q\rangle|p_q\rangle|q\rangle`$). In general, users can implement a more complex look-up scheme in the classical side based on the measured bit-string (e.g., the bit string could represent the index to an entry in the database. 

### Exponential Search with Canonical QAE

In the canonical version of exponential search, we keep increasing the number of iterations of amplitude amplification until we achieve a new best score (or reach a maximum number of iterations). We can instead use Quantum Amplitude Estimation (QAE) to tell us the optimal number of iterations of amplitude amplification to perform. 

The canonical version of QAE relies on Quantum Phase Estimation (QPE). This is explained in detail below. See also the page on [QAE](https://www.notion.so/Quantum-Amplitude-Estimation-45d88c0480a949d38ccd80130094016d). 

In order to do Grover’s search/amplitude amplification, it is necessary to know the initial state. This is because Grover’s operator is defined as $`\mathcal{Q} = \mathcal{A}\mathcal{S}_0\mathcal{A}^\dagger \mathcal{S}_{\Psi_1}`$, whereby $`\mathcal{A}`$ is the unitary circuit to create the initial state (e.g., all Hadamard gates for the base Grover’s algorithm)

[Oblivious Amplitude Amplification](https://arxiv.org/abs/1312.1414) earns this moniker because it can be applied even when the reflection about the input state is unavailable. Hence, the protocol is *oblivious* to the initial state. A nice overview presentation was given by [Dominic Berry](http://www.dominicberry.org/presentations/Durban.pdf).

Each iteration of Amplitude amplification requires that two reflection operators be specified. Specifically, if $`\mathcal{Q}`$ is the amplitude amplification iterate and $`P_0`$ is a *projector* operator onto the **initial subspace** and $`P_1`$ is the *projector* onto the marked subspace then $`\mathcal{Q} = -(I - 2P_0)(I - 2P_1)`$.

Recall that a *projector* is a Hermitian operator that has eigenvalues $`+1`$ and $`0`$ and as a result $`(1 - 2P_0)`$ is unitary because it has eigenvalues that are roots of unity (in this case $`\pm 1`$).

This representation is equivalent to the equation $`\mathcal{Q} = \mathcal{A}\mathcal{S}_0\mathcal{A}^\dagger \mathcal{S}_{\Psi_1}`$, whereby $`\mathcal{A}\mathcal{S}_0\mathcal{A} \equiv (I - 2P_0)`$. Recall that $`\mathcal{S}_0`$ is the reflection about the zero state, hence adding the state preparation $`\mathcal{A}`$ will transform it to the reflection about the initial state.

As an example, consider the case of Grover's search with initial state $`H^{\otimes n} \ket{0}`$ and marked state $`\ket{m}`$, $`P_0 = H^{\otimes n}\ket{0}\bra{0}H^{\otimes n}`$ and $`P_1= \ket{m}\bra{m}`$. 

In most applications of amplitude amplification $`P_0`$ will be a projector onto an initial state meaning that $`P_0 = \ket{\psi}\bra{\psi}`$ for some vector $`\ket{\psi}`$; however, for oblivious amplitude amplification $`P_0`$ will typically project onto many quantum states (for example, the multiplicity of the $`+1`$ eigenvalue of $`P_0`$ is greater than $`1`$).

The logic behind amplitude amplification follows directly from the eigendecomposition of $`\mathcal{Q}`$. Specifically, the eigenvectors of $`\mathcal{Q}`$ that the initial state has non-zero support over can be shown to be linear combinations of the $`+1`$ eigenvectors of $`P_0`$ and $`P_1`$. 

Specifically, the initial state for amplitude amplification (assuming it is a $`+1`$ eigenvector of $`P_0`$) can be written as  $`\ket{\psi}=\frac{-i}{\sqrt{2}}\left(e^{i\theta}\ket{\psi_+} + e^{-i\theta}\ket{\psi_-}\right),`$  where $`\ket{\psi_\pm}`$ are eigenvectors of $`\mathcal{Q}`$ with eigenvalues $`e^{\pm 2i\theta}`$ and only have support on the $`+1`$ eigenvectors of $`P_0`$ and $`P_1`$. 

The fact that the eigenvalues are $`e^{\pm i \theta}`$ implies that the operator $`\mathcal{Q}`$ performs a rotation in a two-dimensional subspace specified by the two projectors and the initial state where the rotation angle is $`2\theta`$. This is why after $`m`$ iterations of $`\mathcal{Q}`$ the success probability is $`\sin^2([2m+1]\theta)`$.

Another useful property that comes out of this is that the eigenvalue $`\theta`$ is directly related to the probability that the initial state would be marked (in the case where $`P_0`$ is a projector onto only the initial state). Since the eigenphases of $`\mathcal{Q}`$ are $`2\theta = 2\sin^{-1}(\sqrt{\Pr(success)})`$ it then follows that if we apply **phase estimation to $`\mathcal{Q}`$** then we can learn the probability of success for a unitary quantum procedure. This is useful because it requires quadratically fewer applications of the quantum procedure to learn the success probability than would otherwise be needed.

Apply phase estimation to $`\mathcal{Q}`$ is exactly what we did for amplitude estimation!!!

This oblivious amplitude amplification procedure can be used in the exponential search procedure above. 

The algorithm proceeds as follows:

1. Apply QPE using the unitary $`Q = \mathcal{A} S_0\mathcal{A}^\dag S_f`$ where $`\mathcal{A}`$ is the state prep circuit, $`S_0`$ is a reflection about 0 and $`S_f`$ is the oracle. Measuring the output bit string $`y`$ yields an approximation for the amplitude $`a`$ of the “good” states: $`a = sin^2(\pi\frac{y}{2^n})`$ . 
2. Then apply amplitude amplification with the optimal number of iterations: $`\frac{\pi}{4a}`$
3. This should yield a string with a better score than the current best score with a high probability

### Exponential Search with MLQAE

In the canonical version of exponential search, we keep increasing the number of iterations of amplitude amplification until we achieve a new best score (or reach a maximum number of iterations). We can instead use Quantum Amplitude Estimation (QAE) to tell us the optimal number of iterations of amplitude amplification to perform. 

Maximum Likelihood QAE (MLQAE) is a version of QAE that is designed for NISQ devices as it does not rely on QPE. It is described below. See also [QAE](https://www.notion.so/Quantum-Amplitude-Estimation-45d88c0480a949d38ccd80130094016d).

This method uses [amplitude amplification](https://www.notion.so/Grover-s-Search-Algorithm-4f4d479e09904b59b7214a9d134f037c) as a subroutine. Essentially it performs a number of runs of amplitude amplification with varying number of iterations and does post-processing to determine the amplitude estimate. We will follow the method in the original paper [[Suzuki2020]](https://arxiv.org/pdf/1904.10246.pdf) where we fix a constant number of shots per run $`N = 1024`$ and vary the number of iterations as an exponentially increasing sequence $`m_0 = 1`$, $`m_1 = 2`$, $`m_2 = 4`$, ... $`m_M = 2^M`$ for some $`M.`$  For larger $`M`$, the more accurate the estimate will be. 

The MLQAE algorithm is as follows:

1. Prepare the advice state
2. Perform amplitude amplification on the advice state with $`m_0`$ iterations and $`N`$ shots. Count the number of shots where the trial score is greater than the current best score $`h_0`$
3. Repeat steps 1 and 2 for $`m_1,...m_M`$ to produce $`h_1,...h_M`$. 
4. Form the Likelihood Function:
$`L(h_i;\theta) = \prod_{i=1}^M [sin^2((2m_i+1)\theta)]^{h_i}[cos^2((2m_i+1)\theta)]^{N-h_i}`$
5. Maximise the likelihood function with respect to $`\theta`$
6. The amplitude estimate is then $`a = sin^2\theta`$

Exponential Search with MLQAE is as follows:

1. Apply MLQAE to determine the approximate amplitude of strings with score greater than the current best
2. Then apply amplitude amplification with the optimal number of iterations: $`\frac{\pi}{4a}`$
3. This should yield a string with a better score than the current best score with a high probability

# qbOS Implementation

**Note:** Each of the three methods described above is implemented in qbOS within the same algorithm where the user declares the method they wish to use. 

## API

Below are the implementation details of the Python class method for the exponential search algorithm. 

Class: `Circuit` , method name: `exponential_search`
Or, function name: `exponential_search`

- Inputs
    - Required input parameters:
        - `method`: `string`
        
        Currently supports: “canonical” (canonical exponential search), “CQAE” (exponential search with canonical QAE), and “MLQAE” (exponential search with MLQAE)
        
        - `oracle`: `function(int, int, list<int>, int, list<int>, list<int>) -> Circuit`
        
        What this means is that the `oracle` is provided as a function that takes inputs and returns a quantum circuit used to flip the sign of the good states in amplitude amplification. The inputs are:
        
        1. `best_score`: the current $`bestScore`$  value. The exponential search will iterate until the score is improved beyond this (see step 5 in the algorithm described above)
        2. `num_score_qubits`: int
        3. `qubits_metric`: [list<int>] - optional
        4. `flag_qubit`: [int] - optional
        5. `qubits_best_score`: [list<int>] - optional
        6. `qubits_ancilla_oracle`: [list<int>] - optional
        
        - `state_prep`: `function(list<int>, list<int>, list<int>, list<int>) -> Circuit`
    
    State preparation is also given as a function mapping inputs to a circuit object. The inputs are:
    
    1. `qubits_string`: [list<int>] the indices of the qubits in the register encoding the trial string
    2. `qubits_metric`: [list<int>] the indices of the qubits in the register encoding the trial score
    3. `qubits_next_letter` [list<int>] the indices of the qubits in the register used to temporarily store the next letter 
    4. `qubits_next_metric`: [list<int>] the indices of the qubits in the register used to temporarily store the next letter probability
    
    Alternatively, `state_preparation` may be provided as a circuit object.
    
    - `f_score`: `std::function<int(int)>`
    
    The function converts the measurement bit-string result to a score value that is used to compare the current $`bestScore`$. This function can be defined to just return the string’s score (sum of probabilities) or it can be more complicated. 
    
    - `best_score`: the current $`bestScore`$  value. The exponential search will iterate until the score is improved beyond this (see step 5 in the algorithm described above)
    - Qubit Registers:
    1. `qubits_string`: [list<int>] 
    2. `qubits_metric`: [list<int>]
    3. `qubits_next_letter`: [list<int>]
    4. `qubits_next_metric`: [list<int>] 
    5. `qubit_flag`: [int] 
    6. `qubits_best_score`: [list<int>]
    7. `qubits_ancilla_oracle`: [list<int>] 
    
    - Method specific paramters:
        - “CQAE”
            1. `CQAE_num_evaluation_qubits`: [int] - Required, the number of precision qubits in QPE
        - “MLQAE”
            1. `MLQAE_is_in_good_subspace`: function(string int) → int - Required, a function that takes a measured string and returns a 1 if the string is in the “good subspace” and a 0 otherwise. For the decoder, a string is in the good subspace if it’s score is greater than the current best. As such, the second input to the function is generally the `best_score`
            2. `MLQAE_um_runs`: [int] - Optional, the number of runs of amplitude amplification used to build the likelihood function
            3. `MLQAE_num_shots`: [int] - Optional, the number of shots per run
            
    - Optional Parameter:
        - `qpu`: [string] which backend to use

## Examples

In this example, we use `exponential_search` to find the maximum value in a classical dataset. The qubit register encodes the index to a value in the dataset and the algorithm will find the index to the maximum value in the set. 

Initialise:

```python
import qbos as qb
import numpy as np
import math
import timeit

tqb = qb.core()
```

e.g., the dataset can be defined as:

```python
dataset = [0,1,1,3,0,1,1,1,2,1,1,1,1,0,1,2]

N = len(dataset)
n_qubits = math.ceil(math.log2(N))
```

- There are 16 elements ⇒ that requires 4 qubits to represent the index)
- If the length is not $`2^n`$, we can pad the dataset (with zero entries) to the nearest $`2^n`$.

Define input parameters and qubit register:

```python
# Registers
trial_score_qubits = [0,1]
trial_qubits = [2,3,4,5]
flag_qubit = 6
best_score_qubits = [7,8]
ancilla_qubits = [9,10,11,12,13]
next_letter = []
next_metric = []
```

Define the state_prep circuit:

```python
#state prep
def state_prep(trial_qubits, trial_score_qubits, a,b):
    circ = qb.Circuit()
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
```

This circuit first applies Hadamard gates to all of the qubits encoding the trial strings (dataset positions). Then it conditionally applies X gates on the trial score qubits to ensure the correct score is entangled with the corresponding to its position in the dataset. 

Define the oracle: 

```python
# Oracle
def oracle(bestScore, num_scoring_qubits, trial_score_qubits, flag_qubit, best_score_qubits, ancilla_qubits):
    circ = qb.Circuit()
    circ.comparator_as_oracle(bestScore,num_scoring_qubits,trial_score_qubits,flag_qubit,best_score_qubits,ancilla_qubits,False)
    return circ
```

We are using the comparator circuit as the main part of the oracle with a phase kickback method.

The scores that we want are encoded in the register so in this case the scoring function is the identity

```python
# Scoring function:
def get_score(x):
    return x
```

Once we have all these components, we can use the `exponential_search` module in a loop to find the maximum value with each of the three methods. This loop can be started with an arbitrary starting best score.

```python
# Can start with any value
bestScore = 0
```

Run canonical exponential search either as a circuit builder object or as a standalone function:

```python
# Success of each run > 1/4 and the number of times we need to change the pivot ~ log(N)
start_canonical_exp_search = timeit.default_timer()
print("Start canonical exponential search!\n")
max_run = 4 *  math.ceil(math.log2(N))
count = 0
while count < max_run:
    result = qb.Circuit().exponential_search(str("canonical"),oracle, state_prep, get_score,bestScore, trial_qubits, trial_score_qubits, [], [],flag_qubit, best_score_qubits, ancilla_qubits)
		# from qbos import exponential_search
		# result = exponential_search(str("canonical"),oracle, state_prep, get_score,bestScore, trial_qubits, trial_score_qubits, [], [],flag_qubit, best_score_qubits, ancilla_qubits)
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
```

An example results log:

```
tart canonical exponential search!

New best score: 1

New best score: 3

Final score: 3
It took 2 runs to find the best score!
Canonical exponential search finished in 7.380996283143759s
```

In the above, we start the maximum search with a guess max value of 0. We run a loop of  `exponential_search` and report better max values as we go. In the end, we report the final max value that it can find.
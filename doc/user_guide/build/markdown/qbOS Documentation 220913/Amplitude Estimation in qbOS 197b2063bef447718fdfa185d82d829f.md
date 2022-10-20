# Amplitude Estimation in qbOS

# Background

## Problem Description

Given an operator (unitary/quantum circuit) $`\mathcal{A}`$   that acts as

$``$
\mathcal{A} |0\rangle = \sqrt{1-a}|\Psi_0\rangle + \sqrt{a}|\Psi_1\rangle
$``$

Quantum Amplitude Estimation is the algorithm to find an estimate for the value of $`\sqrt{a}`$.

**Notes:**

- We can think of $`|\Psi_0\rangle`$ and $`|\Psi_1\rangle`$ as two sub-spaces of the total Hilbert space and we are only interested in the latter, hence trying to determine its amplitude $`a`$.
- This amplitude estimation task is of interest in a variety of quantum algorithms, most notably in financial applications (e.g., [derivative pricing](https://www.nature.com/articles/s41534-019-0130-6)). For our Decoder project, this amplitude estimation procedure is a sub-component of the `ExponentialSearch` algorithm, whereby the amplitude estimate is used to [determine an optimal number of amplitude estimation iterations](https://www.notion.so/Grover-s-Search-Algorithm-4f4d479e09904b59b7214a9d134f037c).

## Approaches

There are two algorithms implemented in qbOS to perform amplitude estimation:

### QPE-based Amplitude Estimation

This is the first approach, proposed in [[Brassard2000](https://arxiv.org/pdf/quant-ph/0005055.pdf)]. In a nutshell, we transform the amplitude estimation task into a phase-estimation procedure for the following Grover operator:

$``$
\mathcal{Q} = \mathcal{A}\mathcal{S}_0\mathcal{A}^\dagger \mathcal{S}_{\Psi_1}
$``$

where $`\mathcal{S}_0`$ and $`\mathcal{S}_{\Psi_1}`$ are the reflections about the $`|0\rangle`$ and $`|\Psi_1\rangle`$, respectively.

- We often encounter reflections about $`|1\rangle`$, which is the $`Z`$ gate. Reflections about $`|0\rangle`$ simply mean the $`X - Z- X`$ gate sequence 💡.
- Reflection about $`|\Psi_1\rangle`$ refers to the oracle circuit, which marks this state, i.e., $`\mathcal{S}_{\Psi_1}|\Psi_1\rangle = -|\Psi_1\rangle`$ .

The circuit to perform QPE looks like:

![Untitled](Amplitude%20Estimation%20in%20qbOS%20197b2063bef447718fdfa185d82d829f/Untitled.png)

which involves the inverse Quantum Fourier Transform circuit ($`IQFT`$ or $`QFT^{-1}`$).

The $`U`$ unitary here is set to the $`\mathcal{Q}`$ unitary defined above in order to perform amplitude estimation.

The lower register, denoted as $`|\Psi\rangle`$ in the above circuit diagram, is the output of the state preparation circuit $`\mathcal{A}`$.

Depending on the target accuracy, we may increase the number of evaluation qubits (those qubits acting as control qubits in the above diagram).

The bit-string result is converted to amplitude value by:

$``$
a = sin(\pi\frac{y}{2^n}) 
$``$

where $`y`$ is the integer value represented by the measured bitstring. Thus, the post-processing of measuring results involves putting bit-strings into amplitude buckets and report the most-likely amplitude value. 

### Maximum Likelihood Estimation (MLE) Technique

This is proposed in [[Yamamoto2020](https://arxiv.org/pdf/1904.10246.pdf)] as a more efficient technique for QAE in the sense that it does not require expensive controlled operations associated with the QPE-based technique above.

This method uses [amplitude amplification](https://www.notion.so/Grover-s-Search-Algorithm-4f4d479e09904b59b7214a9d134f037c) as a subroutine. Essentially it performs a number of runs of amplitude amplification with varying number of iterations and does post-processing to determine the amplitude estimate. We will follow the method in the original paper [[Suzuki2020]](https://arxiv.org/pdf/1904.10246.pdf) where we fix a constant number of shots per run $`N = 1024`$ and vary the number of iterations as an exponentially increasing sequence $`m_0 = 1`$, $`m_1 = 2`$, $`m_2 = 4`$, ... $`m_M = 2^M`$ for some $`M.`$  For larger $`M`$, the more accurate the estimate will be. Proceed as follows:

1. Prepare the advice state
2. Perform amplitude amplification on the advice state with $`m_0`$ iterations and $`N`$ shots. Count the number of shots where the trial score is greater than the current best score $`h_0`$
3. Repeat steps 1 and 2 for $`m_1,...m_M`$ to produce $`h_1,...h_M`$. 
4. Form the Likelihood Function:
$`L(h_i;\theta) = \prod_{i=1}^M [sin^2((2m_i+1)\theta)]^{h_i}[cos^2((2m_i+1)\theta)]^{N-h_i}`$
5. Maximise the likelihood function with respect to $`\theta`$
6. The amplitude estimate is then $`a = sin(\theta)`$

# qbOS API

In qbOS, we have API to perform the canonical amplitude estimation procedure from either the Grover operator circuit ($`\mathcal{Q}`$) or the oracle ($`\mathcal{S}_{\Psi_1}`$), as well as the maximum likelihood amplitude estimation procedure. 

NB: These methods can be implemented as either circuit builder methods, or as standalone functions.

## Amplitude estimation with custom Grover Operator

- `run_canonical_ae(state_prep: Circuit, oracle: Circuit, precision: int, num_state_prep_qubits: int, num_trial_qubits: int, precision_qubits: list(int), trial_qubits: list(int), qpu: string)`
    - `oracle`: a `Circuit` object representing the oracle circuit.
    - `state_prep`: a `Circuit` object representing the state preparation circuit.
    - `precision`: the number qubits encoding the amplitude estimate.
    - `num_state_prep_qubits`: the number of qubits acted on by `state_prep`
    - `num_trial_qubits`: the number of qubits acted on by `state_prep` and the oracle
    - `precision_qubits`: [OPTIONAL] specify the indices of the qubits acted on by `state_prep`
    - `trial_qubits`: [OPTIONAL] specify the indices of the qubits acted on by `state_prep` and `oracle`
    - `qpu`: [OPTIONAL] specify which backend to use (given as a string)
    - Return: JSON object storing the amplitude estimation result.
        - Raw measurement results (in `Measurements` field), these results are post-processed into the following fields:
        - `amplitude-estimation`: the final estimation result.
        - `amplitudes` and `amplitude-probs`: two arrays of the same size representing the amplitude values and corresponding probability values. In other words, the most probable amplitude (highest probability) is selected as the `amplitude-estimation`.

The `grover_op` circuit is constructed manually and is given to the amplitude estimation module.

**Use case:** users have an efficient way to compose the Grover circuit.

## Amplitude estimation with marked state oracle

- `run_canonical_ae_with_oracle(state_prep: Circuit, oracle: Circuit, bits_precision: int, num_state_prep_qubits: int, num_trial_qubits: int, precision_qubits: list(int), trial_qubits: list(int), qpu: string)`
    - `oracle`: a `Circuit` object representing the oracle circuit.
    - `state_prep`: a `Circuit` object representing the state preparation circuit.
    - `bits_precision`: the number qubits encoding the amplitude estimate.
    - `num_state_prep_qubits`: the number of qubits acted on by `state_prep`
    - `num_trial_qubits`: the number of qubits acted on by `state_prep` and the oracle
    - `precision_qubits`: [OPTIONAL] specify the indices of the qubits acted on by `state_prep`
    - `trial_qubits`: [OPTIONAL] specify the indices of the qubits acted on by `state_prep` and `oracle`
    - `qpu`: [OPTIONAL] specify which backend to use (given as a string)
    - Return: JSON object storing the amplitude estimation result.
        - Raw measurement results (in `Measurements` field), these results are post-processed into the following fields:
        - `amplitude-estimation`: the final estimation result.
        - `amplitudes` and `amplitude-probs`: two arrays of the same size representing the amplitude values and corresponding probability values. In other words, the most probable amplitude (highest probability) is selected as the `amplitude-estimation`.

qbOS will construct the Grover operator circuit internally using the formula: $`\mathcal{Q} = \mathcal{A}\mathcal{S}_0\mathcal{A}^\dagger \mathcal{S}_{\Psi_1}`$

In both cases above, the `state_prep` circuit represents the $`\mathcal{A}`$ unitary matrix.

## Maximum Likelihood Quantum Amplitude Estimation

- `run_MLQAE(state_prep: Circuit, oracle: Circuit, is_in_good_subspace: (str,int) -> int, scoring_qubits: array(int), total_num_qubits: int, num_runs: int, shots: int, qpu: string)`
    - `state_prep`: a `Circuit` object representing the state preparation circuit.
    - `oracle`: a `Circuit` object representing the oracle circuit.
    - `is_in_good_subspace`: a function that returns a 1 if the measured string is in the good subspace and a 0 otherwise
    - `total_num_qubits`: the integer number of qubits in the register
    - `score_qubits`: the list of indices of the qubits in the register that you measure to pass to `is_in_good_subspace`
    - `num_runs`: the number of runs of amplitude amplification used to build the likelihood function
    - `shots`: the number of shots per run
    - `qpu`: [OPTIONAL] specify which backend to use (given as a string)
    - Return: JSON object storing the amplitude estimation result.
        - Raw measurement results (in `Measurements` field), these results are post-processed into the following fields:
        - `amplitude-estimation`: the final estimation result.
        - `amplitudes` and `amplitude-probs`: two arrays of the same size representing the amplitude values and corresponding probability values. In other words, the most probable amplitude (highest probability) is selected as the `amplitude-estimation`.

qbOS will construct the Grover operator circuit internally using the formula: $`\mathcal{Q} = \mathcal{A}\mathcal{S}_0\mathcal{A}^\dagger \mathcal{S}_{\Psi_1}`$

Again, w.r.t. the Decode project, $`\mathcal{A} \equiv U_\mu`$ and $`\mathcal{S}_{\Psi_1} \equiv cmp \circ F`$.

# Examples

In the following examples, we’ll use qbOS amplitude estimation modules to estimate the amplitude ($`p`$) of $`|1\rangle`$ from a quantum state prepared in the state:

$`|\Psi\rangle = \sqrt{1-p}|0\rangle + p |1\rangle`$

## Use the oracle

To detect $`|1\rangle`$, we just need a $`Z`$  gate as the oracle. The above state is prepared with an $`R_y(\theta)`$ gate whereby $`\theta`$ is computed from the amplitude value that we want to set.

```python
import numpy as np
import qbos as qb
from qbos import run_canonical_ae_with_oracle
tqb = qb.core()
p = 0.24
theta_p = 2 * np.arcsin(np.sqrt(p))

# State prep circuit: (preparing the state that we want to estimate the amplitude)
state_prep = qb.Circuit()
state_prep.ry(0, theta_p)

# In this case, we don't construct the Grover operator by ourselves,
# instead, just provide the oracle to detect the marked state (|1>)
oracle = qb.Circuit()
oracle.z(0)
bits_precision = 8

# Execute:
result = run_canonical_ae_with_oracle(state_prep, oracle, bits_precision, 1, 1)
```

## Use the Grover operator

In this simple example, we can manually compute $`\mathcal{G}`$ as:

$``$
\mathcal{G} = \mathcal{A}\mathcal{S}_0\mathcal{A}^\dagger \mathcal{S}_{\Psi_1} = R_y(\theta)(XZX)R_y(-\theta) Z = R_y(2\theta)
$``$

This can be used to feed into the amplitude amplification procedure directly.

💡An optimized Grover operator circuit (lower depth) will result in an optimal circuit for hardware execution.

```python
import numpy as np
import qbos as qb
tqb = qb.core()
p = 0.27
theta_p = 2 * np.arcsin(np.sqrt(p))

# State prep circuit: (preparing the state that we want to estimate the amplitude)
state_prep = qb.Circuit()
state_prep.ry(0, theta_p)

# Grover operator circuit:
grover_op = qb.Circuit()
grover_op.ry(0, 2*theta_p)

bits_precision = 8

# Execute:
result = run_canonical_ae(state_prep, grover_op, bits_precision, 1, 1)
print("Result:\n", result)
```

In both cases, qbOS will return same result:

Along with the raw measurement results (in `Measurements` field), we’ll see post-processing results in the following fields:

- `amplitude-estimation`: the final estimation result.
- `amplitudes` and `amplitude-probs`: two arrays of the same size representing the amplitude values and corresponding probability values. In other words, the most probable amplitude (highest probability) is selected as the `amplitude-estimation`.

```json
{
    "AcceleratorBuffer": {
        "name": "qreg_0x23b4400",
        "size": 9,
        "Information": {
            "amplitude-estimation": 0.275193989276886,
            "amplitude-probs": [
                0.0009765625,
                0.0009765625,
                0.0009765625,
                0.0009765625,
                0.0009765625,
                0.0029296875,
                0.0029296875,
                0.0009765625,
                0.0068359375,
                0.005859375,
                0.0068359375,
                0.0087890625,
                0.01953125,
                0.052734375,
                0.3544921875,
                0.44921875,
                0.0400390625,
                0.0126953125,
                0.01171875,
                0.001953125,
                0.0048828125,
                0.001953125,
                0.0009765625,
                0.001953125,
                0.0009765625,
                0.0009765625,
                0.0009765625,
                0.0009765625,
                0.0009765625,
                0.0009765625,
                0.0009765625,
                0.0009765625
            ],
            "amplitudes": [
                0.07757300138473511,
                0.09120800346136093,
                0.12139599770307541,
                0.12952400743961335,
                0.15523000061511994,
                0.17341400682926179,
                0.18280300498008729,
                0.1923840045928955,
                0.20215000212192536,
                0.21209600567817689,
                0.22221499681472779,
                0.23250100016593934,
                0.2429489940404892,
                0.25355100631713869,
                0.26430198550224306,
                0.275193989276886,
                0.28622201085090639,
                0.2973789870738983,
                0.30865800380706789,
                0.32005199790000918,
                0.33155500888824465,
                0.3431589901447296,
                0.35485801100730898,
                0.3666439950466156,
                0.3785099983215332,
                0.39044898748397829,
                0.40245500206947329,
                0.4509910047054291,
                0.6214900016784668,
                0.7026209831237793,
                0.7777850031852722,
                0.8704760074615479
            ]
        },
        "Measurements": {
            "00001011": 8,
            "00001100": 4,
            "00010100": 4,
            "00010101": 1,
            "00011011": 3,
            "00011100": 1,
            "00100011": 1,
            "00100100": 2,
            "00101011": 189,
            "00101100": 1,
            "00110100": 174,
            "00111011": 1,
            "01001011": 22,
            "01001100": 1,
            "01010011": 1,
            "01010100": 9,
            "01011011": 5,
            "01100100": 2,
            "01101011": 11,
            "01101101": 1,
            "01110011": 4,
            "01110100": 19,
            "01111000": 1,
            "01111001": 1,
            "10001010": 1,
            "10001011": 8,
            "10001100": 2,
            "10010011": 1,
            "10010100": 3,
            "10011000": 1,
            "10011011": 3,
            "10100100": 1,
            "10101011": 30,
            "10110100": 233,
            "10111011": 1,
            "11000100": 2,
            "11000111": 1,
            "11001011": 227,
            "11001100": 2,
            "11010011": 2,
            "11010100": 24,
            "11100100": 3,
            "11101000": 1,
            "11101011": 6,
            "11110100": 5,
            "11111011": 1
        }
    }
}
```

## Maximum Likelihood Quantum Amplitude Estimate

When running MLQAE we have freedom to choose the number of runs $`M`$ and the number of shots per run $`N`$. For efficiency, we want $`M`$ and $`N`$ to be as small as possible. However, increasing $`M`$ and $`N`$ will increase accuracy of the estimate. In the following example, we produce a plot of the amplitude estimate obtained from the MLQAE algorithm as a function of $`M`$ and $`N`$. 

First, import relevant libraries and set up the circuit core

```python
import numpy as np
import qbos as qb
import ast
import matplotlib.pyplot as plt
from mpl_toolkits import mplot3d
tqb = qb.core()
```

Next, set up the necessary inputs

```python
# Actual Amplitude
p = 0.24
theta_p = 2 * np.arcsin(np.sqrt(p))

# State prep circuit: (preparing the state that we want to estimate the amplitude)
state_prep = qb.Circuit()
state_prep.ry(0, theta_p)

# In this case, we set up lists of num_runs and shots to loop through
oracle = qb.Circuit()
oracle.z(0)
num_runs = [1,2,3,4,5,6,7,8,9,10]
shots = [20,40,60,80,100,150,200,250,300,500,750,1000]
best_score = 0
total_num_qubits = 1
score_qubits = [0]
```

Now, perform the simulation and extract the results

```python
# Execute:
data = []
for i in num_runs:
    for j in shots:
        result = qb.Circuit().run_MLQAE(state_prep, oracle, i, j, best_score, score_qubits, total_num_qubits)
        result_dict = ast.literal_eval(result)
        amplitude_estimate = float(result_dict['AcceleratorBuffer']['Information']['amplitude-estimation'])
        data_val = [num_runs, shots, amplitude_estimate]
        data.append(data_val)
```

Finally, plot the results

```python
# Plot results
fig = plt.figure()
ax = plt.axes(projection ='3d')
x = [data[i][0] for i in range(len(data))]
y = [data[i][1] for i in range(len(data))]
z = [data[i][2] for i in range(len(data))]
c = z
ax.scatter(x,y,z,c=c,cmap=cm.coolwarm)
lengthx = len(x)
lengthy = len(y)
X,Y = np.meshgrid(x,y)
actual_amplitude = np.array([np.sqrt(p)]*(lengthx*lengthy)).reshape(lengthx,lengthy)
ax.plot_surface(X,Y,actual_amplitude,color='green',alpha=0.1)
ax.set_title('MLQAE as a function of #runs and #shots')
ax.set_xlabel("num_runs")
ax.set_ylabel("num_shots")
ax.set_zlabel("Amplitude Estimate")
plt.savefig("MLQAE_parameter_test.png")
```

The following plot is produced

![Untitled](Amplitude%20Estimation%20in%20qbOS%20197b2063bef447718fdfa185d82d829f/Untitled%201.png)

The horizontal green surface depicts the actual amplitude value. From the plot we can see that the number of shots is far less important than the number of runs. We can get a good estimate with 6 runs and 100 shots. 

# Methods Comparison

Here we compare the two methods presented for QAE:

1. Canonical QAE (based on QPE)
2. MLQAE

## Accuracy

1. For canonical QAE, the accuracy is limited by the number of evaluating qubits used in the QPE routine. Using $`n`$ qubits will ensure that you are within $`\frac{1}{2^n}`$ of the correct $`\theta`$ value.
2. In the MLQAE algorithm, we find the maximum of the likelihood function by using a grid search. For a large number of runs and shots, the accuracy of this algorithm will be limited by the coarseness of this grid. In the example above, we use an accuracy of $`\pm 0.00001`$. To obtain comparable accuracy using canonical QAE, 16 or 17 evaluating qubits would be required - $`log(1/\epsilon)`$.  

## Circuit Width

Both methods require implementation of the operator $`Q = \mathcal{A}S_0\mathcal{A}^\dag S_f`$. Say that $`Q`$ acts on $`n`$ qubits. Then:

1. Canonical QAE requires $`n+log(1/\epsilon)`$ qubits to achieve accuracy $`\epsilon`$
2. MLQAE requires $`n`$ qubits

## $`Q`$ Query Complexity

The circuit depth of each implementation is dominated by the number of $`Q`$ queries. 

1. For accuracy $`log(1/\epsilon)`$, canonical QAE requires $`2^{log(1/\epsilon)}-1`$ queries to $`Q`$. 
2. Using $`M`$ runs, MLQAE requires $`2^M-1`$ queries to $`Q`$.

## Runtime

For a fair comparison we choose parameters such that both algorithms will have accuracy approximately $`\pm 0.001`$. The test is to estimate $`\sqrt{p}`$ in the 1-qubit state $`\sqrt{1-p}\ket{0} + \sqrt{p}\ket{1}`$. We set $`p = 0.24`$ so that $`\sqrt{p} = 0.4898979486`$

|  | Canonical QAE | MLQAE |
| --- | --- | --- |
| Number of qubits | 1 + 10 | 1 |
| ⁍ Queries | 1023 | 63 |
| Amplitude Estimate | 0.4902264719778195 | 0.48999115164423657 |
| Estimate - Actual Amplitude | 0.00032852342118389055 | 9.320308760096818e-05 |
| Runtime (s) | 15.69465487706475 | 0.030203352915123105 |
# Quantum Comparator in qbOS

# Introduction

In some applications it is necessary to compare two values to determine which is larger. To achieve this, the two values are encoded as bit strings within quantum registers and the comparator circuit (see below) is implemented. Then by measuring two “flag” qubits, it is possible to determine which of the two bit strings was larger. 

We require the comparator for the exponential search sub-routine: we want to test whether the trial score is greater than the current best score. In particular, given a trial state $\ket{\psi}$ and scoring function $F$, the comparator should work as:

$cmp \circ F\ket{\psi} = \begin{cases} 1 & \text{trial score > test score} \\ 0 & \text{otherwise} \end{cases}$

## Original Circuit Implementation

Given two bit strings $\ket{a},\ket{b}$ of length $n$, the comparator circuit uses $3n-1$ ancilla qubits to perform the operation:

$U_{CMP}\ket{a}\ket{b}\ket{0^{\otimes 3n-3}}\ket{0}\ket{0} \mapsto \ket{a}\ket{b}\ket{unimportant}\ket{flag_1}\ket{flag_2}$

Then we have the outcomes:

$\ket{flag_1}\ket{flag_2} = \ket{1}\ket{0}$ if $a>b$

$\ket{flag_1}\ket{flag_2} = \ket{0}\ket{0}$ if $a=b$

$\ket{flag_1}\ket{flag_2} = \ket{0}\ket{1}$ if $a<b$

The following circuit from [[Oliveira2007](https://www.researchgate.net/publication/228574906_Quantum_bit_string_comparator_Circuits_and_applications)] is used to implement $U_{CMP}$: 

![Untitled](Quantum%20Comparator%20in%20qbOS%20036f305a7273456ab87315e6f4cb44db/Untitled.png)

## QB Implementation

For our purposes, we only care about whether the trial score is greater than the best score. That is, we don’t need to distinguish between the cases trial score < best score and trial score = best score. Therefore, we can restrict ourselves to just considering the first flag qubit so that:

$\ket{flag_1} = \ket{1}$  if $TrialScore > BestScore$

$\ket{flag_1} = \ket{0}$ otherwise

In anticipation of integrating this circuit with the exponential search algorithm, we will assume the following register structure:

$\ket{TrialScore}\ket{flag}\ket{BestScore}\ket{ancilla}$

NB: By default we assume that the scores are provided with LSB first qubit ordering

## Turning the Comparator into an Oracle for Amplitude Amplification

The comparator circuit may be used as an oracle in an Amplitude Amplification algorithm / Grover’s Search algorithm by using a phase kickback method on the flag qubit. Recall that an oracle performs the following operation:

$Oracle\ket{\psi} = \begin{cases} -\ket{\psi} & \text{if} \ket{\psi} \text{is in the good subspace} \\ \ket{\psi} & \text{otherwise} \end{cases}$

The overall effect of the comparator is to perform a bit flip on the flag qubit if the trial score is greater than the best score. If we prepare the flag qubit in the $\ket{-} = \frac{1}{\sqrt{2}} (\ket{0}-\ket{1}) = HX\ket{0}$ state then the bit flip will map $\ket{-} \mapsto -\ket{-}$ if the trial score is greater than the best score and will be unchanged otherwise. Hence, the comparator is acting like an oracle!

However, the comparator as shown does not reset the ancilla qubits. When we use the comparator as an oracle, we need to reuse these ancilla, so they must be reset. To do this, we need to:

1. perform the comparator
2. copy the result to an external flag
3. perform the comparator inverse

This is shown in the below picture where the top qubit is the (now external) flag, noting that we’re still using the phase kickback method

![Untitled](Quantum%20Comparator%20in%20qbOS%20036f305a7273456ab87315e6f4cb44db/Untitled%201.png)

Both a stand-alone comparator, and an oracle version of the comparator are implemented in qbOS

# qbOS

## Standard Comparator

New qbOS Python `Circuit` class method: `comparator`

Inputs:

- `best_score`: int value
- `num_scoring_qubits`: int value
- Optional inputs:
    - `trial_score_qubits`: list of ints
    - `flag_qubit`: int value
    - `best_score_qubits`: list of ints
    - `ancilla_qubits`: list of ints
    - `is_LSB`: bool

## As Oracle

New qbOS Python `Circuit` class method: `comparator_as_oracle`

Inputs:

- `best_score`: int value
- `num_scoring_qubits`: int value
- Optional inputs:
    - `trial_score_qubits`: list of ints
    - `flag_qubit`: int value
    - `best_score_qubits`: list of ints
    - `ancilla_qubits`: list of ints
    - `is_LSB`: bool

# Example

## Standard Comparator

Use qbOS’s `comparator` to compare two-bit bit strings.

First, we set up the circuit instance:

```python
import qbos as qb
import numpy as np
tqb = qb.core()
tqb.qb12()
circ = qb.Circuit()
```

Now we set the number of qubits used to encode the scores:

```python
# number of qubits encoding scores
num_scoring_qubits = 2
```

Now define the current BestScore and the TrialScore to be compared:

```python
# BestScore
BestScore = 2

# TrialScore
TrialScore = 3
```

Now we have to encode the TrialScore as a bit string in LSB format:

```python
# TrialScore state preparation
TestScore_bin = bin(TestScore)[2:].zfill(num_scoring_qubits)
TestScore_bin_LSB = [TestScore_bin[-i] for i in range(1, num_scoring_qubits+1)]
for k in range(0,num_scoring_qubits):
    if TestScore_bin_LSB[k] == "1":
        circ.x(k)
```

Note that the BestScore is encoded within the comparator module itself.

Finally, finish building the circuit by adding the comparator and measuring the flag qubit. Recall that the comparator assumes the register structure 

$\ket{TrialScore}\ket{flag}\ket{BestScore}\ket{ancilla}$ 

so that the flag qubit has index num_scoring_qubits:

```python
# Add comparator
circ.comparator(BestScore, num_scoring_qubits)

# Measure flag qubit
circ.measure(num_scoring_qubits
```

Then run the circuit and return the measurement results.

```python
# Run:
tqb.ir_target = circ
tqb.acc = "qpp"
tqb.run()
result = tqb.out_raw[0][0]
print("Result:\n", result)
```

In this example, $TrialScore > BestScore$ so we expect the flag qubit to be in the “1” state:

```python
Result:
 {
    "AcceleratorBuffer": {
        "name": "qreg_0x1ec83f0",
        "size": 9,
        "Information": {},
        "Measurements": {
            "1": 1024
        }
    }
}
```

## As Oracle

The `comparator_as_oracle` method is used within the exponential search example.
# Quantum Phase Estimation in qbOS

# Introduction

Quantum phase estimation (QPE) is a commonly used subroutine in many quantum algorithms (including e.g., Shor’s). In our case it is used as a building block for quantum amplitude estimation (QAE). 

## Problem Background

Given a unitary matrix $U$ with eigenvector $\ket{\psi}$ we can write

$U\ket{\psi} = e^{2\pi i \theta} \ket{\psi}$

for some value $\theta$. We would like to estimate the value of $\theta$. 

## Quantum Phase Estimation

QPE works by:

1. First using a sequence of controlled-U gates to imprint phase information onto the amplitudes of the control qubits
2. Next performing the inverse QFT on the control qubits to transcribe the phase information from the amplitudes onto the states of the qubits themselves
3. By reading the final state of the control qubits one obtains an approximation for the phase, $\tilde{\theta}$. 

Using $k$ control qubits we obtain the best k-bit approximation to $\theta$ with probability $\geq \frac{4}{\pi^2}$. Hence we refer to the control qubits as precision qubits. The circuit diagram for this algorithm is shown below:

![Untitled](Quantum%20Phase%20Estimation%20in%20qbOS%20ede1f2c66aff426f870bf620002c31ee/Untitled.png)

# qbOS

Implemented as `qpe` in the `Circuit` class

Inputs:

- `oracle`: A circuit object representing the unitary $U$.
- `precision`: The number of precision qubits used to approximate the phase [int]
- Optional inputs:
    - `trial_qubits`: the indices of the qubits used to encode $\ket{\psi}$ [list of ints]
    - `precision_qubits`: the indices of the qubits used to approximate the phase [list of ints]

# Example

Use the qpe method to approximate the phase of a U1 gate:

$U1(\theta) \ket{1} = e^{i\theta}\ket{1}$

NB: the qpe method will give an approximation for $\theta/2\pi$.

Set up:

```python
import qbos as qb
import numpy as np
tqb = qb.core()
tqb.qb12()
```

Define input parameters:

```python
# 4-bit precision
nb_bits_precision = 4
trial_qubits = [2]
precision_qubits = [0,1,3,4]
```

Create the oracle:

```python
oracle = qb.Circuit()
oracle.u1(trial_qubits[0], -1.96349540849)
```

Here we are using $\theta = -5\pi/8$. 

Finally, we can define and run the circuit

```python
circ = qb.Circuit()

# State prep: eigen state of the oracle |1>
circ.x(trial_qubits[0])

# Add QPE
circ.qpe(oracle, nb_bits_precision, trial_qubits, precision_qubits)

# Measure evaluation qubits
for i in range(nb_bits_precision):
    circ.measure(evaluation_qubits[i])
    #circ.measure(1+i)

# Check the circuit:
print("OpenQASM:\n", circ.openqasm())

# Run:
tqb.ir_target = circ
tqb.acc = "qpp"
tqb.run()
result = tqb.out_raw[0][0]
print("Result:\n", result)
```

The output is:

```python
Result:
 {
    "AcceleratorBuffer": {
        "name": "qreg_0x2f7cae0",
        "size": 5,
        "Information": {},
        "Measurements": {
            "1101": 1024
        }
    }
}
```

We obtain the output $1101 = 13/16$ which is equivalent to $\theta/2\pi = -5/16$.
# Amplitude Amplification in qbOS

# Background

Amplitude Amplification is a generalisation of Grover’s search algorithm where the states don’t start in an equal superposition and the number of solution states may be greater than 1. It is used in a wide range of quantum algorithms. Notably it is a key component of the exponential search algorithm, the main building block of our decoder module. 

## Problem Description

Given some advice state $`\ket{\psi}`$, we can write it as:

$`\ket{\psi} = sin(\theta)\ket{good} + cos{(\theta)}\ket{bad}`$,

where if we measure a state in the “good” subspace we get a valid solution to our problem. We would like to increase the probability of measuring a good state. 

## Amplitude Amplification

Amplitude amplification is a method of increasing the probability of measuring a good state in your advice state. It works by applying a number of Grover’s operators, each called an iteration. Each amplitude amplification iteration occurs in two steps:

1. Apply an oracle: the oracle marks the states in the good subspace $`S_f\ket{good} \mapsto -\ket{good}`$
2. Apply a diffusion operator: amplifies the marked states. Given by:
    
    $`D = -\mathcal{A}S_0\mathcal{A}^\dag`$
    
    where
    
    - $`\mathcal{A}`$ is the operator that prepares the advice state $`\ket{\psi}`$
    - $`S_0`$ is a reflection about 0

There is an optimal number of iterations, approximately $`\frac{\pi}{4\theta}`$ 

# qbOS API

Implemented as `amplitude_amplification` in the `Circuit` class

Inputs:

- `oracle`: The circuit object for the oracle, $`S_f`$
- `state_prep`: The circuit object for state preparation, $`\mathcal{A}`$
- `power`: The number of iterations to apply, default 1

# Example

```python
# Example demonstrating Grover-based amplitude amplification:
import qbos as qb
import numpy as np
tqb = qb.core()
tqb.qb12()
# Target search state:
# |psi> = |101> - |011>
# Hence, the oracle is f = CZ q0, q2; CZ q1, q2,
# because:
# f |psi> = -|101> + |011> = -|psi>
NB_QUBITS = 3

# Oracle: CZ q0, q2; CZ q1, q2 (see above)
oracle = qb.Circuit()
oracle.cz(0, 2)
oracle.cz(1, 2)

# State preparation:
# Use a standard all Hadamards (equal superposition)
state_prep = qb.Circuit()
for i in range(NB_QUBITS):
  state_prep.h(i)

# Construct full amplitude amplification circuit:
full_circuit = qb.Circuit()
# Add amplitude amplification circuit for the above oracle and state preparation sub-circuits.
full_circuit.amplitude_amplification(oracle, state_prep)
# Add measurement:
full_circuit.measure_all()
# Print the circuit:
print("OpenQASM:\n", full_circuit.openqasm())

# Execute:
tqb.ir_target = full_circuit
tqb.acc = "qpp"
tqb.run()
result = tqb.out_raw[0][0]
print("Result:\n", result)
```
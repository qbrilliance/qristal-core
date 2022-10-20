# Quantum Equality Checker in qbOS

# Introduction

The quantum bit string comparator can tell use whether a given bitstring is greater than, less than, or equal to another bitstring. There may be situations where we only need to check whether or not two quantum registers are equal or not. In this case we can use a simpler circuit. 

## Problem Description

Given two quantum bitstrings of length N:

$a = \ket{a_0a_1...a_{N-1}}$

$b = \ket{b_0b_1...b_{N-1}}$

we want to flag whether or not $a_0=b_0$, ... , $a_{N-1}=b_{N-1}$. 

## Circuit Implementation

Note that if $a_i = b_i$ then $a_i \oplus b_i = 0$. So if we use CX gates with $a_i$ as control and $b_i$ as target, then the $b$ register will be $\ket{00...0}$ if and only if $a=b$. We can then use a multi-controlled gate to switch the flag qubit to a $\ket{1}$ conditional on this result. Then the flag qubit will be a $\ket{1}$ if and only if the two states are equal. 

E.g., the following circuit shows the comparison of the state $\ket{101}$ with the (equal) state $\ket{101}$. 

![Untitled](Quantum%20Equality%20Checker%20in%20qbOS%2020fb41766e754abc814df7e5bae350f6/Untitled.png)

We can then either decompose the large mcx gate using the standard no-ancilla decomposition, or use ancilla qubits to decompose using the less deep amcu method. Both methods are implemented in qbOS.

# qbOS Implementation

## Python Binding

New qbOS Python `Circuit` class method: `equality_checker`

- Required inputs:
    - `qubits_a`: list of int (indices of bitstring a)
    - `qubits_b`: list of int (indices of bitstring b)
    - `flag`: int (index of flag)
- Optional inputs:
    - `use_ancilla`: bool. Defaults to false. (use amcu?)
    - `qubits_ancilla`: list of int (indices of ancilla)

### Example

Use `equality_checker` to compare bitstrings of length 3

Set up

```python
import qbos as qb 
import numpy as np 
import ast
tqb = qb.core()
tqb.qb12()

###
# Testing equality checker
# compare all bitstrings of length 3
###

# Set up inputs
qubits_a = [0,1,2]
qubits_b = [3,4,5]
flag = 6

# Optional inputs for ancilla version
use_ancilla = True
qubits_ancilla = [7,8]
```

Run the check

```python
# First do the no ancilla version
for i in range(8):
    for j in range(8):
        circ = qb.Circuit()
        # Prepare input strings
        bin_i = bin(i)[2:].zfill(3)
        bin_j = bin(j)[2:].zfill(3)
        for k in range(3):
            if bin_i[k] == "1":
                circ.x(qubits_a[k])
            if bin_j[k] == "1":
                circ.x(qubits_b[k])
        # Add equality checker
        circ.equality_checker(qubits_a, qubits_b, flag)
        # Measure flag
        circ.measure(flag)
        # Run
        tqb.ir_target = circ
				tqb.acc = "qpp"
				tqb.run()
				result = tqb.out_raw[0][0]
        # Check results
        if i == j:
            assert("1" in list(res.keys())
        else:
            assert("0" in list(res.keys())
        
# Now the ancilla version
for i in range(8):
    for j in range(8):
        circ = qb.Circuit()
        # Prepare input strings
        bin_i = bin(i)[2:].zfill(3)
        bin_j = bin(j)[2:].zfill(3)
        for k in range(3):
            if bin_i[k] == "1":
                circ.x(qubits_a[k])
            if bin_j[k] == "1":
                circ.x(qubits_b[k])
        # Add equality checker
        circ.equality_checker(qubits_a, qubits_b, flag, use_ancilla, qubits_ancilla)
        # Measure flag
        circ.measure(flag)
        # Run
				tqb.ir_target = circ
				tqb.acc = "qpp"
				tqb.run()
				result = tqb.out_raw[0][0]

        # Check results
        if i == j:
            assert("1" in list(res.keys())
        else:
            assert("0" in list(res.keys())
```
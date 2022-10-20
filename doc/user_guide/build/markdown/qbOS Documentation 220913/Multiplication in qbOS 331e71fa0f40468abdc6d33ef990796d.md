# Multiplication in qbOS

# Introduction

## Problem Description

Given two quantum registers $`|q_1\rangle`$ and $`|q_2\rangle`$, and one or more flag qubits $`|f_i\rangle`$ in a (possibly larger) quantum register

$`|q_1\rangle`$...$`|q_2\rangle`$...$`|f_i\rangle`$

We want to perform a multiplication operation between $`|q_1\rangle`$ and $`|q_2\rangle`$ conditional on $`|f_i\rangle`$. 

## Circuit Implementation

A standard shift and add algorithm is used to perform the mapping

$`|a\rangle|b\rangle \mapsto|a\rangle|b\rangle|a*b\rangle`$.

It requires the two n-bit registers, the 2n-bit result register, and n ancilla qubits. 

# qbOS Implementation

## Multiplication

New qbOS Python `Circuit` class method: `multiplication`

- Required Inputs:
    - `qubits_a`: list of int - the indices of the first register of qubits
    - `qubits_b`: list of int - the indices of the second register of qubits. Must be the same length as the `qubits_a` register
    - `qubits_result`: list of int - the indices of the register of qubits that will store the result of the multiplication. Must be twice the length of the `qubits_a` register
    - `qubit_ancilla`: index of the ancilla qubit. Just one qubit for the purpose of carry qubit in the the ripple carry adder
- Optional Inputs:
    - `is_LSB`: optional bool - indicates whether a and b are encoded with least significant bit first ordering (default is true)

### Example

```python
import qbos as qb
import numpy as np
import ast
tqb = qb.core()
tqb.acc = "qsim"
tqb.sn = 1024
tqb.qb12()
tqb.qn = 9

##########################################################################
# Test 1: qubit_a = 1 = |10>, qubit_b = 3 = |11>,
# qubit_result = 1 x 3 = 3 = |1100>
# Inputs
qubits_a = [0,1]
qubits_b = [2,3]
qubits_result = [4,5,6,7]
qubit_ancilla = 8

circ = qb.Circuit()

# Prepare inputs
circ.x(qubits_a[0])
circ.x(qubits_a[1])
circ.x(qubits_b[0])
circ.x(qubits_b[1])

circ.multiplication(qubits_a, qubits_b, qubits_result, qubit_ancilla, is_LSB=True)

# Measure
for i in range(len(qubits_a)):
  circ.measure(qubits_a[i])
for i in range(len(qubits_b)):
  circ.measure(qubits_b[i])
for i in range(len(qubits_result)):
  circ.measure(qubits_result[i])

# Run circuit
tqb.instring = circ.openqasm()
tqb.run()
print(tqb.out_raw[0])
result = tqb.out_raw[0][0]
res = ast.literal_eval(result)
assert(res["11111001"] == 1024)
```

## Controlled Multiplication

New qbOS Python `Circuit` class method: `controlled_multiplication`

- Required inputs:
    - `qubits_a`: list of int - the indices of the first register of qubits
    - `qubits_b`: list of int - the indices of the second register of qubits
    - `qubits_result`: list of int - the indices of the register of qubits that will store the result of the multiplication. Must be twice the length of the `qubits_a` register
    - `qubit_ancilla`: int - the index of the single ancilla qubit
- Optional inputs:
    - `controls_on`: list of int - the indices of control qubits conditional on being $`|1\rangle`$
    - `controls_off`: list of int - the indices of control qubits conditional on being $`|0\rangle`$
    - `is_LSB`: optional bool - indicates whether a and b are encoded with least significant bit first ordering (default is true)

### Example

```python
import qbos as qb
import numpy as np
import ast
tqb = qb.core()
tqb.qb12()
tqb.qn = 10

##########################################################################
# Test 1: qubit_a = 1 = |10>, qubit_b = 3 = |11>,
# qubit_result = 1 x 3 = 3 = |1100>
# Inputs
qubits_a = [0,1]
qubits_b = [2,3]
qubits_result = [4,5,6,7]
qubit_ancilla = 8
control_on = [9]

circ = qb.Circuit()

# Prepare inputs
circ.x(qubits_a[0])
circ.x(qubits_a[1])
circ.x(qubits_b[0])
circ.x(qubits_b[1])
circ.x(control_on[0])

circ.controlled_multiplication(qubits_a, qubits_b, qubits_result, qubit_ancilla, is_LSB=True, controls_on = control_on)

# Measure
for i in range(len(qubits_a)):
  circ.measure(qubits_a[i])
for i in range(len(qubits_b)):
  circ.measure(qubits_b[i])
for i in range(len(qubits_result)):
  circ.measure(qubits_result[i])

# Run circuit
tqb.ir_target = circ
tqb.nooptimise = True
tqb.noplacement = True
tqb.notiming = True
tqb.output_oqm_enabled = False
tqb.acc = "qsim"
#print(tqb.out_raw[0])
result = tqb.out_raw[0][0]
res = ast.literal_eval(result)
assert(res["11111001"] == 1024)
```
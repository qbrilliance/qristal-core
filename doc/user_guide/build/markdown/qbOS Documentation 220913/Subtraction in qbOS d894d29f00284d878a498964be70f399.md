# Subtraction in qbOS

# Introduction

## Problem Description

Given two quantum registers $`|q_1\rangle`$ and $`|q_2\rangle`$ encoding integers $`q_1`$ and $`q_2`$ we want to be able to compute the difference $`q_1-q_2`$. 

NB: Currently in qbOS we assume that we only every subtract a smaller number from a larger number so that we don’t need to consider signs. I.e. we can perform $`|q_1\rangle - |q_2\rangle`$ iff $`q_1>q_2`$.

## Circuit Implementation

To perform a quantum subtraction of $`a-b`$ where $`a\geq b`$ we implement the following algorithm. Let

$`a = a_1...a_n`$

$`b = b_1...b_n`$

be the n-bit binary representations of these numbers. 

### No Ancilla

We can perform the subtraction a-b using the “one up, one down” carry method. That is, looping from the least significant bit (i = 1) to the most significant bit (i = n) perform the following:

1. Apply a CX gate 
2. Apply a CX gate from $`b_i`$ to $`a_i`$
3. (Except for i = n) Conditional on $`\{a_i,b_i\}`$add “1” to $`[b_{i+1},...,b_n]`$

Then work backwards from i = n-1 to i = 1 to undo step 3.

The circuit then acts as $`|a\rangle|b\rangle \mapsto|a-b\rangle|b\rangle`$

### One Ancilla Bit

Alternatively, given access to one ancilla bit we can compute the subtraction using the identity

$`a-b = (a'+b)'`$

where $`'`$ denotes the ones complement. This method is more efficient than the no ancilla method. The circuit still acts as $`|a\rangle|b\rangle \mapsto|a-b\rangle|b\rangle`$.

# qbOS Implementation

In qbOS we provide two modules, `subtraction` and `controlled_subtraction`

## Subtraction

New qbOS Python `Circuit` class method: `subtraction`

- Required inputs:
    - `qubits_larger`: [list of int] The indices of the qubits encoding the larger of the two numbers
    - `qubits_smaller`: [list of int] The indices of the qubits encoding the smaller of the two numbers
- Optional inputs:
    - `is_LSB`: [bool] Specifies whether the numbers are encoded using least significant bit ordering (OPTIONAL, default is `True`)
    - `qubit_ancilla`: [int] The index of an ancilla qubit. If this input is given then the more efficient subtraction method is implemented.

### Example

Use `subtraction` to perform all possible 5 qubit subtractions. 0-0 (|00000> - |00000>), 1-0, 1-1, 2-0, 2-1, 3-0, ,…,31-29, 31-30, 31-31 (|11111>-|11111>)

```python
import qbos as qb
import numpy as np 
import ast
tqb = qb.core()
tqb.qb12()
tqb.qn = 11

###
# Testing subtraction
# In this example we perform every valid 5-qubit subtraction
###

for i in range(32):
    for j in range(i+1):
        # Set up input registers
        qubits_larger = [0,1,2,3,4]
        qubits_smaller = [5,6,7,8,9] 
				qubit_ancilla = 10

        circ = qb.Circuit()

        # Prepare initial state
        bin_i = bin(i)[2:].zfill(5)
        bin_j = bin(j)[2:].zfill(5)

        for k in range(5):
            if bin_i[k] == '1':
                circ.x(qubits_larger[k])
            if bin_j[k] == '1':
                circ.x(qubits_smaller[k])

        # Perform subtraction
        circ.subtraction(qubits_larger, qubits_smaller, is_LSB = False, qubit_ancilla=qubit_ancilla)

        # Measure
        for k in range(10):
            circ.measure(k)

        tqb.ir_target = circ
        tqb.nooptimise = True
        tqb.noplacement = True
        tqb.notiming = True
        tqb.output_oqm_enabled = False
        tqb.acc = "qsim"
        tqb.run()
        result1 = tqb.out_raw[0][0]
        res1 = ast.literal_eval(result1)
        # print(res1)

        expected_result = i-j
        expected_result_bin = bin(expected_result)[2:].zfill(5)

        expected_output = expected_result_bin + bin_j

        assert(res1[expected_output] == 1024)
```

## Controlled Subtraction

New qbOS Python `Circuit` class method: `controlled_subtraction`

- Required inputs:
    - `qubits_larger`: [list of int] The indices of the qubits encoding the larger of the two numbers
    - `qubits_smaller`: [list of int] The indices of the qubits encoding the smaller of the two numbers
- Optional inputs:
    - `controls_on`: [list of int] The indices of control qubits conditional on being in the $`|1\rangle`$ state
    - `controls_off`: [list of int] The indices of control qubits conditional on being in the $`|0\rangle`$ state
    - `is_LSB`: [bool] Specifies whether the numbers are encoded using least significant bit ordering (OPTIONAL, default is `True`)
    - `qubit_ancilla`: [int] The index of an ancilla qubit. If this input is given then the more efficient subtraction method is implemented.
    
    NB: If no control qubits are provided a standard subtraction operation is performed.
    

### Example

Use `controlled_subtraction` to perform all possible 4 qubit subtractions (0-0 (|0000> - |0000>), 1-0, 1-1, 2-0, 2-1, 3-0, ,…, 15-14, 15-15 (|1111>-|1111>)) with a control on qubit and a control off qubit in all configurations (|00>, |01>, |10>, |11>). We expect the same result as the non-controlled example for |10> controls and nothing to happen otherwise.

```python
import qbos as qb
import numpy as np 
import ast
tqb = qb.core()
tqb.qb12()

###
# Testing subtraction
# In this example we perform every valid 4-qubit subtraction with 2 control bits
###
for c in range(4):
    for i in range(16):
        for j in range(i+1):
            # Set up input registers
            qubits_larger = [0,1,2,3]
            qubits_smaller = [4,5,6,7] 
            control_on = [8]
            control_off = [9]

            circ = qb.Circuit()

            # Prepare initial state
            bin_i = bin(i)[2:].zfill(4)
            bin_j = bin(j)[2:].zfill(4)
            bin_c = bin(c)[2:].zfill(2)

            for k in range(4):
                if bin_i[k] == '1':
                    circ.x(qubits_larger[k])
                if bin_j[k] == '1':
                    circ.x(qubits_smaller[k])

            if bin_c[0] == '1':
                circ.x(control_on[0])
            if bin_c[1] == '1':
                circ.x(control_off[0])

            # Perform subtraction
            circ.controlled_subtraction(qubits_larger, qubits_smaller, controls_on = control_on, controls_off = control_off, is_LSB=False)

            # Measure
            for k in range(8):
                circ.measure(k)

            tqb.ir_target = circ
            tqb.nooptimise = True
            tqb.noplacement = True
            tqb.notiming = True
            tqb.output_oqm_enabled = False
            tqb.acc = "qsim"
            tqb.run()
            result1 = tqb.out_raw[0][0]
            res1 = ast.literal_eval(result1)

            if c == 2:
                expected_result = i-j
                expected_result_bin = bin(expected_result)[2:].zfill(4)

                expected_output = expected_result_bin + bin_j

                assert(res1[expected_output] == 1024)
            else:
                expected_output = bin_i + bin_j

                assert(res1[expected_output] == 1024)
```
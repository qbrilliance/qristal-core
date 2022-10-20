# Proper Fraction Division in qbOS

# Introduction

## Problem Description

Given two quantum registers $`|q_1\rangle`$ and $`|q_2\rangle`$ representing integers $`q_1`$ and $`q_2`$ we want to perform a division operation between $`|q_1\rangle`$ and $`|q_2\rangle`$  

NB: Currently in qbOS we assume that we only every divide a number by a larger number so that the result is a float between 0 and 1. Hence why this is called proper fraction division.

## Circuit Implementation

To perform a quantum division of $`a/b`$ where $`b > a`$ we implement the following algorithm. Let

$`a = a_1...a_n`$

$`b = b_1...b_n`$

be the n-bit binary representations of these numbers. Since we assume $`b>a`$, the result can be written as

$`c_12^{-1} + c_22^{-2} + ... + c_k2^{-k}`$

To compute the division, we use the following algorithm:

1. $`c_1`$ gets turned on if and only if $`2\alpha >\beta`$
2. Conditional on $`c_1`$, map $`\alpha\mapsto 2\alpha-\beta = \alpha_2`$ and $`\beta\mapsto2\beta=\beta_2`$. Otherwise, $`\alpha_2 = \alpha`$ and $`\beta_2 = \beta`$
3. $`c_2`$ gets turned on if and only if $`2^2\alpha_2 \geq \beta_2`$
    
    …
    

The process continues until we reach $`c_k`$ turned on if and only if $`2^k\alpha_k \geq \beta_k`$. 

Following this, the conditional mappings are inverted so that the numerator and denominator registers are returned to their original form such that we achieve the mapping:

$`|\alpha\rangle|\beta\rangle|0\rangle \mapsto |\alpha\rangle|\beta\rangle|\alpha/\beta\rangle`$

# qbOS Implementation

In qbOS we provide two modules, `division` and `controlled_division`

## Division

New qbOS Python `Circuit` class method: `proper_fraction_division`

- Required Inputs:
    - `qubits_numerator`: list of indices of the qubits encoding the numerator.
    - `qubits_denominator`: list of indices of the qubits encoding the denominator. Must be the same length as `qubits_numerator`
    - `qubits_fraction`: list of indices of the qubits that will be used to store the result of the division.
    - `qubits_ancilla`: list of indices of ancilla qubits. To divide n-bit numbers with k-bit output precision we require 5k + 3n - 1 ancilla qubits.
- Optional Inputs:
    - `is_LSB`: bool that indicates whether the registers provided are LSB ordering [OPTIONAL, default is true]

### Example

Use `proper_fraction_division` to compute all valid 3-qubit divisions with 3-bit precision.

```python
import qbos as qb
import numpy as np 
import ast
tqb = qb.core()
tqb.qb12()
tqb.qn = 32

###
# Testing proper fraction division
# In this example we perform every valid 3-qubit proper fraction division with 3 precision bits
###

for i in range(8):
    for j in range(i+1,8):
        # Set up input registers
        qubits_numerator = [0,1,2]
        qubits_denominator = [3,4,5] 
        qubits_fraction = [6,7,8]
        qubits_ancilla = list(range(9, 32))

        circ = qb.Circuit()

        # Prepare initial state
        bin_i = bin(i)[2:].zfill(3)
        bin_j = bin(j)[2:].zfill(3)

        for k in range(3):
            if bin_i[k] == '1':
                circ.x(qubits_numerator[k])
            if bin_j[k] == '1':
                circ.x(qubits_denominator[k])

        # Perform subtraction
        circ.proper_fraction_division(qubits_numerator, qubits_denominator, qubits_fraction, qubits_ancilla, is_LSB = False)

        # Measure
        for k in qubits_numerator:
            circ.measure(k)
        for k in qubits_denominator:
            circ.measure(k)
        for k in qubits_fraction:
            circ.measure(k)
        for k in qubits_ancilla: 
            circ.measure(k)
        
        tqb.ir_target = circ
        tqb.nooptimise = True
        tqb.noplacement = True
        tqb.notiming = True
        tqb.output_oqm_enabled = False
        # tqb.debug = True
        tqb.acc = "sparse-sim"
        tqb.run()
        result1 = tqb.out_raw[0][0]
        res1 = ast.literal_eval(result1)

        # res = circ.execute("sparse-sim", 1024, 34)
        # res1 = ast.literal_eval(res)["AcceleratorBuffer"]["Measurements"]

        print(res1)
        assert(len(res1) == 1)

        if j != 0:
            expected_result = i/j
        else:
            expected_result = 0

        expected_result_bin = ""

        for k in range(1, len(qubits_fraction)+1):
            if expected_result - (1/2**k) >= 0:
                expected_result_bin += "1"
                expected_result -= 1/2**k
            else:
                expected_result_bin += "0"

        expected_output = bin_i + bin_j + expected_result_bin
        for bit in qubits_ancilla:
            expected_output += "0"

        assert(res1[expected_output] == 1024)
```

## Controlled Division

New qbOS Python `Circuit` class method: `controlled_proper_fraction_division`

- Required Inputs:
    - `qubits_numerator`: list of indices of the qubits encoding the numerator.
    - `qubits_denominator`: list of indices of the qubits encoding the denominator. Must be the same length as `qubits_numerator`
    - `qubits_fraction`: list of indices of the qubits that will be used to store the result of the division.
    - `qubits_ancilla`: list of indices of ancilla qubits. To divide n-bit numbers with k-bit output precision we require 5k + 3n - 1 ancilla qubits.
- Optional Inputs:
    - `is_LSB`: boolean that indicates whether the registers provided are LSB ordering [OPTIONAL, default is true
    - `controls_on`: list of indices of control qubits conditional being on
    - `controls_off`: list of indices of control qubits conditional being off

NB: If no control bits are provided a standard proper fraction division is implemented.

### Example

Use `controlled_proper_fraction_division` to compute all valid 4-qubit divisions with 3-bit precision with one control off qubit and one control on qubit in all four configurations (|00>, |01>, |10>, |11>).

```python
import qbos as qb
import numpy as np 
import ast
tqb = qb.core()
tqb.qb12()
tqb.qn = 34

###
# Testing proper fraction division
# In this example we perform every valid 3-qubit proper fraction division with 3 precision bits
###

for c in range(4):
    for i in range(8):
        for j in range(i+1,8):
            # Set up input registers
            qubits_numerator = [0,1,2]
            qubits_denominator = [3,4,5] 
            qubits_fraction = [6,7,8]
            qubits_ancilla = list(range(9, 32))
            controls_on = [32]
            controls_off = [33]

            circ = qb.Circuit()

            # Prepare initial state
            bin_i = bin(i)[2:].zfill(3)
            bin_j = bin(j)[2:].zfill(3)

            if c == 1:
                circ.x(controls_on[0])
            if c == 2:
                circ.x(controls_off[0])
            if c == 3:
                circ.x(controls_on[0])
                circ.x(controls_off[0])

            for k in range(3):
                if bin_i[k] == '1':
                    circ.x(qubits_numerator[k])
                if bin_j[k] == '1':
                    circ.x(qubits_denominator[k])

            # Perform subtraction
            circ.controlled_proper_fraction_division(qubits_numerator, qubits_denominator, qubits_fraction, qubits_ancilla, controls_on, controls_off, is_LSB = False)

            # Measure
            for k in qubits_numerator:
                circ.measure(k)
            for k in qubits_denominator:
                circ.measure(k)
            for k in qubits_fraction:
                circ.measure(k)
            # for k in qubits_ancilla: 
            #     circ.measure(k)
            
            tqb.ir_target = circ
            tqb.nooptimise = True
            tqb.noplacement = True
            tqb.notiming = True
            tqb.output_oqm_enabled = False
            tqb.acc = "sparse-sim"
            tqb.run()
            result1 = tqb.out_raw[0][0]
            res1 = ast.literal_eval(result1)
            assert(len(res1) == 1)
            # print(res1)

            if c == 1:
                if j != 0:
                    expected_result = i/j
                else:
                    expected_result = 0

                expected_result_bin = ""

                for k in range(1, len(qubits_fraction)+1):
                    if expected_result - (1/2**k) >= 0:
                        expected_result_bin += "1"
                        expected_result -= 1/2**k
                    else:
                        expected_result_bin += "0"

                expected_output = bin_i + bin_j + expected_result_bin

                assert(res1[expected_output] == 1024)
            
            else:
                expected_output = bin_i + bin_j
                for k in range(len(qubits_fraction)):
                    expected_output += "0"
                
                assert(res1[expected_output] == 1024)
```
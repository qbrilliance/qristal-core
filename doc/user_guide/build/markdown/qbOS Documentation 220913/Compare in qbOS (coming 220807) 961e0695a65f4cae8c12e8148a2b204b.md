# Compare > in qbOS (coming 220807)

# Introduction

## Problem Description

Given two n-bit strings $`|a\rangle`$ and $`|b\rangle`$ we want to flag a result qubit whenever $`a > b`$. That is, we want to perform the mapping

$`|a\rangle|b\rangle|flag\rangle \mapsto |a\rangle|b\rangle X^{a>b}|flag\rangle`$.

This can be used as a standard on/off flag or within an oracle by preprocessing the flag by $`|-\rangle = HX|0\rangle`$ such that the flipping of the flag acts as $`|-\rangle \mapsto -|-\rangle`$. 

## Circuit Implementation

Let $`|a\rangle`$ and $`|b\rangle`$ be n-bit strings and let ‘ denote the one’s complement of a bit string. Then we know

$`a+a' = |1\rangle^{\otimes n}`$.

Hence, if we perform

$`a'+b`$

the overflow qubit of the sum will be $`|1\rangle`$ if and only if

$`a'+b > a'+a`$

which is true if and only if

$`b>a`$.

Hence the overflow qubit can be used as the result qubit to check if $`b>a`$. Similarly we could perform $`a+b'`$ to check whether $`a>b`$. This is exploited to perform the comparison within qbOS.

# qbOS Implementation

New qbOS Python `Circuit` class method: `compare_gt`

Inputs:

- `qubits_a`: list of int - the indices of the first register of qubits
- `qubits_b`: list of int - the indices of the second register of qubits
- `qubit_flag`: int - the flag that will be flipped if $`a>b`$
- `qubit_ancilla`: int - a single ancilla is required for this circuit
- `is_LSB`: optional bool - indicates whether a and b are encoded with least significant bit ordering (default is true)

### Example

Use `compare_gt` to compare all possible combinations of two 5-bit strings

```python
import qbos as qb 
import ast 

tqb = qb.core()
tqb.qb12()

for i in range(32):
    for j in range(32):

        qubits_a = [0,1,2,3,4]
        qubits_b = [5,6,7,8,9]
        qubit_flag = 10
        qubit_ancilla = 11

        circ = qb.Circuit()

        a_bin = bin(i)[2:].zfill(5)
        b_bin = bin(j)[2:].zfill(5)

        for k in range(5):
            ak = a_bin[k]
            bk = b_bin[k]
            if ak == '1':
                circ.x(qubits_a[k])
            if bk == '1':
                circ.x(qubits_b[k])
        
        circ.compare_gt(qubits_a, qubits_b, qubit_flag, qubit_ancilla, is_LSB = False)

        circ.measure_all()

        tqb.ir_target = circ
        tqb.nooptimise = True
        tqb.noplacement = True
        tqb.notiming = True
        tqb.output_oqm_enabled = False
        tqb.qn = 12
        tqb.sn = 1024
        tqb.acc = "qsim"
        tqb.run()
        result1 = tqb.out_raw[0][0]
        res1 = ast.literal_eval(result1)

        expected_output = a_bin + b_bin
        if i > j:
            expected_output += "1"
        else:
            expected_output += "0"
        expected_output += "0"

        assert(res1[expected_output] == 1024)
```
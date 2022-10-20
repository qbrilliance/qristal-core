# Inverse Circuit in qbOS

# Introduction

## Problem Description

Given a circuit, we want to be able to easily generate its inverse. This is useful in many situations including resetting ancilla qubits, and generating Grover’s operators.

## Implementation

Since all quantum gates are unitary, to produce the inverse of a circuit it suffices to reverse the order of gates and convert gates to their conjugate transpose (i.e., $`U \mapsto U^\dagger`$). That is, the circuit:

$`U = U_NU_{N-1}...U_2U_1`$

becomes

$`U^{-1} = U_1^\dagger U_2^\dagger ... U_{N-1}^\dagger U_N^\dagger`$

# qbOS Implementation

New qbOS Python `Circuit` class method: `inverse_circuit`

- Required inputs:
    - `circ`: A `CircuitBuilder` object

### Example

Use `inverse_circuit` to find the inverse of the QFT circuit.

```python
		import qbos as qb 
    import ast

    tqb = qb.core()
    tqb.qb12()
    tqb.nooptimise = True
    tqb.noplacement = True
    tqb.notiming = True
    tqb.output_oqm_enabled = False
    tqb.acc = "qpp"
    tqb.sn = 1024

    qubits = [0,1,2,3]
    circ = qb.Circuit()

    qft = qb.Circuit()
    qft.qft(qubits)

    circ.qft(qubits)
    circ.inverse_circuit(qft)
    for i in qubits:
        circ.measure(i) 
    
    tqb.ir_target = circ
    tqb.run()
    result = tqb.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(res["0000"] == 1024)
```
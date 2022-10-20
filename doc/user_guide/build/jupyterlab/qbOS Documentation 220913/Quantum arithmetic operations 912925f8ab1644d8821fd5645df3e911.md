# Quantum arithmetic operations

# Introduction

Quantum algorithms for commonly used arithmetic operations have been published [1,2] and need only to be implemented. An improved form of division is given in reference [2], which also provides benchmarking on IBM-Q.

# Addition and subtraction

All inputs under this heading are integers corresponding to qubit number. 

The addition of two numbers in binary notation is based on repeated application of the full adder score (below), which is in turn based on the half adder score.

## Half adder score

This adds two qubits and serves as a building block for an addition score and comprises a Toffoli gate, acting on $Y$, followed by a CNOT gate, acting on $X_1$.

$|X,Y,Z=0\rangle \longrightarrow |X,Y, C_1 = XY \oplus Z\rangle \longrightarrow |X,S_1 = X \oplus Z, C_1\rangle$

$S_1$ is the least significant qubit in the sum of $X, Y$ while $C_1$ is the value carried over to the next qubit. 

```jsx
CCX X Y Z
CX X Y
```

For further details see [1].

### Tests

The following imports are necessary and sufficient for all tests on this page:

```python
import sys, os
import numpy as np
import qbos 
import qbos_base_algorithms 
from qbos_base_algorithms.python import bitstring_conversion as bsc
from qbos_base_algorithms import qubit_index_conversion as qic
from qbos_base_algorithms import qbos_arithmetic as arith
```

The following test for the half-adder score is run from the plugins/ directory:

```python
qubits = (0,1,2)
qubit_X = qubits[0]
qubit_Y = qubits[1]
qubit_carry = qubits[2]

tqb = qbos.core()
tqb.qb12()

non_zeros = ([qubit_X,qubit_Y],[qubit_X],[qubit_Y],[])
outcomes = ("101","110","010","000")
for _test in range(len(non_zeros)):
  print("Running half-adder test %i"%_test)
  quantum_score_string = '__qpu__ void QBCIRCUIT(qreg q) {\nOPENQASM 2.0;\ninclude "qelib1.inc";\ncreg c[%i];\n' % len(qubits)
  for _qubit in non_zeros[_test]:
    quantum_score_string += 'x q[%i] ; \n' % _qubit
  quantum_score_string += arith.half_adder(qubit_X,qubit_Y,qubit_carry)
  measure_string = ""
  for qubit in (qubit_X,qubit_Y,qubit_carry):
    measure_string += "measure q[%i] -> c[%i] ; "%(qubit,qubit)
  quantum_score_string += measure_string
  quantum_score_string += '\n}'  
  tqb.instring = quantum_score_string
  tqb.sn = 1
  tqb.acc = "aer"
  tqb.run()
  output = tqb.out_count[0][0]
  output_keys = tuple(output.keys())
  assert len(output_keys) == 1 , "Should be just one outcome, not " + len(output_keys)
  assert output_keys[0] == bsc.BitstringToBase10(outcomes[_test]) , "Output should be %s not %s" %(outcomes[_test],bsc.Base10ToBitstring(output_keys[0]))
  print("half-adder test %i passed successfully"%_test)
```

## Full adder score

This is needed to add two binary numbers with arbitrary numbers of qubits.  it is a superposition of two half-adders and an extra CNOT gate. When adding the $i^{th}$ qubits in $X,Y$ it will also include the carry-over from the $(i-1)^{th}$ qubits.

$|X_i,Y_i,C_{i-1},0\rangle \longrightarrow |X_i,Y_i,C_{i-1}, X_iY_i\rangle \longrightarrow |X_i,S'_i = Y_i \oplus X_i, C_{i-1}, X_iY_i\rangle \longrightarrow |X_i,S'_i,C_{i-1}, C_i = S'_iC_{i-1} \oplus X_iY_i \rangle\longrightarrow |X_i,S'_i,S_i = C_{i-1}\oplus S'_i,C_i\rangle \longrightarrow |X_i,Y_i,S_i,C_i\rangle$

The output qubits are the last two, where the carry-over qubit $\ket{C_i}$ is fed into the next full-adder. Note that the last step, a CNOT controlled by $X_i$ acting on $Y_i$, is optional if only the sum and carry qubits are wanted. The pseudo-code is

```jsx
CCX X Y Z 
CX X Y
CCX Y C0 Z
CX Y C0
CX X Y
```

This is based on the description in [3], that of [1] being confusing and probably error-ridden.

### Tests

```python
qubits = (0,1,2,3)
qubit_X = qubits[0]
qubit_Y = qubits[1]
qubits_carry = qubits[2:4]

tqb = qbos.core()
tqb.qb12()

non_zeros = ([qubit_X,qubit_Y],[qubit_X],[qubit_Y],[],[qubit_X,qubit_Y,qubits_carry[0]],[qubit_X,qubits_carry[0]],[qubit_Y,qubits_carry[0]],[qubits_carry[0]])
outcomes = ["1101","1010","0110","0000","1111","1001","0101","0010"]
for _test in range(len(non_zeros)):
  print("Running half-adder test %i"%_test)
  quantum_score_string = '__qpu__ void QBCIRCUIT(qreg q) {\nOPENQASM 2.0;\ninclude "qelib1.inc";\ncreg c[%i];\n' % len(qubits)
  for _qubit in non_zeros[_test]:
    quantum_score_string += 'x q[%i] ; \n' % _qubit
  quantum_score_string += arith.full_adder(qubit_X,qubit_Y,qubits_carry[0],qubits_carry[1],True)
  measure_string = ""
  for qubit in (qubit_X,qubit_Y,qubits_carry[0],qubits_carry[1]):
    measure_string += "measure q[%i] -> c[%i] ; "%(qubit,qubit)
  quantum_score_string += measure_string
  quantum_score_string += '\n}'
  tqb.instring = quantum_score_string
  tqb.sn = 1
  tqb.acc = "aer"
  tqb.run()
  output = tqb.out_count[0][0]
  output_keys = tuple(output.keys())
  assert len(output_keys) == 1 , "Should be just one outcome, not " + len(output_keys)
  assert output_keys[0] == bsc.BitstringToBase10(outcomes[_test]) , "Output should be %s not " %(outcomes[_test],bsc.Base10ToBitstring(output_keys[0]))
  print("full-adder test %i passed successfully"%_test)
```

### Addition tests

```python
qubits = (0,1,2,3,4,5,6)
qubits_X = qubits[:2]
qubits_Y = qubits[2:4]
qubits_carry = qubits[4:]

tqb = qbos.core()
#tqb = q.qbqe()
tqb.qb12()

for _value_X in range(4):
  _non_zero_X = [_qubit for _qubit in range(4) if _value_X & 2**_qubit]
  quantum_score_X = ''
  for _qubit in _non_zero_X:
    quantum_score_X += 'x q[%i] ; \n' % _qubit
  for _value_Y in range(4):
    _test = _value_X + _value_Y*(2**(len(qubits_X)))
    _non_zero_Y = [_qubit + max(qubits_X) + 1 for _qubit in range(len(qubits_Y)) if _value_Y & 2**_qubit]
    quantum_score_Y = ''
    for _qubit in _non_zero_Y:
      quantum_score_Y += 'x q[%i] ; \n' % _qubit
    print("Running addition test %i.%i"%(_value_X,_value_Y))
    quantum_score_string = '__qpu__ void QBCIRCUIT(qreg q) {\nOPENQASM 2.0;\ninclude "qelib1.inc";\ncreg c[%i];\n' % (len(qubits))#-len(qubits_carry))
    quantum_score_string +=  quantum_score_X
    quantum_score_string +=  quantum_score_Y
    quantum_score_string += arith.addition(qubits_X,qubits_Y,qubits_carry,True)
    measure_string = ""
    for qubit in qubits_X:
      measure_string += "measure q[%i] -> c[%i] ; "%(qubit,qubit)
    for qubit in qubits_Y:
      measure_string += "measure q[%i] -> c[%i] ; "%(qubit,qubit)
    for qubit in qubits_carry:
      measure_string += "measure q[%i] -> c[%i] ; "%(qubit,qubit)
    quantum_score_string += measure_string
    quantum_score_string += '\n}'
    tqb.instring = quantum_score_string
    tqb.sn = 1
    tqb.acc = "aer"
    tqb.run()
    output = qic.non_zero_qindices(tqb.out_count[0][0],len(qubits))
    output_keys = tuple(output.keys())
    assert len(output.keys()) == 1 , "Should be just one outcome, not " + len(output.keys())
    output_key = list(output.keys())[0]
    # Check that lowest member of qubits_Y gives correct result
    if output_key == "zero":
      output_Y = []
    else:
      output_Y = [_qubit for _qubit in qubits_Y if _qubit in output_key and _qubit != min(qubits_Y)]
    if min(qubits_Y) in _non_zero_Y and min(qubits_X) in _non_zero_X:
      assert qubits_Y not in output_key , "output_key[min(Y)] should be '0' for input '1+1'"
    elif min(qubits_Y) not in _non_zero_Y and min(qubits_X) not in _non_zero_X:
      assert output_key == "zero" or min(qubits_Y) not in output_key , "output_key[min(Y)] should be '0' for input '0+0'"
    elif output_key != "zero":
      assert min(qubits_Y) in output_key , "output_key[min(Y)] should be '1' for inputs '1+0' or '0+1'"
    
    if len(_non_zero_Y) > 1:
      assert output_Y == _non_zero_Y[1:] , "Non-minimal Y qubits %s should remain as %s" %(output_Y,_non_zero_Y[1:])
    elif len(_non_zero_Y) == 1:
      assert _non_zero_Y[0] == min(qubits_Y) or _non_zero_Y[0] == output_Y[0] , "Error in non-minimal Y qubits " + str(output_Y)
    elif len(output_Y) > 0:
      assert output_Y[0] == min(qubits_Y) and output_Y[-1] == min(qubits_Y) ,  "Error in non-minimal Y qubits " + str(output_Y)
    else:
      assert len(output_Y) == 0 ,  "Error in non-minimal Y qubits " + str(output_Y)
    total = 0
    if output_key != "zero" and min(qubits_Y) in output_key:
      total = 1
    if output_key == "zero":
      output_carry = []
    else:
      output_carry = [_qubit - min(qubits_carry) for _qubit in qubits_carry if _qubit in output_key ]
    for _qindex in output_carry:
      total += 2**(_qindex+1)
    assert total == _value_X + _value_Y , "%i + %i should yield %i not %i" %(_value_X,_value_Y,_value_X + _value_Y,total)
  output_X = [_qubit - min(qubits_X) for _qubit in qubits_X if _qubit in output_key]
  assert output_X == _non_zero_X , "X qubits %s should remain as %s" %(output_X,_non_zero_X)
  print("addition test %i passed successfully"%_value_X)
```

## Subtraction

This can be treated as adding a negative number and does not require a new score. The subtracted number is made negative using the standard convention of taking the two's complement, *ie.* invert the qubits and add one  , *e.g.* $01001011 \rightarrow 10110101$, *ie.*

$1101 - 0110 = (1101 + 1010) = 0111$.

# References

[1] Fahdil, Moayad A., Ali Foud Al-Azawi, and Sammer Said. "Operations algorithms on quantum computer." *IJCSNS* 10.1 (2010): pp 85.

[2] Singh, Shreya, P. Prathamesh, and Prasanta K. Panigrahi. "Demonstration of a Quantum Calculator on IBM Quantum Experience Platform and its Application for Conversion of a Decimal Number to its Binary Representation." (ResearchGate)

[3] QuTech, Quantum Inspire, [https://www.quantum-inspire.com/kbase/full-adder/](https://www.quantum-inspire.com/kbase/full-adder/)
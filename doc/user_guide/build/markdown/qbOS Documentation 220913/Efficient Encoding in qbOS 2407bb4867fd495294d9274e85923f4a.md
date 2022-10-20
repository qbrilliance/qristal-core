# Efficient Encoding in qbOS

# Introduction

In some cases we want to be able to encode data in quantum states. To do this, we often want to entangle states in one register with corresponding “scores” in a second register. As an example, the first register might correspond to a location in a table and its score corresponds to the value in the table at that location:

![Untitled](Efficient%20Encoding%20in%20qbOS%202407bb4867fd495294d9274e85923f4a/Untitled.png)

NB: By default we use LSB qubit ordering in our circuit implementations, but the example shown here assumes MSB ordering. 

## Naïve Circuit Implementation

To do this, we first prepare the state register (often by forming an equal superposition using $`H`$ gates). Then we use conditional operations to inscribe the score on the score register conditional on the state. In the example above, this looks like:

![Untitled](Efficient%20Encoding%20in%20qbOS%202407bb4867fd495294d9274e85923f4a/Untitled%201.png)

Open circle controls indicate the operation is conditional on the control being in the $`\ket{0}`$ state. We implement these conditionals by performing bit flips. Including all $`X`$ gates the circuit becomes:

![Untitled](Efficient%20Encoding%20in%20qbOS%202407bb4867fd495294d9274e85923f4a/Untitled%202.png)

## Gray Code Bit Flips

Our goal then is to make this process as efficient as possible. One way we can do this is by using gray code for the bit flipping process mentioned above. Gray code is a way to cycle through all binary bitstrings of a given length by flipping only one bit at each step:

![Untitled](Efficient%20Encoding%20in%20qbOS%202407bb4867fd495294d9274e85923f4a/Untitled%203.png)

Using this ordering will reduce the number of bit flips in our circuit. The example above becomes:

![Untitled](Efficient%20Encoding%20in%20qbOS%202407bb4867fd495294d9274e85923f4a/Untitled%204.png)

## MCX

Efficiently decomposing mcx gates into one and two qubit gates has been widely studied. The qbOS implementation of mcx relies on the XACC C-U gate which uses an efficient gray code decomposition to implement multi-controlled gates (this gray code method is distinct from the one mentioned above). For more details see:
[Phys. Rev. Lett. 92, 177902 (2004) - Efficient Decomposition of Quantum Gates (aps.org)](https://journals.aps.org/prl/abstract/10.1103/PhysRevLett.92.177902)

his decomposition of mcx doesn’t use any ancilla qubits to minimise circuit width. However, for the decoder application, it is potentially worth trading a few ancilla qubits for a significant reduction in gate depth. We can decompose multi-controlled unitaries using ancilla in the following way taken from [[NielsenChuang](http://mmrc.amss.cas.cn/tlb/201702/W020170224608149940643.pdf?msclkid=604aa4c0b62811ec8e945e1e09ce6693)]

![Untitled](Efficient%20Encoding%20in%20qbOS%202407bb4867fd495294d9274e85923f4a/Untitled%205.png)

We can then compare this implementation to the XACC implementation in terms of number of CX gates and number of single qubit (SQ) gates:

![Untitled](Efficient%20Encoding%20in%20qbOS%202407bb4867fd495294d9274e85923f4a/Untitled%206.png)

MCX:

- Uses XACC gray-code decomposition of C-U.
- # CX follows the recursion $`CX(n) = 2CX(n-1) + 4`$
- # SQ follows the recursion $`SQ(n) = 2SQ(n-1) + 3`$

AMCU:

- Uses ancilla qubits to decompose multi-controls.
- # CX follows the closed form relation $`CX(n) = 12(n-1) + CU`$
- # SQ follows the closed form relation $`SQ(n) = 18(n-1)`$

Both methods are implemented in qbOS.

# qbOS Implementation

## Python Binding

New qbOS Python `Circuit` class method: `efficient_encoding`

- Required inputs:
    - `scoring_function`: a Python function int → int
    - `num_state_qubits`: int
    - `num_scoring_qubits`: int
- Optional inputs:
    - `state_qubits`: list of ints
    - `scoring_qubits`: list of ints
    - `is_LSB`: bool, default True
    - `use_ancilla`: bool, default False
    - `ancilla_qubits`: list of ints

### Example

Use `efficient_encoding` to prepare the state $`\ket{000}\ket{000} + \ket{111}\ket{111}`$

Set up:

```python
import qbos as qb
import numpy as np
import ast
tqb = qb.core()
```

Define input parameters:

```python
# scoring function
def scoring_function(i):
    return i

# input parameters
num_state_qubits = 3
num_scoring_qubits = 3
```

Prepare the input state $`(\ket{000} + \ket{111})\ket{000}`$

```python
# create circuit
circ = qb.Circuit()

# State prep: |000>|000> + |111>|0000>
circ.h(0)
for k in range(num_state_qubits-1): 
    circ.cnot(k,k+1)
```

Add the efficient encoding circuit:

```python
# Add ee
circ.efficient_encoding(scoring_function, num_state_qubits, num_scoring_qubits)

# Measure
circ.measure_all()
```

Run the circuit and get the results:

```python
# Run:
result = circ.execute()
print("Result:\n", result)

# get results
res = ast.literal_eval(result)
measurements = res["AcceleratorBuffer"]["Measurements"]
allowed_outputs = ['000000', '111111']
for measurement in measurements:
    assert(measurement in allowed_outputs)
```
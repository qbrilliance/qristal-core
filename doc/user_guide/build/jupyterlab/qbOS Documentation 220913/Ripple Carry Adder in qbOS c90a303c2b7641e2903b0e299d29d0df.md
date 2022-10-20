# Ripple Carry Adder in qbOS

Similar to classical electronics, we can make adders in quantum circuits (qubit registers encode integer values in binary form).

A quantum adder can act on superposition states.

The addition circuit module in qbOS implements the ripple carry adder circuit design in [[Cuccaro2004](https://arxiv.org/pdf/quant-ph/0410184.pdf)], the so-called “*Cuccaro adder*”.

This ripple-carry circuit performs *in-place* addition on two-qubit registers.

The circuit diagram for a 6-qubit adder is shown below:

![Untitled](Ripple%20Carry%20Adder%20in%20qbOS%20c90a303c2b7641e2903b0e299d29d0df/Untitled.png)

In particular, it implements: 

$$
|a\rangle |b\rangle \mapsto |a\rangle|s\rangle \equiv |a\rangle |a + b\rangle
$$

(note, there is an input carry bit and an output carry bit as well that we didn’t mention in the above description for simplicity)

The two ingredients required are the **MAJ** (majority) and **UMA** (unmajority) circuits:

![Untitled](Ripple%20Carry%20Adder%20in%20qbOS%20c90a303c2b7641e2903b0e299d29d0df/Untitled%201.png)

![Untitled](Ripple%20Carry%20Adder%20in%20qbOS%20c90a303c2b7641e2903b0e299d29d0df/Untitled%202.png)

## Python Implementation

New qbOS Python `Circuit` class method: `ripple_add`

Inputs:

- `a`: list of int values (`adder_bits`)
- `b`: list of int values (`sum_bits`) (`len(b) > len(a)`)
- `carry_bit`: int (index of carry-in bit)

qbOS also offers a method: `controlled_ripple_carry_adder` that performs the ripple addition conditional on control qubits. 

Inputs:

- `qubits_adder` : list of int values
- `qubits_sum`: list of int values
- `c_in`: int
- `flags_on`: list of int values (indices of control qubits that are conditional on being $|1\rangle$)
- `flags_off`: list of int values (indices of control qubits that are conditional on being $|0\rangle$)

### Example

Using qbOS’s `ripple_add` to add 2 registers in superposition states:

$$
|a\rangle = |0\rangle + |1\rangle
$$

$$
|b\rangle = |0\rangle + |2\rangle
$$

**Notes:**

- We ignore the normalization factor ($1/\sqrt{2}$) in the above expressions.
- The above 2 states can be easily prepared by applying a Hadamard gate on Q0 ($|a\rangle$) or Q1 ($|b\rangle$)

```python
import qbos as qb
import numpy as np
import json
tqb = qb.core()
# Let's do a 2-qubit adder
nb_qubits = 2
# partition the qubit indices:
# q0: carry in
# q1, q2: input
# q3, q4, q5: sum (include the carry over)
c_in = 0
a = range(1, nb_qubits + 1)
# the result register has 3 qubits to catch the carry bit
b = range(nb_qubits + 1, 2 * (nb_qubits + 1))
# Test adding superposition:
# (|0> + |1>) + (|0> + |2>)
# ==> |0> + |1> + |2> + |3>
circ = qb.Circuit()
# Prepare |a>
circ.h(a[0])
# Prepare |b>
circ.h(b[1])
circ.ripple_add(a, b, c_in)
# Measure the result register
for q in b:
    circ.measure(q)
# Check the circuit:
# circ.print()
tqb.ir_target = circ
tqb.acc = "qpp"
tqb.run()
result = tqb.out_raw[0][0]
print(result)
```

As we can see in the code snippet above, the first thing to do is to specify the qubit partitioning scheme (to be given to the `ripple_add` module).

We can add gates to prepare the two operands as needed. In the above code, we need to apply Hadamard gates (`h(a[0])` and `h(b[1])` to prepare the $|a\rangle$ and $|b\rangle$ states)

For validation purposes, we want to measure the result register ($|b\rangle$), which should contain an equal superposition of 4 states:

$$
(|0\rangle + |1\rangle) + (|0\rangle + |2\rangle) = |0\rangle + |1\rangle + |2\rangle + |3\rangle
$$

A sample distribution (1024 shots) may look like this:

```json
"Measurements": {
            "000": 269,
            "010": 249,
            "100": 241,
            "110": 265
        }
```

We also give an example for using `controlled_ripple_carry_adder`

```python
import qbos as qb
import numpy as np 
import ast
tqb = qb.core()
tqb.sn = 1024

###
# Testing controlled addition.
# In this example we test adding |10> to |000> conditional on a flag
# When the flag is off we expect no addition to happen and the output sum to be |000>
# When the flag is on we expect addition to happen and the output sum to be |100>
###

# Set up input registers
qubits_adder = [0,1]
qubits_sum = [2,3,4] # Remember that the sum register needs more qubits than the adder register for overflow
c_in = 5
flag = [6]

### 
# Test 1: flag off
###

circ1 = qb.Circuit()

# Prepare initial state
circ1.x(qubits_adder[0])

# Perform conditional addition
circ1.controlled_ripple_carry_adder(qubits_adder, qubits_sum, c_in, flags_on = flag)

# Measure sum register
for i in range(len(qubits_sum)):
    circ1.measure(qubits_sum[i])

tqb.ir_target = circ
tqb.acc = "qpp"
tqb.run()
result1 = tqb.out_raw[0][0]
res1 = ast.literal_eval(result1)
assert(res1["000"] == 1024)

### 
# Test 1: flag off
###

circ2 = qb.Circuit()

# Prepare initial state
circ2.x(qubits_adder[0])

# Prepare flag
circ2.x(flag[0])

# Perform conditional addition
circ2.controlled_ripple_carry_adder(qubits_adder, qubits_sum, c_in, flags_on = flag)

# Measure sum register
for i in range(len(qubits_sum)):
    circ2.measure(qubits_sum[i])

tqb.ir_target = circ
tqb.acc = "qpp"
tqb.run()
result2 = tqb.out_raw[0][0]
res2 = ast.literal_eval(result2)
assert(res2["100"] == 1024)
```
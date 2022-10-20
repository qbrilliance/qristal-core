# Optimisation with QAOA in qbOS

# Introduction to QAOA

<aside>
⚠️ **Deprecation notice:**

From qbOS Release 220724 onward, the existing QAOA class `qbos_op.qaoa()` will become:

`qbos_op.QaoaSimple()`

Users should transition existing codes to use the new name in preparation for deprecation of the old name.

</aside>

## Quickstart

```python
print("* Runs QAOA on built-in Nelder-Mead. Uses a classical/diagonal 3-qubit Hamiltonian.  Uses qpp and QAOA-depth = 2.  Assert checks the optimum eigenstate.")

import qbos_op
qa = qbos_op.QaoaSimple()
qa.qaoa_step = 2
qa.extended_param = False

# Setup the Hamiltonian derived from the cost-function
qa.qn = 3  # 3 qubits
qa.ham = "1.0 + 3.5 Z0 + -5.5 Z1 + -5.9 Z2"
qa.ham[0].append("1.0 + -3.5 Z0 + -5.5 Z1 + -5.9 Z2")

qa.sn = 1024   # 1024 shots
qa.maxeval = 300
qa.theta = qbos_op.ND()
for ii in range(qa.qaoa_step[0][0]*2) :
    qa.theta[0][0][ii] = 0.1   # Initial parameter values
    
qa.run()
assert qa.out_eigenstate[0][0] == "001", "[qbos_op.QaoaSimple()] Failed eigenstate test: 001"
assert qa.out_eigenstate[0][1] == "000", "[qbos_op.QaoaSimple()] Failed eigenstate test: 000"
```

### About QAOA

QAOA is a heuristic algorithm that helps solve combinatorial optimisation problems such as the [MAXCUT](https://en.wikipedia.org/wiki/Maximum_cut) problem with an approximate solution close to the actual optimum. Being heuristic in nature, it doesn't have a polynomial runtime guarantee but appears to perform well enough on certain instances of such optimisation problems. 

# Usage options and examples

### **General usage operations**

- Importing the Python package
    
    ```python
    import qbos_op
    ```
    
- `qbos_op.QaoaSimple()` creates an instance to access the functionality in qbOS QAOA
    
    ```python
    import qbos_op
    qa = qbos_op.QaoaSimple()
    ```
    

### **Options related to the QAOA ansatz**

- `extended_param` selects if an increased number of parameters should be used, default: `False`
    
    See the discussion in the description of `theta`
    
- `qaoa_step` sets the repeat depth (or layers) of the ansatz, default: 1
- `theta` sets the **initial value** of the parameters (angles) to QAOA, default: 1.0
    
    The **length** of `theta` (the number of elements) must match the **required number of parameters**
    
    `extended_param = True` sets QAOA with **extended** angle parameters which has a **reduced circuit depth** compared to the standard QAOA. The extended parameter scheme requires $(n_{\rm qubits}+n_{\rm nonident-pauli-terms})\times n_{\rm steps}$ angles to be specified.
    
    Example: 
    
    Initial value of theta set to a list of length equal to the number of parameters used by the QAOA with extended angle parameters.
    
    ```python
    import qbos_op
    import numpy as np
    qa = qbos_op.QaoaSimple()
    qa.extended_param = True
    qa.ham = "1.5 + -2.2 Z1 Z0 + 3.1 Z0 + 5.1 Z1"
    qa.qn = 2
    qa.theta = qbos_op.ND() # this type holds a list of maps from integer->double
    
    # 2 qubits + 3 non-Identity Pauli terms = 5 parameters
    qa.theta[0][0][0] = -0.2*np.pi
    qa.theta[0][0][1] = -0.5*np.pi
    qa.theta[0][0][2] = 0.5*np.pi
    qa.theta[0][0][3] = 0.1*np.pi
    qa.theta[0][0][4] = -0.5*np.pi
    
    qa.run()
    qa.out_eigenstate[0]
    ```
    
- `qn` sets the number of physical qubits, default: 1
    
    <aside>
    ❗ The number of qubits must be consistent with the Hamiltonian
    
    </aside>
    
    Example: set up 2 qubits
    
    ```python
    import qbos_op
    qa = qbos_op.QaoaSimple()
    qa.theta = qbos_op.ND() # this type holds a list of maps from integer->double
    
    qa.qn = 2
    ```
    
- `ham` sets the Hamiltonian, default: "1.0 Z0"
    
    See the full examples below (Example 1 and Example 2)
    
- `sn` sets the number of shots, default: 256
    
    <aside>
    ✅ Non-stochastic QAOA is performed when `sn` = 0
    
    </aside>
    
- `noise` enables a noise-model of Quantum Brilliance's hardware, default: `False` (ie. noiseless)
    
    <aside>
    ℹ️ This option automatically forces the use of the `"aer"` back-end.  An important effect of this is that bit-strings will print out in **reverse order** compared to the default (`"qpp"`) back-end.
    
    </aside>
    
- `acc` selects the back-end accelerator, default: `"qpp"`
    
    Valid settings:
    `"aer"`
    
    `"qpp"`
    
    `"tnqvm"`
    

### Built-in qbOS classical optimisers

These internal options can be enabled by setting `maxeval` to a value > 1.

- `maxeval` sets the number of evaluations of the quantum kernel by the internal optimiser, default: 1 (no internal optimisation loop)
- `method` sets the algorithm used by the internal optimiser, default: `"nelder-mead"`
    - `"nelder-mead"`
    - `"l-bfgs"`
    - `"cobyla"`
    - `"adam"`
    - `"sgd"`
    - `"momentum-sgd"`
    - `"momentum-nestorov"`
    - `"rms-prop"`
    - `"gd"`
- `functol` sets the function tolerance, default: 1.0e-6
- `gradient_strategy` sets the method for calculating gradients, default: `"parameter-shift"`

### Example 1:

Extended parameters, with `aer` simulator:

```python
import qbos_op
qa = qbos_op.QaoaSimple()
qa.qn = 2
qa.ham = "-5.0 - 0.5 Z0 + 1.0 Z0 Z1"
qa.extended_param = True

qa.acc="aer"
qa.functol[0][0][0]=1e-5
qa.maxeval=200
qa.qaoa_step=3
# Set up 12 parameters = (qubits + Pauli Terms (non-Identity))* steps
qa.theta[0][0]=qbos_op.ND()
for ii in range((qa.qn[0][0] + 2)*qa.qaoa_step[0][0]) :        
    qa.theta[0][0][ii] = 0.1

qa.run()
```

View the optimum energy:

```python
qa.out_energy[0][0][0]

# Outputs:
# -6.08203125
```

View the optimum eigenstate:

<aside>
🧩 When `acc` = `"aer"`: the bit string is in this order: `q[0]q[1]q[2]...q[n-1]`

</aside>

```python
qa.out_eigenstate[0][0]

# Outputs:
# '01'
# which means q0=0 and q1=1
```

### Example 2:

Standard parameters, with `qpp` simulator:

```python
import qbos_op
qa = qbos_op.QaoaSimple()
qa.qn = 3
qa.ham = "-5.0 - 0.5 Z0 + 1.0 Z0 Z1 + 1.1 Z2"
qa.extended_param = False

qa.acc="qpp"
qa.functol[0][0][0]=1e-5
qa.maxeval=200
qa.qaoa_step=3
# Set up 6 parameters = 2*(3 QAOA steps)
qa.theta[0][0]=qbos_op.ND()
for ii in range(6) :        
    qa.theta[0][0][ii] = 0.1

qa.run()
```

View the optimum energy:

```python
qa.out_energy[0][0][0]

# Outputs:
# -7.053125
```

View the optimum eigenstate:

<aside>
🧩 When `acc` = `"qpp"`: the bit string is in this order: `q[n-1]q[n-2]...q[1]q[0]`

</aside>

```python
qa.out_eigenstate[0][0]

# Outputs
# '110'
```

### Example 3: MaxCut problem for 5-node graph

[qbOS_QAOA_examples.ipynb](Optimisation%20with%20QAOA%20in%20qbOS%2069aae61a872045adac72410f02d977e5/qbOS_QAOA_examples.ipynb)

![Untitled](Optimisation%20with%20QAOA%20in%20qbOS%2069aae61a872045adac72410f02d977e5/Untitled.png)
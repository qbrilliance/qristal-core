# Variational Quantum Eigensolver (VQE) in QB SDK
## General information

VQE is classified as a **quantum-classical (hybrid) algorithm**. It uses a **classical optimizer** to minimize a **quantum kernel** based objective function. The objective function is usually determined by multiple measurements at each iteration.

VQE finds the **ground state energy** of a physical system characterized by its **Hamiltonian** $H$ which is Hermitian. See [[Peruzzo et al. 2013](https://arxiv.org/abs/1304.3061), [O'Malley et al. 2015](https://arxiv.org/abs/1512.06860)] for details.

### Quantum kernels in QB SDK

The quantum kernel has 3 inputs:

- **theta**: Input parameters, usually rotation angles in radians.
- **ansatz**: A quantum circuit that prepares a quantum state and is conditioned by the values of theta (i.e. the input parameters mentioned above).
- **observable/measurement**: Pauli decomposition of the system Hamiltonian. Defines the measurement operations respectively applied after the **ansatz**. May have groups of **commutative** terms which can be measured together after one **ansatz** execution.

The output from measurement is reduced to a scalar quantity: the **energy**. The output is called **stochastic** when this energy is calculated from the shot-count statistics (samples) produced by a quantum device. A quantum computer is inherently **stochastic** but a simulator can operate in **non-stochastic**, i.e. **deterministic** mode. 

### Hamiltonians in QB SDK

While the user provides a Hamiltonian that has been transformed from their problem/system of interest, special attention is needed on the resulting format of the Hamiltonian so that it is valid as an input to QB SDK.  In particular:

- the Hamiltonian must be formatted as a **string**. This string is an expression of a weighted sum of **Pauli terms**.
- **Spaces** must be used to separate **all** of the following: the Pauli terms, their respective weights, and the arithmetic +/- symbols.
- In all Pauli terms, any **Identity (I) operators are implicit - do not show I operator explicitly.**

Examples of valid Hamiltonians (but with no physical meaning):

```
"-1.2 + 1.1 Z0Z1 + 4.1 Z1 - 11.8 Z0"
```

```
"12.3 + 0.002 Z0X1X2 - 1.2 X0Z2 - 3.3 Z1 - 5.0 Y2"
```

```
"5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + (0.3,0.21829) Z0 + 0.34 Z0Z1Z2Z3"

Note: (0.3,0.21829) == 0.3 + 0.21829j
```

Example of a physically valid Hamiltonian (for deuteron):

```
"5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1"
```

### Ansätze in QB SDK

As in all parameterized models, choice of model architecture, or *ansatz*, is an important parameter in the success of the optimization.


**User-defined ansatz:**

This functionality requires using the XASM format.  An example of this is shown below:

❗ **Important:** 
Do **not** change the first two lines:
```
 .compiler xasm
 .circuit ansatz
 ```

You can use the parameter (declared here as `theta`) as a 0-indexed array, ie `theta[0], theta[1], theta[2]`, ..., etc.


```bash
.compiler xasm
.circuit ansatz
.parameters theta
.qbit q
  X(q[0]);
  X(q[1]);
  U(q[0], theta[0], theta[1], theta[2]);
  U(q[1], theta[3], theta[4], theta[5]);
  U(q[2], theta[6], theta[7], theta[8]);
  U(q[3], theta[9], theta[10], theta[11]);
  CNOT(q[0], q[1]);
  CNOT(q[1], q[2]);
  CNOT(q[2], q[3]);
  U(q[0], theta[12], theta[13], theta[14]);
  U(q[1], theta[15], theta[16], theta[17]);
  U(q[2], theta[18], theta[19], theta[20]);
  U(q[3], theta[21], theta[22], theta[23]);
  CNOT(q[0], q[1]);
  CNOT(q[1], q[2]);
  CNOT(q[2], q[3]);
```


## Python API
### Importing the module
```python
import qb.core.optimization as cop
```

### Setting up the parameters for VQE
```python
tvqep = cop.vqee_Params()
```
The following VQE attributes are accepted:
| Attribute | Example | Details |
| ---- | ---- | ---- |
| `circuitString` | `tvqep.circuitString = ' .compiler xasm\n          .circuit ansatz\n          .parameters theta\n          .qbit q\n          Ry(q[0], theta);\n'` | Sets the ansatz circuit with parameter `theta`. |
| `pauliString` | `tvqep.pauliString = '-1.04235464570829 + 0.18125791479311 X0 + -0.78864539363997 Z0'` | Sets the Hamiltonian. |
| `nQubits` | `tvqep.nQubits = 1` | Sets the number of qubits.  This must be consistent with both the `circuitString` and the `pauliString` attributes. |
| `nShots` | `tvqep.nShots = 128` | Set the number of shots in a single VQE iteration. |
| `maxIters` | `tvqep.maxIters = 256` | Sets the upper limit on the number of VQE iterations to run. |
| `isDeterministic` | `tvqep.isDeterministic = True` | When set to `True` the expectation values are calculated using linear algebra operations.  When set to `False` the expectation values are calculated from stochastic samples of size `nShots`. |
| `tolerance` | `tvqep.tolerance = 1e-6` | Sets the function tolerance that is used by the optimizer. |

### Triggering the VQE execution
```python
tvqep.run()
```
### Accessing the results
The results from a completed VQE execution are available be reading the following attributes:
| Attribute | Example | Details |
| ---- | ---- | ---- |
| `optimalValue`  |  `tvqep.optimalValue` | The **minimum energy** corresponding to parameters set at the `optimalParameters`. |
| `optimalParameters` | `tvqep.optimalParameters` | The values for the ansatz parameters corresponding to the minimum energy that VQE has found within `maxIters` iterations. |

## C++ API
<a href="../_cpp_api/classqb_1_1vqee_1_1VQEE.html">See the class documentation.</a>

----


| Deprecation Notice | All sections below this Notice will be deprecated after December 2022|
| ---- | ---- |
|| The functionality described below is undergoing extensive rework.  You may still use it for background understanding, but it will **not** be compatible with future releases of QB SDK.

----

### Classical optimizer

VQE is open to use with any type of optimizer. In particular, the optimizer can be gradient-based or gradient-free.  Stochastic optimization algorithms can also be used. See below for examples.

### Molecular geometries in QB SDK

Instead of directly specifying a Hamiltonian, users can instead input a molecular geometry in terms of the elements and coordinates of all atoms in the system.  QB SDK will then generate the corresponding Hamiltonian automatically.

For this functionality, QB SDK requires PySCF-style XYZ syntax, i.e.,
 `{element symbol} {x_coord} {y_coord} {z_coord};...`

For example, an $H_2$ molecule with an atomic distance of 0.735 angstroms (Å) can be described by the following geometry string:

`H 0.0 0.0 0.0; H 0.0 0.0 0.735`

❗ **Important:** the default unit for coordinates is **angstroms**. 

By default, QB SDK uses the [sto-3g](https://en.wikipedia.org/wiki/STO-nG_basis_sets) basis set and the Jordan-Wigner fermion-to-qubit mapping.


**Built in ansätze:**

- Default ansatz:  the [Hardware Efficient Ansatz](https://www.nature.com/articles/nature23879) (HEA)
    
    HEA provides a framework for general Hamiltonians where the problem substructure is less well defined.   HEA is often parameterized by **depth**: the number of base circuit repetitions.  
    
    A larger depth allows for modelling more complexity at the cost of increasing training difficulty.  
    
    One instance of HEA ansatz is demonstrated in [Pennylane's VQE tutorial](https://pennylane.ai/qml/demos/tutorial_vqe.html). 
    
- [ASWAP](https://arxiv.org/abs/1904.10910) ansatz that is useful for quantum chemistry
    
     ASWAP is well-suited to the types of Hamiltonians generated by quantum chemistry problems and is parameterized by the **number of particles** to use. 
    
- [UCCSD](https://onlinelibrary.wiley.com/doi/full/10.1002/qua.21198) ansatz that is useful for quantum chemistry
    
     *Unitary coupled-cluster singles and doubles* (UCCSD) ansatz is based on the unitary coupled cluster (UCC) theory.
    
     UCCSD ansatz is parameterized by the number of particles (e.g., electrons) and the number of spin orbitals.
    
     (the number of spin orbitals is equal to the number of qubits required)
    
- ❗ **Important:** Convention of mapping spin orbitals onto qubits
    - The built-in ASWAP and UCCSD ansätze in QB SDK map all alpha (up) spins **then** all beta (down) spins. Thus, care must be taken to make sure that the input Pauli Hamiltonian follows the same mapping convention.
    - If the Hamiltonian input is provided as a molecular geometry string (see the previous section above), then the mapping is guaranteed to be compatible with the above convention.
    - Tips: Among some other open-source platforms, Qiskit is using the same qubit mapping as stated above. Pennylane, on the other hand, alternates alpha and beta (up and down) spins.

## Usage Options and Examples

### **General usage operations**

- Importing the Python package
    
    ```python
    import qbos_op
    ```
    
- `qbos_op.vqe()` creates an instance to access the functionality in QB SDK VQE
    
    ```python
    import qbos_op
    qv = qbos_op.vqe()
    ```
    

### **Options related to the quantum kernel**

- `theta` sets the **initial value** of the parameters used by the ansatz, default: 1.0
    
    ⚙ The **length** of `theta` (the number of elements) must match the **required number of parameters** in the ansatz.  
    
    For the default ansatz in QB SDK, it is required that:
     `len(theta) == 3*qn*ansatz_depth`
    
    
    Example:  initial value of theta set by a **scalar**.  Broadcast is performed of this value to all elements required by the ansatz.
    
    ```python
    import qbos_op
    import numpy as np
    qv = qbos_op.vqe()
    qv.theta = qbos_op.ND() # this type holds a list of maps from integer->double
    
    example_scalar = 0.12*np.pi
    qv.theta[0][0][0] = example_scalar
    ```
    
    Example: initial value of theta set to a list of length equal to the number of parameters used by the ansatz.
    
    ```python
    import qbos_op
    import numpy as np
    qv = qbos_op.vqe()
    qv.theta = qbos_op.ND() # this type holds a list of maps from integer->double
    
    example_rx = -0.2*np.pi
    example_ry = -0.5*np.pi
    example_rz =  0.5*np.pi
    
    qv.theta[0][0][0] = example_rx
    qv.theta[0][0][1] = example_ry
    qv.theta[0][0][2] = example_rz
    ```
    
- `qn` sets the number of physical qubits, default: 1
    
    ❗ The number of qubits must be consistent with the Hamiltonian
    
    
    ⚙ For the default ansatz in QB SDK, it is required to that:
     `len(theta) == 3*qn*ansatz_depth`
    
    
    Example: set up 2 qubits
    
    ```python
    import qbos_op
    qv = qbos_op.vqe()
    qv.theta = qbos_op.ND() # this type holds a list of maps from integer->double
    
    qv.qn = 2
    ```
    
- `ansatz` sets the ansätz, default: "default"
    - `"default"` is the built-in Hardware Efficient Ansatz
        - `ansatz_depth` sets the number of repeated layers in the default ansatz, default: 1
            
            Example: using a 1-qubit, 3-layer ansatz
            
            ```python
            import qbos_op
            import numpy as np
            qv = qbos_op.vqe()
            qv.theta = qbos_op.ND() # this type holds a list of maps from integer->double
            
            qv.ansatz = "default"
            qv.qn = 1;
            qv.ansatz_depth = 3
            
            # 3*qn*ansatz_depth = 9 parameters - set the length of initial values to 9
            for ite in range(3*qv.qn[0][0]*qv.ansatz_depth[0][0]) :
                qv.theta[0][0][ite] = 0.125*np.pi   
            ```
            
    - `"aswap"` is the built-in ASWAP ansatz
        - `aswapn` sets the number of particles in the ASWAP ansatz, default: 1
            
            Example: deuteron
            
            ```python
            import qbos_op
            qv = qbos_op.vqe()
            qv.theta = qbos_op.ND() # this type holds a list of maps from integer->double
            
            # Deuteron Hamiltonian with aswap 1-particle ansatz
            qv.qn = 2  # Number of qubits
            qv.ham = "5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1"
            qv.ansatz = "aswap"
            qv.aswapn = 1
            qv.theta[0][0][0] = 0.11
            ```
            
    - User-defined XASM format ansatz
        
        Example: reusing the user-defined 4-qubit ansatz described previously
        
        ```python
        import qbos_op
        qv = qbos_op.vqe()
        qv.theta = qbos_op.ND() # this type holds a list of maps from integer->double
        
        qv.qn = 4
        qv.ham = "1.2 - 1.45 Z0 + 2.1 Z1 + 3.2 Z2 - 0.2 Z3 - 3.1 Z1Z3"
        self_anz = '''
        .compiler xasm
        .circuit qbos_ansatz
        .parameters theta
        .qbit q
          X(q[0]);
          X(q[1]);
          U(q[0], theta[0], theta[1], theta[2]);
          U(q[1], theta[3], theta[4], theta[5]);
          U(q[2], theta[6], theta[7], theta[8]);
          U(q[3], theta[9], theta[10], theta[11]);
          CNOT(q[0], q[1]);
          CNOT(q[1], q[2]);
          CNOT(q[2], q[3]);
          U(q[0], theta[12], theta[13], theta[14]);
          U(q[1], theta[15], theta[16], theta[17]);
          U(q[2], theta[18], theta[19], theta[20]);
          U(q[3], theta[21], theta[22], theta[23]);
          CNOT(q[0], q[1]);
          CNOT(q[1], q[2]);
          CNOT(q[2], q[3]);
        '''
        
        qv.ansatz = self_anz
        
        # 24 parameters in the user-defined ansatz => provide 24 initial values
        for ite in range(24) :
            qv.theta[0][0][ite] = 0.125*np.pi
        
        ```
        
- `ham` sets the Hamiltonian, default: "1.0 Z0"
    
    Example: deuteron
    
    ```python
    import qbos_op
    qv = qbos_op.vqe()
    qv.theta = qbos_op.ND() # this type holds a list of maps from integer->double
    
    # Deuteron Hamiltonian with aswap 1-particle ansatz
    qv.qn = 2  # Number of qubits
    qv.ham = "5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1"
    qv.ansatz = "aswap"
    qv.aswapn = 1
    qv.theta[0][0][0] = 0.11
    ```
    
- `sn` sets the number of shots, default: 256
    
    ✅ Non-stochastic VQE is performed when `sn` = 0
    
    
- `noise` enables a noise-model of Quantum Brilliance's hardware, default: `False` (ie. noiseless)
    
    ℹ️ This option automatically forces the use of the `"aer"` back-end.  An important effect of this is that bit-strings will print out in **reverse order** compared to the default (`"qpp"`) back-end.
    
    
- `acc` selects the back-end accelerator, default: `"qpp"`
    
    ❗ The `acc` option has no effect when `noise` = `True`
    
    The following values are accepted:
    
    - `"qpp"`
    - `"tnqvm"`
    - `"aer"`
    
    ⚠️ **WARNING**
    The `"aer"` accelerator has some known incompatibilities and issues with the VQE for some specific Hamiltonian inputs. If you are facing such problems, please choose the default, ie. `"qpp"`.
        
- `direct_expectation` enables direct expectation calculation if the selected `acc` backend is a state-vector-based simulator. Default: `False` (disabled, ie., running observed circuits to compute the expectation value)
    
    **Important:** If direct expectation evaluation is not feasible (e.g., the back-end accelerator selected doesn’t support retrieval of the state vector), this option will be ignored, i.e., fall back to the conventional method of observing the Hamiltonian expectation value.
    
    **Example**:
    
    ```python
    import qbos_op
    vqe = qbos_op.vqe()
    vqe.sn = 0 
    # Direct energy evaluation
    vqe.direct_expectation = True
    ```
    

### How to make function calls for a Python-level optimizer?

In order to use the optimization algorithms in `scipy.optimize` or `skquant.opt`  for VQE, it is necessary to provide a function that accepts the parameter values at the current iteration and returns the energy.  The function below provides this.  Note: it accepts an input (`theta`) that is a NumPy array:

```python
#
# Wrapper that accepts a parameter, theta (type is NumPy ndarray)
# and calls qbOS VQE for 1 iteration.  The energy is returned.
#
def qbvqe(theta):
    qv = qbos_op.vqe()
    qv.theta = qbos_op.ND()
    qv.sn = 64   # Number of shots.  Set to 0 for non-stochastic VQE

    # Deuteron Hamiltonian with aswap ansatz
    qv.qn = 2  # Number of qubits
    qv.ham = "5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1"
    qv.ansatz = "aswap"

    qv.maxeval = 1  # initial energy only - no internal optimisation steps
    if (theta.size == 1):
        qv.theta[0][0][0] = theta
    else :
        for el in range(theta.size):
            qv.theta[0][0][el] = theta[el]
            
    qv.run()
    return qv.out_energy[0][0][0]
```

With small modifications, the function shown above can be adapted to return other quantities (eg. Jacobian) needed by the optimization algorithm.

### Python `minimize()` from `scipy.optimize`

These classical methods have been tested with QB SDK:

- `"SLSQP"`
    
    ```python
    import qbos_op
    from scipy.optimize import Bounds,minimize
    import numpy as np
    
    theta0 = np.array([0.01*np.pi])
    qvbound = Bounds(-np.pi,np.pi,True)
    
    def qbvqe(theta):
        qv=qbos_op.vqe()
        qv.theta = qbos_op.ND()
        qv.sn = 1024   # Number of shots.  Set to 0 for non-stochastic VQE
    
        # Deuteron Hamiltonian with aswap ansatz
        qv.qn = 2  # Number of qubits
        qv.ham = "5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1"
        qv.ansatz = "aswap"
        qv.aswapn = 1
        qv.maxeval = 1  # initial energy only - no internal optimisation steps
        if (theta.size == 1):
            qv.theta[0][0][0] = theta
        else :
            for el in range(theta.size):
                qv.theta[0][0][el] = theta[el]
                
        qv.run()
        return qv.out_energy[0][0][0]
    
    res = minimize(qbvqe, theta0, method='SLSQP', options={'maxiter': 200, 'ftol': 1e-9}, bounds=qvbound)
    print(res.fun, res.x[0] , res.nit)
    ```
    
- `NFT (Nakanishi-Fujii-Todo)`
    
    [GitHub - ken-nakanishi/nftopt: Nakanishi-Fujii-Todo method for scipy.optimize](https://github.com/ken-nakanishi/nftopt)
    
    ```python
    import numpy as np
    from scipy.optimize import minimize
    from nftopt import nakanishi_fujii_todo
    import qbos_op
    
    def qbvqe(theta):
        qv=qbos_op.vqe()
        qv.sn = 0   # No shots - deterministic VQE
        qv.ham = "0.04207897647782276 + 0.17771287465139946 Z0 + 0.1777128746513994 Z1 + -0.24274280513140462 Z2 - 0.17059738328801052 Z0Z1 + 0.6622334 Z0Z1Z2Z3"
        qv.qn = 4  # Number of qubits
        qv.ansatz = "default"
        qv.ansatz_depth = 2
        qv.maxeval = 1
        qv.theta = qbos_op.ND()
        
        # Adjust all elements of theta to be within [-pi,pi]
        for el in range(len(theta)):
            pz0 = theta[el]
            while (pz0 > 3.14159) :
                pz0 -= 2*3.14159
            while (pz0 < -3.14159) :
                pz0 += 2*3.14159
            qv.theta[0][0][el] = pz0
        qv.run()
        return qv.out_energy[0][0][0]
    
    res = minimize(qbvqe, [0.51*np.pi]*24, args=(), method=nakanishi_fujii_todo, options={'maxfev': 1024})
    print(res.fun, res.x, res.nit)
    
    ```
    
- Other methods
    
    [Optimization and root finding (scipy.optimize) - SciPy v1.7.1 Manual](https://docs.scipy.org/doc/scipy/reference/optimize.html)
    

### Python `minimize()` from `skquant.opt`

These methods have been tested with QB SDK:

- `"imfil"`
    
    ```python
    import qbos_op
    import numpy as np
    import skquant.opt
    
    theta0 = np.array([0.01*np.pi])
    qvboundskq = np.array([[-3.14159,3.14159]],dtype=float)
    
    def qbvqe(theta):
        qv=qbos_op.vqe()
        qv.theta = qbos_op.ND()
        qv.sn = 1024   # Number of shots.  Set to 0 for non-stochastic VQE
    
        # Deuteron Hamiltonian with aswap ansatz
        qv.qn = 2  # Number of qubits
        qv.ham = "5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1"
        qv.ansatz = "aswap"
        qv.aswapn = 1
        qv.maxeval = 1  # initial energy only - no internal optimisation steps
        if (theta.size == 1):
            qv.theta[0][0][0] = theta
        else :
            for el in range(theta.size):
                qv.theta[0][0][el] = theta[el]
                
        qv.run()
        return qv.out_energy[0][0][0]
    
    reskq,history = skquant.opt.minimize(qbvqe, np.array([0.01*np.pi]), qvboundskq, 200, method='imfil')
    print(reskq.optval, reskq.optpar, history.size)
    ```
    
- Other methods
    
    [SnobFit - sckit-quant 0.8.1 documentation](https://scikit-quant.readthedocs.io/en/latest/snobfit.html)
    

### Built-in QB SDK classical optimizers

These internal options can be enabled by setting `maxeval` to a value > 1.

- `maxeval` sets the number of evaluations of the quantum kernel by the internal optimizer, default: 1 (no internal optimization loop)
- `method` sets the algorithm used by the internal optimizer, default: `"nelder-mead"`
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
    - `"parameter-shift"`
    - `"central"`
    - `"forward"`
    - `"backward"`
    - `"autodiff"`
        
        ❗ **Important:** `"autodiff"` is not compatible with `u` (the universal single-qubit rotation gate). 
        
        In your ansatz, convert any `u` gates into the equivalent `Rx` and `Rz` gate sequence using this expression:
        
        $U(\theta, \phi, \lambda) = R_z(\phi)*R_x(-0.5\pi)*R_z(\theta)*R_x(0.5\pi)*R_z(\lambda)$
        
        

### Obtaining convergence information from the internal QB SDK optimizers

- `out_energy`: Energy trace
    
    `out_energy[0][0][0]` is the optimum energy
    
    `out_energy[0][0][1]` is the energy at the first iteration
    
    `out_energy[0][0][m]` is the energy at iteration `m`
    
    `out_energy[0][0][maxeval]` is the energy at the last iteration
    
- `out_theta`: Ansatz parameter trace
    
    Using `stride = len(theta)`, then element `k` of `theta` can be traced using these expressions:
    
    `out_theta[0][0][k]` is the optimum value of the parameter
    
    `out_theta[0][0][k+1*stride]` is the value of the parameter at the first iteration
    
    `out_theta[0][0][k+m*stride]` is the value of the parameter at iteration `m`
    
    `out_theta[0][0][k+maxeval*stride]` is the value of the parameter at the last iteration
    

### Example 1: two-qubit deuteron Hamiltonian

Let us now focus on a physical Hamiltonian matrix, where the ground state energy is known. The following represents the $N$ = 2 (two-qubit) deuteron Hamiltonian: `5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1`. The energy is known to be -1.74886 in sector $q$ = 1 and in the same energy unit as the Hamiltonian elements.

Now, imagine you intend to run VQE for the above with initial angle $\theta_1$ = 0.11, choosing the particle sector 1, setting the classical optimizer to the non-linear [Nelder-Mead](https://en.wikipedia.org/wiki/Nelder%E2%80%93Mead_method) method with 100 maximum function evaluations and 1.0e-5 function tolerance.  The notebook attached below shows how this is done in QB SDK:

[SDK_Deuteron2aswap.ipynb](SDK_Deuteron2aswap.ipynb)

### Example 2: a four-qubit classical Hamiltonian

Many graph optimization problems can be written in terms of a classical Hamiltonian, where it only depends on the spin in the $Z$-direction. For this, we recommend using either a QAOA ansatz if there is an exploitable substructure (e.g. a fixed Hamming distance of solution), or HEA (the default ansatz in QB SDK VQE) for generic problems. 

As an example, for the VQE Hamiltonian string given by `0.04207897647782276 + 0.17771287465139946 Z0 + 0.1777128746513994 Z1 + -0.24274280513140462 Z2 - 0.17059738328801052 Z0Z1 + 0.6622334 Z0Z1Z2Z3` with minimum eigenvalue -1.38892036:

[SDK_VQEClassical4default.ipynb](SDK_VQEClassical4default.ipynb)

### Example 3: same setup as for Example 2, except specifying a file that contains a user-defined ansatz

[SDK_VQECustom3.ipynb](SDK_VQECustom3.ipynb)

### Example 4:  finding the ground state energy of $H_2$ using UCCSD ansatz

[SDK_H2_UCCSD.ipynb](SDK_H2_UCCSD.ipynb)

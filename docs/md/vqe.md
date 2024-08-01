# Variational Quantum Eigensolver (VQE)

## Example

The code below shows how the Qristal interface for VQE (`vqee`) is called.

In general, you will need to provide these inputs:
* [ansatz](#ansätze)
* [Hamiltonian](#hamiltonian) which must be [consistent with the ansatz](#convention-of-mapping-spin-orbitals-onto-qubits)
* **Number of qubits**, which must be consistent with the ansatz and the Hamiltonian
* **Initial values** for ansatz parameters, referred from here on as `theta`

```python
import numpy as np
import qristal.core.optimization.vqee as qbOpt

params = qbOpt.Params()

# Hamiltonian
params.pauliString = "-0.8124419696351861 + 0.17128249292506947 X0X1X2X3"

# ansatz
params.circuitString = """
.compiler xasm
.circuit ansatz
.parameters theta
.qbit q
  X(q[0]);
  X(q[2]);
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
"""

# Number of qubits
params.nQubits = 4

# Number of shots
params.nShots = 400000

# Max iterations for the optimizer
params.maxIters = 100

# When isDeterministic is set to False,
# then expectations are calculated
# from params.nShots samples.
params.isDeterministic = True

# Termination condition for the optimizer
params.tolerance = 1e-6

# theta initial values. These will
# be overwritten with the
# optimized solution by vqee.run()
params.optimalParameters = 24*[0]

print("\nPauli string: ", params.pauliString)

acceleratorNamesList = ["sparse-sim", "qpp", "aer"]
for acceleratorName in acceleratorNamesList:
    print("\n\n\n*** ",acceleratorName, "***")
    params.acceleratorName = acceleratorName
    vqee = qbOpt.VQEE(params)
    vqee.run()
    optVec = params.optimalParameters
    optVal = params.optimalValue
    print("\noptVal, optVec: ", optVal, optVec)

```
Example output:
```
***  sparse-sim ***
Accelerator: sparse-sim
Parameters: [theta22, theta21, theta18, theta17, theta20, theta19, theta16, theta15, theta14, theta13, theta11, theta12, theta10, theta9, theta7, theta6, theta4, theta8, theta3, theta5, theta2, theta1, theta23, theta0]
Ansatz depth: 8

Min energy = -0.983724
Optimal parameters = [0, 0, 0, 0, 0, 0, 1.5708, 0, 0, 0, 1.5708, 0, 1.5708, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

optVal, optVec:  -0.9837244625602556 [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.5707963, 0.0, 0.0, 0.0, 1.5707963, 0.0, 1.5707963, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]



***  qpp ***
Accelerator: qpp
Parameters: [theta22, theta21, theta18, theta17, theta20, theta19, theta16, theta15, theta14, theta13, theta11, theta12, theta10, theta9, theta7, theta6, theta4, theta8, theta3, theta5, theta2, theta1, theta23, theta0]
Ansatz depth: 8

Min energy = -0.983724
Optimal parameters = [0, 0, 0, 0, 0, 0, 1.5708, 0, 0, 0, 1.5708, 0, 1.5708, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

optVal, optVec:  -0.9837244625602556 [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.5707963, 0.0, 0.0, 0.0, 1.5707963, 0.0, 1.5707963, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]



***  aer ***
Accelerator: aer
Parameters: [theta22, theta21, theta18, theta17, theta20, theta19, theta16, theta15, theta14, theta13, theta11, theta12, theta10, theta9, theta7, theta6, theta4, theta8, theta3, theta5, theta2, theta1, theta23, theta0]
Ansatz depth: 8

Min energy = -0.983724
Optimal parameters = [0, 0, 0, 0, 0, 0, 1.5708, 0, 0, 0, 1.5708, 0, 1.5708, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

optVal, optVec:  -0.9837244625602556 [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.5707963, 0.0, 0.0, 0.0, 1.5707963, 0.0, 1.5707963, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

```
## Python API
### Importing the module
```python
import qristal.core.optimization.vqee as qbOpt

```

### Setting up the parameters for VQE
```python
tvqep = qbOpt.Params()

```
The following VQE attributes are accepted:
| Attribute | Example | Details |
| ---- | ---- | ---- |
| `circuitString` | `tvqep.circuitString = '''`<br />`.compiler xasm`<br />`.circuit ansatz`<br />`.parameters theta`<br />`.qbit q`<br />`Ry(q[0], theta);'''` | Defines the [**ansatz**](#ansätze) circuit that<br /> has an associated parameter<br /> [**theta**](#theta). |
| `pauliString` | `tvqep.pauliString = `<br />`'-1.04235464570829 `<br />`+ 0.18125791479311 X0 `<br />`+ -0.78864539363997 Z0'` | Sets the [Hamiltonian](#hamiltonian). |
| `nQubits` | `tvqep.nQubits = `<br />`1` | Sets the number of qubits.  <br />This must be consistent <br />with both the `circuitString` and <br />the `pauliString` attributes. |
| `optimalParameters` | `tvqep.optimalParameters = `<br />`24*[0.11]` | A vector of initial values for <br />ansatz parameters. <br />The length must match the <br />number of ansatz parameters. |
| `nShots` | `tvqep.nShots = `<br />`128` | Set the number of shots <br />in a single VQE iteration. |
| `maxIters` | `tvqep.maxIters = `<br />`256` | Sets the upper limit <br />on the number of VQE <br />iterations to run. |
| `isDeterministic` | `tvqep.isDeterministic = `<br />`True` | When <br />`acceleratorName = 'qpp'` <br />and <br />`isDeterministic = True`,<br /> the expectation values are <br />calculated using linear <br />algebra operations.  <br /><br />When <br />`isDeterministic = False` <br />the expectation values are <br />calculated from stochastic <br />samples of size `nShots`. |
| `enableVis` | `tvqep.enableVis = True` | Enable the convergence trace output. <br /><br />  These will be available in <br /> `tvqep.energyVis` (for energy) <br />and <br /> `tvqep.thetaVis` (for ansatz parameters). |
| `tolerance` | `tvqep.tolerance = `<br />`1e-6` | Sets the function tolerance <br />that is used by the optimizer. |
| `acceleratorName` | `tvqep.acceleratorName = `<br />`'qpp'` | Selects the back-end that <br />will  perform quantum circuit <br />execution. |

### Setting up the optimization algorithm
By default, the `cobyla` (gradient-free) algorithm will be used.  Alternative algorithms for driving the optimization can be used, along with extra options that are specific to individual algorithms.  The setup for each algorithm is discussed in the next sections.

**Note**: These previously described attributes also apply to all algorithms below:

* `maxIters`
* `tolerance`
* `optimalParameters`
### Stochastic gradient algorithms
#### **ADAM [Adaptive Momentum estimator]**

| Attribute | Example | Details |
| ---- | ---- | ---- |
| `algorithm` | `tvqep.algorithm = `<br />` "adam"` | Selects the <br /> ADAM algorithm from <br /> the mlpack C++ library. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"stepsize: 0.5"` | Step size for each <br />iteration. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"beta1: 0.7"` | Exponential decay rate for<br /> the first moment estimates. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"beta2: 0.999"` | Exponential decay rate for <br /> the weighted infinity-norm estimates. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"eps: 1.0e-8"` | Mean-squared gradient initial value. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"momentum: 0.05"` | Momentum |
| `extraOptions` | `tvqep.extraOptions = `<br />`"exactobjective: false"` | ADAM exact objective |

**Note**: `extraOptions` is a YAML string field, so you can for example have all options specified with:

`tvqep.extraOptions = `<br />`"{stepsize: 0.1, beta1: 0.67, beta2: 0.9, momentum: 0.11, exactobjective: true}"`

### Heuristic algorithms
#### **CMA-ES [Covariance Matrix Adapter Evolution Strategy]**

| Attribute | Example | Details |
| ---- | ---- | ---- |
| `algorithm` | `tvqep.algorithm = `<br />` "cmaes"` | Selects the <br /> CMA-ES algorithm from <br /> the mlpack C++ library. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"lambda: 0"` | Population size |
| `extraOptions` | `tvqep.extraOptions = `<br />`"lower: -10.0"` | Lower-bound of decision<br /> variables. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"upper: 10.0"` | Upper-bound of decision<br /> variables. |

**Note**: `extraOptions` is a YAML string field, so you can for example have all options specified with:

`tvqep.extraOptions = `<br />`"{lambda: 0, upper: 10.0, lower: -10.0}"`

### Gradient-based algorithms
#### **L-BFGS**

| Attribute | Example | Details |
| ---- | ---- | ---- |
| `algorithm` | `tvqep.algorithm = `<br />` "l-bfgs"` | Selects the <br /> L-BFGS algorithm from <br /> the mlpack C++ library. |
| `extraOptions` | | None|
### Gradient-free algorithms
#### **Nelder-Mead**

| Attribute | Example | Details |
| ---- | ---- | ---- |
| `algorithm` | `tvqep.algorithm = `<br />` "nelder-mead"` | Selects the <br /> Nelder-Mead algorithm from <br /> the nlopt C++ library. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"stopval: -1.05"` | Exits the optimization <br />as soon as the **energy <br />is below this threshold value**. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"lowerbounds: [-0.01, -0.01, -0.01]"` | Imposes  **lower-bounds** <br />on the value of `theta`. <br /><br />The number of elements <br />in the list must <br /> equal the number of <br />elements in `theta`. |
| `extraOptions` | `tvqep.extraOptions = `<br />`"upperbounds: [0.1, 0.1, 0.1]"` | Imposes  **upper-bounds** <br />on the value of `theta`. <br /><br />The number of elements <br />in the list must <br /> equal the number of <br />elements in `theta`. |

**Note**: `extraOptions` is a YAML string field, so you can for example have all options specified with:

`tvqep.extraOptions = `<br />`"{lowerbounds: [-0.01, -0.01, -0.01], upperbounds: [0.1, 0.1, 0.1], stopval: -1.05}"`

### Triggering the VQE execution
```python
qv = qbOpt.VQEE(tvqep)
qv.run()
```
### Accessing the results
The results from a completed VQE execution are available by reading the following attributes:
| Attribute | Example | Details |
| ---- | ---- | ---- |
| `optimalValue`  |  `tvqep.optimalValue` | The **minimum energy** corresponding to <br />parameters set at the `optimalParameters`. |
| `optimalParameters` | `tvqep.optimalParameters` | The values for [theta](#theta) corresponding to <br />the minimum energy that VQE <br />has found within `maxIters` iterations. |
| `iterationData[<index>].energy` | `tvqep.iterationData[8].energy` | The energy at iteration `<index>`. |
| `iterationData[<index>].params` | `tvqep.iterationData[8].params` | The list of ansatz parameter <br />values at iteration `<index>`. |

### Viewing the convergence trace
**Note**: set `enableVis = True` prior to calling `run()` in order to generate the trace.

This is available via the `vis` attribute.  The iteration that gave the reported optimum is marked with: `**|`.

Color codes:
* Red - indicates the optimum iteration
* Green - indicates an iteration with the same energy as the optimum iteration

**Example output**
```text
Iteration 96
Energy         |################# -0.95
Theta
       Element 0      |####################### 0.33*pi
       Element 1      |##################### 0.19*pi
       Element 2      |################### -0.12*pi
       Element 3      |######################### 0.55*pi
Iteration 97
Energy       **|################ -0.98
Theta
       Element 0    **|####################### 0.3*pi
       Element 1    **|##################### 0.19*pi
       Element 2    **|#################### -0.059*pi
       Element 3    **|######################### 0.5*pi
Iteration 98
Energy         |################# -0.97
Theta
       Element 0      |###################### 0.29*pi
       Element 1      |##################### 0.14*pi
       Element 2      |#################### 0.061*pi
       Element 3      |######################### 0.55*pi
Iteration 99
Energy         |################ -0.98
Theta
       Element 0      |###################### 0.3*pi
       Element 1      |##################### 0.19*pi
       Element 2      |#################### -0.071*pi
       Element 3      |######################## 0.49*pi
Iteration 100
Energy         |################# -0.97
Theta
       Element 0      |####################### 0.34*pi
       Element 1      |##################### 0.15*pi
       Element 2      |#################### -0.048*pi
       Element 3      |########################## 0.62*pi
```

### Attributes related to the convergence trace
Use these attributes to set the amount of information that appears in the convergence trace:

| Attribute | Defaults | Details |
| ---- | ---- | ---- |
| `showTheta`  |  `tvqep.showTheta = False` | When set to `True`, <br />the elements of `theta` will <br />be displayed in the <br />convergence trace. |
| `limitThetaN` | `tvqep.limitThetaN = 0` | Limits the number of <br />elements of `theta` appearing <br />in the trace to this value. <br />If set to 0, this <br />option is ignored.|
| `tail` | `tvqep.tail = 0` | Output only the last <br />n=tail iterations. <br />If set to 0, this <br />option is ignored.|
| `plain` | `tvqep.plain = False`| Setting this to `True` removes <br />color coding from the output. |
| `blocked` | `tvqep.blocked = False`| Setting this to `True` outputs <br />the trace with all elements <br />of `theta` for any given iteration <br />contained in a single block. |


## C++ API
<a href="../_cpp_api/classqb_1_1vqee_1_1VQEE.html">See the class documentation.</a>

## General VQE information

VQE is classified as a **quantum-classical (hybrid) algorithm**. It uses a **classical optimizer** to minimize a **quantum kernel** based objective function. The objective function is usually determined by multiple measurements at each iteration.

VQE finds the **ground state energy** of a physical system characterized by its **Hamiltonian** $H$ which is Hermitian. See [[Peruzzo et al. 2013](https://arxiv.org/abs/1304.3061), [O'Malley et al. 2015](https://arxiv.org/abs/1512.06860)] for details.

### Quantum kernels in Qristal
In this section, the components that make up a quantum kernel are described in more detail.

### Hamiltonian

While the user provides a Hamiltonian that has been transformed from their problem/system of interest, special attention is needed on the resulting format of the Hamiltonian so that it is valid as an input to Qristal.  In particular:

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

### Ansätze
As in all parameterized models, choice of model architecture, or *ansatz*, is an important parameter in the success of the optimization.

Here it is a quantum circuit that prepares a quantum state and is conditioned by the values of [theta](#theta) (i.e. the input parameters mentioned above).  To [specify your ansatz](#user-defined-ansatz), see the instructions below.

#### theta
Input parameters of an ansatz, usually rotation angles (in radians).


#### User defined ansatz

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
Use gates that are in [XASM gate format](https://qristal.readthedocs.io/en/latest/rst/quantum_computing.html) to build the ansatz circuit.

#### Built in ansätze

See the examples folder: `examples/python/vqee_example1.py` for the syntax to call the ansätze described below.

- Default ansatz:  the [Hardware Efficient Ansatz](https://www.nature.com/articles/nature23879) (HEA)

    ⚙ For the default ansatz in Qristal, it is required that:
     `len(theta) == 3*nQubits*ansatz_depth`

    HEA provides a framework for general Hamiltonians where the problem substructure is less well defined.   HEA is often parameterized by **depth**: the number of base circuit repetitions.

    A larger depth allows for modelling more complexity at the cost of increasing training difficulty.

    One instance of HEA ansatz is demonstrated in [Pennylane's VQE tutorial](https://pennylane.ai/qml/demos/tutorial_vqe.html).

- [ASWAP](https://arxiv.org/abs/1904.10910) ansatz that is useful for quantum chemistry

     ASWAP is well-suited to the types of Hamiltonians generated by quantum chemistry problems and is parameterized by the **number of particles** to use.

- [UCCSD](https://onlinelibrary.wiley.com/doi/full/10.1002/qua.21198) ansatz that is useful for quantum chemistry

     *Unitary coupled-cluster singles and doubles* (UCCSD) ansatz is based on the unitary coupled cluster (UCC) theory.

     UCCSD ansatz is parameterized by the number of particles (e.g., electrons) and the number of spin orbitals.

     (the number of spin orbitals is equal to the number of qubits required)

### Convention of mapping spin orbitals onto qubits
❗ **Important:**

The built-in ASWAP and UCCSD ansätze in Qristal map all alpha (up) spins **then** all beta (down) spins. Thus, care must be taken to make sure that the input Pauli Hamiltonian follows the same mapping convention.  Note that e.g. Pennylane, on the other hand, alternates alpha and beta (up and down) spins.

If the Hamiltonian input is provided following the description in [Molecular geometries in Qristal](#molecular-geometries-in-qristal), then the mapping is guaranteed to be compatible with the above convention.

### Molecular geometries in Qristal

See the examples folder: `examples/python/vqee_example3.py` for the syntax to use molecular geometries.

Instead of directly specifying a Hamiltonian, users can instead input a molecular geometry in terms of the elements and coordinates of all atoms in the system. Qristal will then generate the corresponding Hamiltonian automatically.

For this functionality, Qristal requires PySCF-style XYZ syntax, i.e.,
 `{element symbol} {x_coord} {y_coord} {z_coord};...`

For example, an $H_2$ molecule with an atomic distance of 0.735 angstroms (Å) can be described by the following geometry string:

`H 0.0 0.0 0.0; H 0.0 0.0 0.735`

❗ **Important:** the default unit for coordinates is **angstroms**.

By default, Qristal uses the [sto-3g](https://en.wikipedia.org/wiki/STO-nG_basis_sets) basis set and the Jordan-Wigner fermion-to-qubit mapping.

## How to make function calls for a Python-level optimizer?

In order to use the optimization algorithms in `scipy.optimize` or `skquant.opt`  for VQE, it is necessary to provide a function that accepts the parameter values at the current iteration and returns the energy.  The function below provides this.  Note: it accepts an input (`theta`) that is a NumPy array:

```python
import qristal.core.optimization.vqee as qbOpt

#
# Wrapper that accepts a parameter,
# theta (type is NumPy ndarray)
# and calls vqee for 1 iteration.  The energy is returned.
#
def qbvqe(theta):
    params = qbOpt.Params()
    params.nShots = 64   # Number of shots
    params.maxIters = 1  # initial energy only - no internal optimisation steps
    # Deuteron Hamiltonian with ASWAP ansatz
    params.nQubits = 4  # Number of qubits
    params.pauliString = "5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1"
    ansatzID = qbOpt.AnsatzID.ASWAP
    nQubits = 4
    nElectrons = 2 # number of electrons/particles/vqeDepth for (UCCSD/ASWAP/HEA)
    trs = True # time reversal symmetry only when ASWAP, else just a required dummy to conform to the signature
    nOptParams = qbOpt.setAnsatz(params, ansatzID, nQubits, nElectrons, trs)
    if (len(theta) == 1):
        params.optimalParameters = theta
    else :
        params.optimalParameters = list(theta)

    qv = qbOpt.VQEE(params)
    qv.run()
    return params.optimalValue

```

With small modifications, the function shown above can be adapted to return other quantities (eg. Jacobian) needed by the optimization algorithm.




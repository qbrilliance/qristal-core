# In depth: `vqeeCalculator` 
`vqeeCalculator` is a command-line executable for accessing Qristal's MPI-enabled Variational Quantum Eigensolver (VQE) functionality.

## Compiling
```bash
cd $QRISTAL_INSTALL_DIR/examples/cpp/vqeeCalculator
mkdir build
cd build
cmake .. -DENABLE_MPI=ON
make
```

## Limitations
- Noise model: none

## JSON input and output
Instead of supplying command line options, `vqeeCalculator` reads a JSON file (and output results to another JSON file) when called with these options:
```bash
mpiexec -n 2 vqeeCalculator --fromJson=$json_input_file --jsonID=0 --outputJson=$json_output_file

```
**Input**

The fields that are recognized in `$json_input_file` are shown below:
| Key | Example value | Details |
| ---- | ---- | ---- |
| `nQubits` | `"nQubits": 5` | Number of physical qubits |
| `nShots` | `"nShots": 128` | Number of circuit execution shots |
| `pauli` | `"pauli": `<br />`"5.907 - `<br />`2.1433 X0X1 - `<br />`2.1433 Y0Y1 + `<br />`.21829 Z0 - `<br />`6.125 Z1"` | Qubit Hamiltonian expressed as <br />a weighted sum of Pauli terms |
| `geometry` | `"geometry": `<br />`"H 0.0 0.0 0.0; `<br />`H 0.0 0.0 0.7408481486"` | Alternative to specifying `pauli`: <br />atoms in X-Y-Z co-ordinates.<br />Unit is angstroms |
| `ansatz` | `"ansatz": "ASWAP"` | Predefined quantum circuit conditioned <br/>by parameter `thetas` |
| `nElectrons` | `"nElectrons": 2` | Number of particles in the ansatz |
| `circuit` | See this [link](https://qristal.readthedocs.io/en/latest/rst/vqe.html#user-defined-ansatz) for information | Alternative to specifying `ansatz` and `nElectrons` |
| `thetas` | `"thetas": [-0.2,0.14,1.01]` | Initial values for ansatz parameters.<br/>A value is needed for each <br />parameter in the ansatz. |
| `acceleratorName` | `"acceleratorName": "qpp"` | Backend for executing quantum circuits |
| `maxIters` | `"maxIters": 100` | Maximum iteration count <br />for the classical optimizer |
| `algorithm` | `"algorithm": "nelder-mead"` | Selects the algorithm used for optimization |
| `extraOptions` | `"extraOptions": "{<key1>: <value1>, <key2>: <value2>, ...}"` | Extra options specific <br />to the optimization <br />algorithm set in <br />`algorithm`.  <br /><br />More details on the <br />key-value pairs relevant to each <br />algorithm are at this [link](https://qristal.readthedocs.io/en/latest/rst/vqe.html#setting-up-the-optimization-algorithm). |
| `isDeterministic` |`"isDeterministic": true` | When set to `false`, <br />expectations are calculated <br />from samples (`nShots` in size)  <br /><br />When<br /> `"acceleratorName": "qpp"`<br />and <br/>`"isDeterministic": true`, <br />expectations are calculated directly <br />using linear algebra. |
| `enableVis` | `"enableVis": true` | Enable convergence trace <br />visualisation |
| `showTheta` | `"showTheta": true` | Enable the display of <br />theta elements in the<br />convergence trace visualisation |
| `tolerance` | `"tolerance": 1.0e-5` | Function tolerance <br/>(used as a termination criterion <br />in the classical optimizer) |
| `nThreads` | `"nThreads": 2` | Number of OpenMP <br />threads to use |

**Example JSON input file**
```json
[
    {
        "nQubits": 5,
        "acceleratorName": "qpp",
        "pauli": "0.00021123007138713814  +-0.0004797898199899509 X1 Z2 X3 +-0.0004797898199899509 Y1 Z2 Y3 +-0.0008504196943407897 Z1 +-0.0004797898199899509 X2 Z3 X4 +-0.0004797898199899509 Y2 Z3 Y4 +-0.0008504196943407897 Z2 +-0.0009971065473443474 Z3 +-0.0009971065473443474 Z4",
        "ansatz": "ASWAP",
        "nElectrons": 2,
        "maxIters": 1,
        "tolerance": 1e-05,
        "nShots": 1,
        "isDeterministic": true,
        "thetas": [
            0.19282382007161974,
            0.2635945440300202,
            -0.04929291223947813,
            0.04121265889451157,
            0.16609364599654375,
            0.2502175895079587,
            0.0488588580664832,
            1.6109279933369605,
            2.921249087681776
        ]
    }
]
```
**Output**

The fields that are output in `$json_output_file` are shown below:
| Key | Details |
| ---- |  ---- |
| `energy` | The optimum (min.) energy at <br />the end of VQE optimization routine |
| `iterations` | Number of iterations reached <br />before termination of the optimization routine |
| `pauli` | The Hamiltonian expressed as <br />a weighted sum of Pauli terms |
| `theta` | The value of the ansatz parameters <br />that produces the corresponding <br />optimum (min.) energy |
| `vis` | Energy and ansatz parameter <br /> trace visualization |
| `walltime_ms` | The execution wall time, in ms |

**Example output JSON file**
```json
{
    "energy": -6.722747367844044e-05,
    "iterations": 1,
    "pauli": "0.00021123007138713814  +-0.0004797898199899509 X1 Z2 X3 +-0.0004797898199899509 Y1 Z2 Y3 +-0.0008504196943407897 Z1 +-0.0004797898199899509 X2 Z3 X4 +-0.0004797898199899509 Y2 Z3 Y4 +-0.0008504196943407897 Z2 +-0.0009971065473443474 Z3 +-0.0009971065473443474 Z4",
    "theta": [
        0.19282382007161974,
        0.2635945440300202,
        -0.04929291223947813,
        0.04121265889451157,
        0.16609364599654375,
        0.2502175895079587,
        0.0488588580664832,
        1.6109279933369605,
        2.921249087681776
    ],
    "walltime_ms": 30.81
}
```

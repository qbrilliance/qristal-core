# Noise models

$QB-SDK-PRODUCT-NAME$ has built-in noise models that are useful for:
* Emulating the noise inherent in hardware quantum gates, and
* Optimizing qubit assignment for a given quantum circuit.

## Summary of built-in noise models

### `default`
| Model name | `default` |
| ---- | ---- |
| **Total # qubits** | User defined |
| **Fully connected degree** | All-to-all |
| **Fidelity of 2-qubit gate** | 99.9% |
| **Fidelity of 1-qubit gate** | 99.9% |
| **Readout error p(0\|1)** | 0.01 |
| **Readout error p(1\|0)** | 0.01 |

This model is applicable to:

- [X] Open source releases: 202x
- [X] Commercial emulator releases: 202x

Availability on simulators:
- [x] aer
- [x] qsim
- [ ] tnqvm
- [ ] sparse-sim
- [ ] qpp

This model takes the number of qubits specified by the user and generates a fully connected topology between all qubits.  Quantum gate noise is modelled using single-qubit depolarization and 2-qubit depolarization channels.  The channels are uniform across all qubits.  Readout errors are also accounted for in this model.

### `nm1`

This model is applicable to:

- [ ] Open source releases: 202x
- [X] Commercial emulator releases: 202x

Availability on simulators:
- [x] aer
- [x] qsim
- [ ] tnqvm
- [ ] sparse-sim
- [ ] qpp

### `nm2`

This model is applicable to:

- [ ] Open source releases: 202x
- [X] Commercial emulator releases: 202x

Availability on simulators:
- [x] aer
- [x] qsim
- [ ] tnqvm
- [ ] sparse-sim
- [ ] qpp

## Examples showing the use of a built-in noise model
### C++
```c++
#include "session.hpp"
int main(int argc, char * argv[]) {
    auto tqb = qb::session();
    tqb.qb12();          // Set up meaningful defaults
    tqb.set_qn(2);       // 2 qubits
    tqb.set_acc("aer");  // Aer simulator selected
    tqb.set_noise(true); // Set this to true for noise models to be active
    tqb.set_noise_model("default"); // Also available from Quantum Brilliance: "nm1" , "nm2"
    tqb.set_instring(R"(
       OPENQASM 2.0;
       include "qelib1.inc";
       creg c[2];
       h q[0];
       cx q[0],q[1];
       measure q[1] -> c[1];
       measure q[0] -> c[0];
       )");
    tqb.run();
    std::string result = ((tqb.get_out_raws()).at(0)).at(0);
    std::cout << result << std::endl;
    return 0;
}
```
### Python
```python
import core
tqb = core.session()
tqb.qb12()
tqb.qn = 2
tqb.acc = "aer"
tqb.noise = True
tqb.noise_model = "default"
tqb.instring = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[2];
    h q[0];
    cx q[0],q[1];
    measure q[1] -> c[1];
    measure q[0] -> c[0];
'''
tqb.run()
tqb.out_raws[0]
```

# User defined noise models

$QB-SDK-PRODUCT-NAME$ allows an end-user to implement noise models. First use  <a href="../_cpp_api/structqb_1_1NoiseProperties.html">NoiseProperties</a> to set up noise model parameters.  Then pass the parameters to the constructor of qb::NoiseModel.

## Modifying the `default` noise model

See the source code for the `default` noise model [`DefaultNoiseModelFactory`] in:

`core/src/noise_model/noise_model_factory.cpp`

See the constructor [`NoiseModel::NoiseModel(const NoiseProperties &noise_props)`] in:

`core/src/noise_model/noise_model.cpp`

## Noise channels
| Name | Description |
| ---- | ---- |
| `amplitude_damp` | Amplitude damping with parameter $\gamma$ |
| `phase_damp` | Phase damping with parameter $\gamma$ |
| `depolarize` | Single-qubit and two-qubit depolarization with probability $p$ |
| `generalized_phase_amplitude_damp` | Generalized amplitude and phase damping with 3 parameters: `excited state population`, `amplitude damping`, `phase damping` |
| `generalized_amplitude_damp` | Generalized amplitude damping with 2 parameters: `excited state population`, `gamma` |

See the header code in:

`core/include/qb/core/noise_model/noise_model.hpp`

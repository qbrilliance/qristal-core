# Noise models

QB Qristal has built-in noise models that are useful for:
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
| **QObj generator** | "xacc-qobj"|
| **Basis gate set** | u1, u2, u3, cx |

This model is applicable to:

- [X] Open source releases: 2023 onwards
- [X] Commercial emulator releases: 2023 onwards

Availability on simulators:
- [x] aer
- [ ] qsim
- [ ] tnqvm
- [ ] sparse-sim
- [ ] qpp

This model takes the number of qubits specified by the user and generates a fully connected topology between all qubits.  Quantum gate noise is modelled using single-qubit depolarization and 2-qubit depolarization channels.  The channels are uniform across all qubits.  Readout errors are also accounted for in this model.

### `qb-nm1`

| Model name | `qb-nm1` |
| ---- | ---- |
| **QObj generator** | "qristal-qobj"|
| **Basis gate set** | rx, ry, cz |

This model is applicable to:

- [ ] Open source releases: 2023 onwards
- [X] Commercial emulator releases: 2023 onwards

Availability on simulators:
- [x] aer
- [x] qsim
- [ ] tnqvm
- [ ] sparse-sim
- [ ] qpp

### `qb-nm2`

| Model name | `qb-nm2` |
| ---- | ---- |
| **QObj generator** | "qristal-qobj"|
| **Basis gate set** | rx, ry, cz |

This model is applicable to:

- [ ] Open source releases: 2023 onwards
- [X] Commercial emulator releases: 2023 onwards

Availability on simulators:
- [x] aer
- [x] qsim
- [ ] tnqvm
- [ ] sparse-sim
- [ ] qpp

### `qb-nm3`

| Model name | `qb-nm3` |
| ---- | ---- |
| **QObj generator** | "qristal-qobj"|
| **Basis gate set** | rx, ry, cz |

This model is applicable to:

- [ ] Open source releases: 2023 onwards
- [X] Commercial emulator releases: 2023 onwards

Availability on simulators:
- [x] aer
- [x] qsim
- [ ] tnqvm
- [ ] sparse-sim
- [ ] qpp

### `qb-qdk1`

| Model name | `qb-qdk1` |
| ---- | ---- |
| **QObj generator** | "qristal-qobj"|
| **Basis gate set** | rx, ry, cz |

This model is applicable to:

- [ ] Open source releases: 2023 onwards
- [X] Commercial emulator releases: 2023 onwards

Availability on simulators:
- [x] aer
- [ ] qsim
- [ ] tnqvm
- [ ] sparse-sim
- [ ] qpp

### `qb-dqc2`

| Model name | `qb-dqc2` |
| ---- | ---- |
| **QObj generator** | "qristal-qobj"|
| **Basis gate set** | rx, ry, cz |

This model is applicable to:

- [ ] Open source releases: 2023 onwards
- [X] Commercial emulator releases: 2023 onwards

Availability on simulators:
- [x] aer
- [ ] qsim
- [ ] tnqvm
- [ ] sparse-sim
- [ ] qpp

## Examples showing the use of a built-in noise model
### C++
This example can be found at `examples/cpp/noise_model`, along with a `CMakeLists.txt` file for building it.
```C++
#include "qb/core/session.hpp"
int main(int argc, char * argv[])
{
    auto my_sim = qb::session();
    my_sim.qb12();          // Set up meaningful defaults
    my_sim.set_qn(2);       // 2 qubits
    my_sim.set_acc("aer");  // Aer simulator selected
    my_sim.set_noise(true); // Set this to true for noise models to be active
    my_sim.set_noise_model("default"); // Also available from the Qristal Emulator: "qb-nm1" , "qb-nm2" , "qb-qdk1"
    my_sim.set_instring(R"(
       OPENQASM 2.0;
       include "qelib1.inc";
       creg c[2];
       h q[0];
       cx q[0],q[1];
       measure q[1] -> c[1];
       measure q[0] -> c[0];
       )");
    my_sim.run();
    std::string result = ((my_sim.get_out_raws()).at(0)).at(0);
    std::cout << result << std::endl;
    return 0;
}
```
### Python
This example can be found at `examples/python/noise_model.py`.
```python
import qb.core
my_sim = qb.core.session()
my_sim.qb12()
my_sim.qn = 2
my_sim.acc = "aer"
my_sim.noise = True
my_sim.noise_model = "default"
my_sim.instring = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[2];
    h q[0];
    cx q[0],q[1];
    measure q[1] -> c[1];
    measure q[0] -> c[0];
'''
my_sim.run()
print(my_sim.out_raws[0])
```

# User defined noise models

QB Qristal allows an end-user to implement noise models. First use  <a href="../_cpp_api/structqb_1_1NoiseProperties.html">NoiseProperties</a> to set up noise model parameters.  Then pass the parameters to the constructor of `qb::NoiseModel`.

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

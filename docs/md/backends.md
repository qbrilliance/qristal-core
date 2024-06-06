# Backends

Qristal can execute circuits on a wide variety of hardware and simulator backends.


## Hardware backends

### Quantum Brilliance

**Description**: Quantum Brilliance room-temperature QPUs based on diamond NV technology.  Includes all rack-mounted Quantum Development Kit (QDK) and desktop quantum computer (DQC) iterations, as well as virtualised instances.

**Provided by**: Open-source Qristal SDK

**Executes on**: A local or remote Quantum Brilliance quantum computer.

**Parameters**:

* `exclusive_access`: *boolean*, optional. Request exclusive use of the hardware device before attempting to execute any circuits on it.  If this flag is set `true` and the hardware device accepts the request, it will accept circuits only from the current instance of Qristal for the duration of exclusive mode. Default: False
  <br/>The following option is required if `exclusive_access = true`:
  * `exclusive_access_token`: *string*. The JSON web token (JWT) required to execute circuits on the device during exclusive mode.
<br/><br/>
* `init`: *vector of integers*, optional. Values to initialise the qubits to. Indexing of this list matches the indices of qubits in circuits. Default: all qubits initialised to 0.
<br/><br/>
* `poll_secs`: *floating-point*, required. Interval in seconds to wait between queries to the device about the completion status of a running circuit.
<br/><br/>
* `poll_retries`: *integer*, required. Maximum number of times to query the device about the completion status of a running circuit. If this many unsuccessful queries are made, return a timeout error.
<br/><br/>
* `recursive`: *boolean*, required. If less than the requested number of shots is returned, resubmit the circuit to the device (potentially multiple times) in order to generate additional shots. Not compatible with `resample`.
<br/><br/>
* `resample`: *boolean*, required. If less than the requested number of shots is returned, bootstrap resample from the received counts in order to make up any shortfall. Not compatible with `recursive`.
<br/><br/>
* `url`: *string*, required.  The URL of the hardware device (or its circuit server *qcstack*, if *qcstack* is not running on the hardware device itself).
<br/><br/>
* `use_default_contrast_settings`: *boolean*, optional. Run with the default measurement contrast settings defined by the hardware device. Default: True
  <br/>The following options are required if `use_default_contrast_settings = false`:
  * `init_contrast_threshold`: *floating-point*.  The contrast threshold to use for initialisation during post-selection. Shots are only accepted if qubit initialisation using pulsed ODMR results in a photoluminescence contrast exceeding this value. The usable upper bound on this setting is 0.6. Hardware default is 0.1.
  * `qubit_contrast_thresholds`: *{ integer keys $\rightarrow$ floating-point values }*.  The contrast threshold to apply on a per-qubit basis during final readout. Shots are only accepted if readout using pulsed ODMR results in a photoluminescence contrast exceeding this value. The best realistic case is about 0.3. Hardware is unusable when this is less than about 0.05. Indexing of this list matches the indices of qubits in circuits. Hardware default is 0.1.


These parameters are set in the `remote_backends.yaml` configuration file, under the top-level heading corresponding to the named QB hardware resource to which they should be applied. To use an alternative file, assign its path to the `qb.session.remote_backend_database_path` attribute.

**Example**: `tests/qcstack`.

**Notes**:

Entries in `remote_backends.yaml` may contain environment variables, in order to facilitate interoperability with job schedulers such as qtil and SLURM, e.g.
```yaml
qpu1:
  url: $QTIL_qpus_1_IP
  exclusive_access: true
  exclusive_access_token: $QTIL_qpus_1_TOKEN
  ...
```
where `$QTIL_qpus_1_IP` and `$QTIL_qpus_1_TOKEN` are the device IP and JWT provided by qtil via SLURM during job allocation.


### Amazon Braket Rigetti: `aws-braket` (Rigetti)
**Description**: Executor on Rigetti hardware, provided by Amazon Web Services (AWS) Braket.

**Provided by**: Open-source Qristal SDK

**Executes on**: AWS (remote Rigetti QPU)

**Parameters**:

* `device`: *string*, required. Set to `Rigetti` to use this backend.
* `format`: *string*, required. The format of the circuit to be sent for execution.  Must be either `braket` or `openqasm3`.
* `s3`: *string*, required. The name of the AWS S3 bucket where the results of running the circuit should be saved.  Must begin with `"amazon-braket"`.
* `path`: *string*, required. The path to the folder within the AWS S3 bucket where the results of running the circuit should be saved.
* `verbatim`: *boolean*, optional, default `false`. Execute circuits verbatim as provided on AWS Braket. Setting `verbatim = true` causes the circuit submitted to Braket to be executed directly, without any optimisation, placement, transpilation or other IR transformations.

These parameters are set under the `aws-braket` heading in the `remote_backends.yaml` configuration file. To use an alternative file, assign its path to the `qb.session.remote_backend_database_path` attribute.

**Example**: tests/AWSBraketHostedTester.Simple

**Notes**: The example in tests/AWSBraketHostedTester.Simple has the call to the Rigetti backend commented out at present, as no Rigetti devices were available on AWS Braket at the time of writing. 

## State-vector simulators

### Quantum Brilliance state vector: `qsim`

**Description**: A noise-aware, GPU-enabled state-vector simulator developed by Quantum Brilliance, built atop the Google Cirq qsim simulator.

**Provided by**: Open-source Qristal SDK (CPU version), Qristal Emulator (GPU version)

**Executes on**: CPU or GPU 

**Parameters**:

* `noise`: *boolean*, optional. Include noise in the circuit simulation.

These parameters are currently `qb.core.session` attributes.

**Example**: `qsim_noisy.py`


### Microsoft sparse state vector: `sparse-sim`

**Description**: The Microsoft Quantum sparse state-vector simulator allows a high number of qubits to be simulated whenever the quantum circuit being executed preserves sparsity.  It utilises a sparse representation of the state vector and methods to delay the application of some gates (e.g. Hadamard).

**Provided by**: Open-source Qristal SDK

**Executes on**: CPU

**Parameters**: None

**Example**: `parametrization_demo.py`


### Quantum++ state-vector simulator: `qpp` and `cudaq:qpp`

**Description**: The Quantum++ state vector simulator, configured to use XACC IR (`qpp`) or QIR (`cudaq:qpp`).

**Provided by**: Open-source Qristal SDK

**Executes on**: CPU

**Requires**: CUDA Quantum (`cudaq:qpp` version only)

**Parameters**: None

**Example**: `demo1.py`


### cuQuantum state-vector simulators: `custatevec:fp32` and `custatevec:fp64`

**Description**: Single (`custatevec:fp32`) and double-precision (`custatevec:fp64`) versions of the CUDA Quantum state vector simulator, built on CuQuantum libraries.

**Provided by**: Open-source Qristal SDK

**Executes on**: GPU

**Requires**: CUDA Quantum

**Parameters**: None

**Example**: `cudaq_qft.py`


### Aer state vector: `aer` (`statevector`)

**Description**: IBM Qiskit Aer noise-aware state-vector simulator.

**Provided by**: Open-source Qristal SDK

**Executes on**: CPU

**Parameters**:

* `aer_sim_type`: *string*, optional. The type of AER simulator. Can be set as `aer_sim_type = statevector` (default).
* `noise`: *boolean*, optional. Include noise in the circuit simulation.

These parameters are currently `qb.core.session` attributes.

**Example**: `noise_model.py`


### Amazon Braket SV1: `aws-braket` (SV1)
**Description**: State-vector simulator provided by Amazon Web Services (AWS) Braket.

**Provided by**: Open-source Qristal SDK

**Executes on**: AWS (remote CPU virtual machine)

**Parameters**:

* `device`: *string*, required. Set to `SV1` to use this backend.
* `format`: *string*, required. The format of the circuit to be sent for execution.  Must be either `braket` or `openqasm3`.
* `s3`: *string*, required. The name of the AWS S3 bucket where the results of running the circuit should be saved.  Must begin with `"amazon-braket"`.
* `path`: *string*, required. The path to the folder within the AWS S3 bucket where the results of running the circuit should be saved.
* `noise`: *boolean*, required. Include noise in the circuit simulation. 
* `verbatim`: *boolean*, optional, default `false`. Execute circuits verbatim as provided on AWS Braket. Setting `verbatim = true` causes the circuit submitted to Braket to be executed directly, without any optimisation, placement, transpilation or other IR transformations.

These parameters are set under the `aws-braket` heading in the `remote_backends.yaml` configuration file. To use an alternative file, assign its path to the `qb.session.remote_backend_database_path` attribute.

**Example**: tests/AWSBraketHostedTester.Simple

**Notes**: Up to 34 qubits and 10,000 shots supported.


## Tensor network simulators


### Quantum Brilliance Matrix-Product State (MPS): `qb-mps` and `cudaq:qb_mps`

**Description**: Quantum Brilliance’s noise-aware MPS simulator, configured to use XACC IR (`qb-mps`) or QIR (`cudaq:qb_mps`). The MPS method represents the quantum wavefunction as a tensor contraction of individual qubit's quantum state. Each qubit's quantum state is a rank-3 tensor (rank-2 tensor for boundary qubits).

**Provided by**: Qristal Emulator

**Executes on**: GPU

**Requires**: CUDA Quantum (`cudaq:qb_mps` version only)

**Parameters**:

* `max_bond_dimension`: *integer*, optional, range: [1, 50000], default: 256. Maximum number of singular values kept in the virtual index/dimension.
* `initial_bond_dimension`: *integer*, optional, range: [1, 50000], default: 1. Initial number of singular values in the virtual index/dimension. Can be used to speed up simulation if final state's bond dimension is known.
* `svd_cutoff`: *floating-point*, optional, default: 1.0e-8. Smallest absolute value of the singular values to keep.
* `rel_svd_cutoff`: *floating-point*, optional, default: 1.0e-4. Smallest singular value, relative to the largest singular value, to keep.
* `noise`: *boolean*, optional. Include noise in the circuit simulation.
* `measure_sample_sequential`: *string*, optional. Measurement sampling modes: `on` for cutensor-based sampling, `off` for cutensornet-based sampling, `auto` (default) for cutensornet-based sampling with cutensor-based sampling as fallback.

These parameters are currently `qb.core.session` attributes.

**Example**: `qb_mps_noisy.py`

**Notes**: 

In a `qb.core.session`, the `qb12()` method sets the following parameters:

* `max_bond_dimension`: 256 
* `initial_bond_dimension`: 1 
* `svd_cutoff`: 1.0e-6
* `rel_svd_cutoff`: 1.0e-3
* `noise`: `false`
* `measure_sample_sequential`: `auto`

### Quantum Brilliance Matrix-Product Density Operator: `qb-mpdo` and `cudaq:qb_mpdo`

**Description**: Quantum Brilliance’s noise-aware matrix-product density operator (MPDO) simulator, configured to use XACC IR (`qb-mpdo`) or QIR (`cudaq:qb_mpdo`). The MPDO method represents the density matrix as a tensor contraction of individual qubit's density operator. Each qubit's density operator is a rank-4 tensor (rank-3 tensor for boundary qubits).

**Provided by**: Qristal Emulator

**Executes on**: GPU

**Requires**: CUDA Quantum (`cudaq:qb_mpdo` version only)

**Parameters**:

* `max_bond_dimension`: *integer*, optional, range: [1, 50000], default: 256. Maximum number of singular values kept in the virtual index/dimension.
* `initial_bond_dimension`: *integer*, optional, range: [1, 50000], default: 1. Initial number of singular values in the virtual index/dimension. Can be used to speed up simulation if final state's bond dimension is known.
* `svd_cutoff`: *floating-point*, optional, default: 1.0e-8. Smallest absolute value of the singular values to keep.
* `rel_svd_cutoff`: *floating-point*, optional, default: 1.0e-4. Smallest singular value, relative to the largest singular value, to keep.
* `noise`: *boolean*, optional. Include noise in the circuit simulation.
* `measure_sample_sequential`: *string*, optional. Measurement sampling modes: `on` for cutensor-based sampling, `off` for cutensornet-based sampling, `auto` (default) for cutensornet-based sampling with cutensor-based sampling as fallback.

These parameters are currently `qb.core.session` attributes.

**Example**: `qb_mpdo_noisy.py`

**Notes**: 

In a `qb.core.session`, the `qb12()` method sets the following parameters:

* `max_bond_dimension`: 256 
* `initial_bond_dimension`: 1 
* `svd_cutoff`: 1.0e-6
* `rel_svd_cutoff`: 1.0e-3
* `noise`: `false`
* `measure_sample_sequential`: `auto`

### Quantum Brilliance purification: `qb-purification` and `cudaq:qb_purification`

**Description**: Quantum Brilliance’s noise-aware state purification simulator, configured to use XACC IR (`qb-purification`) or QIR (`cudaq:qb_purification`). The purification method represents the purified quantum state as a tensor contraction of individual qubit's purified state. Each qubit's purified state is a rank-4 tensor (rank-3 tensor for boundary qubits).

**Provided by**: Qristal Emulator

**Executes on**: GPU

**Requires**: CUDA Quantum (`cudaq:purification` version only)

**Parameters**:

* `max_bond_dimension`: *integer*, optional, range: [1, 50000], default: 256. Maximum number of singular values kept in the virtual index/dimension.
* `initial_bond_dimension`: *integer*, optional, range: [1, 50000], default: 1. Initial number of singular values in the virtual index/dimension. Can be used to speed up simulation if final state's bond dimension is known.
* `max_kraus_dimension`: *integer*, optional, range: [1, 50000], default: 256. Maximum number of singular values kept in the Kraus index/dimension.
* `initial_kraus_dimension`: *integer*, optional, range: [1, 50000], default: 1. Initial number of singular values in the Kraus index/dimension (purification only). Can be used to speed up simulation if final state's Kraus dimension is known.
* `svd_cutoff`: *floating-point*, optional, default: 1.0e-8. Smallest absolute value of the singular values to keep.
* `rel_svd_cutoff`: *floating-point*, optional, default: 1.0e-4. Smallest singular value, relative to the largest singular value, to keep.
* `noise`: *boolean*, optional. Include noise in the circuit simulation.
* `measure_sample_sequential`: *string*, optional. Measurement sampling modes: `on` for cutensor-based sampling, `off` for cutensornet-based sampling, `auto` (default) for cutensornet-based sampling with cutensor-based sampling as fallback.

These parameters are currently `qb.core.session` attributes.

**Example**: `qb_purification_noisy.py`

**Notes**: 

In a `qb.core.session`, the `qb12()` method sets the following parameters:

* `max_bond_dimension`: 256 
* `initial_bond_dimension`: 1 
* `max_kraus_dimension`: 256
* `initial_kraus_dimension`: 1
* `svd_cutoff`: 1.0e-6
* `rel_svd_cutoff`: 1.0e-3
* `noise`: `false`
* `measure_sample_sequential`: `auto`


### TNQVM-ExaTN MPS: `tnqvm`

**Description**: Matrix product state simulator from the ExaTN package.

**Provided by**: Open-source Qristal SDK

**Executes on**: CPU

**Parameters**

* `max_bond_dimension`: 
* `svd_cutoff`

These parameters are currently `qb.core.session` attributes.

**Example**: `test_random_seed_tnqvm()` in `tests/python_module/execution_test.py`


### Aer MPS: `aer` (`matrix_product_state`)

**Description**: IBM Qiskit Aer noise-aware MPS simulator.

**Provided by**: Open-source Qristal SDK

**Executes on**: CPU

**Parameters**:

* `aer_sim_type`: *string*, required. The type of AER simulator. Must be set as `aer_sim_type = matrix_product_state`.
* `noise`: *boolean*, optional. Include noise in the circuit simulation.

These parameters are currently `qb.core.session` attributes.

**Example**: `aer_simulator/aer_mps_simple.py`


### Amazon Braket TN1: `aws-braket` (TN1)

**Description**: Tensor-network simulator provided by Amazon Web Services (AWS) Braket.

**Provided by**: Open-source Qristal SDK

**Executes on**: AWS (remote CPU virtual machine)

**Parameters**:

* `device`: *string*, required. Set to `TN1` to use this backend.
* `format`: *string*, required. The format of the circuit to be sent for execution.  Must be either `braket` or `openqasm3`.
* `s3`: *string*, required. The name of the AWS S3 bucket where the results of running the circuit should be saved.  Must begin with `"amazon-braket"`.
* `path`: *string*, required. The path to the folder within the AWS S3 bucket where the results of running the circuit should be saved.
* `noise`: *boolean*, required. Include noise in the circuit simulation. 
* `verbatim`: *boolean*, optional, default `false`. Execute circuits verbatim as provided on AWS Braket. Setting `verbatim = true` causes the circuit submitted to Braket to be executed directly, without any optimisation, placement, transpilation or other IR transformations.

These parameters are set under the `aws-braket` heading in the `remote_backends.yaml` configuration file. To use an alternative file, assign its path to the `qb.session.remote_backend_database_path` attribute.

**Example**: tests/AWSBraketHostedTester.Simple

**Notes**: Up to 48 qubits and 999 shots supported.



## Density matrix simulators


### CUDA Quantum density matrix simulator: `cudaq:dm`

**Description**: The CUDA Quantum density matrix simulator, built on CuQuantum libraries.

**Provided by**: Open-source Qristal SDK

**Executes on**: GPU

**Requires**: CUDA Quantum

**Parameters**: None

**Example**: `tests/python_module/cudaq_integration.py`


### Amazon Braket DM1: `aws-braket` (DM1)

**Description**: Density-matrix simulator provided by Amazon Web Services (AWS) Braket.

**Provided by**: Open-source Qristal SDK

**Executes on**: AWS (remote CPU virtual machine)

**Parameters**:

* `device`: *string*, required. Set to `DM1` to use this backend.
* `format`: *string*, required. The format of the circuit to be sent for execution.  Must be either `braket` or `openqasm3`.
* `s3`: *string*, required. The name of the AWS S3 bucket where the results of running the circuit should be saved.  Must begin with `"amazon-braket"`.
* `path`: *string*, required. The path to the folder within the AWS S3 bucket where the results of running the circuit should be saved.
* `noise`: *boolean*, required. Include noise in the circuit simulation. 
* `verbatim`: *boolean*, optional, default `false`. Execute circuits verbatim as provided on AWS Braket. Setting `verbatim = true` causes the circuit submitted to Braket to be executed directly, without any optimisation, placement, transpilation or other IR transformations.

These parameters are set under the `aws-braket` heading in the `remote_backends.yaml` configuration file. To use an alternative file, assign its path to the `qb.session.remote_backend_database_path` attribute.

**Example**: tests/AWSBraketHostedTester.Simple

**Notes**: 
- Up to 17 qubits and 10,000 shots supported.
- Noise must be enabled in order to use this simulator.  This is done by setting the `qb.core.session` attribute `noise = True`.


### Aer density matrix: `aer` (`density_matrix`)

**Description**: IBM Qiskit Aer noise-aware density matrix simulator.

**Provided by**: Open-source Qristal SDK

**Executes on**: CPU

**Parameters**:

* `aer_sim_type`: *string*, required. The type of AER simulator. Must be set as `aer_sim_type = density_matrix`.
* `noise`: *boolean*, required. Include noise in the circuit simulation.

These parameters are currently `qb.core.session` attributes.

**Example**:

**Notes**: Noise must be enabled in order to use this simulator.  This is done by setting the `qb.core.session` attribute `noise = True`.

# Qristal Virtual QPU

The Qristal vQPU is a virtualised quantum hardware solution. It offers an interface indistinguishable from a real QPU, executing quantum circuits and participating in resource allocation by cloud and HPC schedulers in exactly the same manner as a real processor.

## Installation

The vQPU can be installed in three different ways: with the pre-built Docker image or bare metal in source or binary form.

### Binary install

Coming soon.

### Docker

Note that this method presently requires access to a QB internal development container repository, and is therefore only applicable to employees of QB and partners with relevant access agreements.

To launch a Qristal vQPU,

1. Set environment variable `VQPU_PORT` to the port that you would like to use to communicate with the vQPU, e.g. `export VQPU_PORT=8888`.
2. Set the environment variable `VQPU_CONFIG_DIR` to the full path to the directory containing the `vqpu.system_config.json` file that you wish to use to define the vQPU configuration.  Examples can be found in the Qristal emulator repository, in the folder `tests/vqpu/experiment_configs`.
3. Run the following from a Linux command prompt:
```
docker run --rm -d --name qristal_vqpu -v $VQPU_CONFIG_DIR:/mnt/qb/qcstack/packages/system/qb/system/configs/experiment_configs -p${VQPU_PORT}:8080 registry.gitlab.com/qbau/software-and-apps/emulator/qristal-vqpu-dev:latest
```
Step 2 can be skipped, and the `-v` switch removed from step 3, if you are happy to simply run with the default vQPU configuration stored in qcstack.

### Source install

Note that this method requires access to QB internal repositories, and is therefore only applicable to employees of QB and partners with relevant access agreements.

1. Install Qristal from source as per [the installation guide](https://qristal.readthedocs.io/en/latest/rst/getting_started.html#installing-from-source).
2. If planning to use emulator backends or non-default noise models, also install the Qristal Emulator [from source](https://gitlab.com/qbau/software-and-apps/emulator).
3. Clone the [qcstack repository](https://gitlab.com/qbau/control/qcstack).
4. Export the path to your qcstack repository to the environment variable `QcStackPath`
5. Install `qcstack_server` by running `poetry install` from within `$QcStackPath/apps/qcstack_server`.
4. Link `lib/qristal` from within your Qristal installation directory to `$QcStackPath/apps/qcstack_server/qristal`.
6. Launch the vQPU by changing directory to `$QcStackPath/apps/qcstack_server` and running
```
poetry run python3 -m qb.qcstack_server.qcstack_server --system vqpu --max-circuit-depth X --reservation-shared-secret Y
```
  where `X` is the maximum circuit depth that you desire the vQPU to accept, and `Y` is the secret key that you wish to be required to reserve the vQPU for exclusive use.

## Usage

Once the vQPU process is running, you may:
- execute circuits on the vQPU from an instance of Qristal by interacting with it over http as if it were a bare-metal QPU using [the QB hardware backend](https://qristal.readthedocs.io/en/latest/rst/backends.html###hardware-backends#quantum-brilliance).
- schedule and reserve the vQPU from an HPC scheduler as if were a bare metal QPU, using _qtil_.

## Options

The Qristal vQPU offers many of the same options and settings as the Qristal SDK's `session` object.  These can be set by editing or overwriting the file `$QcStackPath/packages/system/qb/system/configs/experiment_configs/vqpu.system_config.json`.

* `debug`: *boolean*. Run the vQPU with debug output.

* `qubits`: *integer*. The number of qubits to emulate.

* `noise`: *boolean*. Include noise in the circuit simulation.

* `noise_model`: *string*. Required and used only if `noise = True`. The name of the Qristal SDK or Qristal Emulator noise model to employ during QPU emulation.

* `mitigation`: *string*. Optional. Used only if `noise = True`. The name of the noise mitigation method to apply to the results produced by the vQPU.  Valid values:
  - `"None"` or `""`:             No noise mitigation (default)
  - `"ro-error"`:                 Simple readout mitigation
  - `"rich-extrap"`:              Richardson extrapolation
  - `"assignment-error-kernel"`:  SPAM correction

* `placement`: *string*. Optional. The name of the circuit placement method to apply to circuits to be executed by the vQPU.  Valid values:
  - `"None"` or `""`:             No placement (default; this is currently the only valid value if `noise = False`)
  - `"swap-shortest-path"`:       Shortest-path topological placement only.
  - `"noise-aware"`:              Noise-aware placement (includes topological placement).

* `qristal_seed`: *integer or 'null'*. Optional. The random seed to pass to Qristal to seed the vQPU. The default value (`null`) causes the vQPU to choose its own seed from the system clock.

* `gpu_device_ids`: *Vector of integers*. Optional. The GPU device IDs to use when running the simulation (for GPU-enabled accelerators only).

* `backend`: *string*. The simulator backend to use for QPU virtualisation. Valid values:
  - `"qsim"`: [Qsim state vector](https://qristal.readthedocs.io/en/latest/rst/backends.html#qsim-state-vector-qsim)
  - `"sparse-sim"`: [Microsoft sparse state vector](https://qristal.readthedocs.io/en/latest/rst/backends.html#microsoft-sparse-state-vector-sparse-sim)
  - `"qpp"`: [Quantum++ state-vector simulator (via XACC IR)](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-state-vector-simulator-qpp-and-cudaq-qpp)
  - `"cudaq:qpp"`: [Quantum++ state-vector simulator (via QIR)](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-state-vector-simulator-qpp-and-cudaq-qpp)
  - `"cudaq:custatevec_fp32"`: [cuQuantum state-vector simulator (single precision)](https://qristal.readthedocs.io/en/latest/rst/backends.html#cuquantum-state-vector-simulators-custatevec-fp32-and-custatevec-fp64)
  - `"cudaq:custatevec_fp64"`: [cuQuantum state-vector simulator (double precision)](https://qristal.readthedocs.io/en/latest/rst/backends.html#cuquantum-state-vector-simulators-custatevec-fp32-and-custatevec-fp64)
  - `"aer"`: Qiskit Aer. The actual simulator chosen is set via the option `aer_sim_type`.
  - `"aws-braket"`: Amazon Braket. The actual simulator chosen is set by the `device` field of the `aws-braket` node in the file `remote_backends.yaml` within the Qristal installation directory.  Valid values are: [SV1](https://qristal.readthedocs.io/en/latest/rst/backends.html#amazon-braket-sv1-aws-braket-sv1), [TN1](https://qristal.readthedocs.io/en/latest/rst/backends.html#amazon-braket-tn1-aws-braket-tn1) and [DM1](https://qristal.readthedocs.io/en/latest/rst/backends.html#amazon-braket-dm1-aws-braket-dm1).
  - `"qb-statevector-cpu"`: [Quantum Brilliance CPU state vector](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-cpu-state-vector-qb-statevector-cpu)
  - `"qb-statevector-gpu"`: [Quantum Brilliance GPU state vector](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-gpu-state-vector-qb-statevector-gpu)
  - `"qb-mpdo"`: [Quantum Brilliance Matrix-Product Density Operator (via XACC IR)](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-matrix-product-density-operator-qb-mpdo-and-cudaq-qb-mpdo)
  - `"cudaq:qb_mpdo"`: [Quantum Brilliance Matrix-Product Density Operator (via QIR)](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-matrix-product-density-operator-qb-mpdo-and-cudaq-qb-mpdo)
  - `"qb-purification"`: [Quantum Brilliance purification (via XACC IR)](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-purification-qb-purification-and-cudaq-qb-purification)
  - `"cudaq:qb_purification"`: [Quantum Brilliance purification (via QIR)](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-purification-qb-purification-and-cudaq-qb-purification)
  - `"qb-mps"`: [Quantum Brilliance Matrix-Product State (MPS; via XACC IR)](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-matrix-product-state-mps-qb-mps-and-cudaq-qb-mps)
  - `"cudaq:qb_mps"`: [Quantum Brilliance Matrix-Product State (MPS; via QIR)](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-matrix-product-state-mps-qb-mps-and-cudaq-qb-mps)
  - `"tnqvm"`: [TNQVM-ExaTN Matrix-Product State (MPS)](https://qristal.readthedocs.io/en/latest/rst/backends.html#tnqvm-exatn-mps-tnqvm)
  - `"cudaq:dm"`: [CUDA Quantum density matrix simulator](https://qristal.readthedocs.io/en/latest/rst/backends.html#cuda-quantum-density-matrix-simulator-cudaq-dm)

* `aer_sim_type`: *string*. Optional. Used only if `backend = "aer"`. The selected simulator from Qiskit Aer to be employed if using the `aer` backend for Qristal. Valid values:
  - `"None"` or `""`: Let the installed version of Aer choose the simulator type for itself according to the passed circuit (default)
  - `"statevector"`: [state vector](https://qristal.readthedocs.io/en/latest/rst/backends.html#aer-state-vector-aer-statevector)
  - `"matrix_product_state"`: [matrix product state](https://qristal.readthedocs.io/en/latest/rst/backends.html#aer-mps-aer-matrix-product-state)
  - `"density_matrix"`: [density matrix](https://qristal.readthedocs.io/en/latest/rst/backends.html#aer-density-matrix-aer-density-matrix)

* `aer_omp_threads`: *integer*. Optional. Used only if `backend = "aer"`. Force Qiskit Aer to use exactly the given number of OpenMP threads. If this option is not given, the number of threads is set to the value of the environment variable `OMP_NUM_THREADS`. If `OMP_NUM_THREADS` is not set, Aer chooses the number of OpenMP threads to use.

Tensor network backend settings used if `backend` is one of `qb-mpdo`, `qb-mps`, `qb-purification`, `cudaq:qb-mpdo`, `cudaq:qb-mps` or `cudaq:qb-purification`:

* `initial_bond_dimension`: *integer*, range: [1, 50000].  Initial number of singular values in the virtual index/dimension. Can be used to speed up simulation if final state's bond dimension is known.

* `max_bond_dimension`: *integer*, range: [1, 50000]. Maximum number of singular values kept in the virtual index/dimension.

* `svd_cutoff`: *floating-point*. Smallest absolute value of the singular values to keep.

* `rel_svd_cutoff`: *floating-point*. Smallest singular value, relative to the largest singular value, to keep.

* `measure_sample_method`: *string*, optional. Measurement sampling modes: `sequential` for sequential cutensor-based sampling, `cutensornet` for single-shot cutensornet-based sampling, `cutensornet_multishot` for multi-shot cutensornet-based sampling, `auto` (default) for single-shot cutensornet-based sampling with cutensor-based sampling as fallback.

Settings used if `backend` is `qb-purification` or `cudaq:qb-purification`:

* `initial_kraus_dimension`: *integer*, range: [1, 50000]. Initial number of singular values in the Kraus index/dimension. Can be used to speed up simulation if final state's Kraus dimension is known.

* `max_kraus_dimension`: *integer*, range: [1, 50000]. Maximum number of singular values kept in the Kraus index/dimension.






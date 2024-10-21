# Changelog

Qristal is a full-stack SDK for quantum accelerators.

## [X.X.X] - YYYY-MM-DD

### Breaking

- Remove qb-lambda remote accelerator superseded by Qristal vQPU.
- Retrieve noise models by smart pointer from emulator plugin.
- Changed "qb" namespace and Python module name to "qristal".
- Changed primary output type. The previous jsonised string outputs `out_raws` have now been superseded by a bit-order-agnostic `results` map of type `std::map<std::vector<bool>, int>`, presented in Python as a `dict`-ish opaquely bound type.  Qubits are now identified programmatically only by their indices in either a `std::vector<bool>` bitvector (in C++) or a `list[bool]` (in Python).  Printing bitvectors directly (via stream overloads or corresponding Python `__str__` methods) displays the highest-index bit to the left.
- Renamed session.qb12() --> session.init()
- Renamed python commands to retrieve and print state-vector
- Removed multiple internal typedefs for STL types, and simplified opaque Python bindings for those types; most opaquely bound STL types' Python type names have changed.
- Flipped state vector output to be consistent with bit string ordering.

### Added

- Added documentation and test triggers for the Qristal Virtual QPU (vQPU).
- Added session::draw_shot function to allow drawing a single random shot from the set of simulated results.
- Updated all examples and tests to use new output map format.
- Use a dynamic check against the actual memory capabilities of the machine in use in order to decide when to populate the `out_counts` vector. This replaces the previous hardcoded limit of 32 qubits.
- Added functions to convert quantum process matrices in their Choi representation to and from their superoperator representation. Also added convenient function delegations involving standard process matrices (chi), and Kraus representations.
- Added a custom pyGSTi model pack to the pyGSTi python examples for gate set tomography protocols using the QDK's native gate set: Rx(pi/2), Ry(pi/2), and CZ.
- Adjusted the pyGSTi_runner.cpp and execute_GST_XYCZ.py examples to use the new custom model pack.
- Added conditional dependency to `cppuprofile` via `cmake` flag `-DWITH_PROFILING=ON`.
- Added workflow wrapper `RuntimeAnalyzer` to `qb::benchmark` to profile runtime resources (CPU, RAM, GPU) in a user-specified interval. Please note that GPU profiling is only working for Nvidia GPUs and is delegated to `nvidia-smi` calls. It will be automatically enabled if `nvidia-smi` was found in the system.
- Added ability to send custom noise models to emulator backends.
- Added maximum likelihood estimation to standard quantum state tomography protocol. Can be enabled by calling `set_maximum_likelihood_estimation()`.
- Added process matrix solver and interpolator.
- Enabled Kraus matrix contribution probability to improve qsim Kraus matrix sampling.
- Added new `qristal::session` member functions `set_SPAM_confusion_matrix()` and `set_SPAM_correction_matrix()` to add a suitable state preparation and measurement (SPAM) confusion or correction matrix, respectively. The latter will be used to automatically correct measured results, overwriting the `results_` variable. Native results are kept separately in`results_native` instead. 
- Added the python binding `qristal.core.session.SPAM_confusion` to interface with `qristal::session::set_SPAM_confusion_matrix()`, and `qristal.core.session.results_native` with 
`qristal::session::results_native()`.
- Added parameterized CRZ, CRX and CRY gates providing controlled Z, X and Y gates respectively. CRZ is equivalent to parameterized CPhase. Also added python wrappers crz, crx and cry.
- Added a confusion matrix metric `ConfusionMatrix` to `qristal::benchmark` to evaluate confusion matrices of arbitrary SPAM workflows. 
- Added a convenience function `qristal::session::run_with_SPAM()` and a corresponding python binding `qristal.core.session.run_with_SPAM()` to (i) automatically execute a SPAMBenchmark, (ii) enable automatic SPAM correction in `qristal::session`, and (iii) finally calling `qristal::session::run()`. 
- Revealed tensor network parameters in CudaQ wrapper.

### Fixed

- Moved the `python_module` sources and includes into the main `src` and `include` paths
- Sectioned the python help strings off into a header localised to the python module and not associated with the `session` class
- Moved the session getters and setters exclusively into the `qb::core` library (i.e. so they are no longer built a second time into the python module *and* the core shared library)
- Rationalised the python STL bindings somewhat
- Ditched the qaoa-local `ValidationTwoDim` class in favour of central core version
- Fixed a bug in `CircuitBuilder::append` where the variables in the appeded circuit would not be added to the original circuit.
- Fixed a bug in `session` where the execution would fail if there were more free parameters than elements in the output probability vector.
- Added a python binding for the parametrized `U3` gate.
- Fixed memory allocation in qristal-qobj.


## [1.6.0] - 2024-06-18

### Breaking

- Removed the need to include /api/v1 in hardware device URLs. URLs containing this suffix will no longer work.
- Support new circuit endpoint /api/v1/circuits in qcstack 2024.3.1 and later. Earlier qcstack versions no longer supported.
- `session` no longer outputs a bitstring-to-counts map through `get_out_bitstrings`. A list (python)/vector(C++) of counts can be accessed through `get_out_counts`.
- QML: No longer part of the core repository as parametrized functionality has been introduced to CircuitBuilder. QML-specific functions should be fetched from the upcoming QML repo.
- Removed lightweight_core cmake target

### Added

- Added functions to transform arbitrary Eigen- and vector-based quantum process matrices to their Choi and Kraus representation.
- Refactored qb::benchmark::Pauli and qb::benchmark::BlochSphereUnitState to the qb namespace in a standalone qb/core/primitives.hpp header to circumvent noise_channel dependencies to qb::benchmark.
- Added pyGSTi wrapper to qb::benchmark including a workflow executor (benchmark/workflows/PyGSTiBenchmark.hpp) and a results evaluator (benchmark/metrics/PyGSTiResults.hpp). Arbitrary pyGSTi experiment designs may now be exported and executed to qb::benchmark through PyGSTiBenchmark and then consecutively evaluated and printed in pyGSTi compatible format using PyGSTiResults.
- Changed qb::benchmark::QuantumStateTomography to use the total number of shots provided by each measured circuit individually rather than taking the number of shots set in qb::session.
- Added pyGSTi wrapper to qb::benchmark including a workflow executor (benchmark/workflows/PyGSTiBenchmark.hpp) and a results evaluator (benchmark/metrics/PyGSTiResults.hpp). Arbitrary pyGSTi experiment designs may now be exported and executed to qb::benchmark through PyGSTiBenchmark and then consecutively evaluated and printed in pyGSTi compatible format using PyGSTiResults.
- Simplified qb_qpu hardware device class
- Added sequential contraction method option for qb-mps, qb-purification and qb-mpdo measurement sampling.
- Updated CUDA Quantum version to latest release (0.7.0) for cutensor2 support.
- Enabled Werror when WARNINGS=ON is set at cmake time.
- Added instance handlers for PowerPC CI pipelines
- Parametrized circuit building now supported using the `CircuitBuilder` (C++)/`Circuit` (Python) objects, with execution using `session`. Optimization examples can be seen in `tests/sessionOptTester.cpp`.
- `session` now calculates and outputs probability jacobians if the property `set_calc_jacobian(s)` is set to `true`. The jacobians can be accessed using `get_out_jacobians`. If the jacobians are calculated, the vectors of output probabilities can also be accessed using `get_out_probs`.
- A mapping from bitstring to vector index can be accessed using the method `session.bitstring_index`. Check the docs and python help strings for details.
- Simplified hardware backend options
- Completed shift of AWS Braket backends to yaml-based option input
- Added documentation of all backends
- Added ability to use custom Kraus matrices and custom noise model parameters to generate noise model object.

### Fixed

- Fixed Werror problem with clang by updating warning flags passed to dependencies
- Fixed benchmark1_cudaq.cpp example.


## [1.5.0] - 2024-02-21

### Breaking

- Increased minimum clang version from 16.0.0 to 16.0.6 for improved compatibility with CUDA Quantum

### Added

- Added ability to extract state vector from qpp and AER accelerator.
- Added YAML interface for remote backend options, including QB hardware
- Restructured remote backend interfaces
- Added HPC scheduler support: reserve QB hardware for exclusive access and send circuits to reserved hardware
- Added installation of py_packages_path.cmake in order to facilitate relocatable builds
- Added qb::benchmark namespace with modular, concept-based functionalities to execute arbitrary benchmarks
- Added dependency to serialization library "cereal" and added serialization wrappers for qb::session, measured bit string counts (given as qb::String), and complex dense Eigen matrices
- Added data loading and generating engine connecting metric evaluation and workflow execution in a separate level of abstraction without introducing dependencies between metrics and workflows
- Added simple circuit execution workflow wrapper for arbitrary quantum circuits (based on qb::CircuitBuilder)
- Added standard SPAM benchmark workflow creating circuits that initialize all possible bit string states |0..0> to |1..1>
- Added rotation sweep benchmark workflow creating circuits that apply single rotation gates Rx, Ry, Rz(, or Ri = I) to an arbitrary amount of qubits from a given start angle to an end angle for a given number of points
- Added standard quantum state tomography workflow wrapper for arbitrary (compatible) workflows, and measurement bases
- Added standard quantum process tomography workflow wrapper for arbitrary (compatible) workflows, measurement bases, and initialization state bases
- Added classical circuit fidelity metric compatible with workflows that can generate measured and ideal bit string counts
- Added quantum state density metric compatible with quantum state tomography workflows that can generate measured bit strings and assemble density matrices from them
- Added quantum process matrix metric compatible with quantum process tomography workflows that can generate measured bit strings and assemble process matrices from them
- Added quantum state fidelity metric compatible with workflows that can generate measured bit string results and ideal quantum densities
- Added quantum process fidelity metric compatible with workflows that can generate measured bit string results and ideal quantum process matrices


## [1.4.0] - 2023-11-20

### Added

- Support for PowerPC CPU architectures
- Support for NVIDIA Volta, Ampere and Hopper GPU architectures
- Bump up compiler requirements in readthedocs
- Add emulator readme to readthedocs
- Removed support for legacy qcstack API in QB hardware interface (QuantumBrillianceRemteAccelerator).
- Added ability to use 'qb-nm3' noise model.


## [1.3.0] - 2023-10-10

### Added

- Access to emulator tensor network parameters from session object
- QML: Option to set the number of shots and seed in QMLExecutor python interface
- QML: Fixed output dimensions for QML Python wrapper
- Nextflow: ReadtheDocs now describes how to use Nextflow and Qristal together
- Build CUDA Quantum examples automatically

### Fixed

- Enforce C++20 more stringently


## [1.2.0] - 2023-09-20

### Breaking

- VQEE: L-BFGS no longer accepts extra options because the underlying library that provides the implementation has been switched from NLOpt to MLPACK.

### Added

- GitHub hosting for publicly available repositories of Qristal
- Standardised bitstring printing order for all backends


## [1.1.0] - 2023-08-25

### Breaking

- VQEE: isDeterministic will no longer fall back to sampling for those backends that don't support isDeterministic.  Instead, an error will now be thrown.  Only the qpp accelerator is supported when isDeterministic = True (see Added section for the reasons).

### Added

- Adopted C++20 across the board, not just for CUDA Quantum mains.
- Added the ability to choose CUDA Quantum backends at runtime rather than compile time.
- Transpilation: wrapping of gate angles to [-pi,pi] for native gates.
- Transpilation: out_transpiled_circuit (OpenQASM 2.0) in native gates.
- Profiler: counting number of gates and timing data after transpilation to the native gate set.
- VQE: convergence trace visualization.
- VQE: For the qpp simulator only: isDeterministic = True now saves the state-vector and calculates expectations directly (without shot sampling)
- VQE: Nelder-Mead optimiser, including extra options for constraining the ansatz parameters [lowerbounds, upperbounds] and for stopping the optimisation upon reaching a set energy threshold [stopval].
- VQE: L-BFGS optimiser, with extra options identical to that of Nelder-Mead shown above.
- VQE: ADAM optimiser, including extra options: beta1, beta2, eps, exactobjective, momentum, stepsize.
- VQE: CMA-ES optimiser, including extra options: lambda, lower, upper. Note this known issue: https://github.com/eclipse/xacc/issues/574
- vqeeCalculator: the command line tool for VQE has been augmented to allow output to JSON file format.
- swap_placement_pass and noise_aware_placement_pass: C++ and Python classes for circuit placement.
- Noise models: improved API for user-defined noise models, with C++ and Python examples.
- circuit_optimizer, redundancy_removal, two_qubit_squash, peephole_optimisation and simplify_initial circuit optimization passes.
- Ability to set circuit optimization passes using the session's circuit_optimization property.
- Ability to use CUDA Quantum simulator backends with non-CUDAQ kernels (e.g., OpenQASM or circuit builder).
- Ability to use NoiseModel instances as input to the noise-aware placement pass.
- Ability in XACC to compile architecture-independent code (useful for creation of universal Docker images).

### Fixed

- Better detection of system installation of TKET
- Corner case issue with EXATN when using non-gcc compilers without OpenMP
- Linking errors with CUDA Quantum C++ examples caused by update of CUDA Quantum version
- CMake error when spdlog is system installed but qasm_simulator is not.
- Minor documentation fixes.
- CUDA Quantum: upgrade to the latest main (commit hash 603affc).
- Profiler: measurement operations are double-counted in the timing profile (one as gate and one as readout).
- AWS accelerator: query backend devices and their Amazon Resource Names (ARNs) for Rigetti provider.
- Rename the XACC plugin for noise-aware IR transformation from "tket" to "noise-aware".
- Harmonise install library dir name across different systems and dependencies (fixes lib vs lib64 issue)
- Improved finding of installed dependencies with cmake.
- Guarantee fixed-point format for real numbers in the XASM output of the QuantumBrillianceRemoteVisitor.
- Harmonise run and run_async methods of the session class.
- Fixed a bug in XACC - TKET rotation parameter conversion.


## [1.0.0] - 2023-05-19

First non-beta release.


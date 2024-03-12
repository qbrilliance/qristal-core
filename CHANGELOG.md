# Changelog

Qristal is a full-stack SDK for quantum accelerators.

## [X.X.X] - YYYY-MM-DD

### Added 

- Added functions to transform arbitrary Eigen- and vector-based quantum process matrices to their Choi and Kraus representation.
- Refactored qb::benchmark::Pauli and qb::benchmark::BlochSphereUnitState to the qb namespace in a standalone qb/core/primitives.hpp header to circumvent noise_channel dependencies to qb::benchmark. 

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

### Fixed

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


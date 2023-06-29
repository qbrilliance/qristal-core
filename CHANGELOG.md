# Changelog

Qristal is a full-stack SDK for quantum accelerators.

## [Not yet released]
### Breaking

- VQEE: isDeterministic will no longer fall back to sampling for those backends that don't support isDeterministic.  Instead, an error will now be thrown.  Only the qpp accelerator is supported when isDeterministic = True (see Added section for the reasons).

### Added

- Transpilation: wrapping of gate angles to [-pi,pi] for native gates.
- Transpilation: out_transpiled_circuit (OpenQASM 2.0) in native gates.
- Profiler: counting number of gates and timing data after transpilation to the native gate set.
- VQE: convergence trace visualization.
- VQE: For the qpp simulator only: isDeterministic = True now saves the state-vector and calculates expectations directly (without shot sampling)
- vqeeCalculator: the command line tool for VQE has been augmented to allow output to JSON file format.
- swap_placement_pass and noise_aware_placement_pass: C++ and Python classes for circuit placement.
- Noise models: improved API for user-defined noise models, with C++ and Python examples.
- circuit_optimizer, redundancy_removal, two_qubit_squash, peephole_optimisation and simplify_initial circuit optimization passes.
- Ability to set circuit optimization passes using the session's circuit_optimization property. 

### Fixed

- CMake error when spdlog is system installed but qasm_simulator is not.
- Minor documentation fixes.
- CUDA Quantum: upgrade to the latest main (commit hash 06557d4).
- Profiler: measurement operations are double-counted in the timing profile (one as gate and one as readout).
- AWS accelerator: query backend devices and their Amazon Resource Names (ARNs) for Rigetti provider.  
- Rename the XACC plugin for noise-aware IR transformation from "tket" to "noise-aware". 
- Harmonise install library dir name across different systems and dependencies (fixes lib vs lib64 issue)
- Improved finding of installed dependencies with cmake.
- Guarantee fixed-point format for real numbers in the XASM output of the QuantumBrillianceRemoteVisitor.  

## [1.0.0] - 2023-05-19

First non-beta release.


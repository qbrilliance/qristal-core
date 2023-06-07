# Changelog

Qristal is a full-stack SDK for quantum accelerators.

## [Not yet released]

### Added

- Transpilation: wrapping of gate angles to [-pi,pi] for native gates.
- Transpilation: out_transpiled_circuit (OpenQASM 2.0) in native gates.
- Profiler: counting number of gates and timing data after transpilation to the native gate set.

- vqeeCalculator: the command line tool for VQE has been augmented to allow output to JSON file format.

### Fixed

- CMake error when spdlog is system installed but qasm_simulator is not.
- Minor documentation fixes.
- CUDA Quantum: upgrade to the latest main (commit hash 06557d4).

## [1.0.0] - 2023-05-19

First non-beta release.


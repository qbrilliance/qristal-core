# First Release Notes (Sept 2021)

### Features & Functionalities

In this first release,  qbOS provides:

- Timing information on near-term Quantum Brilliance Hardware
- Transpiled circuit output (i.e. the user's circuit expressed in the basis gate set that the Quantum Brilliance's Hardware actually uses)
- Noise model of Quantum Brilliance's hardware
- Placement of user-specified circuits subject to the physical constraints of Quantum Brilliance's hardware
- Ability to emulate up to *48 qubits* on Quantum Brilliance's hardware
- The Emulator can accept any Quantum Instruction Sets, including [XASM](https://github.com/eclipse/xacc), [OpenQASM](https://github.com/Qiskit/openqasm), [Quil](https://github.com/rigetti/quil), directly
- Statistics (counts) from multiple shots running a user-specified circuit
- Comparison between simulated results and theoretical ground-truth
- Selectable simulator engines including GPU capable and HPC-aware tensor network simulators
- [Random circuits](https://www.notion.so/Quantum-Emulator-v1-0-User-Guide-for-command-lines-20905c04dd6b4a279cc32a528cdf7bfd) of user-specified depth and number of qubits
- High-level parameters the user can currently input: depth of the random circuit (which is a minimum depth in practice), number of qubits, number of experiments/cycles
- Placement of parameterised and non-parameterised one-qubit and two-qubit gates randomly at each circuit level
- Saving a random quantum circuit in OpenQASM 2.0 format
- Cloud computing platform support using the Pawsey Nimbus Cloud

These features will be useful for:

- Feasibility studies
- Accelerating in-house algorithm development
- Collecting evidence to support grants for HPC resource allocations
- Acquiring simulated data to support research grant applications
- Quantum readiness education
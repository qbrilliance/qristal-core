# Examples #

The `examples` folder contains a series of example C++ and Python main programs.

After installing the core (`make install` in the main project build folder of either the core or SDK), the Python programs can be run by simply invoking
```
python3 <name_of_example>.py
```

The C++ examples are installed as source files with working CMakeLists.txt build scripts, to demonstrate how to use the core library to build your own C++ main program. To build and run an example in the installation directory, do
```
cd <Qristal_installation_directory>
mkdir build
cd build
cmake ../examples/cpp/<name_of_example>
make
./<name_of_example>
```

The C++ example programs are _also_ built and properly linked automatically when running `make install` in the build folder of the core and SDK projects, but the resulting binaries are _not_ installed.  They simply appear in the main project build folder, and can be executed directly from there.

A description of the available gates and the syntax used to implement them in Python is given in the [Quantum Computing](https://qristal.readthedocs.io/en/latest/rst/quantum_computing.html) section.

## Python ##

`demo1.py`

_qubits_: 2
_gate depth_: 3
_noise_: false

A simple example showing a 2 qubit circuit consisting of just a Hadamard and a regular 1-qubit NOT gate.

`bell_with_SPAM.py`

_qubits_: 2
_gate depth_: 2
_noise_: true

A simple example demonstrating a 2 qubit Bell circuit with automatic SPAM measurement and correction.

`quickstart.py`

_qubits_: 2
_gate depth_: 6
_noise_: false

The 2-qubit Bell state quickstart example given in the README of the main project.

`dj_large.py`

_qubits_: 41
_gate depth_: 204
_noise_: true

A relatively large Deutsch-Jozsa circuit example. This example may take some time to run.

`amcu.py`

_qubits_: 5
_gate depth_: 3392
_noise_: false

Stands for "ancilla multi-controlled unitary" and is a Python binding of `MultiControlledUWithAncilla`. Performs a multi-controlled unitary operator U on the target qubit conditioned on the controlled qubits. In this example, the unitary U is defined to be the x gate. The control qubits are first encoded with the bitstring[j] = 1. Ancilla qubits act as scratch qubits used to check whether the desired condition is met, i.e. the jth control qubit is equal to 1. Once the condition is met, the target qubit is flipped by U.

`amplitude_amplification/amp_amplification.py`

_qubits_: 3
_gate depth_: 111
_noise_: false

A simple amplitude amplification example using Grover's algorithm, targeting the state $\ket{111}$.

`amplitude_amplification/amplitude_amplification.py`

_qubits_: 3
_gate depth_: 371
_noise_: false

A slightly more complex example of amplitude amplification using Grover's algorithm, targeting the $\ket{101} - \ket{011}$ state. This example may take some time to run.

`amplitude_amplification/amp_amplification_iteration.py`

_qubits_: 1
_gate depth_: 6318
_noise_: false

A demonstration of iterative amplitude amplification of the $\ket{1}$ state.  Requires `matplotlib` and outputs a plot of the results named `qaa.png`.

`amplitude_estimation/canonical_amplitude_estimation.py`

_qubits_: 1 + number of precision qubits (user defined)
_noise_: false

An implementation of the canonical amplitude estimation example described [here](https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html), implemented by constructing the required Grover operator.

`amplitude_estimation/canonical_amplitude_estimation_oracle.py`

_qubits_: 9
_noise_: false

An implementation of the canonical amplitude estimation example described [here](https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html), implemented by passing the desired oracle. This example may take some time to run.

`amplitude_estimation/methods_comparison.py`

_qubits_: 11
_noise_: false

Compares the results and runtimes of canonical and maximum likelihood amplitude estimation.

`amplitude_estimation/ML_amplitude_estimation.py`

_qubits_: 1
_noise_: false

Example demonstrating maximum likelihood amplitude estimation (MLQAE) as described [here](https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html).

`amplitude_estimation/ML_amplitude_estimation_parameter_testing.py`

_qubits_: 1
_noise_: false

As per `ML_amplitude_estimation.py`, but repeated for different numbers of runs and shots.  Requires `matplotlib` and outputs a plot of the results named `MLQAE_parameter_test.png`.

`amplitude_estimation/MLQAE_parameter_scaling.py`

_qubits_: 15
_noise_: false

As per `ML_amplitude_estimation_parameter_testing.py`, but repeated for different numbers of qubits.  Also offers an internal option to perform the same parameter scan for canonical amplitude estimation. This example requires matplotlib and produces a series of plots named `MLQAE_parameter_test_*_qubits.png`.  It may take some time to run.

`async_example_1.py`

_qubits_: 2
_gate depth_: 6
_noise_: false

An example of asynchronous execution of a single circuit.

`async_example_2.py`

_qubits_: 2
_gate depth_: 6
_noise_: false

An example of asynchronous execution of 100 circuits, using Qristal's thread pool. Note that this uses Python's co-operative multitasking to manage the threads, so it doesn't actually parallelise the execution of the circuits across the threads, as each thread needs to uniquely acquire the global interpreter lock (GIL) in order to execute the circuit. To see how to parallelise circuit execution, please refer to qft.py.

`benchmark/pyGSTi.py`

_qubits_: 2
_gate depth_: variable (depends on the set maximum length)
_noise_: true/false (depends on the options set in session)

A simple example demonstrating the creation of a gate set tomography experiment design for 2-qubit gates Rx(pi/2), Ry(pi/2), and CZ using pyGSTi. The corresponding circuit list is evaluated through qristal.core.benchmark and the gathered results are loaded back into pyGSTi and used to create a human-readable html report.

`benchmark/qst.py`

_qubits_: 2
_gate depth_: 2
_noise_: false

Python implementation of the C++ example `qst` demonstrating a quantum state tomography.

`benchmark/qst_fidelity.py`

_qubits_: 2
_gate depth_: 1
_noise_: false

Python implementation of the C++ example `qst_fidelity` demonstrating a quantum state tomography experiment, evaluating the quantum state fidelity.

`benchmark/qpt.py`

_qubits_: 1
_gate depth_: 1
_noise_: false

Python implementation of the C++ example `qpt` demonstrating a quantum process tomography.

`benchmark/qpt_fidelity_rotation_sweep.py`

_qubits_: 2
_gate depth_: 3
_noise_: false

Python implementation of the C++ example `qpt_fidelity_rotation_sweep` demonstrating a quantum process tomography experiment of a rotation sweep, evaluating the quantum process fidelity.

`benchmark/qpt_fidelity_CZ.py`

_qubits_: 2
_gate depth_: 3
_noise_: false

Python implementation of the C++ example `qpt_fidelity_CZ` demonstrating a quantum process tomography experiment of a single CZ gate application, evaluating the quantum process fidelity.

`circuit_optimization/gate_deferral.py`

An example demonstrating the application of the gate deferral technique.

`comparator.py`

_qubits_: 9 (comparator), 2 (comparator_as_oracle)
_gate depth_: 226 (comparator), 113 (comparator_as_oracle)
_noise_: false

Example comparing whether two bitstrings/registers are equivalent.

`compare_beam_oracle.py`

_qubits_: 11
_gate depth_: 238
_noise_: false

This calls a quantum decoder-specific module. Example to check whether a string ($S_A$) is equal to a string of ones $\ket{11 \ldots 1}$. This module is used in the quantum decoder kernel algorithm. The registers of flags, $F_A$ and $F_B$ are initialised as $\ket{1}$. Then they are turned off if the symbol is neither null or repetition flagged. Therefore the symbols in $S_{A}$ will be flagged in $F_{A/B}$ if and only if they are null or repeated or both. Once this flagging is done, the oracle flag $q_0$ returns 1 true if and only if 1. corresponding symbols are either both flagged or both unflagged, and 2. if both unflagged then the symbols must match.

`compare_gt.py`

_qubits_: 12
_gate depth_: 244
_noise_: false

Example comparing whether the value of bitstring/register $A$ is greater than the value of bitstring/register $B$.

`controlled_addition.py`

_qubits_: 7
_gate depth_: 343
_noise_: false

Example demonstrating adding two numbers condition on the the flag qubit being on or off. If the flag qubit is on (off), the the addition is (is not) performed.

`controlled_subtraction.py`

_qubits_: 10
_noise_: false

Example demonstrating subtracting two numbers condition on the the flag qubit being on or off. If the flag qubit is on (off), the the subtraction is (is not) performed. This example takes a very long time to complete.

`controlled_swap.py`

_qubits_: 10
_gate depth_: 685
_noise_: false

Example performing a swap operation conditioned on the alphabet is "b". If the given alphabet is "b", then a swap operation is performed to move the given alphabet to the end of the string. This example starts with a string "babba" and returns "aabbb".

`cpfd.py`

_qubits_: 13
_noise_: false

Stands for "controlled proper fraction division". Given two registers A and B, this module performs the division between the value of A and the value of B. The example performs every 3-qubit proper fraction division with 3 precision bits. This example takes a very long time to complete.

`efficient_encoding.py`

_qubits_: 14
_gate depth_: 3392 (with ancilla), 10245 (without ancilla)
_noise_: false

This calls the EfficientEncoding module to perform a simple encoding using Gray code flips. This example takes the input $\ket{i}\ket{i}$ with three qubits in each register and returns all possible configurations of the two three-qubit registers $\ket{000}\ket{000} + \ldots + \ket{111}\ket{111}$.

`equality_checker.py`

_qubits_: 9 (with ancilla), 7 (without ancilla)
_gate depth_: 106 (with ancilla), 92 (without ancilla)
_noise_: false

Example checking for equality between the values of two registers. The flag qubit returns 1 (0) if the values are equal (unequal).

`exponential_search.py`

Example demonstrating the use of the exponential search algorithm to find the largest number, or "best score", in a dataset. If sufficient search iterations are done, the algorithm should return the best score of 3.

`generalised_mcx.py`

_qubits_: 3
_gate depth_: 46
_noise_: false

Example performing a generalized MCX gate on all possible 3-qubit bit strings ($\ket{000}, \ldots, \ket{111}$) with all combinations of control qubit conditions ((on,on), $\ldots$ , (off,off)).

`mcx.py`

_qubits_: 3
_gate depth_: 46
_noise_: false

Example of performing an MCX operation on all possible 3-qubit strings ($\ket{000}, \ldots, \ket{111}$). The target qubit is flipped if the $i$th bitstring/control qubit is equal to 1.

`multiplication.py`

_qubits_: 9
_gate depth_: 686
_noise_: false

Example of a multiplication of values encoded in registers $\ket{a}$ and $\ket{b}$.

`noise_mitigation.py`

_qubits_: 2
_gate depth_: 6
_noise_: true

This example shows the effects of various noise mitigation strategies available in Qristal, using a noisy 2-qubit Bell state.

`noise_model.py`

_qubits_: 2
_gate depth_: 6
_noise_: true

A version of `quickstart.py` with noise. There are two options of noise models: `default` or `qdk1` (the latter requires the Qristal Emulator. Details about the noise model are available [here](https://qristal.readthedocs.io/en/latest/rst/noise_models.html)).

`noise_model_custom_kraus.py`

_qubits_: 2
_gate depth_: 6
_noise_: true

A version of `noise_model.py` demonstrating the use of Kraus matrices to generate custom noise channels. Custom noise channels are then used to generate a custom noise model.

`noise_model_custom_parameterized.py`

_qubits_: 2
_gate depth_: 6
_noise_: true

A version of `noise_model.py` demonstrating the use of custom noise model parameters ($t_1$, $t_2$, gate errors, etc) to generate a custom noise model. This noise model consists of common noise channels: amplitude damping, phase damping and depolarization.

`noise_model_custom_channel.py`

_qubits_: 2
_gate depth_: 6
_noise_: true

A version of `noise_model.py` demonstrating the use of noise channels to generate a user-defined noise model. The noise model consists of common noise channels: amplitude damping, phase damping and depolarization.

`noise_model_custom_channel_qb_gateset.py`

_qubits_: 2
_gate depth_: 6
_noise_: true

A version of `noise_model_custom_channel.py` demonstrating the use of a user-defined noise model in a different basis gate set.

`parametrization_demo.py`

_qubits_: 2
_gate depth_: 4
_noise_: false

An extremely simple demonstration of how to construct and execute parametrized circuits using Qristal.

`pfd.py`

_qubits_: 12
_gate depth_: 4690
_noise_: false

Example of proper fraction division. This example performs every 3-qubit proper fraction division with 3 precision bits.

`qft.py`

_qubits_: 5
_noise_: false

Example of a quantum Fourier transform. This example runs 4 different accelerator backends (aer `density_matrix`, aer `matrix_product_state`, `cudaq:dm` and `tnqvm`) in parallel using Python's multithreading library.

`qpe.py`

_qubits_: 5
_gate depth_: 16
_noise_: false

Example of a quantum phase estimation, $\hat{O}\ket{\psi} = e^{i\phi}\ket{\psi}$, where the goal is to estimate the phase $\phi$. The oracle operator $\hat{O}$ in this case is a general $U1$ rotation, i.e. $U1(\theta) \ket{1} = e^{i\theta}\ket{1}$. Test value: $-5\pi/8$.

`qb_mpdo_noisy.py`

_qubits_: 3
_gate depth_: 2
_noise_: false

Example of how to use the Qristal Emulator's matrix product density operator backend to execute a 3-qubit circuit consisting of $X$ and $CX$ gates. Details of this backend are available [here](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-matrix-product-density-operator-qb-mpdo-and-cudaq-qb-mpdo) .

`qb_mps_noisy.py`

_qubits_: 3
_gate depth_: 2
_noise_: false

Example of how to use the Qristal Emulator's matrix product state backend to execute a 3-qubit circuit consisting of $X$ and $CX$ gates. Details of this backend are available [here](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-matrix-product-state-mps-qb-mps-and-cudaq-qb-mps) .

`qb_purification_noisy.py`

_qubits_: 3
_gate depth_: 2
_noise_: false

Example of how to use the Qristal Emulator's purification backend to execute a 3-qubit circuit consisting of $X$ and $CX$ gates. Details of this backend are available [here](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-purification-qb-purification-and-cudaq-qb-purification) .

`qb_statevector_noisy.py`

_qubits_: 3
_gate depth_: 46
_noise_: true

An example of how to use the QB statevector backend from the Qristal emulator, with noise. A generalized mcx gate operates on a target qubit in the state $\ket{1}$ conditioned on 2 control qubits in the state $\ket{11}$. This flips the target qubit to $\ket{0}$. Details about the noise model used are available [here](https://qristal.readthedocs.io/en/latest/rst/noise_models.html) .

`set_circuit.py`

_qubits_: 3
_gate depth_: 46
_noise_: false

Example of performing an MCX operation on a target qubit initially in the $\ket{0}$ state conditional on 2 control qubits in the $\ket{11}$ state. The expected result is that the target qubit is flipped to the $\ket{1}$ state.

`subtraction.py`

_qubits_: 10
_gate depth_: 13834
_noise_: false

Example demonstrating subtraction between two registers, each containing 5 qubits each. Subtraction of all possible values of the 5-qubit configuration is performed.

`superposition_adder.py`

_qubits_: 33
_noise_: false

Example using amplitude estimation to find the mean amplitude of a superposition state. Takes a really long time to complete execution.

`topology.py`

A simple example that prints out circuit topology and connectedness.

`vqee_example_1.py`

_qubits_: 4
_gate depth_: 1 for H2_explicit and 7 for H1_HEA
_noise_: false

Demonstrates the access and run of predefined examples in Qristal's built-in VQE routines.

`vqee_example_2.py`

_qubits_: 4
_gate depth_: 8
_noise_: false

Demonstrates manual problem setup and use and performance of different backends for Qristal's built-in VQE routines. This example may take some time to run.

`vqee_example_3.py`

_qubits_: 4
_gate depth_: 83
_noise_: false

Demonstrates how to inject python code into our c++ libs. Shows inclusion of external python chemistry package pyscf into Qristal's built-in VQE routines.

`vqee_example_4.py`

_qubits_: 4
_gate depth_: 1
_noise_: false

Demonstrates selection of a different classical optimization algorithm (Nelder-Mead), along with extra options to constrain the parameters and terminate the optimization.

`vqee_example_5.py`

_qubits_: 4
_gate depth_: 1
_noise_: false

Similar to `vqee_example_4` except the classical algorithm used here is ADAM and L-BFGS.

`vqee_example_6.py`

_qubits_: 4
_gate depth_: 1
_noise_: false

Similar to `vqee_example_4` except the classical algorithm used here is CMA-ES.

`xing_the_xasm.py`

_qubits_: 2
_gate depth_: 3
_noise_: N/A

An example of how to convert a circuit from OpenQASM2 to XASM.

`simple_placement.py`

A simple example demonstrating circuit placement based on backend topology.

`noise_aware_placement.py`

A simple example demonstrating noise-aware circuit placement by setting up a toy hardware configuration.

`noise_aware_placement_noise_model.py`

A simple example demonstrating noise-aware circuit placement by setting up a toy noise model.

`noise_aware_placement_aws_rigetti.py`

An example demonstrating integrated noise-aware placement during circuit execution on a hardware backend (e.g., Rigetti devices on AWS).
Valid AWS credentials are required to run the example.

`aws_braket_qft.py`

_qubits_: 4
_noise_: true

A simple example demonstrating both synchronous and asynchronous circuit execution on AWS Braket. To run on AWS Braket:
* Set up AWS account (e.g., using CLI) and enable AWS Braket;
* Use an AWS Region that supports AWS Braket (e.g., us-east-1);
* Create an S3 Bucket with prefix `amazon-braket-*` and create a folder inside the S3 bucket to store results.

`circuit_append.py`
A simple example demonstrating the use of append() to combine two circuits. Useful for verifying that circuit appending executes without error and produces correct OpenQASM output.

## C++ ##

`demo1`

_qubits_: 2
_gate depth_: 3
_noise_: false

A C++ implementation of `demo1.py`.

`mpi_demo`

_qubits_: 20
_shot count_: 100000
_noise_: true

An example of how to use MPI parallelisation in Qristal to achieve a significant speed up on a single system. Requires Qristal be built with `-DWITH_MPI=ON`.

`mpi_multi_qpu_demo`

_qubits_: 2
_gate depth_: 3
_noise_: false

Extends `mpi_demo` by showing how to configure Qristal to use a different vQPU for each MPI process. The configuration can be used as a template for assigning each MPI process different instances of the same type of hardware backend (e.g. QB's QPU or AWS Rigetti). Requires Qristal be built with `-DWITH_MPI=ON`.

`bell_with_SPAM`

_qubits_: 2
_gate depth_: 2
_noise_: true

A C++ implementation of `bell_with_SPAM.py`.

`h1`

_qubits_: 2
_gate depth_: 2
_noise_: false

This example allows you to quickly switch circuit execution between a hardware QPU and a simulator.  See cpp/h1/README.md for more information.

`qft`

_qubits_: 5
_noise_: false

Example of a quantum Fourier transform using 4 different accelerator backends (aer `density_matrix`, aer `matrix_product_state`, `cudaq:dm` and `tnqvm`) running in parallel via a thread pool.

`qb_mpdo_noisy`

_qubits_: 2
_gate depth_: 2
_noise_: false

Example of how to use the Qristal Emulator's matrix product density operator backend to create a Bell state. Details of this backend are available [here](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-matrix-product-density-operator-qb-mpdo-and-cudaq-qb-mpdo) .

`qb_mps_noisy`

_qubits_: 2
_gate depth_: 2
_noise_: false

Example of how to use the Qristal Emulator's matrix product state backend to create a Bell state. Details of this backend are available [here](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-matrix-product-state-mps-qb-mps-and-cudaq-qb-mps) .

`qb_purification_noisy`

_qubits_: 2
_gate depth_: 2
_noise_: false

Example of how to use the Qristal Emulator's purification backend to create a Bell state. Details of this backend are available [here](https://qristal.readthedocs.io/en/latest/rst/backends.html#quantum-brilliance-purification-qb-purification-and-cudaq-qb-purification) .

`noise_model`

_qubits_: 2
_gate depth_: 6
_noise_: true

A C++ implementation of `noise_model.py`.

`noise_model_custom_kraus`

_qubits_: 2
_gate depth_: 6
_noise_: true

A C++ implementation of `noise_model_custom_kraus.py`.

`noise_model_custom_parameterized`

_qubits_: 2
_gate depth_: 6
_noise_: true

A C++ implementation of `noise_model_custom_parameterized.py`.

`noise_model_custom_channel`

_qubits_: 2
_gate depth_: 6
_noise_: true

A C++ implementation of `noise_model_custom_channel.py`.

`noise_model_custom_channel_qb_gateset`

_qubits_: 2
_gate depth_: 6
_noise_: true

A C++ implementation of `noise_model_custom_channel_qb_gateset.py`.

`parametrization`

_qubits_: 1-2
_circuits_: 2
_gate depth_: 1
_noise_: false

Demonstrates the use of parametrized gates/circuits in Qristal, as well as how to execute parametrized circuits after providing runtime parameters, and calculating jacobians for these parameters.

`vqee`

_qubits_: 4
_gate depth_: 83
_noise_: false

Demonstrates the use of Qristal's built-in VQE routines. Can be built with MPI support and parallelization over Pauli terms. Requires a functional installation of pyscf to run.

`vqeeCalculator`

`vqeeCalculator` is a C++ compiled executable that enables command-line access to the functionality in `vqee`. Requires a functional installation of pyscf in order to run.

```{toctree}
vqeeCalculator.md
```

%Special note only for markdown readers: For in-depth documentation of the `vqeeCalculator`, see <a href="cpp/vqeeCalculator/README.md">examples/cpp/vqeeCalculator/README.md</a>.

`noise_aware_placement_simple`

A simple example demonstrates noise-aware circuit placement by setting up a toy hardware configuration.

A C++ implementation of `noise_aware_placement.py`.

`noise_aware_placement_aws`

An example demonstrates integrated noise-aware placement during circuit execution on hardware backend (e.g., Rigetti devices on AWS).
Valid AWS credentials are required to run the example.

A C++ implementation of `noise_aware_placement_aws_rigetti.py`.

`circuit_optimization/circuit_optimizer.py`

An example demonstrating the pattern-based circuit optimization pass.

`circuit_optimization/remove_redundant_gates.py`

An example demonstrating the redundant gate removal circuit optimization pass. This example requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`circuit_optimization/two_qubit_squash.py`

An example demonstrating the two-qubit gate synthesis (squash) circuit optimization pass. This example requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`circuit_optimization/simplify_initial_conditions.py`

An example demonstrating the contextual circuit optimization pass, which optimizes the circuit based on the knowledge of its initial state. This example requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`circuit_optimization/session_integration.py`

An example demonstrating setting up a pipeline of circuit optimization passes when using Qristal.

`circuit_optimization/sequence_pass.py`

An example demonstrating the use of a sequence of circuit optimisation passes. The different orderings of circuit passes in a sequence result in different optimised circuits

`circuit_optimization/peephole_optimization.py`
An example demonstrating the full peephole optimization circuit pass that removes redundancies and merges rotations. This example requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`circuit_optimization/decompose_swap.py`
An example demonstrating the decomposition of each SWAP gate into three CNOT gates. This example is using [DecomposeSWAPtoCX](https://docs.quantinuum.com/tket/api-docs/transform.html#pytket.transform.Transform.DecomposeSWAPtoCX) and requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`circuit_optimization/commute_through_multis.py`
An example demonstrating the application of commutation rules to move single-qubit gates past multi-qubit gates with which they commute. The transformation pushes single-qubit operations towards the front of the circuit. This example is using [CommuteThroughMultis](https://docs.quantinuum.com/tket/api-docs/transform.html#pytket.transform.Transform.CommuteThroughMultis) and requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`circuit_optimization/optimise_clifford.py`
An example demonstrating an optimisation pass that applies a number of rewrite rules for simplifying Clifford gate sequences. This example is using [OptimiseCliffords](https://docs.quantinuum.com/tket/api-docs/transform.html#pytket.transform.Transform.OptimiseCliffords) and requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`circuit_optimization/optimise_post_routing.py`
An example demonstrating the post-routing optimization pass that removes redundant gates and simplifies circuits after qubit routing. This example is using [OptimisePostRouting](https://docs.quantinuum.com/tket/api-docs/transform.html#pytket.transform.Transform.OptimisePostRouting) and requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`circuit_optimization/rebase_to_clifford.py`
An example demonstrating how the rebasing of a quantum circuit to Clifford gates, decomposing gates into sequences of Clifford operations. This example is using [RebaseToCliffordSingles](https://docs.quantinuum.com/tket/api-docs/transform.html#pytket.transform.Transform.RebaseToCliffordSingles) and requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`circuit_optimization/rebase_to_rzrx.py`
An example demonstrating the rebase of a circuit to use only Rz and Rx rotations. This example is using [RebaseToRzRx](https://docs.quantinuum.com/tket/api-docs/transform.html#pytket.transform.Transform.RebaseToRzRx) and requires TKET support to be enabled by setting -DWITH_TKET=ON at build time.

`cudaq_qft.py`

Running QFT circuit constructed by the Qristal circuit builder on a CUDA Quantum simulator backend. Requires CUDA Quantum support.

`qst`

_qubits_: 2
_gate depth_: 2
_noise_: false

This example shows the execution of a standard quantum state tomography workflow for a two qubit Bell state, evaluating and printing the quantum state density.

`qst_fidelity`

_qubits_: 2
_gate depth_: 1
_noise_: false

This example shows the execution of a standard quantum state tomography workflow wrapped around a SPAM benchmark. For each circuit, the quantum state fidelity metric is evaluated and printed.

`qpt`

_qubits_: 1
_gate depth_: 1
_noise_: false

This example shows the execution of a standard quantum process tomography workflow for a single Rx(pi/2) gate, evaluating and printing the quantum process matrix.

`qpt_fidelity_rotation_sweep`

_qubits_: 2
_gate depth_: 3
_noise_: false

This example shows the execution of a standard quantum process tomography workflow wrapped around a rotation sweep benchmark rotating qubit 0 from -pi to +pi applying Rx gates in 5 steps. For each circuit, the quantum process fidelity metric is evaluated and printed.

`qpt_fidelity_CZ`

_qubits_: 2
_gate depth_: 3
_noise_: false

This example shows the execution of a standard quantum process tomography workflow wrapped around a simple circuit execution applying a CZ gate. The quantum process fidelity as well as the average gate fidelity are evaluated and printed.


## C++ with CUDA Quantum ##

A number of additional examples are only installed when Qristal is built with CUDA Quantum support.  None of these examples has noise enabled.

`cudaq_qft`

Running QFT circuit constructed by the Qristal circuit builder on a CUDA Quantum simulator backend.

`benchmark1_cudaq`

A 20 qubit GHZ state designed for benchmarking performance of CUDA Quantum and QASM. This is the CUDAQ version.

`benchmark1_qasm`

A 20 qubit GHZ state designed for benchmarking performance of CUDA Quantum and QASM. This is the QASM version.

`cudaq_vqe_cobyla`

Determination of the deuteron's ground state energy using VQE with the Cobyla optimiser.

`cudaq_vqe_hydrogens`

Determination of the ground state of a chain of four hydrogen atoms, using VQE with the L-BFGS optimiser.

`cudaq_vqe_lbfgs`

Determination of the deuteron's ground state energy using VQE with the L-BFGS optimiser.

## C++ with -DWITH_PROFILING=ON ##

`runtime_profiling`

_qubits_: 5
_gate depth_: 1
_noise_: false/true

Example `RotationSweep` workflow wrapped into `RuntimeAnalyzer` to profile CPU, RAM, and GPU resources in intervals of 20 ms.

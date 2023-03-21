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

##  Notes on specific examples ##

### Python ###

`demo1.py`

_qubits_: 2  
_gate depth_: 3  
_noise_: false  

A simple example showing a 2 qubit circuit consisting of just a Hadamard and a regular 1-qubit NOT gate.

`quickstart.py`

_qubits_: 2  
_gate depth_: 6  
_noise_: false  

The 2-qubit Bell state quickstart example given in the README of the main project.

`aer_simulator/aer_mps_simple.py`  

_qubits_: 2  
_gate depth_: 6  
_noise_: true  

Example usage of the aer matrix-product-state-based simulator for computing a 2-qubit Bell state circuit.

`aer_simulator/aer_mps_dj_large.py`  

_qubits_: 41  
_gate depth_: 204  
_noise_: true  

Example usage of the aer matrix-product-state-based simulator for computing a larger Deutsch-Jozsa circuit.  This example may take some time to run.

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

An example of asynchronous circuit execution, using 32 workers to perform 200 circuit executions.

`async_example_2.py`

_qubits_: 2  
_gate depth_: 6  
_noise_: false  

An example of asynchronous circuit execution, using 32 workers to perform 3200 circuit executions.

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

A version of `quickstart.py` with noise.

`pfd.py`

_qubits_: 12  
_gate depth_: 4690  
_noise_: false  

Example of proper fraction division. This example performs every 3-qubit proper fraction division with 3 precision bits.

`qaoa_example.py`

_qubits_: 5  
_noise_: false  

Demonstrates the use of Qristal's built-in implementation of the QAOA algorithm. This example may take some time to run.

`qaoa_API_demo.py`

_qubits_: 9  
_noise_: false  

Demonstrates the use of the QAOA and QUBO APIs at a QAP problem. This example may take some time to run.

`qft.py`

_qubits_: 5  
_noise_: false  

Example of a quantum Fourier transform.

`qml/RDBMS_query_optimization.py`

_qubits_: 4
_variational_params_: 40
_noise_: false

Example showing QML wrapped in pytorch for join order optimization.

`qpe.py`

_qubits_: 5  
_gate depth_: 16  
_noise_: false  

Example of a quantum phase estimation, $\hat{O}\ket{\psi} = e^{i\phi}\ket{\psi}$, where the goal is to estimate the phase $\phi$. The oracle operator $\hat{O}$ in this case is a general $U1$ rotation, i.e. $U1(\theta) \ket{1} = e^{i\theta}\ket{1}$. Test value: $-5\pi/8$.

`qsim_noisy.py`

_qubits_: 3  
_gate depth_: 46  
_noise_: true  

A generalized mcx gate operates on a target qubit in the state $\ket{1}$ conditioned on 2 control qubits in the state $\ket{11}$. This flips the target qubit to $\ket{0}$.  The basic version of the example does not include noise. If you have the Qristal Emulator installed, two lines in the example file can be uncommented to convert it into an example of a simulation in a noisy environment, using noise models provided by the emulator. Details about the noise model are available [here](https://qristal.readthedocs.io/en/latest/rst/noise_models.html) .

`remote_workstation_example.py`

_qubits_: 18  
_noise_: false  

An example of a random circuit, offloaded to the dual-GPU QB Lambda server in Canberra.

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
_gate depth_: 83  
_noise_: false  

Demonstrates the use of Qristal's built-in VQE routines. This example may take some time to run.

`vqee_example_2.py`

_qubits_: 4  
_gate depth_: 83  
_noise_: false  

Demonstrates the use and performance of different backends for Qristal's built-in VQE routines. This example may take some time to run.


### C++ ###

`demo1`

_qubits_: 2  
_gate depth_: 3  
_noise_: false  

A C++ implementation of `demo1.py`.

`qaoa`

_qubits_: 3
_qaoa_steps_: 2
_noise_: false

Demonstrates the use of Qristal's built-in implementation of the QAOA simple algorithm. This example may take some time to run.

`qbsdkcli`

A command-line interface to Qristal.  A simple invocation after compiling the CLI is:
```
./qbsdkcli -q2 --random=2
```
which will run a random circuit on 2 qubits, with gate depth of 2. Further details can be found <a href="cli.html">here</a> if viewing the html docs (or in <a href="cpp/qbsdkcli/README.md">examples/cpp/qbsdkcli/README.md</a> if viewing this document as markdown).

`noise_model`

_qubits_: 2
_gate depth_: 6
_noise_: true

A C++ implementation of `noise_model.py`.

`vqee`

_qubits_: 4  
_gate depth_: 83  
_noise_: false  

Demonstrates the use of Qristal's built-in VQE routines. Can be built with MPI support and parallelization over Pauli terms.


### C++ with CUDA Quantum ###

A number of additional examples are only installed when Qristal is built with CUDA Quantum support.  None of these examples has noise enabled.

`benchmark1_cudaq`

A 20 qubit GHZ state designed for benchmarking performance of CUDA Quantum and QASM. This is the CUDAQ version.

`benchmark1_qasm`

A 20 qubit GHZ state designed for benchmarking performance of CUDA Quantum and QASM. This is the QASM version.

`cudaq_vqe_cobyla`

Determination of the deuteron's ground state energy using VQE with the Cobyla optimiser.

`cudaq_vqe_hydrogens`

Determination of the ground state of a chain of four hydrogen atoms, using VQE with the L-BFGS optimiser.

`cudaq_vqe_cobyla`

Determination of the deuteron's ground state energy using VQE with the L-BFGS optimiser.


### Further notes for beta testers ###

This folder is to some extent a work in progress.  The examples listed above are the ones that have been fully tested and made consistent with the public version of the code.  The others listed below do not presently run or still need additional testing. This is a known issue, and thus not worth reporting as a new bug.


`aws_braket`

These need work on the aws_s3 argument.  You should be able to get the first two to run by just setting `s.aws_s3` to the name of an s3 bucket that you personally have access to (you need to have run `aws configure`, set up the bucket, etc).

  - `aws_noise_demo.py`  
  An example of circuit offload to AWS Braket, and an explicit test of the decay of coherence over time, as implemented in the default noise model.

  - `qft.py`  
  A Fourier transform performed on an AWS simulator.

  - `verbatim.py`  
  A circuit run on Rigetti hardware using the AWS `verbatim` option.

`q_chemistry`

  - `vqe_calculator.py`
  Implementation in Python of a generic VQE calculator for quantum chemistry. Requires `openfermionpyscf` (can be installed with pip).  Parses successfully with Python, but a tested example that uses the class has yet to be developed.

  - `external_potential.py`
  An add-in class for `vqe_calculator.py` allowing for the specification of an external potential arising from a point charge.

`lambda_accelerator`

Further examples using a remote accelerator.

`qap`

Some additional QAOA examples using recursive QAOA routines.

`vqe`

  - `h2_vqe_aswap.py`  
  Example using the variational quantum eigensolver and ASWAP ansatz to obtain the ground state energy of a H2 molecule.

  - `h2_vqe_uccsd.py`  
  Example using the variational quantum eigensolver and UCCSD ansatz to obtain the ground state energy of a H2 molecule.

  - `h4_vqe_uccsd.py`  
  Example using the variational quantum eigensolver and UCCSD ansatz to obtain the ground state energy of H4.

  - `h4_vqe_uccsd_aer`  
  Example using the variational quantum eigensolver, a UCCSD ansatz and the AER simulator to obtain the ground state energy of H4.

  - `h6_vqe_uccsd_aer`  
  Example using the variational quantum eigensolver, a UCCSD ansatz and the AER simulator to obtain the ground state energy of H6.

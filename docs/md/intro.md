```{attention} Placeholder only 

Copied from legacy qbOS documentation, need revision...
```

Quantum Brilliance SDK is our software backbone that provides users with a variety of UI functionalities (see our Jupyter UI, standalone Python API, and CLI for HPC usage), necessary middleware, optimised compilation routines, enables hybrid programming and quantum simulations, parallelisation and task distribution, and communication instructions supporting a number of classical and quantum backends including QB's accelerators. The operating system is specifically designed to provide a quantum computing framework as an accelerator for classical computations, and for integration of a range of quantum hardware with HPC infrastructure. qbOS is mainly built upon the leading, open-source, modular quantum software framework of [XACC](https://xacc.readthedocs.io/en/latest/) (an Eclipse project from ORNL developers) and a diverse range of other open-source software tools. The third-party developers are encouraged to provide additional modules for qbOS following our license and commercial agreement terms, which allows quantum software and application teams to deliver their own products on top of QB’s platform.

**XACC**

[eclipse/xacc](https://github.com/eclipse/xacc)

qbOS presents a *fast* and *dynamic* (see below) quantum computing platform. It is targeted at a wide range of users from quantum scientists to enthusiasts to examine running quantum circuits, algorithms, and to collect data using their existing classical computing resources. Our simulation backends can emulate a gate-based ideal (noiseless) stochastic machine or QB's upcoming diamond-chip-based quantum hardware with a realistic noise model hard-coded.

qbOS is *fast* in the sense that, at its core, it exploits highly scalable *tensor network* simulator backends (allowing simulations and benchmarking with 100s of qubits), compiled C++ executables, supports GPU, MKL, multi-threaded OpenMP and MPI calculations, and provides other state-vector simulation options useful for low qubit number circuits. qbOS is a *dynamic* software package in the sense that it is in part delivered as a set of pre-compiled executables and can accept quantum circuits inputs in several mainstream Quantum Instruction Set formats (extensive support for the assembly language of [OpenQASM v2.0](https://github.com/Qiskit/openqasm/tree/OpenQASM2.x), and partial support for XASM and QUIL) and provides a range of prebuilt Python functions for users to run very high-level QC calculations.

**OpenQASM**

[Qiskit/openqasm](https://github.com/Qiskit/openqasm)

# User Testimonials

[Quantum South](https://quantum-south.com/about-as/) team, April 2021:

> *"[qbOS] works faster compared with all other platforms"*
> 

> "The GUI is great".
> 

> "Integration of OpenQASM, Easy representation of the Hamiltonian (in equation form) for running VQE, [and] Ability to test algorithms with noise without having to model the noise [are my favourite features]".
> 

> "Ease of construction of specific quantum circuits [is my favourite feature]".
> 

> "The details of the outputs [and] being able to create the circuits in IBM and bring them here [are my favourite features]".
> 

> "The IDE is very familiar and easy to use".
> 

> "The course was great and didn't see any bugs".
> 

Samarth Sandeep, CEO, IFF Bio, 6th May 2021:

> "*Wish my first experiences in quantum were this straightforward"*
> 

Angus Mingare, Honours Student, Australian National University, 22nd March 2021:

> "*I'm impressed by how easy to use it is"*
> 

## qbOS Development Team

qbOS is our proprietary quantum software brought to you through the collective efforts of different Quantum Brilliance teams, feedback and contributions from external collaborators and end-users. Within Quantum Brilliance, our *Software & Applications* team is responsible for the development and support of qbOS. You may contact the following senior members from our Software and Applications team for general inquiries and questions regarding qbOS:


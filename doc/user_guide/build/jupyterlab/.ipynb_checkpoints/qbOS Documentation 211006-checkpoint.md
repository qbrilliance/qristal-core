# qbOS Documentation

![Quantum_Brilliance_dark_blue_Logo_RGB.png](qbOS%20Documentation%20211006/Quantum_Brilliance_dark_blue_Logo_RGB.png)

# **qbOS Built 0.0721.002**

*Q*uantum *B*rilliance *O*perating *S*ystem supporting a complete set of quantum computational tasks

[Sep 2021 Release Notes](qbOS%20Documentation%20211006/Sep%202021%20Release%20Notes%2057042bb79eef463d9ebab68eddcad6e5.md)

# 1. About qbOS

*Q*uantum *B*rilliance *O*perating *S*ystem (qbOS) is Quantum Brilliance’s software backbone that provides users with a variety of UI functionalities (see our Jupyter GUI and standalone Python API), necessary middleware, optimised compilation routines, enables hybrid programming and quantum simulations, parallelisation and task distribution, and communication instructions supporting a number of classical and quantum backends including QB's accelerators. The operating system is specifically designed to provide a *q*uantum *c*omputing (QC) framework as an accelerator for classical computations, and for integration of a range of quantum hardware with HPC infrastructure. qbOS is mainly built upon the leading, open-source, modular quantum software framework of [XACC](https://xacc.readthedocs.io/en/latest/) (an Eclipse project from ORNL developers) and a diverse range of other open-source software modules. The third-party developers are encouraged to provide additional modules for qbOS following our licence and commercial agreement terms, which allows quantum software and application teams to deliver their own products on top of QB’s platform.

**XACC**

[eclipse/xacc](https://github.com/eclipse/xacc)

qbOS presents a *fast* and *dynamic* (see below) quantum computing platform. It is targeted at a wide range of users from quantum scientists to enthusiasts to examine running quantum circuits, algorithms, and to collect data using their existing classical computing resources. Our simulation backends can emulate a gate-based ideal (noiseless) stochastic machine or QB's upcoming diamond-chip-based quantum hardware with a realistic noise model hard-coded.

qbOS is *fast* in the sense that, at its core, it exploits highly scalable *tensor network* simulator backends (allowing simulations and benchmarking with 100s of qubits), compiled C++ executables, supports GPU, MKL, multi-threaded OpenMP and MPI calculations, and provides other state-vector simulation options useful for low qubit number circuits. qbOS is a *dynamic* software package in the sense that it is in part delivered as a set of pre-compiled executables and can accept quantum circuits inputs in several mainstream Quantum Instruction Set formats (extensive support for the assembly language of [OpenQASM v2.0](https://github.com/Qiskit/openqasm/tree/OpenQASM2.x), and partial support for XASM and QUIL) and provides a range of prebuilt Python functions for users to run very high-level QC calculations.

**OpenQASM**

[Qiskit/openqasm](https://github.com/Qiskit/openqasm)

## User Testimonials

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

Angus Mingare, PhD Student, Australian National University, 22nd March 2021:

> "*I'm impressed by how easy to use it is"*
> 

## qbOS Development Team

qbOS is our proprietary quantum software brought to you through the collective efforts of different Quantum Brilliance teams, feedback and contributions from external collaborators and end-users. Within Quantum Brilliance, our *Software & Applications* team is responsible for the development and support of qbOS. You may contact the following senior members from our Software and Applications team for general inquiries and questions regarding qbOS:

![NarimanSaadatmand.jpg](qbOS%20Documentation%20211006/NarimanSaadatmand.jpg)

**Dr Nariman Saadatmand**
qbOS Product Owner
Software & Applications Team Lead

nariman.s@quantum-brilliance.com 

![Simon_Yin.jpg](qbOS%20Documentation%20211006/Simon_Yin.jpg)

**Simon Yin**
Senior Software Engineer

simon.y@quantum-brilliance.com 

![Michael_Walker.png](qbOS%20Documentation%20211006/Michael_Walker.png)

**Dr Michael Walker**
Quantum Tools Developer

mlwalker@quantum-brilliance.com

## Get Help: Questions, Bug Reporting, and Issue Tracking

For general inquiries and questions regarding qbOS, please refer to the [qbOS Development Team]() page.

We currently support the following user groups, with specialised issue tracking boards designed, to receive feedback, requests or contributions for new features, and bug reports.

### Pawsey's Quantum Pioneers

Please submit any feedback, requests or contributions for new features, and bug reports through **Pawsey's Help Desk email**: [help@pawsey.org.au](mailto:help@pawsey.org.au). Note you must please include “Quantum Pioneers” in the email subject. Pawsey and Quantum Brilliance have developed a support process that will be used for each of such tickets. Problems and questions related to access to Pawsey systems will be addressed by Pawsey staff directly. Problems and questions related to Quantum Brilliance Emulators and quantum hardware will be triaged to Quantum Brilliance.

### Internal Developers and Research Collaborators

Please submit any feedback, requests or contributions for new features, and bug reports through *qbOS Issue Log* Kanban board linked below.

 **qbOS Issue Log**

[Atlassian](https://qbau.atlassian.net/secure/RapidBoard.jspa?rapidView=13)

# 1. Quickstart

Using a small experimental design, let's run some circuits of the [Quantum Fourier Transform (QFT)](qbOS%20Documentation%20211006/Quantum%20Fourier%20Transform%20(QFT)%20086133df66464393834202921b561874.md).  We perform a 2-qubit QFT, and a 4-qubit QFT.  In both cases, we will perform both a noiseless and a noisy simulation.  

The concept of a **QPU Kernel** appears without details for now — this concept can be reviewed [here](qbOS%20Documentation%20211006/Circuit%20composition%20on%20GUI%20tools%20and%20conversion%20to%204f46ef713a5743e192aa79ec6f804d5d.md).

[QFT Experiment Fixed Global Options](qbOS%20Documentation%20211006/QFT%20Experiment%20Fixed%20Global%20Options%20de0784ece18847ebb02a5475c69aca97.csv)

[QFT Experiment Design](qbOS%20Documentation%20211006/QFT%20Experiment%20Design%2097159967bced45808d9bbef5fc861dea.csv)

- **Step 1 of 3**: Start your qbOS environment
    
    Access to qbOS requires a web-browser, and a virtual machine (VM) host provided via any one of the following options:
    
    [[Pawsey Nimbus Cloud]()](qbOS%20Documentation%20211006/Pawsey%20Nimbus%20Cloud%2039cb71b3471045b388e4593d7ae2d240.md)
    
    [[AWS Spot-Market instances]()](qbOS%20Documentation%20211006/AWS%20Spot-Market%20instances%20695d2d72ff0b4916aec906b3ac43b74a.md)
    
    Once you have completed the login to your VM host, use your Web browser to open: [http://localhost:8889](http://localhost:8889) 
    
    Then login to the JupyterLab session - the instructions for getting the session password are printed once you have completed the login process to your VM host:
    
    ![Untitled](qbOS%20Documentation%20211006/Untitled.png)
    
- **Step 2 of 3**: Python code that calls qbOS and executes all (four) corner conditions of the experiment
    
    ```python
    import qbos
    tqb = qbos.core()  # This object has access to the core API methods for circuit simulation
    tqb.qb12()         # Set up some sensible defaults
    ```
    
    > Setup conditions that apply to all experiment corners:
    > 
    
    ```python
    tqb.xasm = True    # Use XASM circuit format to access XACC's qft()
    n_shots = 1024     
    tqb.sn = n_shots   # Explicitly use 1024 shots
    tqb.acc = "aer"    # Use the aer simulator
    ```
    
    > Setup rows of the experiment table:
    > 
    
    ```python
    tqb.qn.clear()
    tqb.qn.append(qbos.N([2]))  # 2-qubits for the top row
    tqb.qn.append(qbos.N([4]))  # 4-qubits for the bottom row
    
    qpu_kernel_qft_2 = '''
    __qpu__ void QBCIRCUIT(qreg q) 
    {
    qft(q, {{"nq",2}});
    Measure(q[1]);
    Measure(q[0]);
    }
    '''
    
    qpu_kernel_qft_4 = '''
    __qpu__ void QBCIRCUIT(qreg q) 
    {
    qft(q, {{"nq",4}});
    Measure(q[3]);
    Measure(q[2]);
    Measure(q[1]);
    Measure(q[0]);
    }
    '''
    
    tqb.instring.clear()
    tqb.instring.append(qbos.String([qpu_kernel_qft_2]))   # QPU Kernel for the top row
    tqb.instring.append(qbos.String([qpu_kernel_qft_4]))   # QPU Kernel for the bottom row
    ```
    
    > Setup simulation conditions (columns of the experiment table):
    > 
    
    ```python
    tqb.noise.clear()
    tqb.noise.append(qbos.Bool([False, True]))  # noise is False (disabled) for the left (index 0) column, True (enabled) for the right column (index 1)
    ```
    
    > Run the simulation in all experiment corners
    > 
    
    ```python
    tqb.run()    
    ```
    
- **Step 3 of 3**: Inspect the results (shot counts)
    
    After the code from Step 2 has finished executing, you can inspect the settings or results using the Python codes in the table entries below:
    
    [QFT Experiment - Python code for viewing fixed global options](qbOS%20Documentation%20211006/QFT%20Experiment%20-%20Python%20code%20for%20viewing%20fixed%20glo%20d4800077ec6f41edbee7a34819f21dcc.csv)
    
    [QFT Experiment - Python code for viewing results](qbOS%20Documentation%20211006/QFT%20Experiment%20-%20Python%20code%20for%20viewing%20results%20ba1d8065630f480fbb0e2dcf8d999fb4.csv)
    

# 2. Quantum Gates

## 2.1. Introduction

As with the bits of classical computing, qubits must be manipulated and read to be of any use. Here we given brief descriptions, with examples of some of the more important quantum gates.

## 2.2 Quantum measurement with and without noise

Arguably the most important operation in a quantum circuit, the quantum measurement is typically the last operation, although not every qubit is necessarily measured. There are several important differences concerning measurement between quantum and classical circuits:

- Measuring the state of a qubit ends its roll in the calculation, as well as that of any qubit that was entangled with it. This is in contrast to classical bits whose values can be read and used elsewhere whilst allowing the calculation to continue. "Entangled" can be reasonably described as "correlated" for our purposes here and we shall describe it properly below.
- The state of a qubit cannot be duplicated, unlike a classical bit for which a branching gate exists. There is no such gate in quantum computing, a fact expressed by the "no-cloning" theorem. It is possible to copy a qubit's state onto another qubit of known, typically zero value, but the two qubits remain entangled and cannot be regarded as independent.
- A qubit is generally in a superposition of |0> and |1> states but only of those values is returned upon measurement. Which of the two is a genuinely random outcome and identical preparations may return different results. Regardless of how they may be separated in time or space, measurements of qubits will always correlate correctly if they are interdependent in any way.

In order to manage and study these random outcomes qbOS has the capacity to run a circuit multiple times, called shots. The returned qubit values are accompanied by the number of shots for which they were found by the final measurement.

There is another source of randomness in a quantum circuit, namely the thermal, magnetic and other noise acting on the qubits. qbOS can include this noise in its simulations if desired. The parameters for this noise are hard-coded to match the characteristics of the QB's hardware. The following coded example sets up three qubits, setting one of them to |1 >, and measures them for 1024 shots. 

- Python code
    
    ```python
    import qbos as q
    tqb = q.core()
    tqb.qb12()
    
    tqb.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[3];
    
    // ----- q[2] -----;----- q[1] -----;----- q[0] -----;
                                             x q[0]      ;
    
    measure q[2] -> c[2];
    measure q[1] -> c[1];
    measure q[0] -> c[0];
    }
    '''
    
    tqb.sn = 1024
    tqb.acc = "aer"
    tqb.noise[0].clear()
    sweep = [False,True]
    [tqb.noise[0].append(bb) for bb in sweep]
    
    tqb.run()
    ```
    
    > Inspect the results:
    > 
    
    ```python
    tqb.out_count[0]
    ```
    
    > The result is (similar to) this:
    > 
    
    ```python
    [{1: 1024}, {0: 9, 1: 990, 3: 11, 5: 14}]
    ```
    

The 'sweep' variable was used to run both noise-free (False) and with noise (True). The first set of curly brackets is the noise-free result the same correct result is found for each shot. The second indicates the effects of noise although the correct result in this case still comprises the great majority of outcomes.

## 2.3 Simple gates

The simplest operations are those that either switch a qubit's state between |0 > and |1 > or change it's coefficient. The very simplest is the identity operator which does literally nothing. It is equivalent to multiplying a number by one and is generally included only to complete the algebra generated by other operations.

The other operations for simply switching and multiplying qubits are denoted X, Y and Z: 

- X-gate: Interchanges |0 > and |1 > with no other effects. It is also called the NOT gate,
- Y-gate: Maps |0 > to i|1 > and |1 > to -i|0 >,
- Z-gate: Leaves |0 > unchanged but multiplies |1 > by -1.

What makes quantum computing interesting however is that the qubits are not restricted to be multiples of either |0 > or  |1 > but may exist in a superposition of the two. The simplest way to create such a superposition is with the Hadamard gate, usually represented with an 'H'. The effect of the Hadamard gate is as follows:

- $\ket{0} \longrightarrow \frac{1}{\sqrt{2}} \left( \ket{0} + \ket{1} \right)$
- $\ket{1} \longrightarrow \frac{1}{\sqrt{2}} \left( \ket{0} - \ket{1} \right)$

Each of these gates is its own inverse, meaning that applying it twice in a row leaves the qubit unchanged, so 

$I = H^2 = X^2 = Y^2 = Z^2$.

## 2.4 Controlled gates and basic quantum algorithms

Quantum computing also relies heavily on correlation, or *entanglement*, between qubits. This is achieved through gates that act on one qubit but are controlled by one or more others. The simplest of these is the controlled-not or CNOT gate. The CNOT, or *CX*, the gate is given one control qubit and one target qubit, and performs a NOT, or *X*, operation on the target qubit if, but only if, the control qubit is in the |1 > state. Their use is demonstrated in the following examples:

- **Example 2.4.1: Superdense coding**
    
    The gates presented so far are well illustrated by the [superdense coding protocol](https://en.wikipedia.org/wiki/Superdense_coding), which allows one to transmit classical information from one party to another, generally, two classical bits, using a single qubit. In a sense, it is a protocol opposite to [quantum teleportation](https://en.wikipedia.org/wiki/Quantum_teleportation).
    
    Let Alice and Bob be the two parties that wish to exchange two classical bits of information. A third party Charlie produces an entangled state of two qubits (applying a Hadamard gate to his first qubit, which serves as the control qubit for the following CNOT gate on the second qubit as the target) and gives Alice and Bob, each a qubit. Depending on the pair of classical bits that Alice wishes to send to Bob, she does one of the [following](https://qiskit.org/textbook/ch-algorithms/images/superdense_table1.png) operations on her qubit:
    
    For classical pair '00' she applies the identity  $I$ and sends   $\ket{0} + \ket{1}$,
    
    For classical pair '10' she applies the operator $X$ and sends $\ket{1} + \ket{0}$,
    
    For classical pair '01' she applies the operator $Z$ and sends $\ket{0} - \ket{1}$,
    
    For classical pair '11' she applies the operator $ZX$ and sends $-\ket{1} + \ket{0}$,
    
    where the sent states are normalised by a factor of $\sqrt{2}$.
    
    Now Bob performs the reverse operation as Charlie, i.e. using the qubit he received from Alice as the control for a CNOT gate targeting the qubit he received from Charlie to disentangle them. He then applies a Hadamard gate to Alice's qubit and retrieves the two classical qubits sent by Alice by measuring both hers and Charlie's. Refer to [this](https://qiskit.org/textbook/ch-algorithms/images/superdense_table2.png) table to better understand the final operation done by Bob.
    
    - Step-by-step walkthrough
        
        The following Python notebook performs superdense coding protocol broadcast of '01' and '00':
        
        [qbOS_superdense.ipynb](qbOS%20Documentation%20211006/qbOS_superdense.ipynb)
        
    - Images of the output

The next two examples are, admittedly contrived, algorithms that demonstrate quantum utility:

- **Example 2.4.2: Quantum utility with Deutsch-Jozsa Algorithm**
    
    The [Deutsch-Jozsa algorithm](https://en.wikipedia.org/wiki/Deutsch%E2%80%93Jozsa_algorithm) was the first example of a quantum algorithm that performed better than its best classical counterpart. 
    
    In our problem, we are given a secret Boolean function $f$, which takes a string of bits as input and returns either a 0 or 1 i.e.
    
    $\hspace{1cm}f(\{x_0, x_1, x_2, ...\}) \rightarrow \text{0 or 1, where } x_n \text{ is 0 or 1}$
    
    The property of the above Boolean function is that it is either balanced or constant. A balanced function will return 0's for exactly half of the inputs and 1's for the other half, whereas a constant function will return all 0's or all 1's for any input. The problem is to determine the nature of this function $f$ i.e. whether it is balanced or constant.
    
    Classically, in the best case, we require two queries to the oracle and in the worst case, we would require half of all possible inputs plus one query to ascertain the nature of $f$.
    
    The advantage that the Deutsch-Jozsa algorithm provides is that only a single query to $f$ is required to determine if the Boolean function is balanced or constant with 100% confidence! If $f$ is constant the final state consists entirely of "0" but if it is balanced then the final state contains at least one "1".
    
    Balanced functions may be identified by a wrapping string. The following circuit performs the Deutsch-Jozsa algorithm on a balanced function whose wrapping string is "101": see [here](https://algassert.com/quirk#circuit=%7B%22cols%22%3A%5B%5B1%2C1%2C1%2C%22X%22%5D%2C%5B%22H%22%2C%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B%22X%22%2C1%2C%22X%22%5D%2C%5B%22%E2%80%A2%22%2C1%2C1%2C%22X%22%5D%2C%5B1%2C%22%E2%80%A2%22%2C1%2C%22X%22%5D%2C%5B1%2C1%2C%22%E2%80%A2%22%2C%22X%22%5D%2C%5B%22X%22%2C1%2C%22X%22%5D%2C%5B%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B%22Measure%22%2C%22Measure%22%2C%22Measure%22%5D%5D%7D).
    
    - Step-by-step walkthrough ending at a plot of Quantum Utility
        
        [qbOS_DJ.ipynb](qbOS%20Documentation%20211006/qbOS_DJ.ipynb)
        
        - **Concepts covered in the Jupyter notebook**:
            - Circuit generation
            - Execution of an array of circuits
            - Extracting classical execution time
            - Extracting estimated quantum hardware execution time
            - Visualising Quantum Utility
    - Images of the output
        
        ![qbstrDJ.png](qbOS%20Documentation%20211006/qbstrDJ.png)
        
- **Example 2.4.3: Quantum utility with Bernstein-Vazirani Algorithm**
    
    The [Bernstein-Vazirani algorithm](https://en.wikipedia.org/wiki/Bernstein%E2%80%93Vazirani_algorithm) can be viewed as an extension of the [Deutsch-Jozsa]() algorithm.  In the problem, we have a secret message *s* (a bitstring) and an oracle *f* which is defined as:
    
     $\hspace{1cm}f_s(x) = s \cdot x \: (\text{mod 2})$
    
    where $x$ is the input string that we use as queries to the oracle, to find the secret string.
    
    The classical approach is to apply 
    
    $\hspace{1cm} x = 2^i, \;\; \forall i \; \in \; [0, n-1]$
    
    requiring *n* trials, where *n* is the number of bits in the secret string. This clearly has a runtime linear in *n*, while the Bernstein-Vazirani algorithm requires only one call to the function $f_s(x)$. It follows theoretically that the runtime is constant with respect to the size of *s*. In practice, however, the physical characteristics and limitations of the hardware cause the runtime to increase with increasing number of qubits. 
    
    For a graphical view of this circuit, see [here](https://algassert.com/quirk#circuit=%7B%22cols%22%3A%5B%5B%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B1%2C1%2C1%2C%22H%22%5D%2C%5B1%2C1%2C1%2C%22Z%22%5D%2C%5B1%2C%22%E2%80%A2%22%2C1%2C%22X%22%5D%2C%5B1%2C1%2C%22%E2%80%A2%22%2C%22X%22%5D%2C%5B%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B%22Measure%22%2C%22Measure%22%2C%22Measure%22%5D%5D%7D).
    
    - Step-by-step walkthrough showing the impact of noise
        
        The following Python notebook sets up a 2-by-2 experiment: it runs the Bernstein-Vazirani algorithm with a 14-qubit secret, and a 16-qubit secret; where both noise-free as well as noise-enabled conditions are examined:
        
        [qbOS_BV.ipynb](qbOS%20Documentation%20211006/qbOS_BV.ipynb)
        
    - Images of the output

As one might expect, all the simple gates in Section 2.2 have their controlled equivalents, *CY*, *CZ* and *CH.*

A list of available gates, together with mathematical representation and coding syntax is available in the [Glossary of Tools and Quantum Gates]().

# 3. qbOS Modules and Applications

## 3.1 Quantum Fourier Transform (QFT)

[Quantum Fourier Transform (QFT)](qbOS%20Documentation%20211006/Quantum%20Fourier%20Transform%20(QFT)%20086133df66464393834202921b561874.md)

## 3.2 Comparator

Some algorithms, such as the exponential search and other applications of the Grover iteration below, need to compare a quantum register to a given value. The user specifies if they want registers whose value is greater or lesser than this value, as well as the state vector being compared, the qubits-of-interest, and the value being compared against.  This has been implemented in version zero so as to provide a vector which yields positive one if the condition is obeyed and negative one if it is not. The currently implemented comparisons are "greater than" and "less than".

```python
import grover as gr
compare_vector = gr.comparison_vector(qubits_of_interest,initial_vector,compare_value,comparison)
```

## 3.3 Grover iteration

The Grover algorithm was originally a method for finding a given entry in a database [*reference*] but it has since been generalised to finding extremal values of functions. This in turn has a multitude of applications, including finding optimal strings of characters given a measure, which we later illustrate in the *quantum_decoder*.

To perform a Grover iteration the *grover_iteration*() method is called: 

```python
import grover as gr
compare_vector = gr.comparison_vector(qubits_of_interest,initial_vector,compare_value,comparison)
grovered_state = gr.grover_iteration(initial_vector,qubits_of_interest,compare_vector)
```

Note that the Grover iteration is designed to return a state vector for which the desirable components have amplitudes much greater than all the other components combined so that an appropriate result is overwhelmingly likely to be obtained upon measurement. This is not guaranteed. 

The effectiveness of the Grover iteration is also affected by the distribution of the measures and of the probability amplitudes within the initial state.

The input parameter compare_vector specifies which components obey the criteria for being desirable, such as having a measure greater than *compare_value* (see the *comparator* application above). The Grover iteration magnifies the probability amplitude of the desirable components at the expense of the undesirable components. 

## 3.4 Quantum Decoder Application

The decoder is an algorithm that, given a string of symbols such as letters of the alphabet, and a probability table or some other form of the sampler, will predict the subsequent symbols. The applications for such an algorithm range from text autofill to autonomous pathfinding. The quantum decoder encodes a superposition of all possible strings with a score, currently the probability of each letter, and then uses an exponential search algorithm to manipulate the probability amplitudes so that the best scoring string is the one most likely returned upon measurement.

It currently uses a classical sampler but a quantum version will be needed eventually. The easiest way to use the quantum decoder is to instantiate a *quantum_decoder* object from the quantum_decoder.py module:

```python
import quantum_decoder as qd
decoder = qd.quantum_decoder(S,N_string,total_length,table,precision,Alphabet,
				grammar,Starting_String = '!', search_cutoff = 0.9, Encoding = "Bitstring")

decoded_state = decoder.get_decoder_state()
optimal_string = es.measurement(qubits_of_interest, decoded_state**)    #Almost all the time**
```

The quantum decoder is based on the Grover iteration and returns a state for which the most suitable string has an amplitude much greater than all the other components combined so that the correct string is overwhelmingly likely, though not guaranteed, to be obtained upon measurement. 

## 3.5 Variational Quantum Eigensolver (VQE)

[Variational Quantum Eigensolver (VQE) in qbOS](qbOS%20Documentation%20211006/Variational%20Quantum%20Eigensolver%20(VQE)%20in%20qbOS%206b6a211cc87749f6ac36faaf5581f8a2.md)

# 4. Hardware Specifications and Limitations

### 4.1 Qubit Connectivity Diagram

In the context of quantum programming, we say that two qubits are connected if a CNOT operation can be directly applied to them. By this definition, all qubits on a QB diamond chip are effectively connected and this should be sufficient for most quantum programming applications.  

Direct physical connectivity is much more restricted. Qubits are physically arranged in fully connected clusters of three. The clusters are arranged in a two-dimensional grid with complete connectivity between the qubits of horizontal and vertical nearest neighbours. There are no diagonal physical connections. It follows that there can be no more than six qubits in a physically fully connected registry. 

![qbOS%20Documentation%20211006/cluster_grid2.png](qbOS%20Documentation%20211006/cluster_grid2.png)

> **Figure:** Each cluster of three qubits lies inside a black circle. Intra-cluster connections are shown in red. Connections between clusters are in orange and split at each end to indicate that every qubit at one end is physically connected to every qubit at the other.
> 

Physically distant qubits can still be connected in the quantum programming context described above, but in practice this may require the equivalent of a large number of swap operations, rendering such a connectionless reliable. 

# 5. [For users who know quantum circuits] Exporting from a circuit composition GUI and using the circuit in qbOS

See this guide on exporting a circuit from a circuit composition GUI to then execute the circuit in qbOS:

[Circuit composition on GUI tools and conversion to QPU kernel for execution in qbOS](qbOS%20Documentation%20211006/Circuit%20composition%20on%20GUI%20tools%20and%20conversion%20to%204f46ef713a5743e192aa79ec6f804d5d.md)

# 6. Types and Arrays in qbOS

**The design aim of qbOS is to support an array programming syntax**.  

Users who have come from a NumPy, Matlab or Julia background will see the similarity to those environments.  Put simply, qbOS objects have members that are 2D arrays.  **Broadcasting** of values is performed wherever it is consistent to do so prior to execution.

The **leading (or innermost) dimension** is from hereon referred to as the **conditions** dimension

The **second dimension** is from hereon referred to as the **circuits** dimension

A quick way to remind yourself of this is: 
**"crossing rows** changes **circuits"
"crossing columns** changes **conditions"**

Before describing the classes that are provided to support this, let's see how this works with an example - a sweep of the number of shots [sn] for a simple circuit:

## 6.1 Example 1:  |+++> measured using a sweep of 32 shots, 64 shots and 128 shots

```python
import qbos as q
tqb = q.core()
tqb.qb12()

tqb.instring = '''
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
creg c[3];

// ----- q[2] -----;----- q[1] -----;----- q[0] -----;
       h q[2]      ;    h q[1]      ;    h q[0]      ;

measure q[2] -> c[2];
measure q[1] -> c[1];
measure q[0] -> c[0];
}
'''

tqb.sn[0].clear()
sweep = [32,64,128]
[tqb.sn[0].append(nn) for nn in sweep]

tqb.run()

```

Inspect the results:

```python
tqb.out_raw[0]

```

This will look similar to the output below:

```python
String[{
    "000": 3,
    "001": 3,
    "010": 7,
    "011": 2,
    "100": 3,
    "101": 4,
    "110": 5,
    "111": 5
}, {
    "000": 7,
    "001": 8,
    "010": 4,
    "011": 12,
    "100": 9,
    "101": 3,
    "110": 14,
    "111": 7
}, {
    "000": 20,
    "001": 18,
    "010": 9,
    "011": 16,
    "100": 14,
    "101": 15,
    "110": 22,
    "111": 14
}]
```

## 6.2 Example 2: A circuit with 2 parameters, running a sweep over 3 conditions

```python
import qbos as q
tqb = q.core()
tqb.qb12()

tqb.instring = '''
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
creg c[4];

// ----- q[3] -----;----- q[2] -----;----- q[1] -----;----- q[0] -----;
                        x q[2]      ;                     x q[0]      ;  

u3(QBTHETA_0, 1.6, QBTHETA_1) q[3];

measure q[3] -> c[3];
measure q[2] -> c[2];
measure q[1] -> c[1];
measure q[0] -> c[0];
}
'''

tqb.sn = 1024
tqb.noise = False
tqb.acc = "aer"
mth0 = q.ND()
mth0[0] = 0.01
mth0[1] = 0.015
 
mth1 = q.ND()
mth1[0] = 0.2
mth1[1] = 0.25

mth2 = q.ND()
mth2[0] = 1.1
mth2[1] = 1.15

tqb.theta[0].clear()
sweep = q.MapND([mth0, mth1, mth2])
[tqb.theta[0].append(nd) for nd in sweep]

tqb.run()

```

Inspect the results:

```python
tqb.out_transpiled_circuit[0]
```

The result is this:

```python
String[
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
u(3.14159, -1.5708, 1.5708) q[2];
u(3.14159, -1.5708, 1.5708) q[0];
u(0.01, 1.6, 0.015) q[3]; // u3(mth0[0], 1.6, mth0[1]) q[3]
creg c0[1];
measure q[0] -> c0[0];
creg c1[1];
measure q[1] -> c1[0];
creg c2[1];
measure q[2] -> c2[0];
creg c3[1];
measure q[3] -> c3[0];

}
, 
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
u(3.14159, -1.5708, 1.5708) q[2];
u(3.14159, -1.5708, 1.5708) q[0];
u(0.2, 1.6, 0.25) q[3]; // u3(mth1[0], 1.6, mth1[1]) q[3]
creg c0[1];
measure q[0] -> c0[0];
creg c1[1];
measure q[1] -> c1[0];
creg c2[1];
measure q[2] -> c2[0];
creg c3[1];
measure q[3] -> c3[0];

}
, 
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
u(3.14159, -1.5708, 1.5708) q[2];
u(3.14159, -1.5708, 1.5708) q[0];
u(1.1, 1.6, 1.15) q[3]; // u3(mth2[0], 1.6, mth2[1]) q[3]
creg c0[1];
measure q[0] -> c0[0];
creg c1[1];
measure q[1] -> c1[0];
creg c2[1];
measure q[2] -> c2[0];
creg c3[1];
measure q[3] -> c3[0];

}
]
```

## 6.3 `qbos.String`: list/array of strings

Also `qbos.VectorString`: 2-D array of strings

## 6.4 `qbos.N` : list/array of integers

Also: `qbos.VectorN` : 2-D array of integers

## 6.5 `qbos.Bool` : list/array of booleans

Also: `qbos.VectorBool` : 2-D array of booleans

## 6.6 `qbos.MapND` : list/array of maps (integer → double precision real)

Also: `qbos.VectorMapND` : 2-D array of maps

[Usage/examples of `qbos.MapND` type](qbOS%20Documentation%20211006/Usage%20examples%20of%20qbos%20MapND%20type%20807974c9c8c94f0c842b601924105d17.md)

## 6.7 `qbos.MapNC` : list/array of maps (integer → double precision complex)

Also: `qbos.VectorMapNC` : 2-D array of maps

[Usage/examples of `qbos.MapNC` type](qbOS%20Documentation%20211006/Usage%20examples%20of%20qbos%20MapNC%20type%20e04ad5447638476184ed7a21f05e966b.md)

## 6.8 `qbos.MapNN`: list/array of maps (integer → integer)

Also: `qbos.VectorMapNN`: 2-D array of maps

# 7. Download and Installation

qbOS consists of a front-end JupyterLab GUI, supported by a container that is run on any of the following platforms:

## 7.1 Pawsey Nimbus Cloud

[[Pawsey Nimbus Cloud]()](qbOS%20Documentation%20211006/Pawsey%20Nimbus%20Cloud%2039cb71b3471045b388e4593d7ae2d240.md)

## 7.2 AWS

[[AWS Spot-Market instances]()](qbOS%20Documentation%20211006/AWS%20Spot-Market%20instances%20695d2d72ff0b4916aec906b3ac43b74a.md)

## 7.3 HPC (command-line job scripts)

qbOS includes command-line executables that are suited for job scripts that are submitted to queues on HPC systems.  These are executables that are compiled from source code to use site-specific optimised numerical libraries and MPI support.  Execution can be performed either on containers (Singularity); or directly on bare metal.  Please contact Quantum Brilliance to discuss your requirements.

# 8. Starting a qbOS Jupyter session

## 8.1 **Login** to the VM instance

Once all the necessary VM setup is completed and the instance is launched, it is time to log in.

If you are running Windows 10, first open PowerShell (**Windows button** → **Windows PowerShell**) or some other Shell terminal of your choice and then:

```bash
ssh -i your-ssh-private-key-file -L 8889:localhost:8889 ubuntu@your.ip.ad.dress
```

Please ensure you have `-L 8889:localhost:8889` in your login as shown above.

Enter the password as appropriate (if needed, type 'yes' when asked about the identity of the system).

## 8.2 Launching the web browser GUI interface (JupyterLab)

With a web browser, visit [http://localhost:8889](http://localhost:8889)

# 9. Glossary of Tools and Quantum Gates

## 9.1 Quantum Gates

This section is a quick reference on quantum gates expressed in these formats:

- I - Identity
    - Unitary Description and Decomposition Rules
        
        This operator leaves a qubit unchanged, so that
        
         $\ket{0} \rightarrow \ket{0}$ and $\ket{1} \rightarrow \ket{1}$. 
        
        It is expressed by the matrix  $I = \left(\begin{array}{cc} 1 & 0 \\ 0 & 1\end{array} \right)$, but implemented by the absence of any operator.
        
- H - Hadamard
    - Unitary Description and Decomposition Rules
        
        This operator mixes the measurement basis states, so that
        
         $\ket{0} \rightarrow \frac{1}{\sqrt{2}}\Big(\ket{0} + \ket{1}\Big)$ and $\ket{1} \rightarrow \frac{1}{\sqrt{2}}\Big(\ket{0} - \ket{1}\Big)$. 
        
        It is given by the matrix  $H = \frac{1}{\sqrt{2}} \left(\begin{array}{rr} 1 & 1 \\ 1 & -1 \end{array} \right)$.
        
    - OpenQASM
        
        ```python
        h q[0] ;
        ```
        
        where q[0] is the qubit.
        
    - XASM
        
        ```cpp
        H 0
        ```
        
        where 0 is the qubit.
        
    - QB Native Transpilation
        
        ```python
        h(q0)
        ```
        
        where q0 is the qubit.
        
- X - Pauli rotation on x-axis by pi
    - Unitary Description and Decomposition Rules
        
        This operator performs an X, or NOT operation on a qubit, turning
        
         $\ket{0} \rightarrow \ket{1}$ and $\ket{1} \rightarrow \ket{0}$. 
        
        It is given by the matrix  $X = \left(\begin{array}{cc} 0 & 1 \\ 1 & 0\end{array} \right)$.
        
    - OpenQASM
        
        ```python
        x q[0] ;
        ```
        
        where q[0] is the qubit.
        
    - XASM
        
        ```cpp
        X 0
        ```
        
        where 0 is the qubit.
        
    - QB Native Transpilation
        
        ```python
        x(q0)
        ```
        
        where q0 is the qubit.
        
- Y - Pauli rotation on y-axis by pi
    - Unitary Description and Decomposition Rules
        
        This operator performs a Y operation on a qubit, turning
        
         $\ket{0} \rightarrow -i\ket{1}$ and $\ket{1} \rightarrow i\ket{0}$. 
        
        It is given by the matrix  $Y = \left(\begin{array}{cc} 0 & -i \\ i & 0\end{array} \right)$
        
    - OpenQASM
        
        ```python
        y q[0] ;
        ```
        
        where q[0] is the qubit.
        
    - XASM
        
        ```cpp
        Y 0
        ```
        
        where 0 is the qubit.
        
    - QB Native Transpilation
        
        ```python
        y(q0)
        ```
        
        where q0 is the qubit.
        
- Z - Pauli rotation on z-axis by pi
    - Unitary Description and Decomposition Rules
        
        This operator performs a Z operation on a qubit, turning
        
         $\ket{0} \rightarrow \ket{0}$ and $\ket{1} \rightarrow \ket{1}$. 
        
        It is given by the matrix  $Z = \left(\begin{array}{cc} 1 & 0 \\ 0 & -1 \end{array} \right)$
        
    - OpenQASM
        
        ```python
        z q[0] ;
        ```
        
        where q[0] is the qubit.
        
    - XASM
        
        ```python
        Z 0
        ```
        
        where 0 is the qubit.
        
    - QB Native Transpilation
        
        ```python
        z(q0)
        ```
        
        where q0 is the qubit.
        
- CNOT - controlled-X
    - Unitary Description and Decomposition Rules
        
        For a given control qubit $q_0$ and target qubit $q_1$ this operator performs a X operation on the $q_1$ if $q_0$ equals one, but leaves $q_1$ unchanged, *i.e.* the identity operation, if $q_0$equals zero. It is given by the matrix
        
        $CX_{2q_0 + q_1 + 1,2q_0 + q_1 + 1} = \text{diag}(I,X) = \left(\begin{array}{cccc}1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 0 & 1 \\ 0 & 0 & 1 & 0\end{array} \right)$
        
    - OpenQASM
        
        ```python
        CX q[0] , q[1] ;
        ```
        
        where q[0] is the control qubit and q[1] is the target qubit.
        
    - XASM
        
        ```cpp
        CX 0 1
        ```
        
        where 0 is the control qubit and 1 is the target qubit.
        
    - QB Native Transpilation
        
        ```python
        cx(q0, q1)
        ```
        
        where q0 is the control qubit and q1 is the target qubit.
        
- CY - controlled-Y
    - Unitary Description and Decomposition Rules
        
        For a given control qubit $q_0$ and target qubit $q_1$ this operator performs a Y operation on the $q_1$ if $q_0$ equals one, but leaves $q_1$ unchanged, *i.e.* the identity operation, if $q_0$equals zero. It is given by the matrix
        
        $CY_{2q_0 + q_1 + 1,2q_0 + q_1 + 1} = \text{diag}(I,Y) = \left(\begin{array}{cccc}1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 0 & -i \\ 0 & 0 & i & 0\end{array} \right)$
        
        where $i^2 = -1$.
        
    - OpenQASM
        
        ```python
        CY q[0] , q[1] ;
        ```
        
        where q[0] is the control qubit and q[1] is the target qubit.
        
    - XASM
        
        ```cpp
        CY 0 1
        ```
        
        where 0 is the control qubit and 1 is the target qubit.
        
    - QB Native Transpilation
        
        ```python
        cy(q0, q1)
        ```
        
        where q0 is the control qubit and q1 is the target qubit.
        
- CZ - controlled-Z
    - Unitary Description and Decomposition Rules
        
        For a given control qubit $q_0$ and target qubit $q_1$ this operator performs a Z operation on the $q_1$ if $q_0$ equals one, but leaves $q_1$ unchanged, *i.e.* the identity operation, if $q_0$equals zero. It is given by the matrix
        
        $CZ_{2q_0 + q_1 + 1,2q_0 + q_1 + 1} = \text{diag}(I,Z) = \left(\begin{array}{cccc}1 & 0 & 0 & 0 \\ 0 & 1 & 0 & 0 \\ 0 & 0 & 1 & 0 \\ 0 & 0 & 0 & -1\end{array} \right)$
        
    - OpenQASM
        
        ```python
        CZ q[0] , q[1] ;
        ```
        
        where q[0] is the control qubit and q[1] is the target qubit.
        
    - XASM
        
        ```cpp
        CX 0 1
        ```
        
        where 0 is the control qubit and 1 is the target qubit.
        
    - QB Native Transpilation
        
        ```python
        cz(q0, q1)
        ```
        
        where q0 is the control qubit and q1 is the target qubit.
        
- $R_x(\theta)$ - Pauli rotation on *X*-axis by $\theta$
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *X*-axis of the Bloch sphere by an angle of $\theta$. It partially interchanges $\ket{0}$ with $\ket{1}$ and introduces a relative phase between the two components, so
        
         $\ket{0} \rightarrow \cos\left(\frac{\theta}{2}\right)\ket{0} -i \sin\left(\frac{\theta}{2}\right) \ket{1}$   and    $\ket{1} \rightarrow -i\sin(\frac{\theta}{2})\ket{0} + \cos(\frac{\theta}{2})\ket{1}$. 
        
        It is given by the matrix 
        
         $X(\theta) = I \cos\!\left(\frac{\theta}{2}\right) -i \sigma_X  \sin\!\left(\frac{\theta}{2}\right) = \left(\begin{array}{cc} \cos\left(\frac{\theta}{2}\right) & -i\sin\left(\frac{\theta}{2}\right) \\ & \\-i\sin\left(\frac{\theta}{2}\right) & \cos\left(\frac{\theta}{2}\right)\end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_X$ is the Pauli matrix
        
        $\sigma_X = \left(\begin{array}{cc} 0 & 1 \\ 1 & 0 \end{array}\right)$.
        
    - OpenQASM
        
        ```python
        rx(_theta) q[0];
        ```
        
        where q[0] is the qubit and _*theta* is the angle of rotation about the X-axis.
        
    - XASM
        
        ```python
        Rx(_theta) 0
        ```
        
        where 0 is the qubit and _*theta* is the angle of rotation about the X-axis.
        
    - QB Native Transpilation
        
        ```python
        rx(q0,_theta)
        ```
        
        where q0 is the qubit and _*theta* is the angle of rotation about the X-axis.
        
- $R_y(\phi)$ - Pauli rotation on *Y*-axis by $\phi$
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *Y*-axis of the Bloch sphere by an angle of $\phi$, partially interchanging $\ket{0}$ with $\ket{1}$, so
        
         $\ket{0} \rightarrow \cos\left(\frac{\phi}{2}\right)\ket{0} - \sin\left(\frac{\phi}{2}\right) \ket{1}$   and    $\ket{1} \rightarrow \sin(\frac{\phi}{2})\ket{0} + \cos(\frac{\phi}{2})\ket{1}$. 
        
        It is given by the matrix 
        
         $Y(\phi) = I \cos\!\left(\frac{\phi}{2}\right) -i \sigma_Y  \sin\!\left(\frac{\phi}{2}\right) = \left(\begin{array}{cc} \cos\left(\frac{\phi}{2}\right) & -\sin\left(\frac{\phi}{2}\right) \\ & \\ \sin\left(\frac{\phi}{2}\right) & \cos\left(\frac{\phi}{2}\right)\end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_Y$ is the Pauli matrix
        
        $\sigma_Y = \left(\begin{array}{cc} 0 & -i \\ i & 0 \end{array}\right)$.
        
    - OpenQASM
        
        ```python
        ry(_phi) q[0];
        ```
        
        where q[0] is the qubit and *_phi* is the angle of rotation about the Z-axis.
        
    - XASM
        
        ```python
        Ry(_phi) 0
        ```
        
        where 0 is the qubit and _*phi* is the angle of rotation about the Y-axis.
        
    - QB Native Transpilation
        
        ```python
        ry(q0,_phi)
        ```
        
        where q0 is the qubit and *_phi* is the angle of rotation about the Z-axis.
        
- $R_z(\lambda)$ - Pauli rotation on *Z*-axis by $\lambda$
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *Z*-axis of the Bloch sphere by an angle of $\lambda$. It introduces a relative phase between the two components, so
        
         $\ket{0} \rightarrow -i\sin(\frac{\lambda}{2})\ket{0}$   and    $\ket{1} \rightarrow i\sin(\frac{\lambda}{2})\ket{1}$. 
        
        It is given by the matrix 
        
         $Z(\lambda) = I \cos\!\left(\frac{\lambda}{2}\right) -i \sigma_Z  \sin\!\left(\frac{\lambda}{2}\right) = \left(\begin{array}{cc} \cos\left(\frac{\lambda}{2}\right) -i\sin\left(\frac{\lambda}{2}\right) & 0 \\ & \\0 & \cos\left(\frac{\lambda}{2}\right) +  i\sin\left(\frac{\lambda}{2}\right) \end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_Z$ is the Pauli matrix
        
        $\sigma_Z = \left(\begin{array}{cc} 1 & 0 \\ 0 & -1\end{array}\right)$.
        
    - OpenQASM
        
        ```python
        rz(_lambda) q[0];
        ```
        
        where q[0] is the qubit and *_lambda* is the angle of rotation about the Z-axis.
        
    - XASM
        
        ```python
        Rz(_lambda) 0
        ```
        
        where 0 is the qubit and *_lambda* is the angle of rotation about the Z-axis.
        
    - QB Native Transpilation
        
        ```python
        rz(q0,_lambda)
        ```
        
        where q0 is the qubit and *_lambda* is the angle of rotation about the Z-axis.
        
- S - rotation on z-axis by 0.5*pi
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *Z*-axis of the Bloch sphere by an angle of $\frac{\pi}{2}$. It introduces a relative phase between the two components, where
        
         $\ket{0} \rightarrow -i\sin(\frac{\pi}{4})\ket{0}$   and    $\ket{1} \rightarrow i\sin(\frac{\pi}{4})\ket{1}$. 
        
        It is given by the matrix 
        
         $S = Z\left(\frac{\pi}{2}\right) = I \cos\!\left(\frac{\pi}{4}\right) -i \sigma_Z  \sin\!\left(\frac{\pi}{4}\right) = \left(\begin{array}{cc} \cos\left(\frac{\pi}{4}\right) -i\sin\left(\frac{\pi}{4}\right) & 0 \\ & \\0 & \cos\left(\frac{\pi}{4}\right) +  i\sin\left(\frac{\pi}{4}\right) \end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_Z$ is the Pauli matrix
        
        $\sigma_Z = \left(\begin{array}{cc} 1 & 0 \\ 0 & -1\end{array}\right)$.
        
    - OpenQASM
        
        ```python
        import math
        rz(0.5*math.pi) q[0];
        ```
        
        where q[0] is the qubit.
        
    - XASM
        
        ```python
        import math
        Rz(0.5*math.pi) 0
        ```
        
        where 0 is the qubit.
        
    - QB Native Transpilation
        
        ```python
        import math
        rz(q0,0.5*math.pi)
        ```
        
        where q0 is the qubit.
        
- T - rotation on z-axis by 0.25*pi
    - Unitary Description and Decomposition Rules
        
        This operator rotates a qubit around the *Z*-axis of the Bloch sphere by an angle of $\frac{\pi}{4}$. It introduces a relative phase between the two components, where
        
         $\ket{0} \rightarrow -i\sin(\frac{\pi}{8})\ket{0}$   and    $\ket{1} \rightarrow i\sin(\frac{\pi}{8})\ket{1}$. 
        
        It is given by the matrix 
        
         $S = Z\left(\frac{\pi}{4}\right) = I \cos\!\left(\frac{\pi}{8}\right) -i \sigma_Z  \sin\!\left(\frac{\pi}{8}\right) = \left(\begin{array}{cc} \cos\left(\frac{\pi}{8}\right) -i\sin\left(\frac{\pi}{8}\right) & 0 \\ & \\0 & \cos\left(\frac{\pi}{8}\right) +  i\sin\left(\frac{\pi}{8}\right) \end{array} \right)$,
        
        where $I$ is the identity matrix and $\sigma_Z$ is the Pauli matrix
        
        $\sigma_Z = \left(\begin{array}{cc} 1 & 0 \\ 0 & -1\end{array}\right)$.
        
    - OpenQASM
        
        ```python
        import math
        rz(0.25*math.pi) q[0];
        ```
        
        where q[0] is the qubit.
        
    - XASM
        
        ```python
        import math
        Rz(0.25*math.pi) 0
        ```
        
        where 0 is the qubit.
        
    - QB Native Transpilation
        
        ```python
        import math
        rz(q0,0.25*math.pi)
        ```
        
        where q0 is the qubit.
        
- $U(\theta,\phi,\lambda)$ - arbitrary rotation, in x-axis by $\theta$, y-axis by $\phi$, z-axis by $\lambda$
    - Unitary Description and Decomposition Rules
        
        If used for an **ansatz** that will require **automatic differentiation** (AD), replace $U$ with the decomposition shown below.
        
        $U(\theta, \phi, \lambda) = R_z(\phi)*R_x(-0.5\pi)*R_z(\theta)*R_x(0.5\pi)*R_z(\lambda)$
        
    - OpenQASM
        
        ```c
        __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[1];
        
        // theta = 0.2
        // phi = -0.25
        // lambda = 1.1
        // Format: u(theta, phi, lambda) target_qubit
        u(0.2, -0.25, 1.1) q[0];
        
        measure q[0] -> c[0];
        }
        ```
        
    - XASM
    - QB Native Transpilation
- Measurement
    - Description
        
        Measurement is inherently non-unitary. The physical operation selects one of the components of the given quantum registry at random, where the probability of any particular component being selected is given by the square of its amplitude. For this reason it is advisable to call multiple shots (the qbOS default is 1024) so that the distribution of possible outcomes is apparent.
        
    - OpenQASM
        
        ```python
        measure q[0] -> c[0]
        ```
        
        where q[0] is the qubit and c[0] is the corresponding classical bit to which it is measured.
        
    - XASM
        
        ```python
        MEASURE 0 [0]
        MEASURE 1 [1]
        ```
        
        where 0, 1 label the corresponding qubits and the classical bits to which they are measured.
        
    - QB Native Transpilation
        
        ```python
        measure(q0, c0)
        ```
        
        where q0 is the qubit and c0 is the corresponding classical bit to which it is measured.
        

## 9.2 High-level Commands and Tools/Options

- **Noise**: How do I turn on the encoded noise model of diamond quantum accelerators?
    
    ```python
    import qbos
    tqb = qbos.core()
    
    tqb.noise = True
    ```
    
    The default is for the noise parameter to be False.
    
- **Qubit Number**: How do I set the maximum number of qubits in the quantum registry?
    
    ```python
    import qbos
    tqb = qbos.core()
    
    tqb.qn = 15
    ```
    
    This will have 15 qubits in the quantum register instead of the default of 12.
    
- **Shots**: How do I set the number of shots?
    
    ```python
    import qbos
    tqb = qbos.core()
    
    tqb.sn = 256
    ```
    
    There will be 256 shots instead of the usual 1024.
    

[Variational Quantum Eigensolver (VQE) in qbOS](qbOS%20Documentation%20211006/Variational%20Quantum%20Eigensolver%20(VQE)%20in%20qbOS%206b6a211cc87749f6ac36faaf5581f8a2.md)
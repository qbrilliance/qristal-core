# Combined Documentation (draft)

![Quantum_Brilliance_dark_blue_Logo_RGB.png](Combined%20Documentation%20(draft)%20210902/Quantum_Brilliance_dark_blue_Logo_RGB.png)

# **qbOS Built 0.0721.002**

*Q*uantum *B*rilliance *O*perating *S*ystem supporting a complete set of quantum computational tasks

Pre-Alpha release July 2021

# 1. Home

## About qbOS

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

> "The GUI is great".

> "Integration of OpenQASM, Easy representation of the Hamiltonian (in equation form) for running VQE, [and] Ability to test algorithms with noise without having to model the noise [are my favourite features]".

> "Ease of construction of specific quantum circuits [is my favourite feature]".

> "The details of the outputs [and] being able to create the circuits in IBM and bring them here [are my favourite features]".

> "The IDE is very familiar and easy to use".

> "The course was great and didn't see any bugs".

Samarth Sandeep, CEO, IFF Bio, 6th May 2021:

> "*Wish my first experiences in quantum were this straightforward"*

Angus Mingare, PhD Student, Australian National University, 22nd March 2021:

> "*I'm impressed by how easy to use it is"*

## qbOS Development Team

qbOS is our proprietary quantum software brought to you through the collective efforts of different Quantum Brilliance teams, feedback and contributions from external collaborators and end-users. Within Quantum Brilliance, our *Software & Applications* team is responsible for the development and support of qbOS. You may contact the following senior members from our Software and Applications team for general inquiries and questions regarding qbOS:

![NarimanSaadatmand.jpg](Combined%20Documentation%20(draft)%20210902/NarimanSaadatmand.jpg)

**Dr Nariman Saadatmand**
qbOS Product Owner
Software & Applications Team Lead

nariman.s@quantum-brilliance.com 

![Simon_Yin.jpg](Combined%20Documentation%20(draft)%20210902/Simon_Yin.jpg)

**Simon Yin**
Senior Software Engineer

simon.y@quantum-brilliance.com 

![Michael_Walker.png](Combined%20Documentation%20(draft)%20210902/Michael_Walker.png)

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

Using a small experimental design, let's run some [Quantum Fourier Transform (QFT)]() circuits.  We perform a 2-qubit QFT, and a 4-qubit QFT.  In both cases, we will perform both a noiseless and a noisy simulation.  

The concept of a **QPU Kernel** appears without details for now - this concept will be fully covered in later sections.

[QFT Experiment Design](Combined%20Documentation%20(draft)%20210902/QFT%20Experiment%20Design%2097159967bced45808d9bbef5fc861dea.csv)

- **Step 1 of 3**: Start your qbOS environment

    Access to qbOS requires a web-browser, and a virtual machine (VM) host provided via any one of the following options:

    [[Pawsey Nimbus Cloud]()](Combined%20Documentation%20(draft)%20210902/Pawsey%20Nimbus%20Cloud%2039cb71b3471045b388e4593d7ae2d240.md)

    [[AWS Spot-Market instances]()](Combined%20Documentation%20(draft)%20210902/AWS%20Spot-Market%20instances%20695d2d72ff0b4916aec906b3ac43b74a.md)

    Once you have completed the login to your VM host, use your Web browser to open: [http://localhost:8889](http://localhost:8889) 

    Then login to the JupyterLab session - the instructions for getting the session password are printed once you have completed the login process to your VM host:

    ![Untitled](Combined%20Documentation%20(draft)%20210902/Untitled.png)

- **Step 2 of 3**: Python code that calls qbOS and executes all (four) corner conditions of the experiment

    ```python
    import qbos
    tqb = qbos.core()  # This object has access to the core API methods for circuit simulation
    tqb.qb12()         # Set up some sensible defaults

    ```

    > Setup conditions that apply to all experiment corners:

    ```python
    tqb.xasm = True    # Use XASM circuit format to access XACC's qft()
    n_shots = 1024     
    tqb.sn = n_shots   # Explicitly use 1024 shots

    ```

    > Setup rows of the experiment table:

    ```python
    tqb.qn.clear()
    tqb.qn.append(qbos.N([2])  # 2-qubits for the top row
    tqb.qn.append(qbos.N([4])  # 4-qubits for the bottom row

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

    ```python
    tqb.noise.clear()
    tqb.noise.append(qbos.Bool([False, True]))  # noise is False (disabled) for the left (index 0) column, True (enabled) for the right column (index 1)

    ```

    > Run the simulation in all experiment corners

    ```python
    tqb.run()    

    ```

- **Step 3 of 3**: Inspect the results (shot counts)

    After the code from Step 2 has finished executing, you can inspect the settings or results using the Python codes in the table entries below:

    [QFT Experiment - Python code for viewing settings and results](Combined%20Documentation%20(draft)%20210902/QFT%20Experiment%20-%20Python%20code%20for%20viewing%20settings%20%20ba1d8065630f480fbb0e2dcf8d999fb4.csv)

# 2. Quantum Circuit Examples

## 2.1. Basic Quantum Algorithms and Protocols

### Example 1: Basis measurements with and without noise

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

    ```python
    tqb.out_count[0]
    ```

    > The result is (similar to) this:

    ```python
    [{1: 1024}, {0: 9, 1: 990, 3: 11, 5: 14}]
    ```

### Example 2: Superdense Coding Protocol

- Theory

    The [superdense coding protocol](https://en.wikipedia.org/wiki/Superdense_coding) allows one to transmit classical information from one party to another, generally, two classical bits, using a single qubit. In a sense, it is a protocol opposite to [quantum teleportation](https://en.wikipedia.org/wiki/Quantum_teleportation).

    Let Alice and Bob be the two parties that wish to exchange two classical bits of information. A third party Charlie produces an entangled state of two qubits (applying a Hadamard gate to his first qubit, which serves as the control qubit for the following CNOT gate on the second qubit as the target) and gives Alice and Bob, each a qubit. Depending on the pair of classical bits that Alice wishes to send to Bob, she does one of the [following](https://qiskit.org/textbook/ch-algorithms/images/superdense_table1.png) operations on her qubit:

    For classical pair '00' she applies the identity  $I$ and sends   $\ket{0} + \ket{1}$,

    For classical pair '10' she applies the operator $X$ and sends $\ket{1} + \ket{0}$,

    For classical pair '01' she applies the operator $Z$ and sends $\ket{0} - \ket{1}$,

    For classical pair '11' she applies the operator $ZX$ and sends $-\ket{1} + \ket{0}$,

    where the sent states are normalised by a factor of $\sqrt{2}$.

    Now Bob performs the reverse operation as Charlie, i.e. using the qubit he received from Alice as the control for a CNOT gate targeting the qubit he received from Charlie to disentangle them. He then applies a Hadamard gate to Alice's qubit and retrieves the two classical qubits sent by Alice by measuring both hers and Charlie's. Refer to [this](https://qiskit.org/textbook/ch-algorithms/images/superdense_table2.png) table to better understand the final operation done by Bob.

- Python notebook

    The following Python notebook performs superdense coding protocol broadcast of '01' and '00':

    [qbOS_superdense.ipynb](Combined%20Documentation%20(draft)%20210902/qbOS_superdense.ipynb)

### Example 3: Bernstein-Vazirani Algorithm

- Theory

    The [Bernstein-Vazirani algorithm](https://en.wikipedia.org/wiki/Bernstein%E2%80%93Vazirani_algorithm) can be viewed as an extension of the [Deutsch-Jozsa]() algorithm.  In the problem, we have a secret message *s* (a bitstring) and an oracle *f* which is defined as:

     $\hspace{1cm}f_s(x) = s \cdot x \: (\text{mod 2})$

    where $x$ is the input string that we use as queries to the oracle, to find the secret string.

    The classical approach is to apply 

    $\hspace{1cm} x = 2^i, \;\; \forall i \; \in \; [0, n-1]$

    requiring *n* trials, where *n* is the number of bits in the secret string. This clearly has a runtime linear in *n*, while the Bernstein-Vazirani algorithm requires only one call to the function $f_s(x)$. It follows theoretically that the runtime is constant with respect to the size of *s*. In practice, however, the physical characteristics and limitations of the hardware cause the runtime to increase with increasing number of qubits. 

    For a graphical view of this circuit, see [here](https://algassert.com/quirk#circuit=%7B%22cols%22%3A%5B%5B%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B1%2C1%2C1%2C%22H%22%5D%2C%5B1%2C1%2C1%2C%22Z%22%5D%2C%5B1%2C%22%E2%80%A2%22%2C1%2C%22X%22%5D%2C%5B1%2C1%2C%22%E2%80%A2%22%2C%22X%22%5D%2C%5B%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B%22Measure%22%2C%22Measure%22%2C%22Measure%22%5D%5D%7D).

- Python notebook

    The following Python notebook sets up a 2-by-2 experiment: it runs the Bernstein-Vazirani algorithm with a 14-qubit secret, and a 16-qubit secret; where both noise-free as well as noise-enabled conditions are examined:

    [qbOS_BV.ipynb](Combined%20Documentation%20(draft)%20210902/qbOS_BV.ipynb)

### Example 4: Quantum Utility with Deutsch-Jozsa Algorithm

- Theory

    The [Deutsch-Jozsa algorithm](https://en.wikipedia.org/wiki/Deutsch%E2%80%93Jozsa_algorithm) was the first example of a quantum algorithm that performed better than its best classical counterpart. 

    In our problem, we are given a secret Boolean function $f$, which takes a string of bits as input and returns either a 0 or 1 i.e.

    $\hspace{1cm}f(\{x_0, x_1, x_2, ...\}) \rightarrow \text{0 or 1, where } x_n \text{ is 0 or 1}$

    The property of the above Boolean function is that it is either balanced or constant. A balanced function will return 0's for exactly half of the inputs and 1's for the other half, whereas a constant function will return all 0's or all 1's for any input. The problem is to determine the nature of this function $f$ i.e. whether it is balanced or constant.

    Classically, in the best case, we require two queries to the oracle and in the worst case, we would require half of all possible inputs plus one query to ascertain the nature of $f$.

    The advantage that the Deutsch-Jozsa algorithm provides is that only a single query to $f$ is required to determine if the Boolean function is balanced or constant with 100% confidence! If $f$ is constant the final state consists entirely of "0" but if it is balanced then the final state contains at least one "1".

    Balanced functions may be identified by a wrapping string. The following circuit performs the Deutsch-Jozsa algorithm on a balanced function whose wrapping string is "101": see [here](https://algassert.com/quirk#circuit=%7B%22cols%22%3A%5B%5B1%2C1%2C1%2C%22X%22%5D%2C%5B%22H%22%2C%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B%22X%22%2C1%2C%22X%22%5D%2C%5B%22%E2%80%A2%22%2C1%2C1%2C%22X%22%5D%2C%5B1%2C%22%E2%80%A2%22%2C1%2C%22X%22%5D%2C%5B1%2C1%2C%22%E2%80%A2%22%2C%22X%22%5D%2C%5B%22X%22%2C1%2C%22X%22%5D%2C%5B%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B%22Measure%22%2C%22Measure%22%2C%22Measure%22%5D%5D%7D).

- Python notebook

    [qbOS_DJ.ipynb](Combined%20Documentation%20(draft)%20210902/qbOS_DJ.ipynb)

    - **Concepts covered in the Jupyter notebook**:
        - Circuit generation
        - Execution of an array of circuits
        - Extracting classical execution time
        - Extracting estimated quantum hardware execution time
        - Visualising Quantum Utility

# 3. Quantum Utility/Advantage: Obtaining Timing and Benchmarking Data

## 3.1. Random Quantum Circuits Benchmarking

### 3.1.1 [Optional Reading] What is "random quantum circuits" sampling?

Given an $n$-qubit quantum register, one can construct a circuit of depth $d$, where at each cycle one- and two-qubit gates are (pseudo-)randomly applied to different qubits. This is, in fact, the minimal requirement to create a pseudo-random circuit, which we refer to as $U(n,d)$. Let us denote the distribution of observing bit-string $x$, in the computational basis, in the output of $U(n,d)$ with $p_{U(n,d)}(x)$. Interestingly, for large enough depth, $d$>>1, the distribution $p_{U(n,d)}(x)$ approaches the exponential or Peter-Thomas distribution. See, for example, [Boxio et al. 2018](https://doi.org/10.1038/s41567-018-0124-x) to understand how large $d$ should become and more information on random quantum circuits sampling.

It has been argued sampling from the distribution $p_{U(n,d)}$ requires classical computational resources that scale asymptotic exponential with the circuit size, while it scales linearly for a quantum computer with small enough error rates. Due to inherent and extreme difficulties for classical computers to sample $p_{U(n,d)}$, it is typically the problem of choice to demonstrate quantum supremacy on near-term devices. In fact, both groundbreaking theory work by Boxio et al. 2018 and experimental work by the Google AI team has exploited random quantum circuits sampling to characterise quantum supremacy.

The metric employed to demonstrate the quality of algorithm $A$ to (classically or quantumly) sample $p_U(x)$ is usually a variant of *experimental* *cross-entropy difference:*

$$⁍$$

Here, $p_U$ is the distribution from the ideal rand circuit $U$, $p_A$ is its experimental implementation using $A$, $H$ denotes the cross-entropy between two, and $H_0$ is a fixed circuit-size-dependent quantity. 

$\alpha(p_A, p_U)$ is unity for ideal random quantum circuits and zero for an uncorrelated distribution over bit-strings (see Boxio et al. 2018 for more details). Since precisely estimating $p_U$ even for moderate-size quantum circuits is a difficult problem itself, currently, in QBQE we report $|1-\alpha(p_A,p_A)|$ as our quality metric for more simplicity. Although, general values of $|1-\alpha(p_A,p_A)|$ bare no special meaning, it must approach zero when $p_A \rightarrow p_U$.

### 3.1.2 Strategy to collect the scaling data

In practice, one strategy to collect timing data to demonstrate or predict the occurrence of quantum supremacy is as follows:

1. Consider a range of qubit numbers, {$n$}.
2. Find a large enough depth for each qubit number, so, the algorithm $A$ can experimentally generate random circuits satisfying a desired cross-entropy difference criteria such as $|1-\alpha(p_A,p_A)| \leq 0.05$. Keep in mind harder criteria mean larger runtime for the classical simulator, but also that the quantum supremacy will be achieved with at a smaller $n$. One may need to individually study the behaviour of $|1-\alpha(p_A,p_A)|$ versus $d$ for each $n$-value to ensure the convergence to a fixed cross-entropy difference is achieved. The idea is to ideally increase depth slowly and stop as soon as the criterion $|1-\alpha(p_A,p_A)| \leq 0.05$ is achieved for a fair comparison of the runtimes. However, for more practicality, we suggest choosing a *formula* ensuring large enough depth so that the convergence is guaranteed for all (but especially largest) $n$-values. It might be useful to know, theoretically, the required depth for our purpose scales as $\sqrt{n}$. Therefore, the formula can be something similar to $d = a_0 n^{a_1} + a_2$ with $a_1\geq0.5$. 
3. Measure the runtime (wall-clock time) of a highly-optimised stochastical classical simulator (such as qbOS) to reach $|1-\alpha(p_A,p_A)|$ < 0.05 for sampling $p_{U(n,d)}$. Notice that increasingly large enough number of experiments (repetitions) is required to satisfy the criteria as $n$ increases.
4. Estimate the runtime on the equivalent quantum device with accurate knowledge on the timing for qubit initialisation, native one- and two-qubit gates, and readout.
5. Plot data from the previous two steps for sanity checks and comparison purposes. If it was possible to go to large enough $n$-values, the linear quantum timing curve (surely on top the classical curve for small qubit numbers) must cross the exponential classical timing curve and define when quantum supremacy happens. Even if the qubit numbers are not large enough, it must be possible to accurately fit linear and exponential functions to timing data to predict the occurrence of quantum supremacy.       

### 3.1.3 Using qbOS to obtain the scaling data (no coding required)

qbOS v1.0 comes with the tools to allow users to generate (pseudo-)random quantum circuits at will and even collect the aforesaid scaling data with no conventional or quantum programming required.

As a reminder, if the purpose is to sample random quantum circuits and read the timing and $|1-\alpha(p_A,p_A)|$ values for a single $n$-value, simply run

```bash
[DEPRECATED AFTER SEPT 2021] qbqe --random=40 -s1024 -r256 -n -q9
```

Make sure that you have downloaded and compiled all required components (by running `source install-r.sh --only-dependencies` and similar as described in Sec. 1.4) before running the above.

This shall sample random quantum circuits with noise and minimal depth of 40 (since qbOS always transpiles the gates to our hardware native ones and perform some optimisations and placements the actual depth is equal or larger than the requested value), 1024 shots, 256 experiments (each with a different random circuit), and 9 qubits. You may change these numbers to your desired values. The critical final lines of the output will be similar, but not equivalent, to the following

```bash
* |1-XEB_diff(sigma_exp,sigma_exp)| is estimated to 0.1265370168819966
* For all experiments performed: Max depth gate time + Initialisation time + Readout time, in ms/shot: 1280.05
which for 1024 shots will take, in ms: 1.31077e+06
4.0 Transpiled output in OpenQASM format stored in file: QBCIRCUIT.inc
```

which contains the required information of $|1-\alpha(p_A,p_A)|$, the total quantum run-time, and where the last random circuit is saved in OpenQASM 2.0 format to provide an example of the generated random circuits.

More interestingly, if you would like to directly get the timing data for a range of qubit numbers, simply run this script

```bash
source /mnt/qb/share/qbqe/04_RandomCircuits_benchmarking/RandomCircuits-get_scaling_data.sh 4 15
```

```bash
source /mnt/qb/share/qbqe/04_RandomCircuits_benchmarking/RandomCircuits-get_scaling_data.sh 4 15
```

where the first input is the initial qubit number and the next one is the final qubit number (you may change these as desired, but keep in mind these are typically very lengthy calculations). These will automatically sample the quantum random circuits with noise for 

$n = 5,6,7,\cdots,15$, 

$d = n+40$, 

1024 shots, 

256 experiments, 

and checking the criteria of $|1-\alpha(p_A,p_A)|$ < 0.05. 

This information is hard-coded in the BASH script; if you are interested in other values and criteria, you may create your own modified script from this example with the updated values or contact our support for more flexible options. The script also produces an output data file, RandomCircuits_scaling.dat: the first column is the qubit number, the second is the classical wall-clock time in second, the third is the quantum runtime in seconds, and the last prints a "yes" or "no" to indicate if the criterion is satisfied. An example data file would look like

```bash
4	45.366150	1048.6200	yes
5	59.135038	1310.7700	yes
6	75.876013	1572.9100	yes
7	111.449461	1835.0800	yes
8	205.427017	2097.2300	yes
```

One can accurately plot these timing data and predict the occurrence of quantum supremacy by fitting the relevant functions (in this particular case happening with just 15 qubits).

![Combined%20Documentation%20(draft)%20210902/RandomCircuits_scaling.png](Combined%20Documentation%20(draft)%20210902/RandomCircuits_scaling.png)

# 4. qbOS Modules and Applications

## 4.1 Quantum Fourier Transform (QFT)

The quantum Fourier transform is the quantum implementation of the discrete Fourier transform over the amplitudes of a wavefunction.

The discrete Fourier transform maps a vector $(x_0, x_1, ..., x_{N-1})$ to the vector $(y_0, y_1, ..., y_{N-1})$ given by the transformation

$$y_k = \frac{1}{\sqrt{N}} \sum_{j=0}^{N-1} x_j \omega_N^{jk}$$

where $\omega_N^{jk} = e^{2 \pi i \frac{jk}{N}}$.

In a similar fashion, QFT transforms a quantum state $\sum_{i=0}^{N-1} x_i |i \rangle$ to the quantum state $\sum_{i=0}^{N-1} y_i |i \rangle$ given by

$$|x \rangle \rightarrow \frac{1}{\sqrt{N}} \sum_{y=0}^{N-1} \omega_N^{xy} |y \rangle$$

The corresponding unitary matrix is then given by

$$U_{QFT} = \frac{1}{\sqrt{N}} \sum_{x=0}^{N-1} \sum_{y=0}^{N-1} \omega_N^{xy} |y \rangle \langle x |$$

It is a good point to recall that QFT transforms between two bases, i.e. the computation (Z) basis and the Fourier basis. For example, a single-qubit QFT is the Hadamard gate operation, since it transforms between the *Z*-basis ($|0 \rangle$ and $|1 \rangle$) and the *X*-basis ($|+ \rangle$ and $|- \rangle$).

By default, running `qbos` with no input file runs a 4-qubit [QFT](https://en.wikipedia.org/wiki/Quantum_Fourier_transform).

```bash
qbos
```

For a graphical view of this circuit, see [here](https://algassert.com/quirk#circuit=%7B%22cols%22%3A%5B%5B%22%E2%80%A6%22%2C%22%E2%80%A6%22%2C%22%E2%80%A6%22%2C%22%E2%80%A6%22%2C%22%E2%80%A6%22%2C%22%E2%80%A6%22%2C%22%E2%80%A6%22%2C%22%E2%80%A6%22%5D%2C%5B%22Swap%22%2C1%2C1%2C%22Swap%22%2C%22QFT4%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22H%22%5D%2C%5B1%2C%22Swap%22%2C%22Swap%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22%E2%80%A2%22%2C%7B%22id%22%3A%22Z%5Eft%22%2C%22arg%22%3A%221%2F2%22%7D%5D%2C%5B%22H%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22H%22%5D%2C%5B%22Z%5E%C2%BD%22%2C%22%E2%80%A2%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22%E2%80%A2%22%2C1%2C%7B%22id%22%3A%22Z%5Eft%22%2C%22arg%22%3A%221%2F4%22%7D%5D%2C%5B1%2C%22H%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22%E2%80%A2%22%2C%7B%22id%22%3A%22Z%5Eft%22%2C%22arg%22%3A%221%2F2%22%7D%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22H%22%5D%2C%5B%22Z%5E%C2%BC%22%2C%22Z%5E%C2%BD%22%2C%22%E2%80%A2%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22%E2%80%A2%22%2C1%2C1%2C%7B%22id%22%3A%22Z%5Eft%22%2C%22arg%22%3A%221%2F8%22%7D%5D%2C%5B1%2C1%2C%22H%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22%E2%80%A2%22%2C1%2C%7B%22id%22%3A%22Z%5Eft%22%2C%22arg%22%3A%221%2F4%22%7D%5D%2C%5B%22Z%5E%E2%85%9B%22%2C%22Z%5E%C2%BC%22%2C%22Z%5E%C2%BD%22%2C%22%E2%80%A2%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22%E2%80%A2%22%2C%7B%22id%22%3A%22Z%5Eft%22%2C%22arg%22%3A%221%2F2%22%7D%5D%2C%5B1%2C1%2C1%2C%22H%22%2C1%2C1%2C1%2C1%2C%22H%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22Swap%22%2C1%2C1%2C%22Swap%22%5D%2C%5B1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C1%2C%22Swap%22%2C%22Swap%22%5D%5D%2C%22init%22%3A%5B1%2C1%2C0%2C0%2C1%2C1%2C0%2C0%2C1%2C1%5D%7D).

## 4.2 Grover Algorithm

The Grover algorithm was originally a method for finding a given entry in a database [] but it has since been generalised to finding extremal values of functions. This in turn has a multitude of applications, including finding optimal strings of characters given a measure, which we later illustrate in the quantum decoder.

To perform a Grover iteration the grover() method is called with

```jsx
import grover as gr
import exponential_search as es

grovered_state = gr.grover_iteration(initial_state,score_qubits,target_value,comparator)
optimal_value = es.measurement(qubits_of_interest, grovered_state**)    #Almost all the time**
```

Note that the Grover iteration is designed to return a state for which the desired component has an amplitude much greater than all the other components combined, so that the correct answer is overwhelmingly likely to be obtained upon measurement. This is not guaranteed. 

The effectiveness of the Grover iteration is also affected by the distribution of the scores and of the probability amplitudes within the initial state.

The input parameter *comparator* specifies the quantum circuit which compares the qubits specified by *score_qubits* to *target_value* and returns 1 if the corresponding component should be favoured by the Grover iteration and 0 otherwise. The only currently implemented option is "greater". The options "lesser" and "equal" may be added later. *NOTE THAT THIS API FOR THE GROVER ITERATION IS NOT YET IMPLEMENTED.*

## 4.3 Hybrid Optimisation Modules

## 4.4 Quantum Chemistry Application

## 4.5 Quantum Decoder Application for NLP

The decoder is an algorithm that, given a string of symbols such as letters of the alphabet, and a probability table or some other form of the sampler, will predict the subsequent symbols. The applications for such an algorithm range from text autofill to autonomous pathfinding. The quantum decoder encodes a superposition of all possible strings with a score, currently the probability of each letter, and then uses an exponential search algorithm to manipulate the probability amplitudes so that the best scoring string is the one most likely returned upon measurement.

It currently uses a classical sampler but a quantum version will be needed eventually. The easiest way to use the quantum decoder the user is to invoke the call_quantum_decoder() method in the [QuantumDecoder.py](http://quantumdecoder.py) module:

```python
import QuantumDecoder as qd
decoder = qd.quantum_decoder(S,N_string,total_length,table,precision,Alphabet,
				grammar,Starting_String = '!', search_cutoff = 0.9, Encoding = "Bitstring")

decoded_state = decoder.state
optimal_string = es.measurement(qubits_of_interest, decoded_state**)    #Almost all the time**
```

The quantum decoder is based on the Grover iteration and returns a state for which the most suitable string has an amplitude much greater than all the other components combined so that the correct string is overwhelmingly likely to be obtained upon measurement. This is not guaranteed. 

**WARNING**
`aer` accelerator has some known incompatibilities and issues with the VQE and QAOA modules for some specific Hamiltonian inputs. If you are facing such problems, please choose `--acc="qpp"` and `--acc="tnqvm-exatn-mps"` as your classical simulation backends. By *default*, the accelerator is set to `qpp` for both modules.  

## 4.6 Variational Quantum Eigensolver (VQE)

VQE is a heuristic, yet powerful, quantum-classical algorithm that can be used to find eigenvalues and eigenvectors of a square matrix, $H$. VQE is designed for problems where $H$ is a hermitian Hamiltonian of a physical system, in which case it finds the ground state energy. In this algorithm, a quantum subroutine is performed inside a *classical optimization loop*, where it prepares a quantum state (corresponding to a circuit called *ansatz*) and measure Hamiltonian's *expectation values*. See [[Peruzzo et al. 2013](https://arxiv.org/abs/1304.3061), [O'Malley et al. 2015](https://arxiv.org/abs/1512.06860)] for details.

qbOS v1.0 comes equipped with built-in options, initial parameters, and optimisation settings to efficiently run VQE iterations with no coding required.  Your Hamiltonian may be specified either as:

- [Deprecated since March 2021] a dense matrix (file input); or,
- a sum of Pauli terms (string input command-line option)

In the following, we exclusively show VQE options currently included in our main tool `qbos`. You may check these by requesting `qbos` help message.   

```bash
qbos --help  **  
```

### 4.6.1 Available Options

**Ansatz parameters** — As in all parameterized models, choice of model architecture, or *ansatz*, is an important parameter in the success of the optimisation. Arguably, two ansatze have appeared most useful for on-demand VQE tasks -- [ASWAP](https://arxiv.org/abs/1904.10910) and the [Hardware Efficient Ansatz](https://www.nature.com/articles/nature23879) (HEA). ASWAP is well-suited to the types of Hamiltonians generated by quantum chemistry problems and is parameterized by the number of particles to use. HEA provides a framework for general Hamiltonians where the problem substructure is less well defined. HEA is often parameterized by the number of base circuit repetitions, where a larger number allows for modelling more complexity at the cost of increasing training difficulty. One instance of HEA ansatz is demonstrated in [Pennylane's VQE tutorial](https://pennylane.ai/qml/demos/tutorial_vqe.html). Before looking at an optimization parameter, we first have to choose

- *VQE ansatz*

```bash
--vqe-ansatz=[vqe_ansatz]
               
																				--vqe-ansatz="aswap" sets the ansatz for
                                        VQE algorithm to ASWAP, default: pennylane
```

- *initial angles,* $\vec{\theta}_{\rm init}$

```bash
--vqe=[vqe] 
                      
                                        --vqe="0 0.11" runs a two-parameter VQE
                                        algorithm with initial parameters
                                        theta0=0 and theta1=0.11. The user has
                                        to specify the Hamiltonian using the
                                        --vqe-ham option.
```

- *excitation/particle number* *sector* **(⚠️ ASWAP ANSATZ ONLY)**

```bash
--vqe-aswap-particles=[vqe_aswap_particles]

                                        --vqe-aswap-particles=2 sets number of
                                        particles/excitations for the ASWAP
                                        ansatz of VQE to 2, default=1
```

- *VQE depth — number of repetitions of base circuit* **(⚠️ PENNYLANE ANSATZ ONLY)**

```bash
--vqe-depth=[vqe_depth]               

																				--vqe-depth=10 sets the circuit depth
                                        for depth-parametrized ansatz of VQE to
                                        10, default=3
```

**Optimiser parameters** — The classical optimiser used with VQE accepts the following settings:

- *Method*

```bash
--hybrid-optimizer-method=[hybrid_optimizer_method]
(Previously called: --vqe-optimizer-method)

                                        --hybrid-optimizer-method="cobyla" will
                                        set the classical optimizer method to
                                        COBYLA nonlinear optimizations. You may
                                        choose between "cobyla", "l-bfgs", and
                                        "nelder-mead", default="nelder-mead"
```

- *Tolerance*

```bash
--hybrid-optimizer-functol=[hybrid_optimizer_functol]
(Previously called: --vqe-optimizer-functol)

                                        --hybrid-optimizer-functol=0.001 sets
                                        function tolerance for the classical
                                        optimizer of VQE and QAOA to 0.001,
                                        default=1e-6
```

- *Max evaluations*

```bash
--hybrid-optimizer-maxeval=[#hybrid_optimizer_maxeval]
(Previously called: --vqe-optimizer-maxeval)

                                        --hybrid-optimizer-maxeval=500 sets
                                        maximum number of function evaluations
                                        for the classical optimizer of VQE and
                                        QAOA to 500, default=200
```

- *Stochastic sampling size to use when determining expected values*

```bash
-s[#samples]                        -s2048 sets 2048 samples, default: no stochastic sampling
```

- *Enable QB noise model*

This option automatically forces the use of the **aer** backend.  An important effect of this is that bit-strings will print out in **reverse order** compared to the default qpp backend.

```bash
-n, --noise                               Enable QB noise model, a simulation of
                                          noise sources within the QB hardware
                                          and their effect on results. The noise
                                          has three main sources, internal
                                          thermal and magnetic fluctuations, and
                                          also fluctuations in the control
                                          mechanism. The inputs for the
                                          noise-model are already hard-coded
                                          with realistic parameters. 
                                          Default: simulation results are noise-free
```

### 4.6.2 Hamiltonian input: a string of Pauli term summations

The Hamiltonian must be specified from the `qbqe` command by using the `—vqe-ham` option.  Here is an example of this:

```bash
qbos -q4 --vqe="0.11" --vqe-ansatz="aswap" --vqe-aswap-particles=1 --hybrid-optimizer-maxeval=100 --hybrid-optimizer-functol=1e-5 --hybrid-ham="5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + (0.3,0.21829) Z0 + 0.34 Z0Z1Z2Z3"
```

The `-q` option must be used with this and must equal the number of qubits that have Pauli terms applied to them (in the string that you supply to `—-hybrid-ham`).

Also, notice that the above Hamiltonian serves as a generic example while containing no physical meaning neither any meaningful properties.

The user must ensure that **'+' or '-' operators** between the Pauli terms (of the string supplied to`--hybrid-ham`) are **space-separated.**

This will produce results similar to this:

```bash
* Mode used in backend: no sampling

* Selected accelerator backend: qpp

## 0.0 Running VQE algorithm with the following inputs:

    Input Hamiltonian string:

        5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + (0.3,0.21829) Z0 + 0.34 Z0Z1Z2Z3

    Ansatz: ASWAP (VQE module will now utilise the universal ansatz of
                   ASWAP -- see https://arxiv.org/abs/1904.10910 for details.)
            Number of particles/excitations is set to 1

    Input initial theta: 0.11 0.11 0.11 
    Classical optimizer: nelder-mead
                         Maximum function evaluation is set to 100
                         Function tolerance is set to 1e-05

## 1.0 IR: 
## 2.0 After placement + optimizer: 
## 3.0 VQE algorithm results:

    Ground Energy: 1.26992
    Optimized theta: {0.59031, 1.07749, 0.956929}
    Predicted eigenstate (assuming a classical Hamiltonian): 0001
```

### 4.6.3 Convergence information

If the `—verbose` option is specified at the command line, then the per-iteration information for the energy (`vqe-energy-trace`) and ansatz parameters (`vqe-ansatz-parameter-trace`) is output to the JSON file named: `QBINFO.json`.  This file's contents should resemble the text shown below:

```json
{
    "hybrid_lower_bound": null,
    "optimal_states": null,
    "output_amplitudes": null,
    "output_probabilities": null,
    "qaoa_steps": null,
    "vqe-ansatz-parameter-trace": [[0.11, 0.11, 0.11], [0.11, 0.11, 1.6807963000000001], [0.11, 1.6807963000000001, 0.11], [1.6807963000000001, 0.11, 0.11], [1.1571975333333335, -1.4607963, 1.1571975333333335], [0.37179938333333334, 0.89539815, 0.37179938333333334], [-1.2862633777777779, 0.6335987666666667, 1.3317304555555558], [0.9390313805555556, 0.24089969166666667, 0.4154326138888889], [0.8372205092592592, 0.7208652277777777, -1.0826416351851853], [0.2918051273148148, 0.2627163069444444, 0.9899368162037038], [0.9584239274691356, 0.8226760990740739, 1.0747792089506172], [0.1423209115226335, 1.079627345679012, 1.2089109917695473], [0.6898910209019201, 1.6024180895576128, 0.7803895731652947], [1.2044219762802926, 1.1340342134087793, 0.2757344518632827], [0.4078461777120483, 1.0932290626114538, 0.9756168567929812], [0.4688219714410914, 0.27178411823273896, 0.8344073928859929], [0.6346237585367129, 1.2697595967263944, 0.7938940280954692], [-0.015577714414405919, 1.3495817738178246, 0.35276096986390515], [0.7149235169982502, 0.9544025177600115, 0.8942746491789392], [0.3617556268257083, 0.6922602235212489, 0.7005665647746999], [0.5664067256089618, 1.125384753425108, 0.7705621622652769], [0.6942402395816485, 0.8902278848452925, 0.3821406063920517], [0.4794446931794484, 1.0424787681699135, 0.8272477941927487], [0.23017701774957866, 1.0877719299700026, 0.4187982440153001], [0.5937368921860823, 0.9877448708125093, 0.7754055478880294], [0.3969139201902808, 0.8250297725631739, 0.5457396546774641], [0.6082642870372077, 1.0081041243637308, 1.060462614505495], [0.549148061111239, 0.9799276307727982, 0.8882968067124545], [0.3566008908012299, 0.9105459101914143, 0.7321172891670821], [0.5344528918398692, 0.9684451306572355, 0.7645834832077926], [0.6451165105634237, 1.1688712471701241, 1.1076790680645332], [0.7692178057499951, 1.3407919844735994, 1.3886487747580676], [0.6730336158302394, 1.0356839042301917, 1.013125111130438], [0.5278419238421461, 1.040780052184983, 0.873717123427171], [0.5891261563857201, 1.1388033225687637, 0.9423563097538764], [0.5591425849298592, 1.0196465537217896, 0.90181168247281], [0.6202811210504169, 1.1844201047273624, 1.1575550994352164], [0.5559099491425061, 1.0224388741747672, 0.8628263872646486], [0.6456041059150466, 1.099857731192804, 1.0411609684408234], [0.5572824693603713, 1.0555494719369383, 0.9155780846805841], [0.6130633677810082, 1.1449265084660967, 1.0222440108670339], [0.5726227806426465, 1.0509665424078665, 0.9319197645713659], [0.6251503575386796, 1.1059683038982333, 1.0193720619197808], [0.6726831500206605, 1.1947651881427153, 1.176487542439138], [0.5851032493620447, 1.065520452666754, 0.9412416760582709], [0.5434677477988236, 0.979432285478445, 0.8206766003017449], [0.6197043198722736, 1.1215115067472043, 1.0359284511238362], [0.5598032090459635, 1.0526973639829829, 0.9200211992492011], [0.6088135704155007, 1.0926505689194208, 0.9945343462521359], [0.6364579791238993, 1.1354884764810529, 1.0492165510514626], [0.6005455460620228, 1.0742614919642806, 0.9540665977840765], [0.614914626419711, 1.1096990030514733, 1.0154629877888963], [0.6155036661882693, 1.1144880525467658, 1.0094131303469507], [0.573889715522784, 1.0576498623622759, 0.928195311744616], [0.5895317814230628, 1.0771095158919701, 0.9584506215713277], [0.5785111715625404, 1.061713011018853, 0.9239406308621367], [0.5532604687101625, 1.0217406005049523, 0.8730088219808729], [0.5999428668187426, 1.0913011895363125, 0.9753120532554312], [0.6045407601733597, 1.0942410943778378, 0.99272893639455], [0.5850185687152452, 1.0698450318585992, 0.94113770724524], [0.5978922286093222, 1.0933167055245003, 0.9753585786563952], [0.5883004941738641, 1.0724695158811905, 0.949770901707802], [0.5926428383821717, 1.0786343089587644, 0.952363153234321], [0.59030954566284, 1.0774907141586687, 0.9569287544870759], [0.5758095388825567, 1.0552356517293262, 0.9232461890379807], [0.5939095348346961, 1.082284805084566, 0.9622955872010686]],
    "vqe-energy-trace": [4.929978983725535, 5.859239431336859, 6.097905154044288, 5.866980998906481, 8.703579692657904, 1.3439449197787714, 7.102732625601551, 4.827837939734074, 7.34155225942156, 4.234837799373533, 2.48881844129329, 3.180222168194888, 2.665407007228578, 4.690632453646885, 1.519259827014301, 3.960273105292483, 1.6208127330464648, 3.6192136131462096, 1.4950674232498384, 1.6685600905478277, 1.375942108320854, 2.1333103085703025, 1.2952296407596018, 1.6811771986533608, 1.3234544005072015, 1.3196200065875814, 1.3381276478565196, 1.2860896053742987, 1.345171960845517, 1.2826809413246163, 1.2707753758073315, 1.2929326060790955, 1.3286646434532179, 1.274251144549426, 1.290705449274372, 1.273111574591681, 1.290549607267605, 1.270894183195555, 1.2771270533581243, 1.2712753928759901, 1.273994131900568, 1.2706470648251027, 1.2707367939404015, 1.273221049911332, 1.269985389711438, 1.277712280338958, 1.2702868871070008, 1.271031786335561, 1.270194213822031, 1.270021719223232, 1.270743394730617, 1.2700154286136724, 1.2699972453447952, 1.270017458704567, 1.2699559718589495, 1.2699861893496305, 1.2705230248457138, 1.2699403108457368, 1.2700567825323872, 1.269926587508123, 1.2700584775429886, 1.2699239837135825, 1.2700091030804592, 1.2699213066001365, 1.2699731561377725, 1.2699213464948533],
    "vqe_aswap_particles": null
}
```

### 4.6.4 Example 1: two-qubit deuteron Hamiltonian

Let us now focus on a physical Hamiltonian matrix, where the ground state energy is known. The following represents $N=2$ (two-qubit) deuteron Hamiltonian: `5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1`. The energy is known to be $-1.74886$ in sector $q=1$ and in the same energy unit as the Hamiltonian elements.

Now, imagine you intend to run VQE for the above with initial angle $\theta_1=0.11$, choosing the particle sector $1$, setting the classical optimiser to the nonlinear [Nelder-Mead](https://en.wikipedia.org/wiki/Nelder%E2%80%93Mead_method) method with $100$ maximum function evaluations and $1.0e-5$ function tolerance. Then you simply run

```bash
qbos -q2 --vqe="0.11" --hybrid-ham="5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1" --vqe-aswap-particles=1 --hybrid-optimizer-maxeval=100 --hybrid-optimizer-functol=1e-5 --hybrid-optimizer-method="nelder-mead" --vqe-ansatz="aswap"

```

### 4.6.5 Example 2: a four-qubit classical Hamiltonian

Many graph optimization problems can be written in terms of a classical Hamiltonian, where it only depends on the spin in the $Z$-direction. For this, we recommend using either a QAOA ansatz if there is exploitable substructure (e.g. a fixed Hamming distance of solution), or HEA (`pennylane` ansatz here) for generic problems. 

As an example, for the VQE Hamiltonian string given by `0.04207897647782276 + 0.17771287465139946 Z0 + 0.1777128746513994 Z1 + -0.24274280513140462 Z2 - 0.17059738328801052 Z0Z1 + 0.6622334 Z0Z1Z2Z3` with minimum eigenvalue $-1.38892036...$ we can use HEA to find the minimum eigenvalue with the command:

```bash
qbos -q4 --vqe="0" --hybrid-ham="0.04207897647782276 + 0.17771287465139946 Z0 + 0.1777128746513994 Z1 + -0.24274280513140462 Z2 - 0.17059738328801052 Z0Z1 + 0.6622334 Z0Z1Z2Z3" --hybrid-optimizer-maxeval=300 --vqe-depth=4

```

Producing output:

```bash
    Ground Energy: -1.38892
    Optimized theta: {0, 0, 0, 0, 0, 0, 0, 1.5708, 1.5708, 0, 1.5708, 1.5708, 0, 1.5708, 1.5708, 0, 1.5708, 1.5708, 0, 1.5708, 1.5708, 1.5708, 1.5708, 1.5708, 0, 0, 1.5708, 0, 1.5708, 1.5708, 0, 0, 1.5708, 0, 1.5708, 1.5708, 0, 1.5708, 1.5708, 0, 1.5708, 1.5708, 0, 1.5708, 1.5708, 1.5708, 1.5708, 1.5708}
    Predicted eigenstate (assuming a classical Hamiltonian): 1011
```

Since this Hamiltonian has a two-fold degenerate ground state, currently, your result may point to "Predicted eigenstate (assuming a classical Hamiltonian): 0111" with the same energy — both answers are valid. You can use the `--verbose` option to view the counts on all detected states, which may help you to identify the degeneracy in some cases.

### 4.6.6 Example 3: same setup as for Example 2, except specifying a file that contains a user-defined ansatz

This functionality requires using files written in the XASM format.  An example of this is shown below:

**my-example-ansatz.xasm**

**Important**

Do **not** change the first two lines:
`.compiler xasm
 .circuit d_ansatz`

You can use the parameter (declared here as `theta`) as a 0-indexed array, ie `theta[0], theta[1], theta[2]`, ..., etc.

```bash
.compiler xasm
.circuit d_ansatz
.parameters theta
.qbit q
  X(q[0]);
  X(q[1]);
  U(q[0], theta[0], theta[1], theta[2]);
  U(q[1], theta[3], theta[4], theta[5]);
  U(q[2], theta[6], theta[7], theta[8]);
  U(q[3], theta[9], theta[10], theta[11]);
  CNOT(q[0], q[1]);
  CNOT(q[1], q[2]);
  CNOT(q[2], q[3]);
  U(q[0], theta[12], theta[13], theta[14]);
  U(q[1], theta[15], theta[16], theta[17]);
  U(q[2], theta[18], theta[19], theta[20]);
  U(q[3], theta[21], theta[22], theta[23]);
  CNOT(q[0], q[1]);
  CNOT(q[1], q[2]);
  CNOT(q[2], q[3]);
```

Now call `qbos`:

```bash
qbos -q4 --verbose --vqe-ansatz="my-example-ansatz.xasm" --vqe="0" --hybrid-ham="0.04207897647782276 + 0.17771287465139946 Z0 + 0.1777128746513994 Z1 + -0.24274280513140462 Z2 - 0.17059738328801052 Z0Z1 + 0.6622334 Z0Z1Z2Z3" --hybrid-optimizer-maxeval=300

```

The screen output will resemble the following:

```bash
.
.
.
## 3.0 VQE algorithm results:

    Ground Energy: -1.38857
    Optimized theta: {-0.00649653, 0.678851, 0.0848913, 0.0436693, 0.0117668, -0.128289, -0.00951194, 0.249633, 0.228561, -0.161612, 1.3279, 0.141406, 0.00449596, 0.127039, 0.507278, 0.0240884, 0.699826, 2.93368, 3.06813, 3.06954, 3.0206, 2.98477, 2.52891, 2.00765}
* Pauli observable for optimal ansatz evaluation: "1.0 Z3Z2Z1Z0"
* Optimal circuit:
X q0
X q1
U(-0.00649653,0.678851,0.0848913) q0
U(0.0436693,0.0117668,-0.128289) q1
U(-0.00951194,0.249633,0.228561) q2
U(-0.161612,1.3279,0.141406) q3
CNOT q0,q1
CNOT q1,q2
CNOT q2,q3
U(0.00449596,0.127039,0.507278) q0
U(0.0240884,0.699826,2.93368) q1
U(3.06813,3.06954,3.0206) q2
U(2.98477,2.52891,2.00765) q3
CNOT q0,q1
CNOT q1,q2
CNOT q2,q3

Measure q3
Measure q2
Measure q1
Measure q0

* State:0001 Count:1
* State:0111 Count:1
* State:1011 Count:1022
    Predicted eigenstate (assuming a classical Hamiltonian): 1011
```

The output file named **QBINFO.json** will contain keys `vqe-energy-trace` and `vqe-ansatz-parameter-trace` which contain values relevant to the convergence properties of the VQE: 

```json
{
"vqe-ansatz-parameter-trace": [-0.031807227082019066, 0.6696013427762451, ...],
    "vqe-energy-trace": [0.421032049018418, 0.421032049018418, ...]
}
```

## 4.7 Quantum Approximate Optimization Algorithm (QAOA)

QAOA is a heuristic algorithm that helps solve combinatorial optimization problems such as the [MAXCUT](https://en.wikipedia.org/wiki/Maximum_cut) problem with an approximate solution close to the actual optimum. Being heuristic in nature, it doesn't have a polynomial runtime guarantee but appears to perform well enough on certain instances of such optimization problems. 

By default, qbOS employs the QAOA algorithm with *extended* angle parameter size, which features more parameters but reduced circuit depth. This scheme requires $(n_{\rm qubits}+n_{\rm nonident-pauli-terms})\times n_{\rm steps}$ angles to be specified.

A similar workflow as the VQE is implemented for QAOA. We start with some initial parameters, measure the state and update the angles to obtain a state which is approximately closer to the ideal solution. Increasing the number of these iterations will yield a solution closer to the optimum compared to the previous iteration.

QBQE v1.0 is also equipped with built-in options, initial parameters, and optimisation settings to efficiently run QAOA iterations with no coding required.  Your Hamiltonian may be specified either as:

- a dense matrix (file input — developers-only option); or,
- a sum of Pauli terms (string input command-line option)

In the following, we exclusively show VQE options currently included in our main tool `qbqe`. You may check these by requesting `qbqe` help message.   

```bash
qbos --help  **  
```

### 4.7.1 Available options

- *initial angles,* $\vec{\theta}_{\rm init}$

```bash
--qaoa=[qaoa] 
                                        --qaoa="0 0.11" runs a QAOA algorithm
                                        with initial parameters set to theta0=0
                                        and theta1=0.11 (the code will match the
                                        other parameters as needed). The user
                                        has to insert the Hamiltonian either as
                                        a matrix (the first file input) or using
                                        --ham option.
```

- *Method*

```bash
--hybrid-optimizer-method=[hybrid_optimizer_method]

                                        --hybrid-optimizer-method="cobyla" will
                                        set the classical optimizer method to
                                        COBYLA nonlinear optimizations. You may
                                        choose between "cobyla", "l-bfgs", and
                                        "nelder-mead", default: "nelder-mead"
```

- *Tolerance*

```bash
--hybrid-optimizer-functol=[hybrid_optimizer_functol]

                                        --hybrid-optimizer-functol=0.001 sets
                                        function tolerance for the classical
                                        optimizer of VQE and QAOA to 0.001,
                                        default: 1e-6
```

- *Max evaluations*

```bash
--hybrid-optimizer-maxeval=[#hybrid_optimizer_maxeval]

                                        --hybrid-optimizer-maxeval=500 sets
                                        maximum number of function evaluations
                                        for the classical optimizer of VQE and
                                        QAOA to 500, default=200
```

- *Stochastic sampling size to use when determining expected values*

```bash
-s[#samples]                        -s2048 sets 2048 samples, default: no stochastic sampling
```

- *Enable QB noise model*

This option automatically forces the use of the **aer** backend.  An important effect of this is that bit-strings will print out in **reverse order** compared to the default qpp backend.

```bash
-n, --noise                               Enable QB noise model, a simulation of
                                          noise sources within the QB hardware
                                          and their effect on results. The noise
                                          has three main sources, internal
                                          thermal and magnetic fluctuations, and
                                          also fluctuations in the control
                                          mechanism. The inputs for the
                                          noise-model are already hard-coded
                                          with realistic parameters. 
                                          Default: simulation results are noise-free
```

### 4.7.2 Example 1: two-qubit Ising Hamiltonian

Let us first consider a classical Ising model with two sites, which can be mapped to a QUBO problem.

```bash
qbos -q2 --qaoa="2" --hybrid-ham="-5.0 - 0.5 Z0 + 1.0 Z0 Z1" --acc="aer" --hybrid-optimizer-method="nelder-mead" --hybrid-optimizer-functol=1e-5 --hybrid-optimizer-maxeval=200 --qaoa-steps=3
```

This will produce results as in the following:

```
Ground Energy: -6.5
Optimized theta: {1.82026, 2.93351, 2.91321, 2.37611, 2.22671, 2.22538, 1.86693, 2.38394, 1.70478, 2.13984, 2.27602, 1.81998}
```

### 4.7.3 Example 2: four-qubit Ising Hamiltonian

Now consider a classical Ising model with four sites, which can be mapped to a QUBO problem.

```bash
qbos -q4 --qaoa="2" --hybrid-ham="-1.5 - 0.5 Z1 + 1.0 Z2 - 0.7 Z0 Z3" --hybrid-optimizer-maxeval=400 --qaoa-steps=3
```

This will produce results as in the following:

```
    Ground Energy: -3.69999
    Optimized theta: {2.94638, 1.94525, 3.0127, 3.04623, 3.0513, 2.01781, 2.15042, 2.10567, 1.94967, 2.96997, 1.96494, 2.02227, 2.3233, 2.04826, 1.99952, 1.96232, 2.68723, 2.25095, 2.16804, 2.10478, 2.12602}
```

# 5. Hardware limitations

### 5.1 Qubit Connectivity Diagram

In the context of quantum programming, we say that two qubits are connected if a CNOT operation can be directly applied to them. By this definition, all qubits on a QB diamond chip are effectively connected and this should be sufficient for most quantum programming applications.  

Direct physical connectivity is much more restricted. Qubits are physically arranged in fully connected clusters of three. The clusters are arranged in a two-dimensional grid with complete connectivity between the qubits of horizontal and vertical nearest neighbours. There are no diagonal physical connections. It follows that there can be no more than six qubits in a physically fully connected registry. 

![Combined%20Documentation%20(draft)%20210902/cluster_grid2.png](Combined%20Documentation%20(draft)%20210902/cluster_grid2.png)

> **Figure:** Each cluster of three qubits lies inside a black circle. Intra-cluster connections are shown in red. Connections between clusters are in orange and split at each end to indicate that every qubit at one end is physically connected to every qubit at the other.

Physically distant qubits can still be connected in the quantum programming context described above, but in practice this may require the equivalent of a large number of swap operations, rendering such a connectionless reliable. 

# 6. Appendices

## Version 1

The goal of this document is to guide the user in the setup of QBOS v1.0, which includes setting up the development environment, implementing a basic algorithm, and common debugging steps. 

## Features & Functionalities

In this preview release, the QBOS v1.0 provides:

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

### 1.3.1 Option 1: For users who don't handle quantum circuits

Proceed to run the `qbqe` command-line tool with well-known protocols: see [Section 2.](https://www.notion.so/Quantum-Emulator-v1-0-User-Guide-for-command-lines-20905c04dd6b4a279cc32a528cdf7bfd) 

## Version 2

# 8. Types and Arrays in QBOS v2

**The design aim of QBQE v2 is to support an array programming syntax**.  

Users who have come from a NumPy, Matlab or Julia background will see the similarity to those environments.  Put simply, QBOS v2 objects have members that are 2D arrays.  **Broadcasting** of values is performed wherever it is consistent to do so prior to execution.

The **leading (or innermost) dimension** is from hereon referred to as the **conditions dimension**

The **second dimension** is from hereon referred to as the **circuits dimension**

A quick way to remind yourself of this is: 
**"traversing rows** changes **conditions"
"traversing columns** changes **circuits"**

Before describing the classes that are provided to support this, let's see how this works with an example - a sweep of the number of shots [sn] for a simple circuit:

## 8.1 Example 2:  |+++> measured using a sweep of 32 shots, 64 shots and 128 shots

```python
import qbemulator as q
tqb = q.qbqe()
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

The result is (similar to) this:

```python
Invoked run()
# OpenQASM compiler: staq
# Qubits: 12
# Shots: 32
# Repetitions: 1
# Output transpiled circuit: enabled
# Noise model: disabled
# tnqvm accelerator: initialised
# Quantum Brilliance accelerator: initialised
# OpenQASM compiler: staq
# Qubits: 12
# Shots: 64
# Repetitions: 1
# Output transpiled circuit: enabled
# Noise model: disabled
# tnqvm accelerator: initialised
# Quantum Brilliance accelerator: initialised
# OpenQASM compiler: staq
# Qubits: 12
# Shots: 128
# Repetitions: 1
# Output transpiled circuit: enabled
# Noise model: disabled
# tnqvm accelerator: initialised
# Quantum Brilliance accelerator: initialised
```

Inspect the results:

```python
tqb.out_raw[0]

```

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

## 8.3 Example 4: A circuit with 2 parameters, running a sweep over 3 conditions

```python
import qbemulator as q
tqb = q.qbqe()
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

The result is this:

```python
Invoked run()
# OpenQASM compiler: staq
# Qubits: 12
Quantum Brilliance OpenQASM pretranspiler
# Shots: 1024
# Repetitions: 1
# Output transpiled circuit: enabled
# Noise model: disabled
# aer accelerator: initialised
# Quantum Brilliance accelerator: initialised
# OpenQASM compiler: staq
# Qubits: 12
Quantum Brilliance OpenQASM pretranspiler
# Shots: 1024
# Repetitions: 1
# Output transpiled circuit: enabled
# Noise model: disabled
# aer accelerator: initialised
# Quantum Brilliance accelerator: initialised
# OpenQASM compiler: staq
# Qubits: 12
Quantum Brilliance OpenQASM pretranspiler
# Shots: 1024
# Repetitions: 1
# Output transpiled circuit: enabled
# Noise model: disabled
# aer accelerator: initialised
# Quantum Brilliance accelerator: initialised
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

# 9. Download and Installation

qbOS consists of a front-end JupyterLab GUI, supported by a container that is run on any of the following platforms.

## 9.1 Local machine

TBA

# 10. Starting a qbOS Jupyter session

## 10.1 **Login** to the VM instance

Once all the necessary VM setup is completed and the instance is launched, it is time to log in.

If you are running Windows 10, first open PowerShell (**Windows button** → **Windows PowerShell**) or some other Shell terminal of your choice and then:

```bash
ssh -i your-ssh-private-key-file -L 8889:localhost:8889 ubuntu@your.ip.ad.dress
```

Please ensure you have `-L 8889:localhost:8889` in your login as shown above.

Enter the password as appropriate (if needed, type 'yes' when asked about the identity of the system).

## 10.2 Launching the web browser GUI interface (JupyterLab)

With a web browser, visit [http://localhost:8889](http://localhost:8889)

# 11. Pricing and Free Trials

TBA

# 12. Glossary of Tools and Quantum Gates

This section is a quick reference on quantum gates expressed in these formats:

- OpenQASM
- XASM
- QB Native
- CZ - controlled-Z
    - Unitary Description and Decomposition Rules
    - OpenQASM
    - XASM
    - QB Native
- CNOT - controlled-X
- H - Hadamard
- X - Pauli rotation on x-axis by pi
- Y - Pauli rotation on y-axis by pi
- Z - Pauli rotation on z-axis by pi
- S - rotation on z-axis by 0.5*pi
- T - rotation on z-axis by 0.25*pi
- I - Identity
- Rx(theta)
- Ry(phi)
- Rz(lambda)
- Measure

Circuit generation snippets for common patterns:

- H on all `qn` qubits
- Measure on all `qn` qubits
- CNOT ladder on all `qn` qubits
- CNOT controls on `qn` qubits, all targeting qubit `qn+1`
- Bernstein-Vazirani algorithm - secret string of `qn` qubits
- Quantum Fourier Transform of `qn` qubits
# qbOS Documentation

> **qbOS version 2 - 220918**
> 

![Quantum_Brilliance_dark_blue_Logo_RGB.png](https://images.squarespace-cdn.com/content/v1/5e54471da3971960f69d9535/a06ff23d-9911-4787-9887-2ff08de7d8c3/logo-quantum-brilliance.png?format=1500w)

<aside>
ℹ️ qbOS (Quantum Brilliance Operating System) is designed for quantum computational tasks and application development

</aside>

[First Release Notes (Sept 2021)](qbOS%20Documentation%20220913/First%20Release%20Notes%20(Sept%202021)%2057042bb79eef463d9ebab68eddcad6e5.md)

# 0. About qbOS

*Q*uantum *B*rilliance *O*perating *S*ystem (qbOS) is our software backbone that provides users with a variety of UI functionalities (see our Jupyter UI, standalone Python API, and CLI for HPC usage), necessary middleware, optimised compilation routines, enables hybrid programming and quantum simulations, parallelisation and task distribution, and communication instructions supporting a number of classical and quantum backends including QB's accelerators. The operating system is specifically designed to provide a *q*uantum *c*omputing (QC) framework as an accelerator for classical computations, and for integration of a range of quantum hardware with HPC infrastructure. qbOS is mainly built upon the leading, open-source, modular quantum software framework of [XACC](https://xacc.readthedocs.io/en/latest/) (an Eclipse project from ORNL developers) and a diverse range of other open-source software tools. The third-party developers are encouraged to provide additional modules for qbOS following our licence and commercial agreement terms, which allows quantum software and application teams to deliver their own products on top of QB’s platform.

**XACC**

[eclipse/xacc](https://github.com/eclipse/xacc)

qbOS presents a *fast* and *dynamic* (see below) quantum computing platform. It is targeted at a wide range of users from quantum scientists to enthusiasts to examine running quantum circuits, algorithms, and to collect data using their existing classical computing resources. Our simulation backends can emulate a gate-based ideal (noiseless) stochastic machine or QB's upcoming diamond-chip-based quantum hardware with a realistic noise model hard-coded.

qbOS is *fast* in the sense that, at its core, it exploits highly scalable *tensor network* simulator backends (allowing simulations and benchmarking with 100s of qubits), compiled C++ executables, supports GPU, MKL, multi-threaded OpenMP and MPI calculations, and provides other state-vector simulation options useful for low qubit number circuits. qbOS is a *dynamic* software package in the sense that it is in part delivered as a set of pre-compiled executables and can accept quantum circuits inputs in several mainstream Quantum Instruction Set formats (extensive support for the assembly language of [OpenQASM v2.0](https://github.com/Qiskit/openqasm/tree/OpenQASM2.x), and partial support for XASM and QUIL) and provides a range of prebuilt Python functions for users to run very high-level QC calculations.

**OpenQASM**

[Qiskit/openqasm](https://github.com/Qiskit/openqasm)

## 0.0. User Testimonials

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

## 0.1. qbOS Development Team

qbOS is our proprietary quantum software brought to you through the collective efforts of different Quantum Brilliance teams, feedback and contributions from external collaborators and end-users. Within Quantum Brilliance, our *Software & Applications* team is responsible for the development and support of qbOS. You may contact the following senior members from our Software and Applications team for general inquiries and questions regarding qbOS:

![**Dr Thien Nguyen**
  Software Engineer (E: thien.n@quantum-brilliance.com)](qbOS%20Documentation%20220913/ThienNguyen_(1).jpg)

**Dr Thien Nguyen**
  Software Engineer (E: thien.n@quantum-brilliance.com)

![**Simon Yin**
  Senior Software Engineer (E: simon.y@quantum-brilliance.com)](qbOS%20Documentation%20220913/Simon_Yin.jpg)

**Simon Yin**
  Senior Software Engineer (E: simon.y@quantum-brilliance.com)

![**Dr Michael Walker**
  Senior Quantum Tools Developer (E: mlwalker@quantum-brilliance.com)](qbOS%20Documentation%20220913/Michael_Walker.png)

**Dr Michael Walker**
  Senior Quantum Tools Developer (E: mlwalker@quantum-brilliance.com)

### 0.1.1 Past contributors

**Dr Nariman Saadatmand**  qbOS Product Owner, Software & Applications Team Lead

### 0.1.2 Full list of contributors

**Angus Mingare** during internship (2022)

**Marzieh Talebi** during internship (2021-2022)

## 0.2. Get Help: Questions, Bug Reporting, and Issue Tracking

### Pawsey's Quantum Pioneers

Please submit any feedback, requests or contributions for new features, and bug reports through **Pawsey's Help Desk email**: [help@pawsey.org.au](mailto:help@pawsey.org.au). Note you must please include “Quantum Pioneers” in the email subject. Pawsey and Quantum Brilliance have developed a support process that will be used for each of such tickets. Problems and questions related to access to Pawsey systems will be addressed by Pawsey staff directly. Problems and questions related to qbOS and QDK hardware will be triaged to Quantum Brilliance.

### Internal Developers and Research Collaborators

Please submit any feedback, requests or contributions for new features, and bug reports through *qbOS Issue Log* Kanban board linked below.

 **qbOS Issue Log**

[Atlassian](https://qbau.atlassian.net/secure/RapidBoard.jspa?rapidView=13)

# 1. Quickstart

Using a small experimental design, let's run some circuits of the [Quantum Fourier Transform (QFT)](qbOS%20Documentation%20220913/Quantum%20Fourier%20Transform%20(QFT)%20086133df66464393834202921b561874.md).  We perform a 2-qubit QFT, and a 4-qubit QFT.  In both cases, we will perform both a noiseless and a noisy simulation.  

The concept of a **QPU Kernel and GUI circuit composition** appear without details for now — these concepts can be reviewed [here](qbOS%20Documentation%20220913/GUI%20circuit%20composition%20and%20conversion%20to%20QPU%20kern%204f46ef713a5743e192aa79ec6f804d5d.md).

[QFT Experiment Fixed Global Options](qbOS%20Documentation%20220913/QFT%20Experiment%20Fixed%20Global%20Options%20de0784ece18847ebb02a5475c69aca97.csv)

[QFT Experiment Design](qbOS%20Documentation%20220913/QFT%20Experiment%20Design%2097159967bced45808d9bbef5fc861dea.csv)

### **Step 1 of 3**: Start your qbOS environment

Access to qbOS requires a web browser, and a virtual machine (VM) host provided via any one of the following options:

[AWS Spot-Market instances](qbOS%20Documentation%20220913/AWS%20Spot-Market%20instances%20695d2d72ff0b4916aec906b3ac43b74a.md)

[Pawsey Nimbus Cloud](qbOS%20Documentation%20220913/Pawsey%20Nimbus%20Cloud%2039cb71b3471045b388e4593d7ae2d240.md)

Once you have completed the login to your VM host, use your Web browser to open: [http://localhost:8889](http://localhost:8889) 

![Untitled](qbOS%20Documentation%20220913/Untitled.png)

### **Step 2 of 3**: Python code that calls qbOS and executes all (four) corner conditions of the experiment

```python
import qbos
tqb = qbos.core()  # This object has access to the core API methods for circuit simulation
tqb.qb12()         # Set up some sensible defaults
```

> Setup conditions that apply to all experiment corners:
> 

```python
tqb.xasm = True    # Use XASM circuit format to access XACC's qft()    
tqb.sn = 1024      # Explicitly use 1024 shots
tqb.acc = "aer"    # Use the aer state-vector simulator
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

> Enable the noise model
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

### **Step 3 of 3**: Inspect the results (shot counts)

After the code from Step 2 has finished executing, you can inspect shot counts (in each state and two easy-to-understand formats) by calling: `tqb.out_raw[0]` or `tqb.out_count[0]` for the first experiment and `tqb.out_raw[1]` or `tqb.out_count[1]` for the second one. More details on how to use Python codes to inspect settings or results, as configured for experiments above, are listed in the tables below:

[QFT Experiment - Python code for viewing fixed global options](qbOS%20Documentation%20220913/QFT%20Experiment%20-%20Python%20code%20for%20viewing%20fixed%20glo%20d4800077ec6f41edbee7a34819f21dcc.csv)

[QFT Experiment - Python code for viewing shot results](qbOS%20Documentation%20220913/QFT%20Experiment%20-%20Python%20code%20for%20viewing%20shot%20resu%20ba1d8065630f480fbb0e2dcf8d999fb4.csv)

# 2. Learn Quantum Gates

## 2.1. Introduction

As with the bits of classical computing, qubits must be manipulated and read to be of any use. Here we have given brief descriptions, with examples of some of the more important quantum gates.

## 2.2. Quantum measurement with and without noise

Arguably the most important operation in a quantum circuit, the quantum measurement is typically the last operation, although not every qubit is necessarily measured. There are several important differences concerning measurement between quantum and classical circuits:

- Measuring the state of a qubit ends its role in the calculation, as well as that of any qubit that was entangled with it. This is in contrast to classical bits whose values can be read and used elsewhere whilst allowing the calculation to continue. "Entangled" can be reasonably described as "correlated" for our purposes here and we shall describe it properly below.
- The state of a qubit cannot be duplicated, unlike a classical bit for which a branching gate exists. There is no such gate in quantum computing, a fact expressed by the "no-cloning" theorem. It is possible to copy a qubit's state onto another qubit of known, typically zero value, but the two qubits remain entangled and cannot be regarded as independent.
- A qubit is generally in a superposition of $`|0\rangle`$ and $`|1\rangle`$ states but only of those values is returned upon measurement. Which of the two are a genuinely random outcome and identical preparations may return different results. Regardless of how they may be separated in time or space, measurements of qubits will always correlate correctly if they are interdependent in any way.

In order to manage and study these random outcomes qbOS has the capacity to run a circuit multiple times, called shots. The returned qubit values are accompanied by the number of shots for which they were found by the final measurement.

There is another source of randomness in a quantum circuit, namely the thermal, magnetic and other noise acting on the qubits. qbOS can include this noise in its simulations if desired. The parameters for this noise are hard-coded to match the characteristics of the QB's hardware. The following code example sets up three qubits, setting one of them to $`|1\rangle`$, and measures them for 1024 shots. 

Python code:

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

Inspect the results:

```python
tqb.out_count[0]
```

The result is (similar to) this:

```python
[{1: 1024}, {0: 9, 1: 990, 3: 11, 5: 14}]
```

The 'sweep' variable was used to run both noise-free (False) and with noise (True). The first set of curly brackets is the noise-free result the same correct result is found for each shot. The second indicates the effects of noise although the correct result in this case still comprises the great majority of outcomes.

## 2.3 Simple gates

The simplest operations are those that either switch a qubit's state between $`|0\rangle`$ and $`|1\rangle`$ or change its coefficient. The very simplest is the identity operator which does literally nothing. It is equivalent to multiplying a number by one and is generally included only to complete the algebra generated by other operations.

The other operations for simply switching and multiplying qubits are denoted X, Y and Z: 

- X-gate: Interchanges  $`|0\rangle`$ and $`|1\rangle`$ with no other effects. It is also called the NOT gate,
- Y-gate: Maps $`|0\rangle`$ to $`i|1\rangle`$ and $`|1\rangle`$ to $`-i|0\rangle`$,
- Z-gate: Leaves $`|0\rangle`$ unchanged but multiplies $`|1\rangle`$ by -1.

What makes quantum computing interesting however is that the qubits are not restricted to be multiples of either $`|0\rangle`$ or  $`|1\rangle`$ but may exist in a superposition of the two. The simplest way to create such a superposition is with the Hadamard gate, usually represented with an 'H'. The effect of the Hadamard gate is as follows:

- $`|0\rangle \longrightarrow \frac{1}{\sqrt{2}} \left( |0\rangle + |1\rangle \right)`$
- $`|1\rangle \longrightarrow \frac{1}{\sqrt{2}} \left( |0\rangle - |1\rangle \right)`$

Each of these gates is its own inverse, meaning that applying it twice in a row leaves the qubit unchanged, so 

$`I = H^2 = X^2 = Y^2 = Z^2`$.

## 2.4 Simple quantum circuit with a Hadamard gate

We start with qbOS libraries import and some global configs: 

```python
import qbos
tqb = qbos.core()  # This object has access to the core API methods for circuit simulation
tqb.qb12()         # Set up some sensible defaults
```

Setting up some experiments' options:

```python
tqb.xasm = True    # Use XASM circuit format 
tqb.noise = True   
tqb.acc = "aer"    # Use the aer state-vector simulator with QB's embedded noise model 
```

Inserting the quantum circuits through the assembly language:

```python
# Option 1: a single H-gate (minimising the circuit depth)
tqb.instring = '''            
__qpu__ void QBCIRCUIT(qreg q) {
H(q[0]); // notice we initialise at |0> by default 
Measure(q[0]);
}
'''
```

```python
# Option 2: the equivalent circuit thru optimal native gate set of QB hardware
tqb.instring = '''            
__qpu__ void QBCIRCUIT(qreg q) {
Ry(q[0],pi/2);
X(q[0]);
Measure(q[0]);
}
'''
```

Setting up going through two experiments with different number of shots:

```python
tqb.sn[0].clear()
sweep = [1,100]
[tqb.sn[0].append(nn) for nn in sweep]
```

Running the circuits:

```python
tqb.run()
```

Inspecting the stochastic results:

```python
print(tqb.out_raw[0][0])
```

```python
print(tqb.out_raw[0][1])
```

```
{
"1":1
}
```

```
{
"0":56,
"1":44,
}
```

Using Python tools to get histograms:

```python
import numpy as np
import matplotlib.pyplot as plt
from pylab import figure, show, legend, ylabel
fig1 = figure()
ax1 = fig1.add_subplot(111)
xdat=[format(iix,'01b') for iix in iter(tqb.out_count[0][0])]
ydat=[tqb.out_count[0][0][iiy] for iiy in iter(tqb.out_count[0][0])]
ax1.bar(xdat,ydat)
plt.yticks(np.arange(0, 1.001, step=1))
plt.xlabel("States")
plt.ylabel("Shots measured")
```

```python
import numpy as np
import matplotlib.pyplot as plt
from pylab import figure, show, legend, ylabel
fig1 = figure()
ax1 = fig1.add_subplot(111)
xdat=[format(iix,'01b') for iix in iter(tqb.out_count[0][1])]
ydat=[tqb.out_count[0][1][iiy] for iiy in iter(tqb.out_count[0][1])]
ax1.bar(xdat,ydat)
plt.yticks(np.arange(0, 100.001, step=10))
plt.xlabel("States")
plt.ylabel("Shots measured")
```

![histo-left.png](qbOS%20Documentation%20220913/histo-left.png)

![histo-right.png](qbOS%20Documentation%20220913/histo-right.png)

And we can find the estimated quantum time in ms (for QB's commercial accelerators) simply through:

```
tqb.out_total_init_maxgate_readout_time[0][0][0]
```

```
tqb.out_total_init_maxgate_readout_time[0][1][0]
```

```
480.0
```

```
48000.0
```

Printing out the exact compiled/transpiled circuit instructions that qbOS/XACC compiler will send to QB hardware: 

- For this example, we set up the `loopback`:
    
    Open a Terminal in JupyterLab and run the command:
    
    `python3 /mnt/qb/bin/qbqe_if_model.py`
    
    ![Untitled](qbOS%20Documentation%20220913/Untitled%201.png)
    
    **Note: returned data when using the `loopback` are synthetic** and do not correspond to the input circuit nor to the results on real QB hardware.
    

Now issue the commands:

```python
tqb.acc = "loopback"
tqb.run()
```

And then inspect the JSON data sent to the QB hardware:

```python
print(tqb.out_qbjson[0][0])
```

```python
print(tqb.out_qbjson[0][1])
```

```python
# Result:

{"circuit":["Ry(q[0],1.5708)","Rx(q[0],3.14159)"],"command":"circuit","hwbackend":"gen1_canberra","init":[0,0,0,0,0,0,0,0,0,0,0,0],"measure":[[0,0]],"settings":{"cycles":1,"results":"normal","shots":1}}
```

```python
# Result:

{"circuit":["Ry(q[0],1.5708)","Rx(q[0],3.14159)"],"command":"circuit","hwbackend":"gen1_canberra","init":[0,0,0,0,0,0,0,0,0,0,0,0],"measure":[[0,0]],"settings":{"cycles":1,"results":"normal","shots":100}}
```

## 2.5 Controlled gates and basic quantum algorithms

Quantum computing also relies heavily on correlation, or *entanglement*, between qubits. This is achieved through gates that act on one qubit but are controlled by one or more others. The simplest of these is the controlled-not or CNOT gate. The CNOT, or *CX*, the gate is given one control qubit and one target qubit and performs a NOT, or *X*, operation on the target qubit if, but only if, the control qubit is in the $`|1\rangle`$ state. Their use is demonstrated in the following examples:

- **Example 2.5.1: Superdense coding**
    
    The gates presented so far are well illustrated by the [superdense coding protocol](https://en.wikipedia.org/wiki/Superdense_coding), which allows one to transmit classical information from one party to another, generally, two classical bits, using a single qubit. In a sense, it is a protocol opposite to [quantum teleportation](https://en.wikipedia.org/wiki/Quantum_teleportation).
    
    Let Alice and Bob be the two parties that wish to exchange two classical bits of information. A third party Charlie produces an entangled state of two qubits (applying a Hadamard gate to his first qubit, which serves as the control qubit for the following CNOT gate on the second qubit as the target) and gives Alice and Bob, each a qubit. Depending on the pair of classical bits that Alice wishes to send to Bob, she does one of the [following](https://qiskit.org/textbook/ch-algorithms/images/superdense_table1.png) operations on her qubit:
    
    For classical pair '00' she applies the identity  $`I`$ yielding the state $`|{00}\rangle + |{11}\rangle`$,
    
    For classical pair '10' she applies the operator $`X`$ yielding the state $`|{10}\rangle + |{01}\rangle`$,
    
    For classical pair '01' she applies the operator $`Z`$ yielding the state $`|{00}\rangle - |{11}\rangle`$,
    
    For classical pair '11' she applies the operator $`ZX`$ yielding the state $`-|{10}\rangle + |{01}\rangle`$,
    
    where the sent states are normalised by a factor of $`\sqrt{2}`$.
    
    Now Bob performs the reverse operation as Charlie, i.e. using the qubit he received from Alice as the control for a CNOT gate targeting the qubit he received from Charlie to disentangle them. He then applies a Hadamard gate to Alice's qubit and retrieves the two classical qubits sent by Alice by measuring both hers and Charlie's. Refer to [this](https://qiskit.org/textbook/ch-algorithms/images/superdense_table2.png) table to better understand the final operation done by Bob.
    
    - Step-by-step walkthrough
        
        The following Python notebook performs superdense coding protocol broadcast of '01' and '00':
        
        [qbOS_superdense.ipynb](qbOS%20Documentation%20220913/qbOS_superdense.ipynb)
        
    - Images of the output
        
        

The next two examples are, admittedly contrived, algorithms that demonstrate quantum utility:

- **Example 2.5.2: Quantum utility with Deutsch-Jozsa Algorithm**
    
    The [Deutsch-Jozsa algorithm](https://en.wikipedia.org/wiki/Deutsch%E2%80%93Jozsa_algorithm) was the first example of a quantum algorithm that performed better than its best classical counterpart. 
    
    In our problem, we are given a secret Boolean function $`f`$, which takes a string of bits as input and returns either a 0 or 1 i.e.
    
    $`\hspace{1cm}f(\{x_0, x_1, x_2, ...\}) \rightarrow \text{0 or 1, where } x_n \text{ is 0 or 1}`$
    
    The property of the above Boolean function is that it is either balanced or constant. A balanced function will return 0's for exactly half of the inputs and 1's for the other half, whereas a constant function will return all 0's or all 1's for any input. The problem is to determine the nature of this function $`f`$ i.e. whether it is balanced or constant.
    
    Classically, in the best case, we require two queries to the oracle and in the worst case, we would require half of all possible inputs plus one query to ascertain the nature of $`f`$.
    
    The advantage that the Deutsch-Jozsa algorithm provides is that only a single query to $`f`$ is required to determine if the Boolean function is balanced or constant with 100% confidence! If $`f`$ is constant the final state consists entirely of "0" but if it is balanced then the final state contains at least one "1".
    
    Balanced functions may be identified by a wrapping string. The following circuit performs the Deutsch-Jozsa algorithm on a balanced function whose wrapping string is "101": see [here](https://algassert.com/quirk#circuit=%7B%22cols%22%3A%5B%5B1%2C1%2C1%2C%22X%22%5D%2C%5B%22H%22%2C%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B%22X%22%2C1%2C%22X%22%5D%2C%5B%22%E2%80%A2%22%2C1%2C1%2C%22X%22%5D%2C%5B1%2C%22%E2%80%A2%22%2C1%2C%22X%22%5D%2C%5B1%2C1%2C%22%E2%80%A2%22%2C%22X%22%5D%2C%5B%22X%22%2C1%2C%22X%22%5D%2C%5B%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B%22Measure%22%2C%22Measure%22%2C%22Measure%22%5D%5D%7D).
    
    - Step-by-step walkthrough ending at a plot of Quantum Utility
        
        **Concepts covered in the Jupyter notebook below**:
        
        - Circuit generation
        - Execution of an array of circuits
        - Extracting classical execution time
        - Extracting estimated quantum hardware execution time
        - Visualising Quantum Utility
        
        [qbOS_DJ.ipynb](qbOS%20Documentation%20220913/qbOS_DJ.ipynb)
        

### Visualising Quantum Utility

[DJ-alternative-q-util.ipynb](qbOS%20Documentation%20220913/DJ-alternative-q-util.ipynb)

![DJgrid (4).png](qbOS%20Documentation%20220913/DJgrid_(4).png)

The visualisation above shows that beyond 34 qubits, the speed of the Deutsch-Jozsa algorithm on forthcoming QB hardware is superior to a comparable classical algorithm (running on an AWS *m5a.large* virtual machine) that performs the same task.  However, the fidelity on QB hardware is below that of the classical algorithm.

- **Example 2.5.3: Quantum utility with Bernstein-Vazirani Algorithm**
    
    The [Bernstein-Vazirani algorithm](https://en.wikipedia.org/wiki/Bernstein%E2%80%93Vazirani_algorithm) can be viewed as an extension of the Deutsch-Jozsa algorithm.  In the problem, we have a secret message *s* (a bitstring) and an oracle *f* which is defined as:
    
     $`\hspace{1cm}f_s(x) = s \cdot x \: (\text{mod 2})`$
    
    where $`x`$ is the input string that we use as queries to the oracle, to find the secret string.
    
    The classical approach is to apply 
    
    $`\hspace{1cm} x = 2^i, \;\; \forall i \; \in \; [0, n-1]`$
    
    requiring *n* trials, where *n* is the number of bits in the secret string. This clearly has a runtime linear in *n*, while the Bernstein-Vazirani algorithm requires only one call to the function $`f_s(x)`$. It follows theoretically that the runtime is constant with respect to the size of *s*. In practice, however, the physical characteristics and limitations of the hardware cause the runtime to increase with an increasing number of qubits. 
    
    For a graphical view of this circuit, see [here](https://algassert.com/quirk#circuit=%7B%22cols%22%3A%5B%5B%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B1%2C1%2C1%2C%22H%22%5D%2C%5B1%2C1%2C1%2C%22Z%22%5D%2C%5B1%2C%22%E2%80%A2%22%2C1%2C%22X%22%5D%2C%5B1%2C1%2C%22%E2%80%A2%22%2C%22X%22%5D%2C%5B%22H%22%2C%22H%22%2C%22H%22%5D%2C%5B%22Measure%22%2C%22Measure%22%2C%22Measure%22%5D%5D%7D).
    
    - Step-by-step walkthrough showing the impact of noise
        
        The following Python notebook sets up a 2-by-2 experiment: it runs the Bernstein-Vazirani algorithm with a 14-qubit secret, and a 16-qubit secret; where both noise-free, as well as noise-enabled conditions, are examined:
        
        [qbOS_BV.ipynb](qbOS%20Documentation%20220913/qbOS_BV.ipynb)
        
    - Images of the output

As one might expect, all the simple gates in Section 2.3 have their controlled equivalents, *CY*, *CZ* and *CH.*

A list of available gates, together with mathematical representation and coding syntax is available in the Glossary of Tools and Quantum Gates.

# 3. Circuit builders

## 3.1. pyQuirk GUI circuit builder

A basic drag-and-drop interface is provided for implementing simple circuits.  See this guide for more details:

[GUI circuit composition with pyQuirk ](qbOS%20Documentation%20220913/GUI%20circuit%20composition%20with%20pyQuirk%20f7161f17d431491c9497e928d3486cdc.md)

## 3.2. Importing from a circuit composition GUI and running in qbOS

For users who understand quantum circuits, see the following guides on exporting a circuit from third-party circuit composition GUIs and how to execute the circuit in qbOS:

[GUI circuit composition and conversion to QPU kernel for execution in qbOS](qbOS%20Documentation%20220913/GUI%20circuit%20composition%20and%20conversion%20to%20QPU%20kern%204f46ef713a5743e192aa79ec6f804d5d.md)

## 3.3 qbOS circuit builder

### 3.3.1 Introduction

qbOS provides a `Circuit` class for users to construct quantum circuits from elementary gates, such as X, Y, Z, Hadamard, CNOT, etc. 

Users can use `Circuit`’s methods to compose the circuit as in the example below: 

- Construct the circuit:

Note: the list of supported gates is listed below.

```python
import qbos as qb
import re
tqb = qb.core()

nb_qubits = 12
circ = qb.Circuit()
circ.h(0)
# Entangle all qubits
for i in range(nb_qubits - 1):
    circ.cnot(i, i + 1)
circ.measure_all()
```

- Execute with qbOS:

We use the `ir_target` method of `tqb` to assign the circuit ready to run.

```python
tqb.qb12()
tqb.ir_target = circ
tqb.run()
```

- Inspect the shot outcomes:

```python
tqb.out_raw[0]
```

A sample result may look like the following, indicating entanglement as expected.

```
String[{
    "000000000000": 554,
    "111111111111": 470
}]
```

### 3.3.2 Basic Methods

| Name | Inputs | Outputs | Descriptions |
| --- | --- | --- | --- |
| print | None | None  | Print the quantum circuit that has been built. |
| openqasm | None | str | Get the OpenQASM representation of the circuit. |
| append | arg0: qbos.Circuit | None | Append another quantum circuit to this circuit. |
| h | qubit: int | None | Add Hadamard gate |
| x | qubit: int | None | Add Pauli-X gate |
| y | qubit: int | None | Add Pauli-Y gate |
| z | qubit: int | None | Add Pauli-Z gate |
| u1 | qubit: int, theta: float | None | Add ⁍ gate |
| u3 | qubit: int, theta: float, phi: float, lambda:float | None | Add ⁍ gate |
| t | qubit: int | None | Add T gate |
| tdg | qubit: int | None | Add T-dagger gate |
| s | qubit: int | None | Add S gate |
| sdg | qubit: int | None | Add S-dagger gate |
| rx | qubit: int, theta: float | None | Add rotation around X gate (⁍) |
| ry | qubit: int, theta: float | None | Add rotation around Y gate (⁍) |
| rz | qubit: int, theta: float | None | Add rotation around Z gate (⁍) |
| cnot | control_qubit: int, target_qubit: int | None | Add Controlled-X gate |
| mcx | control_qubits: array[int], target_qubit: int | None | Add Multi-controlled-X gate (control qubits are given as a list of indices) |
| generalised_mcx | target: int, controls_on: array[int], controls_off: array[int] | None | Add Multi-controlled-X gate conditional on controls_on being ⁍ and controls_off being ⁍. |
| amcu | U: CircuitBuilder, control_qubits: array[int], ancilla_qubits: array[int] | None | Add multi-controlled-U gate using ancilla qubits as shown here |
| ccx | control_qubit1: int, control_qubit2: int, target_qubit: int | None | Add CCNOT (Toffoli gate) |
| swap | qubit1: int, qubit2: int | None | Add SWAP gate |
| controlled_swap | qubits_a: array[int], qubits_b: array[int], flags_on: array[int], flags_off: array[int] | None | Add SWAP operation between register a and register b conditional on on flags being ⁍ and off flags being ⁍. |
| cphase | control_qubit: int, target_qubit: int, theta: float | None | Add controlled-phase rotation gate (also known as ⁍ gate) |
| cz | control_qubit: int, target_qubit: int | None | Add Controlled-Z gate |
| ch | control_qubit: int, target_qubit: int | None | Add Controlled-Hadamard gate |
| measure | qubit: int | None | Add Measure to a qubit |
| measure_all | None | None | Add Measure to all qubits in the circuit |

# 4. qbOS Modules and Applications

qbOS `Circuit` class also provides high-level methods to construct quantum circuits for commonly-used quantum algorithms, such as QFT, amplitude amplification, etc. 

## 4.1 Quantum Fourier Transform (QFT)

[Quantum Fourier Transform (QFT)](qbOS%20Documentation%20220913/Quantum%20Fourier%20Transform%20(QFT)%20086133df66464393834202921b561874.md)

## 4.2 Arithmetic Operations

[Quantum arithmetic operations](qbOS%20Documentation%20220913/Quantum%20arithmetic%20operations%20912925f8ab1644d8821fd5645df3e911.md)

[Ripple Carry Adder in qbOS](qbOS%20Documentation%20220913/Ripple%20Carry%20Adder%20in%20qbOS%20c90a303c2b7641e2903b0e299d29d0df.md)

[Subtraction in qbOS](qbOS%20Documentation%20220913/Subtraction%20in%20qbOS%20d894d29f00284d878a498964be70f399.md)

[Proper Fraction Division in qbOS ](qbOS%20Documentation%20220913/Proper%20Fraction%20Division%20in%20qbOS%200a0a79f708ec4025a5893b0509c3e931.md)

[Multiplication in qbOS ](qbOS%20Documentation%20220913/Multiplication%20in%20qbOS%20331e71fa0f40468abdc6d33ef990796d.md)

## 4.3 Inverse Circuit

[Inverse Circuit in qbOS](qbOS%20Documentation%20220913/Inverse%20Circuit%20in%20qbOS%205c88dd6ec8eb45d998fc2e184584d6c9.md)

## 4.4 Bit String Comparator

[v0 Python Plugin](qbOS%20Documentation%20220913/v0%20Python%20Plugin%2017de6406325c411db2bffa0b9c6e38d0.md)

[Quantum Comparator in qbOS](qbOS%20Documentation%20220913/Quantum%20Comparator%20in%20qbOS%20036f305a7273456ab87315e6f4cb44db.md)

[Compare > in qbOS (coming 220807)](qbOS%20Documentation%20220913/Compare%20in%20qbOS%20(coming%20220807)%20961e0695a65f4cae8c12e8148a2b204b.md)

[Quantum Equality Checker in qbOS ](qbOS%20Documentation%20220913/Quantum%20Equality%20Checker%20in%20qbOS%2020fb41766e754abc814df7e5bae350f6.md)

## 4.5 Amplitude Amplification

[v0 Python Plugin](qbOS%20Documentation%20220913/v0%20Python%20Plugin%20cb380f16dca64c419ff2b339dd34f0ed.md)

[Amplitude Amplification in qbOS](qbOS%20Documentation%20220913/Amplitude%20Amplification%20in%20qbOS%20625436cc7b4d465c93c94af9da6d9457.md)

## 4.6 Quantum Phase Estimation (QPE)

[Quantum Phase Estimation in qbOS](qbOS%20Documentation%20220913/Quantum%20Phase%20Estimation%20in%20qbOS%20ede1f2c66aff426f870bf620002c31ee.md)

## 4.7 Quantum Amplitude Estimation (QAE)

[Amplitude Estimation in qbOS](qbOS%20Documentation%20220913/Amplitude%20Estimation%20in%20qbOS%20197b2063bef447718fdfa185d82d829f.md)

## 4.8 Quantum Exponential Search Algorithm (QESA)

[Exponential Search in qbOS](qbOS%20Documentation%20220913/Exponential%20Search%20in%20qbOS%209daf531c584f4cadb31697f8cb94b544.md)

## 4.9 Efficient Encoding

[Efficient Encoding in qbOS](qbOS%20Documentation%20220913/Efficient%20Encoding%20in%20qbOS%202407bb4867fd495294d9274e85923f4a.md)

## 4.10 Quantum Decoder Application

[v0 Python Plugin](qbOS%20Documentation%20220913/v0%20Python%20Plugin%20c7728f45b0344bca8115c48dbbfd078b.md)

## 4.11 Variational Quantum Eigensolver (VQE)

[Variational Quantum Eigensolver (VQE) in qbOS](qbOS%20Documentation%20220913/Variational%20Quantum%20Eigensolver%20(VQE)%20in%20qbOS%206b6a211cc87749f6ac36faaf5581f8a2.md)

## 4.12 Random Circuit Generator

[Random quantum circuit sampling with qbOS](qbOS%20Documentation%20220913/Random%20quantum%20circuit%20sampling%20with%20qbOS%20ad6a723f9ebf49e7a0967b999a63f132.md)

## 4.13 Visualisation and Data Science

qbOS provides a specialised visualisation module for exploring Quantum Utility:

[Visualisation and Data Science in qbOS](qbOS%20Documentation%20220913/Visualisation%20and%20Data%20Science%20in%20qbOS%20e375e48655ea4a04889a3b915daded2c.md)

## 4.14 Quantum Approximate Optimization Algorithm (QAOA)

[Optimisation with QAOA in qbOS](qbOS%20Documentation%20220913/Optimisation%20with%20QAOA%20in%20qbOS%2069aae61a872045adac72410f02d977e5.md)

[QAOA extensions in qbOS](qbOS%20Documentation%20220913/QAOA%20extensions%20in%20qbOS%201ce0bfe95e304a978cad6dada58ef0fd.md)

# 5. Hardware Specifications and Limitations

## 5.1. Qubit Connectivity Diagram

In the context of quantum programming, we say that two qubits are connected if a CNOT operation can be directly applied to them. By this definition, all qubits on a QB diamond chip are effectively connected and this should be sufficient for most quantum programming applications.  

Direct physical connectivity is much more restricted. Qubits are physically arranged in fully connected clusters of three. The clusters are arranged in a two-dimensional grid with complete connectivity between the qubits of horizontal and vertical nearest neighbours. There are no diagonal physical connections. It follows that there can be no more than six qubits in a physically fully connected registry. 

![qbOS%20Documentation%20220913/cluster_grid2.png](qbOS%20Documentation%20220913/cluster_grid2.png)

> **Figure 5.1:** Each cluster of three qubits lies inside a black circle, which represents a single NV centre. Intra-cluster connections are shown in red. Connections between clusters are in orange and split at each end to indicate that every qubit at one vertex is physically connected to every qubit at the nearest-neighbour vertex. There is no difference in the strength of intra- and inter- couplings — different colours are only used to facilitate presentation.
> 

Physically distant qubits can still be connected in the quantum programming context described above, but in practice this may require the equivalent of a large number of swap operations, rendering such a connection less reliable. 

## 5.2 Q-state tomography

![four_state_errors.png](qbOS%20Documentation%20220913/four_state_errors.png)

**Figure 5.2:** Magnitude plot for the q-state tomography of a quantum state after a simulation of qbOS. On the absence of noise and error the state would be pure |00>

## 5.3 Configuring Quantum Brilliance hardware from qbOS

[Configuring qbOS for Quantum Brilliance (QDK) hardware](qbOS%20Documentation%20220913/Configuring%20qbOS%20for%20Quantum%20Brilliance%20(QDK)%20hard%20ff8b9b8aff664cba9c89e2637922ebba.md)

# 6. Types and Arrays in qbOS (more examples)

**The design aim of qbOS is to support an array programming syntax**.  Users who have come from a NumPy, Matlab, or Julia background will see the similarity to those environments.  Put simply, qbOS objects have members that are 2D arrays.  **Broadcasting** of values is performed wherever it is consistent to do so prior to execution.

The **leading (or innermost) dimension** is from hereon referred to as the **conditions** dimension.
The **second dimension** is from hereon referred to as the **circuits** dimension.

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

[Usage/examples of `qbos.MapND` type](qbOS%20Documentation%20220913/Usage%20examples%20of%20qbos%20MapND%20type%20807974c9c8c94f0c842b601924105d17.md)

## 6.7 `qbos.MapNC` : list/array of maps (integer → double precision complex)

Also: `qbos.VectorMapNC` : 2-D array of maps

[Usage/examples of `qbos.MapNC` type](qbOS%20Documentation%20220913/Usage%20examples%20of%20qbos%20MapNC%20type%20e04ad5447638476184ed7a21f05e966b.md)

## 6.8 `qbos.MapNN`: list/array of maps (integer → integer)

Also: `qbos.VectorMapNN`: 2-D array of maps

# 7. Download and Installation

qbOS consists of a front-end JupyterLab GUI, supported by a container that is run on any of the following platforms:

## 7.1 qbOS Jupyter UI through Pawsey Nimbus Cloud

## 7.2 qbOS Jupyter UI through AWS

## 7.3 qbOS CLI and job scripts (for HPC usage)

qbOS includes command-line executables that are suited for job scripts that are submitted to queues on HPC systems.  These are executables that are compiled from source code to use site-specific optimised numerical libraries and MPI support.  Execution can be performed either on containers (Singularity); or directly on bare metal.  Please contact Quantum Brilliance to discuss your requirements.

## 7.4 Offload to standalone simulators

### 7.4.1 Aer standalone `qasm_simulator` (tensor-network capable)

<aside>
⚠️ **Deprecated in qbOS version 2.0**

**Replacement functionality:** 
From qbOS version 2.0 onward, use the `aer-mps` simulator directly from qbOS as follows:

```python
import qbos
tqb = qbos.core()
tqb.qb12()

tqb.acc = "aer"
tqb.aer_sim_type = "matrix_product_state"
```

- Previous description (for qbOS version 1)
    
    <aside>
    ✅ This is an advanced option: qbOS is pre-installed with a standalone matrix-product-state (MPS) tensor network simulator configured to use the Quantum Brilliance 48-qubit noise model.  
    
    qbOS provides the means to generate the required qobj JSON input. The noise model is also made available in JSON format.
    
    </aside>
    
    Usage:
    
    ```python
    # In qbOS Jupyter notebook:
    resultjson = ! qasm_simulator -c /mnt/qb/share/aer_noise_model_qb.json <your-input.qobj>
    
    ```
    
    Example:
    
    Below is a full qbOS Jupyter notebook that generates a qobj, and then calls the standalone Aer `qasm_simulator`:
    
    ```python
    import qbos
    import json
    tqb=qbos.core()
    tqb.qb12()
    tqb.acc = "aer"
    tqb.nosim = True  # bypass execution with XACC-aer, but generate the qobj
    tqb.instring='''
    __qpu__ void QBCIRCUIT(qreg q) {
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[2];
    h q[1];
    cx q[1],q[0];
    measure q[1] -> c[1];
    measure q[0] -> c[0];
    }
    '''
    
    tqb.run()
    # Save qobj to a file
    ofp = open("btest.qobj", mode='w')
    print(tqb.out_qobj[0][0],file=ofp)
    ofp.close()
    
    aerd = ! qasm_simulator -c /mnt/qb/share/aer_noise_model_qb.json btest.qobj
    resj = json.loads(''.join(aerd))
    
    # Show the counts
    resj['results'][0]['data']['counts']
    
    # Output: {'0x0': 493, '0x1': 11, '0x2': 10, '0x3': 510}
    ```
    
</aside>

## 7.5 AWS Braket offloading

[AWS Braket offload in qbOS](qbOS%20Documentation%20220913/AWS%20Braket%20offload%20in%20qbOS%204631d07005d74d5898dc4ee153199663.md)

# 8. Starting a qbOS Jupyter session

## 8.1 **Login** to the VM instance

Once all the necessary VM setup is completed and the instance is launched, it is time to log in.

<aside>
ℹ️ If you are running Windows 10, first open PowerShell (**Windows button** → **Windows PowerShell**) or some other Shell terminal of your choice and then:

</aside>

```bash
ssh -i your-ssh-private-key-file -L 8889:localhost:8889 ubuntu@your.ip.ad.dress
```

<aside>
ℹ️ Please ensure you have `-L 8889:localhost:8889` in your login as shown above.

</aside>

Enter the password as appropriate (if needed, type 'yes' when asked about the identity of the system).

## 8.2 Launching the web browser GUI interface (JupyterLab)

With a web browser, visit [http://localhost:8889](http://localhost:8889)

# 9. Glossary of Tools and Quantum Gates

### 9.1 Quantum Gates

[Quantum Gates](qbOS%20Documentation%20220913/Quantum%20Gates%207820f4f28e564f69bd74a585bc527471.md)

### 9.1.1 Custom OpenQASM gates

By default, qbOS includes all custom gates that have been defined in: `/mnt/qb/share/qblib.inc`.

A user may set the path and filename for custom gates by changing `qb_include`:

```python
import qbos
tqb=qbos.core()
tqb.qb12()
tqb.qb_include = "/path/to/my/openqasm/includefile.inc"  # default: "/mnt/qb/share/qblib.inc"
```

## 9.2 High-level Commands and Tools/Options

**Accelerator:** What accelerators can I use in qbOS and how do I use them?

```python
import qbos
tqb = qbos.core()

tqb.acc = "accelerator_of_choice"
```

where “accelerator_of_choice” can be any of the following:

- “aer”
- “tnqvm”
- “qpp”
- “qsim”
- “qb-lambda”
- “sparse-sim”
- “aws_acc” [new in qbOS version 2]

The default is “tnqvm”

When using `aer`, there is an option, `aer_sim_type` to select the simulation method that AER will use.

Available `aer_sim_type` options are: `statevector` `density_matrix` or `matrix_product_state`

For example, to select the matrix product state method of AER, we can do

```python
import qbos
tqb = qbos.core()
tqb.qb12()
tqb.acc = "aer"
tqb.aer_sim_type = "matrix_product_state"
```

**Noise**: How do I turn on the encoded noise model of diamond quantum accelerators?

```python
import qbos
tqb = qbos.core()

tqb.noise = True
```

The default is for the noise parameter to be False.

**Qubit Number**: How do I set the maximum number of qubits in the quantum registry?

```python
import qbos
tqb = qbos.core()

tqb.qn = 15
```

This will have 15 qubits in the quantum register instead of the default of 12.

**Shots**: How do I set the number of shots?

```python
import qbos
tqb = qbos.core()

tqb.sn = 256
```

There will be 256 shots instead of the usual 1024.

**Transpiled circuit using Quantum Brilliance native gates**: How do I view the result from transpiling a circuit into the native QB gate set?

```python
import qbos
tqb = qbos.core()
tqb.qb12()

tqb.instring='''
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
creg c[2];
x q[0];
cx q[0], q[1];
measure q[1] -> c[1];
measure q[0] -> c[0];
}
'''
tqb.run()

# Show the transpiled circuit
print(tqb.out_transpiled_circuit[0][0])
```

Example output:

```python
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
u(3.14159, -1.5708, 1.5708) q[0];
u(1.5708, 0, 0) q[1];
u(3.14159, -1.5708, 1.5708) q[1];
cz q[0],q[1];
u(1.5708, 0, 0) q[1];
u(3.14159, -1.5708, 1.5708) q[1];
creg c0[1];
measure q[1] -> c0[0];
creg c1[1];
measure q[0] -> c1[0];
}
```

See **Visualisation and Data Science in qbOS** to obtain this output:

![Untitled](qbOS%20Documentation%20220913/Untitled%202.png)

**Histogram of shot counts:** How do I get output shot counts and visualise them?

See Section 2.4

**Noise mitigation:** How do I apply noise mitigation technique when performing noisy simulation (**tqb.noise = True**)?

```python
import qbos as qb
tqb = qb.core()
tqb.noise_mitigation = "assignment-error-kernel"
```

Valid options: 

- "`ro-error`": simple expectation value calculation adjustment based on readout error rates.
- "`rich-extrap`":  Richardson extrapolation (to the zero noise level) for two-qubit gate error (CNOT).
- "`assignment-error-kernel`": readout distribution correction by multiplying the inverse of the error assignment matrix.

**Notes:** 

- Only `assignment-error-kernel` adjusts the whole bitstring distribution. The others only adjust the expectation value Z estimation.

**Qubit Placement**: How do I select the qubit placement method?

```python
import qbos
tqb = qbos.core()
tqb.placement = "swap-shortest-path"
```

Valid options:

- “`swap-shortest-path`”: permutation-based mapping (swap) where for a CNOT gate between uncoupled qubits, the endpoints are swapped along the shortest (weighted) path in the coupling graph until they
are adjacent
- “`tket`”: placement module provided by the tket library

**Seed**: How do I set the ***seed value*** used by the simulator to generate random samples?

```python
import qbos
tqb = qbos.core()
tqb.seed = 1234
```

The simulator will be seeded with 1234 rather than a random value.

<aside>
🧩 `seed` can be fixed for only the following back-ends [`acc`]:

”`aer`"
”`qpp`”
”`qsim`”
”`tnqvm`” [requires restart of Jupyter kernel]

</aside>

# 10. Parallel Operations in qbOS

qbOS supports asynchronous (non-blocking) dispatch of quantum jobs in the job table (array programming syntax) to multiple backend devices in a “*fire-and-forget*” fashion.

Specifically, the qbOS `run(i, j)` method executes a circuit instance indexed by `(i,j)` in a blocking manner, hence the caller will need to wait until the execution of that circuit is complete to proceed with other steps.

Therefore, the asynchronous job dispatch mechanism is designed to support:

- Non-blocking job submission: i.e., the circuit execution can be launched without blocking the caller. The main thread can continue useful work while waiting for the result to be available.
- Flexibility: users can specify how many backends should be *pooled* together for parallel execution.
- Scalability: it needs to be able to handle thousands of jobs, potentially hundreds of connected backends.
- User-friendly: consistent/similar API with the existing synchronous counterpart.

## 10.1 Asynchronous APIs and data structures

The following methods in `qbos.core()` can be used to execute circuits asynchronously. 

- `set_parallel_run_config(config_json)`

Specify a list of accelerator backends that should be participated in parallel job dispatch.

Currently, these backends are assigned jobs in a FIFO fashion.

**Example**:

```python
import qbos, json
tqb = qbos.core()
qpu_configs = {"accs": [{"acc": "qpp"},
                        {"acc": "aer"},
                        {"acc": "qb-lambda"}]} 

# Set up the pool of QPU for parallel task distribution
tqb.set_parallel_run_config(json.dumps(qpu_configs))
```

In short, we need to supply a JSON configuration that includes a list of accelerators (`accs` field). Each accelerator instance is specified by its name (`acc` field) and any other accelerator-specific settings if necessary (none is needed in the above example).

The above setting assumes that we want to use 3 simulators as the backends.

- `handle = tqb.run_async(i, j)`

Launch the job `(i, j)` (row `i`, column `j`) asynchronously (the function should return almost *immediately* without any blocking) and return a handle that the caller can use to wait for the result.

In particular, the `handle` has the following methods:

- `handle.complete()` returns `True`/`False` depending on the job status.
- `handle.qpu_name()` returns the name of the QPU accelerator that executed the job. Only valid once the job has been completed.
- `handle.get()` gets the string result. If not yet available, this call will wait till the result is ready.

<aside>
ℹ️ After the first successful call to `handle.get()`, subsequent calls will result in null values.

</aside>

- `handle.terminate()` terminate the underlying job. Do nothing if the job has been completed.

- `tqb.run_complete(i, j)`

Return `True`/`False` indicating if the result of the `(i,j)` run is available, thus can be retrieved in the `out_raws`

**Note**: users can use the `handle` that was returned by `run_async` to check this as well.
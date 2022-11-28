```{attention} Placeholder only 

Copied from legacy qbOS documentation, need revision...
```

# Learn Quantum Gates

## Introduction

As with the bits of classical computing, qubits must be manipulated and read to be of any use. Here we have given brief descriptions, with examples of some of the more important quantum gates.

## Quantum measurement with and without noise

Arguably the most important operation in a quantum circuit, the quantum measurement is typically the last operation, although not every qubit is necessarily measured. There are several important differences concerning measurement between quantum and classical circuits:

- Measuring the state of a qubit ends its role in the calculation, as well as that of any qubit that was entangled with it. This is in contrast to classical bits whose values can be read and used elsewhere whilst allowing the calculation to continue. "Entangled" can be reasonably described as "correlated" for our purposes here and we shall describe it properly below.
- The state of a qubit cannot be duplicated, unlike a classical bit for which a branching gate exists. There is no such gate in quantum computing, a fact expressed by the "no-cloning" theorem. It is possible to copy a qubit's state onto another qubit of known, typically zero value, but the two qubits remain entangled and cannot be regarded as independent.
- A qubit is generally in a superposition of $|0\rangle$ and $|1\rangle$ states but only of those values is returned upon measurement. Which of the two are a genuinely random outcome and identical preparations may return different results. Regardless of how they may be separated in time or space, measurements of qubits will always correlate correctly if they are interdependent in any way.

In order to manage and study these random outcomes qbOS has the capacity to run a circuit multiple times, called shots. The returned qubit values are accompanied by the number of shots for which they were found by the final measurement.

There is another source of randomness in a quantum circuit, namely the thermal, magnetic and other noise acting on the qubits. qbOS can include this noise in its simulations if desired. The parameters for this noise are hard-coded to match the characteristics of the QB's hardware. The following code example sets up three qubits, setting one of them to $|1\rangle$, and measures them for 1024 shots. 


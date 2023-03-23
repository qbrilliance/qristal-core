## Example of circuit execution on QB hardware
This circuit has a single Hadamard gate.  This example demonstrates how a circuit can be run either on QPU hardware, or on a simulator.

### Compiling the example
```
mkdir build && cd build
cmake ..
make

```

### Running the example circuit [on simulator]
```
./h1qb
```

### Running the example circuit on QPU hardware
```
./h1qb --qdk
```

### Example output
```
h1qb : single Hadamard gate demo... 

* To run on hardware QPU add the option: --qdk


    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      measure q[1] -> c[1];
      measure q[0] -> c[0];
    }
    
Results:
{
    "00": 19,
    "01": 13
}

* Time used for circuit execution, in ms: 0.532
```

# Experiment 1 (4-qubit QFT)

Options 0: <to be simulated> Results: shot counts
Options 1: <to be simulated> Results: shot counts
QPU Kernel: __qpu__ void QBCIRCUIT(qreg q) 
{
qft(q, {{"nq",4}});
Measure(q[3]);
Measure(q[2]);
Measure(q[1]);
Measure(q[0]);
}
Qubits: 4
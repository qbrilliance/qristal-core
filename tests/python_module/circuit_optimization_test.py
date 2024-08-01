# Test cases for circuit optimization

import pytest
import qristal.core
import random
# Test optimization setting in session
def test_opt_pass():
    s = qristal.core.session()
    s.init()
    # A circuit on two qubits that are extremely long!!!
    circ = qristal.core.Circuit()
    # Repeat a random circuits...
    # 20 layers == 20 CNOTs
    num_layers = 20
    for i in range(num_layers):
        circ.u3(0, random.random(), random.random(), random.random())
        circ.u3(1, random.random(), random.random(), random.random())
        circ.cnot(0, 1)
    circ.measure_all()

    print("Original circuit:")
    circ.print()

    my_sim = qristal.core.session(True)

    # Set up meaningful defaults for session parameters
    my_sim.init()

    # Choose a simulator backend
    my_sim.acc = "qpp"

    # Choose how many qubits to simulate
    my_sim.qn = 2

    # Choose how many 'shots' to run through the circuit
    my_sim.sn = 100

    # Set the quantum program to run
    my_sim.ir_target = circ

    # Set the circuit optimization pipeline:
    my_sim.circuit_optimization = [qristal.core.two_qubit_squash(), qristal.core.redundancy_removal()]
    # Make sure we run those passes..
    my_sim.nooptimise = False
    # Run the circuit 100 times and count up the results in each of the classical registers
    
    print("About to run quantum program...")
    my_sim.run()
    print("Ran successfully!")
    print("Optimized circuit:")
    circ.print()
    for qubit, gate_count_2q in my_sim.out_double_qubit_gate_qty[0][0].items():
        assert(gate_count_2q < num_layers)

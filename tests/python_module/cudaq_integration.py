# Test CUDAQ integration with the Qristal Python package

import qristal.core

def test_simple():

    # Create a quantum computing session using Qristal
    my_sim = qristal.core.session()

    # Choose a simulator backend from CUDA Quantum
    my_sim.acc = "cudaq:dm"
    my_sim.gpu_device_ids = [0]

    # Set the number of qubits to simulate
    my_sim.qn = 2

    # Choose how many 'shots' to run through the circuit
    my_sim.sn = 1024

    circ = qristal.core.Circuit()
    circ.h(0)
    circ.cnot(0, 1)
    circ.measure_all()
    # Set the circuit
    my_sim.irtarget = circ
    my_sim.run()

    # Print the cumulative results in each of the classical registers
    print("Results:")
    print(my_sim.results)
    # only 00 and 11 are expected
    assert (len(my_sim.results) == 2)
    assert (my_sim.results[[0,0]] + my_sim.results[[1,1]] == 1024)

# Test CUDAQ integration with the Qristal Python package

import ast

def test_simple():
    # Import the core of Qristal
    import qristal.core
    # Create a quantum computing session using Qristal
    my_sim = qristal.core.session()

    # Set up meaningful defaults for session parameters
    my_sim.init()

    # Choose a simulator backend from CUDA Quantum
    my_sim.acc = "cudaq:dm"
    my_sim.gpu_device_id = [0]

    my_sim.qn = 2

    # Choose how many 'shots' to run through the circuit
    my_sim.sn = 1024

    circ = qristal.core.Circuit()
    circ.h(0)
    circ.cnot(0, 1)
    circ.measure_all()
    # Set the circuit
    my_sim.ir_target = circ
    my_sim.run()

    # Print the cumulative results in each of the classical registers
    res = my_sim.results[0][0]
    print("Results:")
    print(res)
    # only 00 and 11 are expected
    assert (len(res) == 2)
    assert (res[[0,0]] + res[[1,1]] == 1024)

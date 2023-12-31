# Test CUDAQ integration with the Qristal Python package

import ast

def test_simple():
    # Import the core of the QB SDK
    import qb.core
    # Create a quantum computing session using the QB SDK
    my_sim = qb.core.session()

    # Set up meaningful defaults for session parameters
    my_sim.qb12()

    # Choose a simulator backend from CUDA Quantum
    my_sim.acc = "cudaq:dm"

    my_sim.qn = 2

    # Choose how many 'shots' to run through the circuit
    my_sim.sn = 1024

    circ = qb.core.Circuit()
    circ.h(0)
    circ.cnot(0, 1)
    circ.measure_all()
    # Set the circuit
    my_sim.ir_target = circ
    my_sim.run()

    # Print the cumulative results in each of the classical registers
    print("Results:\n", my_sim.out_raw[0][0])
    res = ast.literal_eval(my_sim.out_raw[0][0])
    # only 00 and 11 are expected
    assert (len(res) == 2)
    assert (res["00"] + res["11"] == 1024)

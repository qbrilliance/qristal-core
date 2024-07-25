# In this example, we perform an mcx gate on all possible bitstrings.
# This examples also exists as test_amcu in tests/python_module/circuit_test.py

import numpy as np
import qb.core
import timeit

s = qb.core.session()
s.init()

num_qubits = 5
control_bits = range(num_qubits-1)
target_bit = num_qubits-1
ancilla_bits = range(num_qubits, num_qubits + num_qubits - 2)

for i in range(2**num_qubits):
    circ = qb.core.Circuit()

    # Prepare the input state
    bitstring = [int(bit) for bit in bin(i)[2:].zfill(num_qubits)]
    for j in range(num_qubits):
        if bitstring[j] == 1:
            circ.x(j)

    # Add the amcu gate
    U = qb.core.Circuit()
    U.x(target_bit)
    circ.amcu(U, control_bits, ancilla_bits)

    # Measure and check results
    for j in range(num_qubits):
        circ.measure(j)

    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.run()
    res = s.results[0][0]

    if bitstring == [1,1,1,1,1]:
        assert([1,1,1,1,0] in res)
    elif bitstring == [1,1,1,1,0]:
        assert([1,1,1,1,1] in res)
    else:
        assert(bitstring in res)

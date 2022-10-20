import numpy as np
import qbos as qb
import ast
import timeit
tqb = qb.core()
tqb.qb12()

# In this example, we perform an mcx gate on all possible bitstrings
num_qubits = 5
control_bits = range(num_qubits-1)
target_bit = num_qubits-1
ancilla_bits = range(num_qubits, num_qubits + num_qubits - 2)

for i in range(2**num_qubits):
    circ = qb.Circuit()

    # Prepare the input state
    bitstring = bin(i)[2:].zfill(5)
    for j in range(num_qubits):
        if bitstring[j] == "1":
            circ.x(j)

    # Add the amcu gate
    U = qb.Circuit()
    U.x(target_bit)
    circ.amcu(U, control_bits, ancilla_bits)

    # Measure and check results
    for j in range(num_qubits):
        circ.measure(j)

    tqb.ir_target = circ
    tqb.nooptimise = True
    tqb.noplacement = True
    tqb.notiming = True
    tqb.output_oqm_enabled = False
    tqb.acc = "qpp"
    tqb.run()

    result = tqb.out_raw[0][0]
    res = ast.literal_eval(result)

    if bitstring == "11111":
        assert("11110" in list(res.keys()))
    elif bitstring == "11110":
        assert("11111" in list(res.keys()))
    else:
        assert(bitstring in list(res.keys()))
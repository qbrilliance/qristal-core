import qristal.core
import numpy as np

s = qristal.core.session()
s.sn = 1024

###
# Testing equality checker
# compare all bitstrings of length 3
###

# Set up inputs
qubits_a = [0,1,2]
qubits_b = [3,4,5]
flag = 6

# Optional inputs for ancilla version
use_ancilla = True
qubits_ancilla = [7,8]

# First do the no ancilla version
for i in range(8):
    for j in range(8):
        circ = qristal.core.Circuit()
        # Prepare input strings
        bin_i = bin(i)[2:].zfill(3)
        bin_j = bin(j)[2:].zfill(3)
        for k in range(3):
            if bin_i[k] == "1":
                circ.x(qubits_a[k])
            if bin_j[k] == "1":
                circ.x(qubits_b[k])
        # Add equality checker
        circ.equality_checker(qubits_a, qubits_b, flag)
        # Measure flag
        circ.measure(flag)
        # Run
        s.irtarget = circ
        s.qn = circ.num_qubits()
        s.nooptimise = True
        s.noplacement = True
        s.notiming = True
        s.output_oqm_enabled = False
        s.acc = "qpp"
        s.run()
        # Check results
        if i == j:
            assert([1] in s.results)
        else:
            assert([0] in s.results)

# Now the ancilla version
for i in range(8):
    for j in range(8):
        circ = qristal.core.Circuit()
        # Prepare input strings
        bin_i = bin(i)[2:].zfill(3)
        bin_j = bin(j)[2:].zfill(3)
        for k in range(3):
            if bin_i[k] == "1":
                circ.x(qubits_a[k])
            if bin_j[k] == "1":
                circ.x(qubits_b[k])
        # Add equality checker
        circ.equality_checker(qubits_a, qubits_b, flag, use_ancilla, qubits_ancilla)
        # Measure flag
        circ.measure(flag)
        # Run
        s.irtarget = circ
        s.qn = circ.num_qubits()
        s.nooptimise = True
        s.noplacement = True
        s.notiming = True
        s.output_oqm_enabled = False
        s.acc = "qpp"
        s.run()
        # Check results
        if i == j:
            assert([1] in s.results)
        else:
            assert([0] in s.results)

import qristal.core
import numpy as np

s = qristal.core.session()
s.init()
s.qn = 10

###
# Testing subtraction
# In this example we perform every valid 5-qubit subtraction
###

for i in range(32):
    for j in range(i+1):
        # Set up input registers
        qubits_larger = [0,1,2,3,4]
        qubits_smaller = [5,6,7,8,9]

        circ = qristal.core.Circuit()

        # Prepare initial state
        bin_i = bin(i)[2:].zfill(5)
        bin_j = bin(j)[2:].zfill(5)

        for k in range(5):
            if bin_i[k] == '1':
                circ.x(qubits_larger[k])
            if bin_j[k] == '1':
                circ.x(qubits_smaller[k])

        # Perform subtraction
        circ.subtraction(qubits_larger, qubits_smaller, is_LSB = False)

        # Measure
        for k in range(10):
            circ.measure(k)

        s.ir_target = circ
        s.nooptimise = True
        s.noplacement = True
        s.notiming = True
        s.output_oqm_enabled = False
        s.acc = "qsim"
        s.run()
        res = s.results[0][0]

        expected_result = i-j
        expected_result_bin = bin(expected_result)[2:].zfill(5)
        expected_output = [int(x) for x in (expected_result_bin + bin_j)]
        assert(res[expected_output] == 1024)

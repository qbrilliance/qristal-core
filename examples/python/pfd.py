import qristal.core
import numpy as np

s = qristal.core.session()
s.init()
s.qn = 11

###
# Testing proper fraction division
# In this example we perform every valid 3-qubit proper fraction division with 3 precision bits
###

for i in range(4):
    for j in range(i+1,4):
        # Set up input registers
        qubits_numerator = [0,1]
        qubits_denominator = [2,3]
        qubits_fraction = [4,5]
        qubits_ancilla = list(range(6,11))

        circ = qristal.core.Circuit()

        # Prepare initial state
        bin_i = bin(i)[2:].zfill(2)
        bin_j = bin(j)[2:].zfill(2)


        for k in range(2):
            if bin_i[k] == '1':
                circ.x(qubits_numerator[k])
            if bin_j[k] == '1':
                circ.x(qubits_denominator[k])

        # Perform subtraction
        circ.proper_fraction_division(qubits_numerator, qubits_denominator, qubits_fraction, qubits_ancilla, is_LSB = False)

        # Measure
        for k in qubits_numerator:
            circ.measure(k)
        for k in qubits_denominator:
            circ.measure(k)
        for k in qubits_fraction:
            circ.measure(k)
        for k in qubits_ancilla:
            circ.measure(k)

        s.ir_target = circ
        s.nooptimise = True
        s.noplacement = True
        s.notiming = True
        s.output_oqm_enabled = False
        # s.debug = True
        s.acc = "qsim"
        s.run()
        res = s.results[0][0]
        print(res)
        assert(len(res) == 1)

        if j != 0:
            expected_result = i/j
        else:
            expected_result = 0

        expected_result_bin = ""

        for k in range(1, len(qubits_fraction)+1):
            if expected_result - (1/2**k) >= 0:
                expected_result_bin += "1"
                expected_result -= 1/2**k
            else:
                expected_result_bin += "0"

        expected_output = bin_i + bin_j + expected_result_bin
        for bit in qubits_ancilla:
            expected_output += "0"

        assert(res[[int(x) for x in expected_output]] == 1024)

import qristal.core
import numpy as np

s = qristal.core.session()
s.sn = 1024

###
# Testing subtraction
# In this example we perform every valid 4-qubit subtraction with 2 control bits
###
for c in range(4):
    for i in range(16):
        for j in range(i+1):
            # Set up input registers
            qubits_larger = [0,1,2,3]
            qubits_smaller = [4,5,6,7]
            control_on = [8]
            control_off = [9]

            circ = qristal.core.Circuit()

            # Prepare initial state
            bin_i = bin(i)[2:].zfill(4)
            bin_j = bin(j)[2:].zfill(4)
            bin_c = bin(c)[2:].zfill(2)

            for k in range(4):
                if bin_i[k] == '1':
                    circ.x(qubits_larger[k])
                if bin_j[k] == '1':
                    circ.x(qubits_smaller[k])

            if bin_c[0] == '1':
                circ.x(control_on[0])
            if bin_c[1] == '1':
                circ.x(control_off[0])

            # Perform subtraction
            circ.controlled_subtraction(qubits_larger, qubits_smaller, controls_on = control_on, controls_off = control_off, is_LSB=False)

            # Measure
            for k in range(8):
                circ.measure(k)

            s.irtarget = circ
            s.qn = circ.num_qubits()
            s.nooptimise = True
            s.noplacement = True
            s.notiming = True
            s.output_oqm_enabled = False
            s.acc = "qsim"
            s.run()

            if c == 2:
                expected_result = i-j
                expected_result_bin = bin(expected_result)[2:].zfill(4)
                expected_output = [int(x) for x in (expected_result_bin + bin_j)]
                assert(s.results[expected_output] == 1024)
            else:
                expected_output = [int(x) for x in (bin_i + bin_j)]
                assert(s.results[expected_output] == 1024)

import qbos as qb
import numpy as np 
import ast
tqb = qb.core()
tqb.qb12()
tqb.qn = 13

###
# Testing proper fraction division
# In this example we perform every valid 3-qubit proper fraction division with 3 precision bits
###

for c in range(4):
    for i in range(4):
        for j in range(i+1,4):
            # Set up input registers
            qubits_numerator = [0,1]
            qubits_denominator = [2,3] 
            qubits_fraction = [4,5]
            qubits_ancilla = list(range(6,11))
            controls_on = [11]
            controls_off = [12]

            circ = qb.Circuit()

            # Prepare initial state
            bin_i = bin(i)[2:].zfill(2)
            bin_j = bin(j)[2:].zfill(2)

            if c == 1:
                circ.x(controls_on[0])
            if c == 2:
                circ.x(controls_off[0])
            if c == 3:
                circ.x(controls_on[0])
                circ.x(controls_off[0])


            for k in range(2):
                if bin_i[k] == '1':
                    circ.x(qubits_numerator[k])
                if bin_j[k] == '1':
                    circ.x(qubits_denominator[k])

            # Perform subtraction
            circ.controlled_proper_fraction_division(qubits_numerator, qubits_denominator, qubits_fraction, qubits_ancilla, controls_on, controls_off, is_LSB = False)

            # Measure
            for k in qubits_numerator:
                circ.measure(k)
            for k in qubits_denominator:
                circ.measure(k)
            for k in qubits_fraction:
                circ.measure(k)
            for k in qubits_ancilla: 
                circ.measure(k)
            
            tqb.ir_target = circ
            tqb.nooptimise = True
            tqb.noplacement = True
            tqb.notiming = True
            tqb.output_oqm_enabled = False
            tqb.acc = "qsim"
            tqb.run()
            result1 = tqb.out_raw[0][0]
            res1 = ast.literal_eval(result1)
            assert(len(res1) == 1)
            # print(res1)

            if c == 1:
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
                for k in range(len(qubits_ancilla)):
                    expected_output += "0"

                assert(res1[expected_output] == 1024)
            
            else:
                expected_output = bin_i + bin_j
                for k in range(len(qubits_fraction)):
                    expected_output += "0"
                for k in range(len(qubits_ancilla)):
                    expected_output += "0"
                
                assert(res1[expected_output] == 1024)
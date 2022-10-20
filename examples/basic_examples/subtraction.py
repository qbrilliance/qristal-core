import qbos as qb
import numpy as np 
import ast
tqb = qb.core()
tqb.qb12()
tqb.qn = 10

###
# Testing subtraction
# In this example we perform every valid 5-qubit subtraction
###

for i in range(32):
    for j in range(i+1):
        # Set up input registers
        qubits_larger = [0,1,2,3,4]
        qubits_smaller = [5,6,7,8,9] 

        circ = qb.Circuit()

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

        tqb.ir_target = circ
        tqb.nooptimise = True
        tqb.noplacement = True
        tqb.notiming = True
        tqb.output_oqm_enabled = False
        tqb.acc = "qsim"
        tqb.run()
        result1 = tqb.out_raw[0][0]
        res1 = ast.literal_eval(result1)
        # print(res1)

        expected_result = i-j
        expected_result_bin = bin(expected_result)[2:].zfill(5)

        expected_output = expected_result_bin + bin_j

        assert(res1[expected_output] == 1024)
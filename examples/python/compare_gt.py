import qb.core
import ast

s = qb.core.session()
s.qb12()

for i in range(32):
    for j in range(32):

        qubits_a = [0,1,2,3,4]
        qubits_b = [5,6,7,8,9]
        qubit_flag = 10
        qubit_ancilla = 11

        circ = qb.core.Circuit()

        a_bin = bin(i)[2:].zfill(5)
        b_bin = bin(j)[2:].zfill(5)

        for k in range(5):
            ak = a_bin[k]
            bk = b_bin[k]
            if ak == '1':
                circ.x(qubits_a[k])
            if bk == '1':
                circ.x(qubits_b[k])

        circ.compare_gt(qubits_a, qubits_b, qubit_flag, qubit_ancilla, is_LSB = False)

        circ.measure_all()

        s.ir_target = circ
        s.nooptimise = True
        s.noplacement = True
        s.notiming = True
        s.output_oqm_enabled = False
        s.qn = 12
        s.sn = 1024
        s.acc = "qsim"
        s.run()
        result1 = s.out_raw[0][0]
        res1 = ast.literal_eval(result1)

        expected_output = a_bin + b_bin
        if i > j:
            expected_output += "1"
        else:
            expected_output += "0"
        expected_output += "0"

        assert(res1[expected_output] == 1024)

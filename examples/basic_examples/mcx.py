import qb.core
import numpy as np
import ast
s = qb.core.session()
s.qb12()

n = 3

ctrl_qubits = list(range(n-1))
tgt_qubit = n-1

for i in range(2**n):
    circ = qb.Circuit()
    bitstring = bin(i)[2:].zfill(n)
    print("Input = " + bitstring)
    for j in range(n):
        if bitstring[j] == '1':
            circ.x(j)
    circ.mcx(ctrl_qubits,tgt_qubit)
    circ.measure_all()
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.run()
    result = s.out_raw[0][0]
    print("Output = " + result)

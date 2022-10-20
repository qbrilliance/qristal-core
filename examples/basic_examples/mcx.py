import qbos as qb
import numpy as np
import ast
tqb = qb.core()
tqb.qb12()

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
    tqb.ir_target = circ
    tqb.nooptimise = True
    tqb.noplacement = True
    tqb.notiming = True
    tqb.output_oqm_enabled = False
    tqb.acc = "qpp"
    tqb.run()
    result = tqb.out_raw[0][0]
    print("Output = " + result)
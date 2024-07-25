import qb.core
import numpy as np

s = qb.core.session()
s.init()

n = 3

ctrl_qubits = list(range(n-1))
tgt_qubit = n-1

for i in range(2**n):
    circ = qb.core.Circuit()
    bitstring = [int(x) for x in bin(i)[2:].zfill(n)]
    print("Input:", list(reversed(bitstring)))
    for j in range(n):
        if bitstring[j] == 1:
            circ.x(j)
    circ.mcx(ctrl_qubits,tgt_qubit)
    circ.measure_all()
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "aer"
    s.run()
    print("Output:", s.results[0][0])

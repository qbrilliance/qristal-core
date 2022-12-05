import qb.core
import numpy as np
import random, math
s = qb.core.session()
s.qb12()
# s.debug = True
s.noise = False
s.nooptimise = True
s.noplacement = True
s.sn = 1024
s.acc = "qb-lambda"

def gen_circ(n_qubits, n_layers) :
    circ = qb.Circuit()
    for i in range(n_layers):
        for q in range(n_qubits):
            circ.rx(q, random.uniform(-math.pi, math.pi))
            circ.ry(q, random.uniform(-math.pi, math.pi))
            circ.rz(q, random.uniform(-math.pi, math.pi))
        for q in range(n_qubits - 1):
            circ.cnot(q, q+1)
    circ.measure_all()
    return circ

nb_qubits = 30
nb_layers = 30
ran_circ = gen_circ(nb_qubits, nb_layers)
s.qn = nb_qubits
ran_circ.print()
s.instring = ran_circ.openqasm()
s.run()
print(s.out_raw[0])

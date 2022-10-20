import qbos as qb
import numpy as np
import random, math
tqb = qb.core()
tqb.qb12()
# tqb.debug = True
tqb.noise = False
tqb.nooptimise = True
tqb.noplacement = True
tqb.sn = 1024     
tqb.acc = "qb-lambda"    

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
tqb.qn = nb_qubits
ran_circ.print()
tqb.instring = ran_circ.openqasm()
tqb.run()
print(tqb.out_raw[0])
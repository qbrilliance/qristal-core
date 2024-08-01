import qb.core
import os
import numpy as np
s = qb.core.session(True)
s.init()
s.qpu_config = os.getcwd() + "/lambda_config.json"
s.xasm = True

s.sn = 1024
s.acc = "qb-lambda"

# Generate DJ Circuit
def qbdj(qn) :
    import xacc
    import re
    bitstr = [1,0]*(qn//2)
    xgates_str=''.join(['X(q['+str(mye[0])+']);' for mye in enumerate(bitstr) if mye[1]==1])

    generator = '''
__qpu__ void qristal_circuit(qreg q) {\n'
for (int i=0; i<%d; i++) {
  H(q[i]);
}

Z(q[%d]);
// End of init

// Start of oracle
%s
for (int i=0; i<%d; i++) {
  CNOT(q[i], q[%d]);
}
%s
// End of oracle
for (int i=0; i<%d; i++) {
  H(q[i]);
}

// Measurements
for (int i=0; i<%d; i++) {
  Measure(q[i]);
}
}
''' % (qn+1,qn,xgates_str,qn,qn,xgates_str,qn,qn)
    return generator

qubits_range = [20, 22, 24, 26, 28]
for nb_qubits in qubits_range:
    print("DJ -", nb_qubits, ";Total Number of qubits:", nb_qubits + 1)
    s.qn = nb_qubits + 1
    s.instring = qbdj(nb_qubits)
    s.run()
    print(s.results[0][0])
    print("==================================")

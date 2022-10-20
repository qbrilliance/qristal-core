import qbos as qb
tqb = qb.core(True)
tqb.qb12()
tqb.xasm = True   
tqb.acc = "aer"
tqb.aer_sim_type = "matrix_product_state"
tqb.sn = 1000       
tqb.noise = True
tqb.nooptimise = True
tqb.noplacement = True
tqb.output_oqm_enabled = False

# Generate DJ Circuit
def qbdj(qn) :
    bitstr = [1,0]*(qn//2)
    xgates_str=''.join(['X(q['+str(mye[0])+']);' for mye in enumerate(bitstr) if mye[1]==1])
    
    generator = '''
__qpu__ void QBCIRCUIT(qreg q) {\n'
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

# Very large number of qubits (MPS)
nb_qubits = 40
print("DJ -", nb_qubits, ";Total Number of qubits:", nb_qubits + 1)
tqb.qn = nb_qubits + 1
tqb.instring = qbdj(nb_qubits)
tqb.run()
print(tqb.out_raw[0][0])
print("==================================")


import qbos as qb
tqb = qb.core()
tqb.qb12()
tqb.noise = True
tqb.nooptimise = True
tqb.noplacement = True
tqb.sn = 8192   
tqb.acc = "aer"    
# Simple Bell circuit
circ = qb.Circuit()
circ.h(0)    
circ.cnot(0, 1)    
circ.measure_all()

tqb.qn = 2
tqb.instring = circ.openqasm()
tqb.run()
print("Without mitigation:")
print(tqb.out_raw[0][0])

tqb.noise_mitigation = "assignment-error-kernel"
tqb.run()
print("With mitigation:")
print(tqb.out_raw[0][0])

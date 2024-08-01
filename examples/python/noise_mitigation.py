import qristal.core
s = qristal.core.session()
s.init()
s.noise = True
s.nooptimise = True
s.noplacement = True
s.sn = 8192
s.acc = "aer"
# Simple Bell circuit
circ = qristal.core.Circuit()
circ.h(0)
circ.cnot(0, 1)
circ.measure_all()

s.qn = 2
s.instring = circ.openqasm()
s.run()
print("Without mitigation:")
print(s.results[0][0])

s.noise_mitigation = "assignment-error-kernel"
s.run()
print("With mitigation:")
print(s.results[0][0])

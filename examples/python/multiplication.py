import qristal.core
import numpy as np

s = qristal.core.session()
s.acc = "qsim"
s.sn = 1024
s.init()
s.qn = 9

##########################################################################
# Test 1: qubit_a = 1 = |10>, qubit_b = 3 = |11>,
# qubit_result = 1 x 3 = 3 = |1100>
# Inputs
qubits_a = [0,1]
qubits_b = [2,3]
qubits_result = [4,5,6,7]
qubit_ancilla = 8

circ = qristal.core.Circuit()

# Prepare inputs
circ.x(qubits_a[0])
circ.x(qubits_b[0])
circ.x(qubits_b[1])

circ.multiplication(qubits_a, qubits_b, qubits_result, qubit_ancilla, is_LSB=True)

# Measure
for i in range(len(qubits_a)):
  circ.measure(qubits_a[i])
for i in range(len(qubits_b)):
  circ.measure(qubits_b[i])
for i in range(len(qubits_result)):
  circ.measure(qubits_result[i])

circ.print()

# Run circuit
s.ir_target = circ
s.nooptimise = True
s.noplacement = True
s.notiming = True
s.output_oqm_enabled = False
s.run()
res = s.results[0][0]
print(res)
assert(res[[1,0,1,1,1,1,0,0]] == 1024)

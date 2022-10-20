import qbos as qb
import numpy as np
import ast
tqb = qb.core()
tqb.acc = "qsim"
tqb.sn = 1024
tqb.qb12()
tqb.qn = 9

##########################################################################
# Test 1: qubit_a = 1 = |10>, qubit_b = 3 = |11>,
# qubit_result = 1 x 3 = 3 = |1100>
# Inputs
qubits_a = [0,1]
qubits_b = [2,3]
qubits_result = [4,5,6,7]
qubit_ancilla = 8

circ = qb.Circuit()

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
tqb.ir_target = circ
tqb.nooptimise = True
tqb.noplacement = True
tqb.notiming = True
tqb.output_oqm_enabled = False
tqb.run()
result = tqb.out_raw[0][0]
res = ast.literal_eval(result)
print(res)
assert(res["10111100"] == 1024)
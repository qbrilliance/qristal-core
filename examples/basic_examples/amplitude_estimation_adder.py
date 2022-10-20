import qbos as qb
import numpy as np
import ast
tqb = qb.core()
tqb.acc = "qsim"
tqb.sn = 1024
tqb.qb12()
tqb.qn = 18

#Inputs
q0 = 0
q1 = 1
q2 = 2
qubits_flags = [3,4]
qubits_string = [5,6]
qubits_string_copy = [7,8]
total_metric = [9,10]
qubits_metric = [11,12]
evaluation_qubits = [13,14,15]
qubits_ancilla = [16,17]
precision_bits = [1,2]

circ = qb.Circuit()

for i in range(len(qubits_string)):
  circ.h(qubits_string[i])

controls_on0 = []
controls_off0 = []
for i in range(len(qubits_string)):
  controls_on0.append(qubits_string[i])
for i in range(len(total_metric)):
  circ.generalised_mcx(total_metric[i], controls_on0, controls_off0)

controls_on1 = []
controls_off1 = []
controls_on1.append(qubits_string[0])
controls_off1.append(qubits_string[1])
circ.generalised_mcx(total_metric[0], controls_on1, controls_off1)

controls_on2 = []
controls_off2 = []
controls_on2.append(qubits_string[1])
controls_off2.append(qubits_string[0])
circ.generalised_mcx(total_metric[1], controls_on2, controls_off2)

# State preparation
state_prep = qb.Circuit()
for i in range(len(total_metric)):
  state_prep.cnot(total_metric[i], qubits_metric[i])
# Create copy of state preparation for the circuit
for i in range(len(total_metric)):
  circ.cnot(total_metric[i], qubits_metric[i])

state_qubits = []
for i in range(len(total_metric)):
  state_qubits.append(total_metric[i])
for i in range(len(qubits_metric)):
  state_qubits.append(qubits_metric[i])

circ.amplitude_estimation_adder(state_prep, q0, q1, q2, qubits_flags, qubits_string, qubits_metric, precision_bits, evaluation_qubits, state_qubits, qubits_ancilla)

# Measure
for i in range(len(evaluation_qubits)):
  circ.measure(evaluation_qubits[i])
for i in range(len(qubits_string)):
  circ.measure(qubits_string[i])
for i in range(len(qubits_metric)):
  circ.measure(qubits_metric[i])

#circ.print()

#Run
tqb.ir_target = circ #tqb.instring = circ.openqasm()
tqb.run()
print(tqb.out_raw[0])
result = tqb.out_raw[0][0]
res = ast.literal_eval(result)
assert(res["0000000"] > 0)
assert(res["0010101"] > 0)
assert(res["1001010"] > 0)
assert(res["1011111"] > 0)
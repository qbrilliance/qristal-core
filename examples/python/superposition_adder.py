import qristal.core
import numpy as np

s = qristal.core.session()
s.acc = "sparse-sim"
s.sn = 1024
s.qn = 32

#Inputs
q0 = 0
q1 = 1
q2 = 2
qubits_string = [3,4]
qubits_metric = [5,6]
qubits_superfluous_flags = [7,8]
qubits_beam_metric = [9,10,11]
qubits_ancilla = []
for i in range(12,32):
  qubits_ancilla.append(i)

# Let's define the state
# |string>|metric>|flags> = |00>|11>|11> + |10>|10>|01> + |10>|11>|01> + |11>|01>|01>

# Strings
ae_state_prep_circ = qristal.core.Circuit()
for i in range(len(qubits_string)):
  ae_state_prep_circ.h(qubits_string[i])

# Metrics
controls_on0 = []
controls_off0 = []
for i in range(len(qubits_string)):
  controls_off0.append(qubits_string[i])
ae_state_prep_circ.generalised_mcx(qubits_metric[0], controls_on0, controls_off0)

controls_on1 = []
controls_off1 = []
for i in range(len(qubits_string)):
  controls_off1.append(qubits_string[i])
ae_state_prep_circ.generalised_mcx(qubits_metric[1], controls_on1, controls_off1)

controls_on2 = []
controls_off2 = []
controls_on2.append(qubits_string[0])
controls_off2.append(qubits_string[1])
ae_state_prep_circ.generalised_mcx(qubits_metric[0], controls_on2, controls_off2)

controls_on3 = []
controls_off3 = []
controls_on3.append(qubits_string[1])
controls_off3.append(qubits_string[0])
ae_state_prep_circ.generalised_mcx(qubits_metric[0], controls_on3, controls_off3)

controls_on4 = []
controls_off4 = []
controls_on4.append(qubits_string[1])
controls_off4.append(qubits_string[0])
ae_state_prep_circ.generalised_mcx(qubits_metric[1], controls_on4, controls_off4)

controls_on5 = []
controls_off5 = []
for i in range(len(qubits_string)):
  controls_on5.append(qubits_string[i])
ae_state_prep_circ.generalised_mcx(qubits_metric[1], controls_on5, controls_off5)

# Flags
ae_state_prep_circ.x(qubits_superfluous_flags[1])

controls_on6 = []
controls_off6 = []
for i in range(len(qubits_string)):
  controls_off6.append(qubits_string[i])
ae_state_prep_circ.generalised_mcx(qubits_superfluous_flags[0], controls_on6, controls_off6)

# String -> beam
qubits_a = []
qubits_b = []
swap_flags_on = []
swap_flags_off = []
qubits_a.append(qubits_string[0])
qubits_b.append(qubits_string[1])
swap_flags_on.append(qubits_metric[0])
swap_flags_on.append(qubits_metric[1])
swap_flags_on.append(qubits_superfluous_flags[1])
swap_flags_off.append(qubits_superfluous_flags[0])
ae_state_prep_circ.controlled_swap(qubits_a, qubits_b, swap_flags_on, swap_flags_off)

circ = qristal.core.Circuit()
circ.superposition_adder(q0, q1, q2, qubits_superfluous_flags, qubits_string, qubits_metric, ae_state_prep_circ, qubits_ancilla, qubits_beam_metric)

# Measure
for i in range(len(qubits_beam_metric)):
  circ.measure(qubits_beam_metric[i])
for i in range(len(qubits_string)):
  circ.measure(qubits_string[i])
for i in range(len(qubits_metric)):
  circ.measure(qubits_metric[i])
for i in range(len(qubits_superfluous_flags)):
  circ.measure(qubits_superfluous_flags[i])
#circ.print()

#Run
s.irtarget = circ
s.run()
res = s.results
print(res)
assert(res[[0,0,0,0,0,1,1,1,1]] > 0)
assert(res[[0,0,1,1,0,1,0,0,1]] > 0)
assert(res[[0,0,1,1,0,1,1,0,1]] > 0)
assert(res[[0,0,1,1,1,0,1,0,1]] > 0)

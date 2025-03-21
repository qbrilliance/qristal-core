import qristal.core
import numpy as np
import timeit

s = qristal.core.session()
s.init()
s.qn = 7

###
# Testing controlled addition.
# In this example we test adding |10> to |000> conditional on a flag
# When the flag is off we expect no addition to happen and the output sum to be |000>
# When the flag is on we expect addition to happen and the output sum to be |100>
###

# Set up input registers
qubits_adder = [0,1]
qubits_sum = [2,3,4] # Remember that the sum register needs more qubits than the adder register for overflow
c_in = 5
flag = [6]

###
# Test 1: flag off
###

circ1 = qristal.core.Circuit()

# Prepare initial state
circ1.x(qubits_adder[0])

# Perform conditional addition
circ1.controlled_ripple_carry_adder(qubits_adder, qubits_sum, c_in, flags_on = flag)

# Measure sum register
for i in range(len(qubits_sum)):
    circ1.measure(qubits_sum[i])

s.ir_target = circ1
s.nooptimise = True
s.noplacement = True
s.notiming = True
s.output_oqm_enabled = False
s.acc = "qsim"
start = timeit.default_timer()
s.run()
end = timeit.default_timer()
print("runtime " + str(end-start))
res = s.results[0][0]
assert(res[[0,0,0]] == 1024)

###
# Test 2: flag off
###

circ2 = qristal.core.Circuit()

# Prepare initial state
circ2.x(qubits_adder[0])

# Prepare flag
circ2.x(flag[0])

# Perform conditional addition
circ2.controlled_ripple_carry_adder(qubits_adder, qubits_sum, c_in, flags_on = flag)

# Measure sum register
for i in range(len(qubits_sum)):
    circ2.measure(qubits_sum[i])

s.ir_target = circ2
s.nooptimise = True
s.noplacement = True
s.notiming = True
s.output_oqm_enabled = False
s.acc = "qpp"
s.run()
res = s.results[0][0]
assert(res[[1,0,0]] == 1024)

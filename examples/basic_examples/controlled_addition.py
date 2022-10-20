import qbos as qb
import numpy as np 
import ast
import timeit
tqb = qb.core()
tqb.qb12()
tqb.qn = 7

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

circ1 = qb.Circuit()

# Prepare initial state
circ1.x(qubits_adder[0])

# Perform conditional addition
circ1.controlled_ripple_carry_adder(qubits_adder, qubits_sum, c_in, flags_on = flag)

# Measure sum register
for i in range(len(qubits_sum)):
    circ1.measure(qubits_sum[i])

tqb.ir_target = circ1
tqb.nooptimise = True
tqb.noplacement = True
tqb.notiming = True
tqb.output_oqm_enabled = False
tqb.acc = "qsim"
start = timeit.default_timer()
tqb.run()
end = timeit.default_timer()
print("runtime " + str(end-start))
result1 = tqb.out_raw[0][0]
res1 = ast.literal_eval(result1)
assert(res1["000"] == 1024)

### 
# Test 2: flag off
###

circ2 = qb.Circuit()

# Prepare initial state
circ2.x(qubits_adder[0])

# Prepare flag
circ2.x(flag[0])

# Perform conditional addition
circ2.controlled_ripple_carry_adder(qubits_adder, qubits_sum, c_in, flags_on = flag)

# Measure sum register
for i in range(len(qubits_sum)):
    circ2.measure(qubits_sum[i])

tqb.ir_target = circ2
tqb.nooptimise = True
tqb.noplacement = True
tqb.notiming = True
tqb.output_oqm_enabled = False
tqb.acc = "qpp"
tqb.run()
result2 = tqb.out_raw[0][0]
res2 = ast.literal_eval(result2)
assert(res2["100"] == 1024)
import qbos as qb
import numpy as np
import ast
tqb = qb.core()
tqb.qb12()
tqb.qn = 14

# Efficient Encoding: given the input |state>|00...0> 
# produces the output |state>|score> where the score for
# each bitstring is given by a scoring function

###
# Example 1: specify qubit registers
###

# In this example, we produce the state |i>|i> with three qubits in each register
# I.e., we produce |000>|000> + ... + |111>|111>

# scoring function
def scoring_function(i):
    return i

# input parameters
num_state_qubits = 5
num_scoring_qubits = 5
state_qubits = [5,6,7,8,9]
scoring_qubits = [0,1,2,3,4]

# create circuit
circ = qb.Circuit()

# State prep
for k in range(num_state_qubits):
    circ.h(state_qubits[k])

# Add ee
circ.efficient_encoding(scoring_function, num_state_qubits, num_scoring_qubits, state_qubits, scoring_qubits)

cx = 0
sq = -4
circ_string = circ.openqasm()
for i in circ_string.splitlines():
    if i[0:2] == "CX":
        cx += 1
    else:
        sq += 1
print("Efficient Encoding without ancilla uses: ")
print(cx, " CX gates")
print(sq, " SQ gates")

# Measure
circ.measure_all()

# Check the circuit:
#print("OpenQASM:\n", circ.openqasm())

# Run:
tqb.ir_target = circ
tqb.nooptimise = True
tqb.noplacement = True
tqb.notiming = True
tqb.output_oqm_enabled = False
tqb.acc = "qpp"
tqb.run()
result = tqb.out_raw[0][0]
res = ast.literal_eval(result)
    

###
# Example 2: use ancilla
###

# In this example, we produce the state |i>|i> with three qubits in each register
# I.e., we produce |000>|000> + ... + |111>|111>

# input parameters
num_state_qubits = 5
num_scoring_qubits = 5
state_qubits = [5,6,7,8,9]
scoring_qubits = [0,1,2,3,4]

# create circuit
circ = qb.Circuit()

# State prep
for k in range(num_state_qubits):
    circ.h(state_qubits[k])

# Add ee
circ.efficient_encoding(scoring_function, num_state_qubits, num_scoring_qubits, state_qubits, scoring_qubits, use_ancilla = True)

cx = 0
sq = -4
circ_string = circ.openqasm()
for i in circ_string.splitlines():
    if i[0:2] == "CX":
        cx += 1
    else:
        sq += 1
print("Efficient Encoding with ancilla uses: ")
print(cx, " CX gates")
print(sq, " SQ gates")

# Measure
for i in state_qubits:
    circ.measure(i)
for i in scoring_qubits:
    circ.measure(i)

# Check the circuit:
#print("OpenQASM:\n", circ.openqasm())

# Run:
tqb.ir_target = circ
tqb.nooptimise = True
tqb.noplacement = True
tqb.notiming = True
tqb.output_oqm_enabled = False
tqb.acc = "qpp"
tqb.run()
result = tqb.out_raw[0][0]
res = ast.literal_eval(result)


# ###
# # Example 3: dont' specify qubit registers
# ###
# # In this example, we produce the state |000>|000> + |111>|111>

# # scoring function
# def scoring_function(i):
#     return i

# # input parameters
# num_state_qubits = 3
# num_scoring_qubits = 3

# # create circuit
# circ = qb.Circuit()

# # State prep: |000>|000> + |111>|0000>
# circ.h(0)
# for k in range(num_state_qubits-1): 
#     circ.cnot(k,k+1)

# # Add ee
# circ.efficient_encoding(scoring_function, num_state_qubits, num_scoring_qubits)

# # Measure
# circ.measure_all()

# # Check the circuit:
# #print("OpenQASM:\n", circ.openqasm())

# # Run:
# result = circ.execute()
# print("Result:\n", result)

# # get results
# res = ast.literal_eval(result)
# measurements = res["AcceleratorBuffer"]["Measurements"]
# allowed_outputs = ['000000', '111111']
# for measurement in measurements:
#     assert(measurement in allowed_outputs)


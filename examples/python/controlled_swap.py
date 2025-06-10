import qristal.core
import numpy as np

s = qristal.core.session()
s.sn = 1000

###
# Testing controlled swap
# Define an input string from the alphabet {a,b}
# Entangle the input string letters to a flag indicating whether the letter is "b"
# Conditional on this input flag, use controlled swap to move all b's to the end of the string
# E.g., baba -> aabb
###

# Set up
qubits_string = [0,1,2,3,4]
b_flags = [5,6,7,8,9]

input_string = "babba"
assert(len(input_string) == len(qubits_string))

# Prepare input state
circ = qristal.core.Circuit()
for i in range(len(input_string)):
    if input_string[i] == "b":
        circ.x(qubits_string[i])

for i in range(len(input_string)):
    circ.cnot(qubits_string[i], b_flags[i])

# Perform conditional swaps
for i in range(len(input_string)):
    k = len(input_string) - 1 - i
    for j in range(k, len(input_string)-1):
        qubits_a = [qubits_string[j]]
        qubits_b = [qubits_string[j+1]]
        flags_on = [b_flags[k]]
        circ.controlled_swap(qubits_a=qubits_a, qubits_b=qubits_b, flags_on=flags_on)

# Measure
for i in range(len(input_string)):
    circ.measure(qubits_string[i])

# Execute circuit

s.irtarget = circ
s.qn = circ.num_qubits()
s.nooptimise = True
s.noplacement = True
s.notiming = True
s.output_oqm_enabled = False
s.acc = "qpp"
s.run()
assert(len(s.results) == 1)
output_measurement = list(next(iter(s.results)))

output_string = ""
for x in output_measurement:
    if x == 0:
        output_string += "a"
    else:
        output_string += "b"

expected_string = ""
num_b = 0
for i in range(len(input_string)):
    if input_string[i] == "b":
        num_b += 1
for i in range(len(input_string) - num_b):
    expected_string += "a"
for i in range(num_b):
    expected_string += "b"

expected_measurement = []
for i in range(len(expected_string)):
    if expected_string[i] == "a":
        expected_measurement.append(0)
    else:
        expected_measurement.append(1)

assert(output_measurement == expected_measurement)

print("The input string is ", input_string)
print("The expected output string is ", expected_string)
print("The measured output string is ", output_string)

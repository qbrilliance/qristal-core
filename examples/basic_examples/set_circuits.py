import qbos as qb
import numpy as np
import ast
import timeit
tqb = qb.core()
tqb.qb12()
tqb.qn = 3

# In this test we use generalised mcx to
# perform mcx on all possible 3-qubit bit strings (|000>,...,|111>)
# with all combinations of control qubit conditions ((on,on),...,(off,off))

# Expected outcomes:

# - When the control qubits are required to be (on,on) then we expect
# |110> -> |111>, |111> -> |110>, and all other bit strings remain unchanged

# - When the control qubits are required to be (on,off) then we expect
# |100> -> |101>, |101> -> |100>, and all other bit strings remain unchanged

# - When the control qubits are required to be (off,on) then we expect
# |010> -> |011>, |011> -> |010>, and all other bit strings remain unchanged

# - When the control qubits are required to be (off,off) then we expect
# |000> -> |001>, |001> -> |000>, and all other bit strings remain unchanged

control_qubits = [0, 1]
target_qubit = 2

conditions = [[0, 0], [0, 1], [1, 0], [1, 1]]
input_bitstrings = ["000", "001", "010", "011", "100", "101", "110", "111"]

# We will set up an array of circuits and execute them all at once
circuit_conditions = []
circuits = []
for condition in conditions:
    for input_bitstring in input_bitstrings:
        circ = qb.Circuit()
        
        # Prepare the input bitstring
        for i in range(len(input_bitstring)):
            if input_bitstring[i] == "1":
                circ.x(i)
        
        # Add the generalised mcx
        controls_on = []
        controls_off = []
        for i in range(len(condition)):
            if condition[i] == 0:
                controls_off.append(control_qubits[i])
            else:
                controls_on.append(control_qubits[i])
        circ.generalised_mcx(target_qubit, controls_on, controls_off)
        
        # Measurements
        circ.measure_all()
        circ.execute()

        # Add to the temp list
        circuits.append([circ])
        circuit_conditions.append([condition, input_bitstring])

# Run the circuits
tqb.ir_targets = circuits
tqb.sn[0].clear()
sweep = [1,100]
[tqb.sn[0].append(nn) for nn in sweep]
tqb.run()

# Check results
result_col1 = [tqb.out_raw[i][0] for i in range(len(circuits))]
res_col1 = [ast.literal_eval(result_col1[i]) for i in range(len(result_col1))]
result_col2 = [tqb.out_raw[i][1] for i in range(len(circuits))]
res_col2 = [ast.literal_eval(result_col2[i]) for i in range(len(result_col2))]

for i in range(len(res_col1)):
    con = circuit_conditions[i][0]
    inp = circuit_conditions[i][1]
    expected_output = inp[0:2]
    if int(inp[0]) == con[0] and int(inp[1]) == con[1]:
        if inp[2] == "0":
            expected_output += "1"
        else:
            expected_output += "0"
    else:
        expected_output += inp[2]
    output = "The input string was " + inp + ". MCX was applied if the control qubits were " + str(con)
    output += ". Hence, the expected result is " + expected_output + "."
    assert(list(res_col1[i].keys())[0] == expected_output)
    assert(res_col1[i][expected_output] == 1)
    output += "The actual output is " + expected_output
    print(output)

for i in range(len(res_col2)):
    con = circuit_conditions[i][0]
    inp = circuit_conditions[i][1]
    expected_output = inp[0:2]
    if int(inp[0]) == con[0] and int(inp[1]) == con[1]:
        if inp[2] == "0":
            expected_output += "1"
        else:
            expected_output += "0"
    else:
        expected_output += inp[2]
    output = "The input string was " + inp + ". MCX was applied if the control qubits were " + str(con)
    output += ". Hence, the expected result is " + expected_output + "."

    assert(list(res_col2[i].keys())[0] == expected_output)
    assert(res_col2[i][expected_output] == 100)
    output += " The actual output is " + expected_output
    print(output)

print("All circuits successful")
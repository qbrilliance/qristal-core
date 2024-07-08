import qb.core
import numpy as np
import ast
s = qb.core.session()
s.init()

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
for condition in conditions:
    for input_bitstring in input_bitstrings:
        circ = qb.core.Circuit()

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

        # Run the circuit and check results
        s.ir_target = circ
        s.nooptimise = True
        s.noplacement = True
        s.notiming = True
        s.output_oqm_enabled = False
        s.acc = "qpp"
        s.run()
        result = s.out_raw_json[0][0]
        res = ast.literal_eval(result)
        expected_output = input_bitstring[0:2]
        if int(input_bitstring[0]) == condition[0] and int(input_bitstring[1]) == condition[1]:
            if input_bitstring[2] == "0":
                expected_output += "1"
            else:
                expected_output += "0"
        else:
            expected_output += input_bitstring[2]
        assert(list(res.keys())[0] == expected_output)
        assert(res[expected_output] == 1024)

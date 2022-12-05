import qb.core
import numpy as np
import ast
import timeit
s = qb.core.session()
s.qb12()
s.sn = 1024
s.qn = 3
s.acc = "qpp"

# In this test we use generalised mcx to
# perform a standard mcx on |111>

# Expected outcome: |110>

control_qubits = [0, 1]
target_qubit = 2

circ = qb.core.Circuit()

# prepare initial state
circ.x(0)
circ.x(1)
circ.x(2)

# add generalised mcx
circ.generalised_mcx(target_qubit, control_qubits)

# measure
circ.measure_all()

# run the circuit and check results
s.ir_target = circ
s.run()

results = s.out_raw[0][0]
res = ast.literal_eval(results)
assert(res["110"] == 1024)
print("success")

import qbos as qb
import numpy as np
import ast
import timeit
tqb = qb.core()
tqb.qb12()
tqb.sn = 1024
tqb.qn = 3
tqb.acc = "qsim"
tqb.noise = True
# In this test we use generalised mcx to
# perform a standard mcx on |111>

# Expected outcome: |110>

control_qubits = [0, 1]
target_qubit = 2

circ = qb.Circuit()

# prepare initial state
circ.x(0)
circ.x(1)
circ.x(2)

# add generalised mcx
circ.generalised_mcx(target_qubit, control_qubits)

# measure
circ.measure_all()

# run the circuit and check results
tqb.ir_target = circ
tqb.run()

results = tqb.out_raw[0][0]
res = ast.literal_eval(results)
print(res)
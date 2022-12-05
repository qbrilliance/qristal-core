import qb.core
import ast
s = qb.core.session(True)
s.qb12()

# Use the aer accelerator, matrix product state method
s.acc = "aer"
s.aer_sim_type = "matrix_product_state"
s.sn = 1000
s.noise = True

s.instring='''
__qpu__ void QBCIRCUIT(qreg q) {
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[2];
    h q[1];
    cx q[1],q[0];
    measure q[1] -> c[1];
    measure q[0] -> c[0];
}
'''

s.run()
results = s.out_raw[0][0]
res = ast.literal_eval(results)
print(res)

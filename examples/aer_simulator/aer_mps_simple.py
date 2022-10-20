import qbos
import ast 
tqb=qbos.core(True)
tqb.qb12()

# Use the aer accelerator, matrix product state method
tqb.acc = "aer"
tqb.aer_sim_type = "matrix_product_state"
tqb.sn = 1000       
tqb.noise = True

tqb.instring='''
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

tqb.run()
results = tqb.out_raw[0][0]
res = ast.literal_eval(results)
print(res)
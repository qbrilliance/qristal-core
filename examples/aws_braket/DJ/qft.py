import qbos
#from braket.aws import AwsDevice

import numpy as np

#from braket.circuits import Circuit
#from braket.devices import LocalSimulator

tqb = qbos.core()
tqb.qb12()

tqb.xasm = True
tqb.sn = 1024
tqb.acc = "aws_acc"
tqb.aws_device = "DM1"
tqb.verbatim = True
tqb.noise = True
print("Backend chosen: ", tqb.acc)
print("Device from qft.py: ", tqb.aws_device)

tqb.qn.clear()
#tqb.qn.append(qbos.N([2]))  # 2-qubits for the top row
tqb.qn.append(qbos.N([4]))

#device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/sv1")

qpu_kernel_qft_4 = '''
__qpu__ void QBCIRCUIT(qreg q)
{
   X(q[3]);
    // Hadamard on all qubits
    H(q[0]);
    H(q[1]);
    H(q[2]);
    H(q[3]);
    // Balanced Oracle
    X(q[0]);
    X(q[2]);
    CX(q[0],q[3]);
    CX(q[1],q[3]);
    CX(q[2],q[3]);
    X(q[0]);
    X(q[2]);   
    // Hadamard on q[0-2]
    H(q[0]);
    H(q[1]);
    H(q[2]);

    Measure(q[0]);
    Measure(q[1]);
    Measure(q[2]);
    Measure(q[3]);
}
'''


tqb.instring.clear()
tqb.instring.append(qbos.String([qpu_kernel_qft_4]))

tqb.nosim = False

tqb.out_qobj = 'Hello'
print("out_qobj: ", tqb.out_qobj)
tqb.run()

print("Output count: \n", tqb.out_count[0])

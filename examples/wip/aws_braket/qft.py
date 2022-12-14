import qb.core
#from braket.aws import AwsDevice

import numpy as np

#from braket.circuits import Circuit
#from braket.devices import LocalSimulator

s = qb.core.session()
s.qb12()

s.xasm = True
s.sn = 1024
s.acc = "aws_acc"
s.aws_device = "DM1"
s.aws_verbatim = True
s.noise = True
print("Backend chosen: ", s.acc)
print("Device from qft.py: ", s.aws_device)

s.qn.clear()
#s.qn.append(qb.core.N([2]))  # 2-qubits for the top row
s.qn.append(qb.core.N([4]))

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


s.instring.clear()
s.instring.append(qb.core.String([qpu_kernel_qft_4]))

s.nosim = False

s.out_qobj = 'Hello'
print("out_qobj: ", s.out_qobj)
s.run()

print("Output count: \n", s.out_count[0])

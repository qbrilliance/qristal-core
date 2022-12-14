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
s.aws_device = "Rigetti"
s.nooptimise = True
s.noplacement = True
s.aws_verbatim = True
s.aws_format = "braket"
s.noise = False
print("Backend chosen: ", s.acc)
print("Device from qft.py: ", s.aws_device)

s.qn.clear()
#s.qn.append(qb.core.N([2]))  # 2-qubits for the top row
s.qn.append(qb.core.N([4]))

#device = AwsDevice("arn:aws:braket:::device/quantum-simulator/amazon/sv1")

qpu_kernel_qft_4 = '''
__qpu__ void QBCIRCUIT(qreg q)
{
    Rz(q[0], 0.63);
    Ry(q[1], 0.63);
    H(q[1]);
    CNOT(q[0], q[1]);
    CZ(q[0], q[1]);
    XY(q[0], q[1], 0.1);
    CPhase(q[0], q[1], 0.1);

    Measure (q[0]);
    Measure (q[1]);
}
'''

test = '''
__qpu__ void QBCIRCUIT(qreg q)
{
    Rx(q[0], 3.14159);
    Rx(q[1], 1.5708);
    H(q[0]);

    Measure (q[0]);
    Measure (q[1]);
}
'''

s.instring.clear()
s.instring.append(qb.core.String([test]))

s.nosim = False

s.out_qobj = 'Hello'
print("out_qobj: ", s.out_qobj)
s.run()

print("Output count: \n", s.out_count[0])

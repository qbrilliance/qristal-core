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
tqb.aws_device = "Rigetti"
tqb.nooptimise = True
tqb.noplacement = True
tqb.verbatim = True
tqb.format = "Braket"
#tqb.format = "OPENQASM 3"
tqb.noise = False
print("Backend chosen: ", tqb.acc)
print("Device from qft.py: ", tqb.aws_device)

tqb.qn.clear()
#tqb.qn.append(qbos.N([2]))  # 2-qubits for the top row
tqb.qn.append(qbos.N([4]))

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

tqb.instring.clear()
#tqb.instring.append(qbos.String([qpu_kernel_qft_4]))
tqb.instring.append(qbos.String([test]))

tqb.nosim = False

tqb.out_qobj = 'Hello'
print("out_qobj: ", tqb.out_qobj)
tqb.run()

print("Output count: \n", tqb.out_count[0])

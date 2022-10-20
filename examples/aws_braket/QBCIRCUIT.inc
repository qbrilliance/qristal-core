
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
u(3.14159, -1.5708, 1.5708) q[0];
u(1.5708, -1.5708, 1.5708) q[1];
u(1.5708, 0, 0) q[0];
u(3.14159, -1.5708, 1.5708) q[0];
creg c0[1];
measure q[0] -> c0[0];
creg c1[1];
measure q[1] -> c1[0];

}


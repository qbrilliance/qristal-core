#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
// Copyright (c) Quantum Brilliance Pty Ltd
"""

import qristal.core
import json as json
import time as time

# Set up the session
s = qristal.core.session()
s.acc = "aer"
s.sn = 100000
s.qn = 2
s.instring='''
__qpu__ void qristal_circuit(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
creg c[2];
h q[1];
cx q[1],q[0];
measure q[1] -> c[1];
measure q[0] -> c[0];
}
'''

# Set the circuit off async
handle = s.run_async()

# Print diagnostic until the job acquires the GIL from this thread
while (not handle.complete()):
    print("Waiting for run to start...")

print("Completed!")

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
// Copyright (c) Quantum Brilliance Pty Ltd
"""

import qristal.core
import json as json
import time as time

NJ = 100 # number of jobs

circuit = '''
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

# Set NJ circuits off async
handles = []
s = []
for i in range(NJ):
    s.append(qristal.core.session())
    s[i].acc = "aer"
    s[i].sn = 100000
    s[i].qn = 2
    s[i].instring = circuit
    handles.append(s[i].run_async())
print("Complete posting all", NJ,"jobs")

all_done = False
completed_counter_prev = 0
while (not all_done):
    all_done = True
    completed_counter = 0
    for handle in handles:
        if (not handle.complete()):
            all_done = False
        else:
            completed_counter = completed_counter + 1
    if (not all_done):
        if (completed_counter_prev != completed_counter):
            print(completed_counter,  "/", NJ,"completed")
            completed_counter_prev = completed_counter
    else:
        print("All done!")


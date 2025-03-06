#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
// Copyright (c) Quantum Brilliance Pty Ltd
"""

import qristal.core
import json as json
import time as time

s = qristal.core.session()
s.init()

NW = 32  # number of async workers
NJ = 200 # number of jobs
ss = 0.5 # seconds to sleep between progress

# Set up workers

# Set up the pool of QPUs for parallel task distribution
qpu_config = {"accs": NW*[{"acc": "aer"}]}
s.set_parallel_run_config(json.dumps(qpu_config))
s.acc = "aer"

# Set the number of threads to use in the thread pool
s.num_threads=2


# Set up jobs
circuit_str='''
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
s.instring.clear()
for i in range(NJ):
    s.instring.append(qristal.core.VectorString([circuit_str]))

handles = []
for i in range(NJ):
    handles.append(s.run_async(i, 0))
    time.sleep(0.0001)
print("Complete posting all", NJ,"jobs")

all_done = False
loopCounter = 0
while (not all_done):
    time.sleep(ss)
    all_done = True
    completed_counter = 0
    for handle in handles:
        if (not handle.complete()):
            all_done = False
        else:
            completed_counter = completed_counter + 1
    loopCounter = loopCounter + 1
    if (not all_done):
        print(completed_counter,  "/", NJ,"completed")
    else:
        print("All done in ", loopCounter, " iterations!")

#print([s.run_complete(i,0) for i in range(NJ)])
#print([handles[i].get() for i in range(NJ)])

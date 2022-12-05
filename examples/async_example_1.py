#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
// Copyright (c) 2022 Quantum Brilliance Pty Ltd
"""

import qb.core
import json as json
import time as time
#%%
s = qb.core.session()
s.qb12()

NW = 32 #64  #number of async workers
NJ = 200 #NW*20 #number of jobs
ss = 0.5  # seconds to sleep between progress

#%% set up workers

# Set up the pool of QPU for parallel task distribution
qpu_config = {"accs": NW*[{"acc": "aer"}]}
#qpu_config = {"acc": NW*[{"acc": "aer"}]} # first key is wrong should be accs. Use this to test the silent infinite loop fix
s.set_parallel_run_config(json.dumps(qpu_config))
s.acc = "aer"

s.num_threads=2


#%% set up jobs
circuit_str='''
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
s.instring.clear()
for i in range(NJ):
    s.instring.append(qb.core.String([circuit_str]))

#%%
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

#%%
#print([s.run_complete(i,0) for i in range(NJ)])
#print([handles[i].get() for i in range(NJ)])

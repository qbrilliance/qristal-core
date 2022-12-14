#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
// Copyright (c) 2022 Quantum Brilliance Pty Ltd
"""

import qb.core
import json as json
import time as time

s = qb.core.session()
s.qb12()

NOL = 50   # number of outer loops
NW = 32    # number of async workers
NJ = NW*20 # number of jobs
ss = 2     # seconds to sleep between progress

# Set up workers

# Set up the pool of QPUs for parallel task distribution
qpu_config = {"accs": NW*[{"acc": "aer"}]}
s.set_parallel_run_config(json.dumps(qpu_config))
s.acc = "aer"


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

for outerLoop in range(NOL):
  print("\nOuterLoop: (",outerLoop+1,"/",NOL,")")

  s.instring.clear()
  for i in range(NJ):
    s.instring.append(qb.core.String([circuit_str]))

  handles = []
  for i in range(NJ):
    handles.append(s.run_async(i, 0))
    time.sleep(0.0001)
  print("\tComplete posting all", NJ,"jobs")

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
      print("\tWorkers ready: (",completed_counter,"/",NJ,")")
    else:
      print("finished all workers in ",loopCounter," iterations!")

print("End")

#print([s.run_complete(i,0) for i in range(NJ)])
#print([handles[i].get() for i in range(NJ)])

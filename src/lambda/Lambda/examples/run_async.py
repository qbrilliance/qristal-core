import qb.core
import json
import numpy as np
s = qb.core.session()
s.init()
qpu_configs = {"accs": [{"acc": "aer"}, {"acc": "qb-lambda"}]}
s.set_parallel_run_config(json.dumps(qpu_configs))

nb_jobs = 20
s.qn[0].clear()
s.sns[0].clear()
for i in range(nb_jobs):
    # All circuits use 4 qubits
    s.qn[0].append(4)
    s.sns[0].append(1024)

openQASM = '''
OPENQASM 2.0;
include "qelib1.inc";
qreg q[4];
creg c[4];
x q[0];
x q[2];
barrier q;
h q[0];
cu1(pi/2) q[1],q[0];
h q[1];
cu1(pi/4) q[2],q[0];
cu1(pi/2) q[2],q[1];
h q[2];
cu1(pi/8) q[3],q[0];
cu1(pi/4) q[3],q[1];
cu1(pi/2) q[3],q[2];
h q[3];
measure q -> c;
'''
s.instrings.clear()
s.xasms[0].clear()
s.quil1s[0].clear()
for i in range(nb_jobs):
    s.instrings.append(qb.core.String())
    s.instrings[i].append(openQASM)
    s.xasms[0].append(False)
    s.quil1s[0].append(False)

all_handles = []
for i in range(nb_jobs):
    handle = s.run_async(i, 0)
    all_handles.append(handle)
print("Complete posting all", nb_jobs, "jobs")

import time
all_done = False
while (not all_done):
    all_done = True
    complete_count = 0
    for handle in all_handles:
        completed = handle.complete()
        if not completed:
            all_done = False
        else:
            complete_count = complete_count + 1
    if not all_done:
        print(complete_count, "/", nb_jobs, "complete.")
        time.sleep(1)
    else:
        print("ALL DONE")

print("Result:")
for handle in all_handles:
    print(handle.get())

import qristal.core
import numpy as np
import timeit

s = qristal.core.session()
s.sn = 1024
s.qn = 3

# Use the CPU version of the statevector emulator backend.
# You will need to have the Qristal emulator installed for this to work.
s.acc = "qb-statevector-cpu"
# To instead use GPU-enabled qb-statevector, replace the above line with:
#s.acc = "qb-statevector-gpu"
#s.gpu_device_ids = [0]

# Noise modelling
s.noise = True
nm = qristal.core.NoiseModel("qb-nm1", s.qn)
s.noise_model = nm

# In this test we use generalised mcx to
# perform a standard mcx on |111>

# Expected outcome: |011>

control_qubits = [0, 1]
target_qubit = 2

circ = qristal.core.Circuit()

# prepare initial state
circ.x(0)
circ.x(1)
circ.x(2)

# add generalised mcx
circ.generalised_mcx(target_qubit, control_qubits)
# turn off circuit optimisation, as it fails for the generalised_mcx gate.
s.nooptimise = True

# measure
circ.measure_all()

# run the circuit and check results
s.irtarget = circ
s.run()
print(s.results)

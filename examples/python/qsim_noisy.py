import qristal.core
import numpy as np
import timeit

n_qbits = 3
s = qristal.core.session()
s.init()
s.sn = 1024
s.qn = n_qbits
s.acc = "qsim"

# Uncomment the following lines to introduce noise to the simulation.
# You will need to have the Qristal emulator installed for this to work.
#s.noise = True
#nm = qristal.core.NoiseModel("qb-nm1", n_qbits)
#s.noise_model = nm
#s.gpu_device_id = [0] # Uncomment to use GPU-enabled cirq-qsim, otherwise cirq-qsim will use its CPU backend

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

# measure
circ.measure_all()

# run the circuit and check results
s.ir_target = circ
s.run()
print(s.results[0][0])

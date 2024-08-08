import qristal.core
import numpy as np
import timeit

n_qbits = 3
s = qristal.core.session()
s.init()
s.sn = 1024
s.qn = n_qbits

# Use the purification emulator backend.
# You will need to have the Qristal emulator installed for this to work.
s.acc = "qb-purification"

# Set purification tensor network parameters
s.initial_bond_dimension = 1
s.max_bond_dimension = 256
s.initial_kraus_dimension = 1
s.max_kraus_dimension = 256
s.svd_cutoffs[0][0][0] = 1.0e-6 # Set the absolute SVD cut-off
s.rel_svd_cutoffs[0][0][0] = 1.0e-3 # Set the relative SVD cut-off
s.measure_sample_sequential = "auto"

# Uncomment the following lines to introduce noise to the simulation.
# You will need to have the Qristal emulator installed for this to work.
#s.noise = True
#nm = qristal.core.NoiseModel("qb-nm1", n_qbits)
#s.noise_model = nm

# In this test we use cx to perform a standard
# cx on |111>. Expected outcome: |011>
circ = qristal.core.Circuit()

# prepare initial state
circ.x(0)
circ.x(1)
circ.x(2)

# add cx
circ.cnot(1, 2)

# measure
circ.measure_all()

# run the circuit and check results
s.ir_target = circ
s.run()
print(s.results[0][0])

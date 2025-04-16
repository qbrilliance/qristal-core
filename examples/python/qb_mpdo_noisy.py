import qristal.core
import numpy as np
import timeit

n_qbits = 3
s = qristal.core.session()
s.init()
s.sn = 1024
s.qn = n_qbits

# Use the MPDO emulator backend.
# You will need to have the Qristal emulator installed for this to work.
s.acc = "qb-mpdo"

# Set MPDO tensor network parameters
s.initial_bond_dimension = 1
s.max_bond_dimension = 256
s.svd_cutoffs[0][0][0] = 1.0e-15 # Set the absolute SVD cut-off
s.rel_svd_cutoffs[0][0][0] = 1.0e-10 # Set the relative SVD cut-off
s.measure_sample_method = "auto"
s.gpu_device_id = [0]

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

# CudaQ has no transpiler, so we need to transpile the circuit to QB's native gate set {rx, ry, cz} first.
if s.acc[0][0] == "cudaq:qb_mpdo" and s.noise[0][0] == True:
  # To do this, we simply run the program without executing the circuit, i.e. by setting execute_circuit = False
  s.execute_circuit = False
  s.run()

  # Now we can get the transpiled circuit using "out_transpiled_circuit"
  circ_qasm = s.out_transpiled_circuit[0][0]
  # print(circ_qasm)
  # The transpiled circuit is in openQasm, so we feed it back into session via "instring"
  s.instring = circ_qasm

  # Execute the transpiled circuit by setting execute_circuit = True
  s.execute_circuit = True
s.run()
print(s.results[0][0])

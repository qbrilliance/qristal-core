import qbos_op
import xacc
vqe = qbos_op.vqe(True)
vqe.sn = 0   # No shots - deterministic VQE

# H2 with ASWAP ansatz
# H2 => needs 4 qubits (4 spin orbitals)
vqe.qn = 4  # Number of qubits
vqe.uccsdn = 2  # Number of electrons
vqe.geometry = "H 0.0 0.0 0.0; H 0.0 0.0 0.735" #unit: Angstrom 
vqe.ansatz = "aswap"
vqe.maxeval = 400
vqe.run()
optimum_energy = vqe.out_energy[0][0][0]  
print("Min. energy:", optimum_energy)

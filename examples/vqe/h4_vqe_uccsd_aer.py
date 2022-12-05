import qb.core
import qb.op
import xacc
vqe = qb.op.vqe(True)
vqe.sn = 0
vqe.acc = "aer"
vqe.theta = qb.core.ND()
nb_params = 26
for i in range(nb_params):
    vqe.theta[0][0][i] = 0.0
# Direct energy evaluation
vqe.direct_expectation = True
# H4 with uccsd ansatz
# H4 => needs 8 qubits (8 spin orbitals)
vqe.qn = 8  # Number of qubits
vqe.uccsdn = 4  # Number of electrons
vqe.geometry = "H 0 0 0; H 0 0 0.75; H 0 0 1.875; H 0 0 2.625" #unit: Angstrom
vqe.ansatz = "uccsd"
vqe.method = "cobyla"
vqe.maxeval = 600 # may need more iterations if using nelder-mead
vqe.run()
optimum_energy = vqe.out_energy[0][0][0]
# Expected: -2.238588393687
print("Min. energy:", optimum_energy)

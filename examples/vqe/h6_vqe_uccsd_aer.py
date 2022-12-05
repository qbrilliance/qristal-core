import qb.core
import qb.op
import xacc
vqe = qb.op.vqe(True)
vqe.sn = 0
vqe.acc = "aer"
# Disable output transpilation, this one has many terms...
vqe.output_oqm_enabled = False
vqe.theta = qb.core.ND()
nb_params = 117
for i in range(nb_params):
    vqe.theta[0][0][i] = 0.0
# Direct energy evaluation
vqe.direct_expectation = True
# H6 with uccsd ansatz
# H6 => needs 12 qubits (12 spin orbitals)
vqe.qn = 12  # Number of qubits
vqe.uccsdn = 6  # Number of electrons
vqe.geometry = "H 0 0 0; H 0 0 0.75; H 0 0 1.875; H 0 0 2.625; H 0 0 3.75; H 0 0 4.5" #unit: Angstrom
vqe.ansatz = "uccsd"
vqe.method = "cobyla"
vqe.maxeval = 1000 # may need more iterations if using nelder-mead
vqe.run()
optimum_energy = vqe.out_energy[0][0][0]
# Expected: -3.340283234981
print("Min. energy:", optimum_energy)

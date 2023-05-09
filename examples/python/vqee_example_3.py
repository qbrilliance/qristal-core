import numpy as np
import qb.core.optimization.vqee as qbvqe

# build default params with empty circuit and pauli strings. 
params = qbvqe.Params()

geometry = "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486"  # Angstrom

# qbvqe.pauliStringFromGeometry(geometry) would try to load an python interpreter in cpp code bound with pybind11 to python, 
# i.e creating nested instances of python interpreter. This is not allowed. We use pyscf directly from python and add exposed xacc functionality.
def pauliStringFromGeometry(geometry, basis='sto-3g'):
  import xacc
  opts = {'basis':basis, 'geometry':geometry}
  fermion = xacc.getObservable('pyscf', opts) # pyscf returns a fermion
  observable = xacc.getObservable('fermion', fermion)
  pauli = xacc.transformToPauli('jw', observable)
  return pauli.toString()

# sets ansatz in params according to ansatzID, sets its circuit string and returns number of optimization parameters in ansatz 
ansatzID = qbvqe.AnsatzID.UCCSD
nQubits = 4
nElectrons = 2 # number of electrons/particles/vqeDepth for (UCCSD/ASWAP/HEA)
trs = True # time reversal symmetry only when ASWAP, else just a required dummy to conform to the signature
nOptParams = qbvqe.setAnsatz(params, ansatzID, nQubits, nElectrons, trs)

# access params members directly
#params.[circuitString, pauliString, tolerance, nQubits, nShots, maxIters, isDeterministic] = something
params.nQubits = nQubits
params.nShots = 1024
params.maxIters = 100
params.isDeterministic = True #if true, nShots option is rendered useless
params.tolerance = 1e-6
params.pauliString = pauliStringFromGeometry(geometry) #qbvqe.pauliStringFromGeometry(geometry)
params.acceleratorName = "qpp"

# Set initial parameter values. These will be overwritten with the optimized solution by vqee.run()
params.optimalParameters = nOptParams*[0.1]

print("\nPauli string: ", params.pauliString)
print("\nAnsatz string: ", params.circuitString) # is set by setAnsatz, vqee will prefer ansatz over circuitString

vqee = qbvqe.VQEE(params)
vqee.run()

# output is stored in params
#params.energies
optVec = params.optimalParameters
optVal = params.optimalValue

print("\noptVal, optVec: ", optVal, optVec)
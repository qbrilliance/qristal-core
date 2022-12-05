import numpy as np
import qb.core.optimization as qbOpt

# build default params with empty circuit and pauli strings. 
params = qbOpt.vqee_Params()

# select predefined job
params = qbOpt.makeJob(qbOpt.vqee_JobID.H2_UCCSD)

# access params members directly
#params.[circuitString, pauliString, tolerance, nQubits, nShots, maxIters, isDeterministic] = something

print("\nPauli string: ", params.pauliString)

vqee = qbOpt.vqee_VQEE(params)
vqee.run()

# output is stored in params
#params.energies
optVec = params.optimalParameters
optVal = params.optimalValue

print("\noptVal, optVec: ", optVal, optVec)
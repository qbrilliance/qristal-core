#%% init
import numpy as np
import qb.core.optimization.vqee as qbOpt

# build default params with empty circuit and pauli strings. 
params = qbOpt.Params()

# select predefined job  
# qbOpt.vqee_JobID.H5_UCCSD takes too long to test on simple laptop
joblist = [qbOpt.JobID.H2_explicit, 
           qbOpt.JobID.H1_HEA, 
           qbOpt.JobID.H2_UCCSD, 
           qbOpt.JobID.H2_ASWAP]

#%% run all jobs
for jobID in joblist:
    print("\n****************** Job: ", jobID, " ******************")
    params = qbOpt.makeJob(jobID)

    # access params members directly
    #params.[circuitString, pauliString, tolerance, nQubits, nShots, maxIters, isDeterministic] = something

    print("\nPauli string: ", params.pauliString)

    vqee = qbOpt.VQEE(params)
    vqee.run()

    # output is stored in params
    #params.energies
    optVec = params.optimalParameters
    optVal = params.optimalValue

    print("\noptVal, optVec: ", optVal, optVec)
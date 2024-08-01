#%% init
import numpy as np
import qristal.core.optimization.vqee as qbOpt

# build default params with empty circuit and pauli strings. 
params = qbOpt.Params()

# select predefined job  
joblist = [qbOpt.JobID.H2_explicit]
#%% run all jobs
for jobID in joblist:
    print("\n****************** Job: ", jobID, " ******************")
    params = qbOpt.makeJob(jobID)

    print("\nPauli string: ", params.pauliString)

    params.algorithm = "nelder-mead"
    params.extraOptions = "{stopval: -1.84, lowerbounds: [-1.2]}"

    vqee = qbOpt.VQEE(params)
    vqee.run()

    # output is stored in params
    #params.energies
    optVec = params.optimalParameters
    optVal = params.optimalValue

    print("\noptVal, optVec: ", optVal, optVec)
#%% init
import numpy as np
import qb.core.optimization.vqee as qbOpt

# build default params with empty circuit and pauli strings. 
params = qbOpt.Params()

# select predefined job  
joblist = [qbOpt.JobID.H2_explicit]
#%% run all jobs
for jobID in joblist:
    print("\n****************** Job: ", jobID, " ******************")
    params = qbOpt.makeJob(jobID)

    print("\nPauli string: ", params.pauliString)

    params.algorithm = "cmaes"

    # Reverse upper and lower : see https://github.com/eclipse/xacc/issues/574
    params.extraOptions = "{upper: -10.0, lower: 10.0}"

    vqee = qbOpt.VQEE(params)
    vqee.run()

    # output is stored in params
    #params.energies
    optVec = params.optimalParameters
    optVal = params.optimalValue

    print("\noptVal, optVec: ", optVal, optVec)
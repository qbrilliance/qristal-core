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

    params.algorithm = "adam"
    params.extraOptions = "{stepsize: 0.1, beta1: 0.67, beta2: 0.9, momentum: 0.11, exactobjective: true}"

    vqee = qbOpt.VQEE(params)
    vqee.run()

    # output is stored in params
    #params.energies
    optVec = params.optimalParameters
    optVal = params.optimalValue

    print("\noptVal, optVec: ", optVal, optVec)
    
    params_lbfgs = qbOpt.makeJob(jobID)

    print("\nPauli string: ", params_lbfgs.pauliString)

    params_lbfgs.algorithm = "l-bfgs"
    params_lbfgs.isDeterministic = True

    vqee_lbfgs = qbOpt.VQEE(params_lbfgs)
    vqee_lbfgs.run()

    # output is stored in params
    #params.energies
    optVec = params_lbfgs.optimalParameters
    optVal = params_lbfgs.optimalValue

    print("\noptVal, optVec: ", optVal, optVec)
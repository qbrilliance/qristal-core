#%% init
import numpy as np
import qb.core.optimization.vqee as qbOpt

# build default params with empty circuit and pauli strings. 
params = qbOpt.Params()

# select predefined job  
joblist = [qbOpt.JobID.H2_explicit, qbOpt.JobID.H1_HEA] 
# The other predefined examples are C++ only for now as they require an embedded python interpreter loading pyscf project to C++. 
# Python does not allow an embedded interpreter in the with Pybind11 generated interface.
# the solution is to implement the function, that requires pyscf, directly in python shown in vqee_example_3.py

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
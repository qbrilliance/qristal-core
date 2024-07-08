# Please note: The current implementation of XACC QAOA, used as a base for all Qristal QAOA variants, does not scale well with the number of qubits.
# Small problems are correctly solved with a reasonable amount of iterations.
# This example will not give the correct results as of now and is currently under investigation (fixing XACC upstream).
# It should just demonstrate the current and future APIs.

#%%
import numpy as np
import qb.core.optimization as qbOpt
import qb as qb

# We try qap problem. The primary input are flow and distance matrix and a list of constraints with a constant penalty value
flow = np.array([[0,5,2], [5,0,3], [2,3,0]])
distance = np.array([[0,8,15], [8,0,13], [15,13,0]])
constraints = [ [1,2,3], [4,5,6], [7,8,9], [1,4,7], [2,5,8], [3,6,9] ]
penalty = 200

# the correct solution is known
solRef = [1,0,0,0,1,0,0,0,1]

# problem generator is not yet integrated in Python interface
# quboMatrix = qbOpt.qubo.constructQAPQUBO(flow, distance, constraints, penalty)

# we can enter any valid qubo matrix here
quboMatrix = np.array( 
[   [-400,  200,  200,  200,   40,   75,  200,   16,   30],
    [ 200, -400,  200,   40,  200,   65,   16,  200,   26],
    [ 200,  200, -400,   75,   65,  200,   30,   26,  200],
    [ 200,   40,   75, -400,  200,  200,  200,   24,   45],
    [  40,  200,   65,  200, -400,  200,   24,  200,   39],
    [  75,   65,  200,  200,  200, -400,   45,   39,  200],
    [ 200,   16,   30,  200,   24,   45, -400,  200,  200],
    [  16,  200,   26,   24,  200,   39,  200, -400,  200],
    [  30,   26,  200,   45,   39,  200,  200,  200, -400]
])

print("\nflow: \n", flow, "\ndistance: \n", distance, "\nconstraints: \n", constraints,  "\nqubo: \n", quboMatrix)

# the pauli terms would be calculated internally during the initial solver construction
# quboSolver = qbOpt.qubo.qubo_QuantumQuboSolver(A, maximize)
# pauliString = quboSolver.preparePauliStr()
# quboSolver.solverMethod("qaoa")
# quboSolver.run()

# we can set the pauli string directly as well
pauliString = """
 + 400 Z0 Z1 + 400 Z0 Z2 + 400 Z0 Z3 +  80 Z0 Z4 + 150 Z0 Z5 + 400 Z0 Z6 +  32 Z0 Z7 +  60 Z0 Z8
 + 400 Z1 Z2 +  80 Z1 Z3 + 400 Z1 Z4 + 130 Z1 Z5 +  32 Z1 Z6 + 400 Z1 Z7 +  52 Z1 Z8
 + 150 Z2 Z3 + 130 Z2 Z4 + 400 Z2 Z5 +  60 Z2 Z6 +  52 Z2 Z7 + 400 Z2 Z8
 + 400 Z3 Z4 + 400 Z3 Z5 + 400 Z3 Z6 +  48 Z3 Z7 +  90 Z3 Z8
 + 400 Z4 Z5 +  48 Z4 Z6 + 400 Z4 Z7 +  78 Z4 Z8
 +  90 Z5 Z6 +  78 Z5 Z7 + 400 Z5 Z8
 + 400 Z6 Z7 + 400 Z6 Z8
 + 400 Z7 Z8
 - 2400"""
# " - 2400" or " + 4800 - 800 Z0 - 800 Z1 - 800 Z2 - 800 Z3 - 800 Z4 - 800 Z5 - 800 Z6 - 800 Z7 - 800 Z8" 


nPaulis = pauliString.count('+') + pauliString.count('-') - 1
print("\nPauli string: ", pauliString)

nOptVars = 9
nQaoaSteps = 1

#%% qaoa simple 
print("\n*************** running simple qaoa... ***************\n")

qa = qbOpt.qaoa_QaoaSimple()
qa.qn = nOptVars
qa.ham = pauliString
qa.acc='qpp'
qa.functol[0][0][0]=1e-5
qa.maxeval=100 #800
qa.qaoa_step = nQaoaSteps
qa.theta[0][0]=qb.core.MapIntDouble()

extendedParams = True
if (extendedParams):
    nThetas = (nOptVars+nPaulis)*nQaoaSteps
else:
    nThetas =                  2*nQaoaSteps

qa.extended_param = extendedParams
for ii in range(nThetas): 
    qa.theta[0][0][ii] = 0.25

qa.run()
print("\ncost: " + str(qa.out_energy[0][0][0]))
print("\neigenstate: " + str(qa.out_eigenstate[0][0]))
print("\ncorrect solution: ", solRef)

#%% qaoa recursive
print("\n*************** running recursive qaoa... ***************\n")

qa = qbOpt.qaoa_QaoaRecursive()
qa.qaoa_step = 1
qa.sn = 1024
qa.extended_param = True

qa.qn = nOptVars
qa.ham = pauliString
qa.n_c = 5 # threshold number of variables
qa.maxeval=100 #800

qa.run()
print("\ncost: " + str(qa.out_energy[0][0][0]))
print("\neigenstate: " + str(qa.out_eigenstate[0][0]))
print("\ncorrect solution: ", solRef)


#%% qaoa warm_start
print("\n*************** running warm started qaoa... ***************\n")

qa = qbOpt.qaoa_QaoaWarmStart()
nQaoaSteps = 1
qa.qaoa_step = nQaoaSteps
qa.sn = 1024
qa.good_cut = "100"


extendedParams = False
if (extendedParams):
    nThetas = (nOptVars+nPaulis)*nQaoaSteps
else:
    nThetas =                  2*nQaoaSteps
qa.extended_param = extendedParams


qa.theta[0][0]=qb.core.MapIntDouble()
for ii in range(nThetas): 
    qa.theta[0][0][ii] = 0.25

qa.qn = nOptVars
qa.ham = pauliString
qa.maxeval=100 #800

qa.run()
print("\ncost: " + str(qa.out_energy[0][0][0]))
print("\neigenstate: " + str(qa.out_eigenstate[0][0]))
print("\ncorrect solution: ", solRef)

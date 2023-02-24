import numpy as np
import qb.core.optimization as qbOpt
import qb as qb

# Consider a max-cut problem for a 5-node
#                *
#               /|\
#              * | *
# kite graph:   \|/
#                *
#                |
#                *
# This can be encoded into a 5-qubit Hamiltonian:
pauli_string = """ + 0.0 + 0.5 Z0 Z2 + 0.5 Z1 Z2 + 0.5 Z2 Z3 + 0.5 Z1 Z4 + 0.5 Z2 Z4 + 0.5 Z3 Z4 """
n_qubits = 5

print ("\nApply the QAOA to find the maximal cut of a uniformly weighted 5-node kite graph.")
print ("The graph can be encoded into the cost Hamiltonian '''", pauli_string,"'''.")

# Generic setup of QAOA method.
nQaoaSteps = 1
# Construct a QAOA object.
qa = qbOpt.qaoa_QaoaSimple()
qa.qaoa_step = nQaoaSteps
# Set the accelerator and optimizer tolerance.
qa.acc='qpp'
qa.functol[0][0][0]=1e-5
# Set problem statement.
qa.qn = n_qubits
qa.ham = pauli_string

# The quality of results depends on the number of shots used to evaluate
# the circuit (more complex circuits need more shots) and the number of
# steps in the optimization method (more complex problems may require more
# evaluations). The values below ensure the 5-qubit test case does on average yield correct
# results. For more complex problems, increasing the number of shots may be necessary.
qa.sn      = 10000
qa.maxeval = 1000

# Construct parameter list and initial angle values. 
qa.theta[0][0]=qb.core.ND()

# Extended flexibility: Per QAOA step, assign one parameter to each term in the driver Hamiltonian
# and one parameter to each term in the cost Hamiltonian
extendedParams = True
if (extendedParams):
    nPaulis = pauli_string.count('+') + pauli_string.count('-') - 1
    nThetas = (n_qubits+nPaulis)*nQaoaSteps

# Basic method: Per QAOA step, assign one parameter to the sum of driver terms, and one parameter
# to the sum of cost terms.
else:
    nThetas =                  2*nQaoaSteps

print ("Employ", nThetas,"optimization parameters.")
qa.extended_param = extendedParams

# Set initial angles.
for ii in range(nThetas): 
    qa.theta[0][0][ii] = 0.25

# Run the QAOA algorithm
print ("Run the QAOA algorithm.")
qa.run()

# Extract the results
print ("Results:")
print("QAOA cost:          " + str(qa.out_energy[0][0][0]))
print("QAOA solution:      " + str(qa.out_eigenstate[0][0]))

reference_solution = [[1,0,1,0,0],[0,1,0,1,1]] # two valid solutions because of symmetry of kite graph.
print("Reference solution: ", reference_solution)

print ("\n")

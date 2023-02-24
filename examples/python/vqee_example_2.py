import numpy as np
import qb.core.optimization as qbOpt

# build default params with empty circuit and pauli strings. 
params = qbOpt.vqee_Params()

# select predefined job
#params = qbOpt.makeJob(qbOpt.vqee_JobID.H2_UCCSD)

# access params members directly
#params.[circuitString, pauliString, tolerance, nQubits, nShots, maxIters, isDeterministic] = something
params.nQubits = 4
params.nShots = 400000
params.maxIters = 100
params.isDeterministic = True #if true, nShots option is rendered useless
params.tolerance = 1e-6
params.pauliString = "-0.8124419696351861 + 0.17128249292506947 X0X1X2X3"
params.circuitString = """
.compiler xasm
.circuit ansatz
.parameters theta
.qbit q
  X(q[0]);
  X(q[2]);
  U(q[0], theta[0], theta[1], theta[2]);
  U(q[1], theta[3], theta[4], theta[5]);
  U(q[2], theta[6], theta[7], theta[8]);
  U(q[3], theta[9], theta[10], theta[11]);
  CNOT(q[0], q[1]);
  CNOT(q[1], q[2]);
  CNOT(q[2], q[3]);
  U(q[0], theta[12], theta[13], theta[14]);
  U(q[1], theta[15], theta[16], theta[17]);
  U(q[2], theta[18], theta[19], theta[20]);
  U(q[3], theta[21], theta[22], theta[23]);
  CNOT(q[0], q[1]);
  CNOT(q[1], q[2]);
  CNOT(q[2], q[3]);
"""

# Set initial parameter values. These will be overwritten with the optimized solution by vqee.run()
params.optimalParameters = 24*[0]

print("\nPauli string: ", params.pauliString)

acceleratorNamesList = ["sparse-sim", "qpp", "aer"]
for acceleratorName in acceleratorNamesList:
  print("\n\n\n*************** ",acceleratorName, "***************")

  params.acceleratorName = acceleratorName

  vqee = qbOpt.vqee_VQEE(params)
  vqee.run()

  # output is stored in params
  #params.energies
  optVec = params.optimalParameters
  optVal = params.optimalValue

  print("\noptVal, optVec: ", optVal, optVec)
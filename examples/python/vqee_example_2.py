import numpy as np
import qb.core.optimization.vqee as qbOpt

# build default params with empty circuit and pauli strings. 
params = qbOpt.Params()

# select predefined job
#params = qbOpt.makeJob(qbOpt.JobID.H2_UCCSD)

# access params members directly
#params.[circuitString, pauliString, tolerance, nQubits, nShots, maxIters, isDeterministic] = something
params.nQubits = 4
params.nShots = 400
params.maxIters = 100
params.isDeterministic = False #if true, nShots option is rendered useless
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
params.enableVis = True
params.showTheta = True
params.limitThetaN = 4
params.tail = 5
params.plain = False
params.blocked = True
print("\nPauli string: ", params.pauliString)

acceleratorNamesList = ["qpp", "aer","sparse-sim", "aer"]
for acceleratorName in acceleratorNamesList:
  print("\n\n\n*************** ",acceleratorName, "***************")
  params.optimalParameters = 24*[0]
  params.acceleratorName = acceleratorName

  vqee = qbOpt.VQEE(params)
  vqee.run()

  optVec = params.optimalParameters
  optVal = params.optimalValue

  print("\nConvergence trace:\n", params.vis)
  print("\nFinal iteration, energy:\n", params.iterationData[-1].energy)
  print("\nFinal iteration, theta:\n", params.iterationData[-1].params)
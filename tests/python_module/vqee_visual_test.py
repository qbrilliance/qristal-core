import numpy as np
import qb.core.optimization.vqee as qbOpt

def test_vqee_toggle_visual_off() :
  print("* vqee_toggle_visual:")
  print("* This test disables convergence visualisation features.")
  print("* When the enableVis attribute is set to True, the energyVis and thetaVis attributes are populated with text-art")
  params = qbOpt.Params()
  params.nQubits = 4
  params.nShots = 400
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
  params.enableVis = False
  acceleratorNamesList = ["aer"]
  for acceleratorName in acceleratorNamesList:
    params.optimalParameters = 24*[0]
    params.acceleratorName = acceleratorName
  vqee = qbOpt.VQEE(params)
  vqee.run()
  assert len(params.vis) == 0

def test_vqee_toggle_visual_detect_optimum_mark() :
  print("* vqee_toggle_visual:")
  print("* This tests the enableVis attribute.  It looks for the mark '**|' which indicates the optimal iteration")
  params = qbOpt.Params()
  params.nQubits = 4
  params.nShots = 400
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
  params.enableVis = True
  acceleratorNamesList = ["aer"]
  for acceleratorName in acceleratorNamesList:
    params.optimalParameters = 24*[0]
    params.acceleratorName = acceleratorName
  vqee = qbOpt.VQEE(params)
  vqee.run()
  assert (" **|" in params.vis) == True

test_vqee_toggle_visual_off()
test_vqee_toggle_visual_detect_optimum_mark()
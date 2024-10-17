import pytest
import qristal.core 
import numpy as np

def test_SPAM_correction_random():
  n_qubits = 2 
  n_shots = 1024 

  #(1) Build noise model using readout errors only 
  nm = qristal.core.NoiseModel() 
  q0_ro_error = qristal.core.ReadoutError() 
  q0_ro_error.p_01  = 0.1 
  q0_ro_error.p_10 = 0.05
  q1_ro_error = qristal.core.ReadoutError() 
  q1_ro_error.p_01  = 0.05 
  q1_ro_error.p_10 = 0.1
  nm.set_qubit_readout_error(0, q0_ro_error)
  nm.set_qubit_readout_error(1, q1_ro_error)
  nm.add_qubit_connectivity(0, 1)

  #(2) Construct the corresponding SPAM confusion matrix 
  confusion_q0 = np.array([
    [0.9, 0.1], 
    [0.05, 0.95]
  ])
  confusion_q1 = np.array([
    [0.95, 0.05],
    [0.1, 0.9]
  ])
  confusion = np.kron(confusion_q0, confusion_q1)

  #(3) Build session
  s = qristal.core.session()
  s.init()
  s.qn = n_qubits
  s.sn = 1024
  s.acc = "aer"
  circ = qristal.core.Circuit() 
  circ.h(0)
  circ.cnot(0, 1)
  circ.measure(0)
  circ.measure(1)
  s.ir_target = circ 
  s.noise = True 
  s.noise_model = nm

  #(4) Run with SPAM correction
  s.SPAM_confusion = confusion
  s.run()

  #(4) Obtain native and SPAM-corrected <ZZ> 
  native_ZZ = 0.0 
  for key in s.results_native[0][0]:
    if key[0] == key[1]: 
      native_ZZ += s.results_native[0][0][key] / s.results_native[0][0].total_counts()
    else: 
      native_ZZ -= s.results_native[0][0][key] / s.results_native[0][0].total_counts()
  corrected_ZZ = 0.0 
  for key in s.results[0][0]:
    if key[0] == key[1]:  
      corrected_ZZ += s.results[0][0][key] / s.results[0][0].total_counts()
    else: 
      corrected_ZZ -= s.results[0][0][key] / s.results[0][0].total_counts()

  #(6) test if the SPAM correction improved the result 
  assert(corrected_ZZ > native_ZZ)
  assert(corrected_ZZ > 0.9)
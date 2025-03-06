# Copyright (c) Quantum Brilliance Pty Ltd
import pytest
from exponent_call import call_circuit

def test_CI_220727_1_exponent() :
  print("\nTesting exponent (base 2) algorithm with MSB convention")
  log_value = 3

  print("\n*** Testing log_value:" , log_value , " ***" )
  qubits_log = [0,1]

  return call_circuit(log_value,qubits_log, is_LSB=False)

"""
def test_CI_220727_2_exponent() :
  print("Testing exponent (base 2) algorithm with MSB convention")
  log_value = 4

  print("\n*** Testing log_value:" , log_value , " ***" )
  qubits_log = [0,1,2]

  return call_circuit(log_value,qubits_log, min_significance_=3, is_LSB=False)
"""


def test_CI_220727_3_exponent() :
  print("Testing exponent (base 2) algorithm with MSB convention")
  log_value = 3

  print("\n*** Testing log_value:" , log_value , " ***" )
  qubits_log = [3,2]

  return call_circuit(log_value,qubits_log, is_LSB=True)

"""
def test_CI_220727_4_exponent() :
  print("Testing exponent (base 2) algorithm with MSB convention")
  log_value = 4

  print("\n*** Testing log_value:" , log_value , " ***" )
  qubits_log = [15,14,13]

  return test_circuit(log_value,qubits_log, min_significance_=3, is_LSB=True)

def test_CI_220727_5_exponent() :
  print("Testing exponent (base 2) algorithm with MSB convention")
  log_value = 4

  print("\n*** Testing log_value:" , log_value , " ***" )
  qubits_log = [0,1,2]
  qubits_exponent = [qubit for qubit in range(16)]
  qubits_ancilla = [16,17,18]

  return test_circuit(log_value,qubits_log, qubits_exponent, qubits_ancilla, min_significance_=3, is_LSB=False)


def test_CI_220727_6_exponent() :
  print("Testing exponent (base 2) algorithm with MSB convention")
  log_value = 4

  print("\n*** Testing log_value:" , log_value , " ***" )
  qubits_log = [15,14,13]
  qubits_exponent = [qubit for qubit in range(15,-1,-1)]
  qubits_ancilla = [18,17,16]
  return call_circuit(log_value,qubits_log, qubits_exponent, qubits_ancilla, min_significance_=3, is_LSB=True)
"""



if __name__ == "__main__" :
  test_CI_220727_1_exponent()
  #test_CI_220727_2_exponent()
  test_CI_220727_3_exponent()
  #test_CI_220727_4_exponent()
  #test_CI_220727_5_exponent()
  #test_CI_220727_6_exponent()



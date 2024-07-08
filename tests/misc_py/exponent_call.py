# Copyright (c) 2022 Quantum Brilliance Pty Ltd
import os
import pytest
import random as rand
import qb.core


def call_circuit(log_value,qubits_log,qubits_exponent=[],qubits_ancilla=[], min_significance_=1,is_LSB=True) :
    s = qb.core.session()
    circ = qb.Circuit()
    for qindex in range(len(qubits_log)) :
      if log_value & pow(2,qindex) :
        circ.x(qubits_log[qindex])
    expand_ok = circ.exponent(qubits_log, qubits_exponent, qubits_ancilla, min_significance=min_significance_, is_LSB=is_LSB)
    assert expand_ok, "Error in circ.expand()"

    nb_qubits_log = len(qubits_log)
    nb_qubits_exp = pow(2, pow(2,nb_qubits_log-1))
    nb_qubits = nb_qubits_exp + nb_qubits_log
    if not is_LSB :
        nb_qubits -= (min_significance_ - 1)
    #Check calculation of exponent qubits
    #print("nb_qubits_log:", nb_qubits_log, " nb_qubits_exp:", nb_qubits_exp )

    qubits = []
    if is_LSB :
      for qindex in range(nb_qubits_exp,0,-1) :
        qubits.append(qindex-1)
      for qindex in range(nb_qubits_exp, nb_qubits) :  #, -1) :
        qubits.append(qindex)
    else :
      for qindex in range(nb_qubits) :
        qubits.append(qindex)

    for qindex in range(nb_qubits) :
      circ.measure(qindex)

    print("HOWDY: Comparator circuit:\n")
    print("Testing log_value:", log_value)
    #print(circuit->toString(), '\n'
    s.ir_target = circ
    s.init()
    #s.nooptimise = True
    #s.noplacement = True
    #s.notiming = True
    #s.output_oqm_enabled = False
    s.acc = "qpp"
    s.sn = 1024
    s.rn = 1
    s.qn = nb_qubits
    s.run()
    result = s.out_raw_json[0][0][2:-2].strip()

    expected_measurement = ""
    exp_value = pow(2,log_value)
    #print("log_value:", log_value, " exp_value:", exp_value, "is_LSB", is_LSB)
    if is_LSB:
      for qindex in range(min_significance_ - 1) :
        expected_measurement += '0'
    start_exp = 0 + (min_significance_ - 1)*is_LSB
    for qindex in range(start_exp, nb_qubits_exp) :
      qubit = qubits[qindex] + min_significance_ - 1
      if exp_value & pow(2,qubit) :
        expected_measurement += '1'
      else :
        expected_measurement += '0'
    for qindex in range(nb_qubits_exp, nb_qubits) :
      qubit = qubits[qindex] - nb_qubits_exp + min_significance_ - 1
      if log_value & pow(2,qubit) :
        expected_measurement += '1'
      else :
        expected_measurement += '0'



    #print("expected_measurement:", expected_measurement)
    print(result)
    measurement = result[1:1+nb_qubits]
    assert result.find(":") == result.rfind(":"), "Should be single output %i %i "%(result.find(":") == result.rfind(":"))
    assert result[-4:] == "1024", "Should be a single output"
    assert measurement == expected_measurement, "Measured %s (length %i), expected %s (length %i)"%(measurement, len(measurement), expected_measurement, len(expected_measurement))
    output_string = measurement[:nb_qubits_exp]
    input_string = measurement[nb_qubits_exp : nb_qubits]
    print("Input value:", input_string)
    print("Output value:", output_string)
    print("Expected output:", expected_measurement[:nb_qubits_exp])
    print("Successfully found two to the power of ", log_value, "\n" )
    return True



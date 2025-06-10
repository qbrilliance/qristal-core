# Copyright (c) Quantum Brilliance Pty Ltd
import os
import pytest

#
# 210826 - Disabled until a backend implementation of rzz works
#
# def test_CI_210826_3_init_random_5() :
#     print("* CI_210826_3_init_random_5:")
#     print("* With default init settings, with the default (TNQVM-ExaTN-MPS) backend, run a depth 5 circuit, check the length of the results set is >0")
#     import qristal.core, ast
#     s = qristal.core.session()
#     s.random = 5
#     s.run()
#     assert (len(s.results) > 0, "Failed test: CI_210826_3_init_random_5 - TNQVM-ExaTN-MPS"

def test_example_hardware_device_rx_ry_rz() :
    print(" Loopback QCStack check transpiling into discrete angles")
    import qristal.core
    import json
    s = qristal.core.session()
    s.qn = 1
    s.sn = 2
    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void qristal_circuit(qbit q) {
        Rx(q[0], 0.125*pi);
        Ry(q[0], 0.25*pi);
        Rz(q[0], 0.5*pi);
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "example_hardware_device"

    # Run the circuit on the back-end
    s.run()
    trj = json.loads(s.qbjson)
    assert (trj['circuit'][0] == "Rx(q[0],0.392699)")
    assert (trj['circuit'][-1] == "Rx(q[0],3.14159)")

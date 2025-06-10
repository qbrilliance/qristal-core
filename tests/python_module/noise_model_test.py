# Test cases for noise model

import pytest
import ast
import qristal.core
import numpy as np

def test_default_qobj_compiler_aer():
    session = qristal.core.session()
    session.qn = 1
    session.sn = 1000
    session.acc = "aer"
    # Define the circuit
    circ = qristal.core.Circuit()
    circ.rx(0, np.pi)
    circ.measure_all()
    # Load circuit in session
    session.irtarget = circ
    my_nm = qristal.core.NoiseModel()
    my_nm.name = "my_nm"
    # Default is the IBM's standard "xacc-qobj" compiler (IBM universal gate set)
    assert(my_nm.qobj_compiler == "xacc-qobj")
    my_nm.add_gate_error(qristal.core.DepolarizingChannel.Create(0, 0.5), "u3", [0])
    session.noise = True
    session.noise_model = my_nm
    session.run()
    print(session.results)
    # The result is "1" if no noise
    good_count = int(session.results[[1]])
    # Since rx is noisy, there'll be some other values.
    assert (good_count < 1000)

def test_set_qobj_compiler_aer():
    session = qristal.core.session()
    session.qn = 1
    session.sn = 1000
    session.acc = "aer"
    # Define the circuit
    circ = qristal.core.Circuit()
    circ.rx(0, np.pi)
    circ.measure_all()
    # Load circuit in session
    session.irtarget = circ
    my_nm = qristal.core.NoiseModel()
    my_nm.name = "my_nm"
    my_nm.add_gate_error(qristal.core.DepolarizingChannel.Create(0, 0.5), "rx", [0])
    session.noise = True
    my_nm.qobj_compiler = "qristal-qobj"
    # rx is in the basis gate set
    assert("rx" in my_nm.qobj_basis_gates)
    session.noise_model = my_nm
    session.run()
    print(session.results)
    # The result is "1" if no noise
    good_count = int(session.results[[1]])
    # Since rx is noisy, there'll be some other values.
    assert (good_count < 1000)

# Simple test cases to check for simple init/validations/regressions

import pytest


def test_CI_210826_1_init_check_qn_equals_12() :
    print("* CI_210826_1_init_check_qn_equals_12:")
    print("* Imports core.core() and calls init(), which should give a default setup for 12 qubits.")
    import qristal.core
    s = qristal.core.session()
    s.init()
    assert (s.qn[0][0] == 12)

def test_CI_210826_2_init_check_sn_equals_1024() :
    print("* CI_210826_2_init_check_sn_equals_1024:")
    print("* Imports qristal.core Python module, creates and initialises a session that should receive the default number of shots (1024).")
    import qristal.core
    s = qristal.core.session()
    s.init()
    assert (s.sn[0][0] == 1024)

def test_CI_210826_4_init_random_5() :
    print("* CI_210826_4_init_random_5:")
    print("* With default init settings, with the Aer backend, run a depth 5 circuit, check the length of the results map is >0")
    import qristal.core, ast
    s = qristal.core.session()
    s.init()
    s.random = 5
    s.acc = 'aer'
    s.run()
    res = s.results[0][0]
    assert (len(res) > 0)

def test_CI_210826_5_init_random_5() :
    print("* CI_210826_5_init_random_5:")
    print("* With default init settings, with the qpp backend, run a depth 3 circuit, check the length of the results map is >0")
    import qristal.core, ast
    s = qristal.core.session()
    s.init()
    s.random = 3
    s.acc = 'qpp'
    s.run()
    res = s.results[0][0]
    assert (len(res) > 0)

def test_CI_210826_6_init_random_0() :
    print("* CI_210826_4_init_random_0:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import qristal.core
    s = qristal.core.session()
    s.init()
    s.random = 0
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_7_init_random_minus_one() :
    print("* CI_210826_5_init_random_minus_one:")
    print("* [Edge condition] check that a TypeError exception occurs")
    import qristal.core
    s = qristal.core.session()
    s.init()
    with pytest.raises(TypeError):
        s.random = -1

def test_CI_210826_8_init_random_256() :
    print("* CI_210826_8_init_random_256:")
    print("* With default init settings, run a depth 256 circuit, check the length of the results map is >0")
    import qristal.core, ast
    s = qristal.core.session()
    s.init()
    s.acc = "aer"
    s.random = 256
    s.run()
    res = s.results[0][0]
    assert (len(res) > 0)

def test_CI_210826_9_qn_minus_one() :
    print("* CI_210826_9_qn_minus_one:")
    print("* [Edge condition] check that a TypeError exception occurs")
    import qristal.core
    s = qristal.core.session()
    s.init()
    with pytest.raises(TypeError):
        s.qn = -1

def test_CI_210826_10_qn_0() :
    print("* CI_210826_10_qn_0:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import qristal.core
    s = qristal.core.session()
    s.init()
    s.qn = 0
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_11_init_no_infile_no_instring_no_random() :
    print("* CI_210826_11_init_no_infile_no_instring_no_random:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import qristal.core
    s = qristal.core.session()
    s.init()
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_12_init_nonexistent_infile() :
    print("* CI_210826_12_init_nonexistent_infile:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import qristal.core
    s = qristal.core.session()
    s.init()
    s.infile = "nonexist.inc"
    with pytest.raises(ValueError):
        s.run()


def test_CI_210826_13_openqasm_index_out_of_range() :
    print("* CI_210826_13_openqasm_index_out_of_range:")
    print("* [XACC exit condition] when an indexing error is detected in OpenQASM")
    import qristal.core
    s = qristal.core.session()
    s.init()
    s.instring = '''
    __qpu__ void qristal_circuit(qreg q) {
        OPENQASM 2.0; include "qelib1.inc";
        //
        creg c[4];
        measure q[4] -> c[4];
    }'''
    with pytest.raises((ValueError, RuntimeError)):
        s.run()

def test_raw_openqasm_str():
    print(" Testing raw OpenQASM string input ")
    import qristal.core, ast
    s = qristal.core.session()
    s.debug = True
    nb_qubits = 12
    circ = qristal.core.Circuit()
    circ.h(0)
    # Entangle all qubits
    for i in range(nb_qubits - 1):
        circ.cnot(i, i + 1)
    circ.measure_all()

    s.init()
    s.instring = circ.openqasm()
    s.run()
    res = s.results[0][0]
    print(res)
    assert (len(res) == 2)

def test_raw_openqasm_file():
    print(" Testing raw OpenQASM file input ")
    import qristal.core, ast
    import os
    s = qristal.core.session()
    s.debug = True
    dir_path = os.path.dirname(os.path.realpath(__file__))
    s.infile = dir_path + "/test_openqasm.qasm"
    s.init()
    s.run()
    res = s.results[0][0]
    print(res)
    assert (len(res) == 2)

def test_raw_openqasm_custom_qreg_names():
    print(" Testing raw OpenQASM input using different qreg variable names")
    import qristal.core, ast
    s = qristal.core.session()
    s.init()
    s.instring = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg qa1_bb[2];
    creg c[2];
    h qa1_bb[0];
    cx qa1_bb[0],qa1_bb[1];
    measure qa1_bb[0] -> c[0];
    measure qa1_bb[1] -> c[1];
    '''
    s.run()
    res = s.results[0][0]
    print(res)
    assert (len(res) == 2)

    s.instring = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q1[2];
    creg c[2];
    h q1[0];
    cx q1[0],q1[1];
    measure q1[0] -> c[0];
    measure q1[1] -> c[1];
    '''
    s.run()
    res = s.results[0][0]
    print(res)
    assert (len(res) == 2)

    s.instring = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg qb_reg123[2];
    creg c[2];
    h qb_reg123[0];
    cx qb_reg123[0],qb_reg123[1];
    measure qb_reg123[0] -> c[0];
    measure qb_reg123[1] -> c[1];
    '''
    s.run()
    res = s.results[0][0]
    print(res)
    assert (len(res) == 2)

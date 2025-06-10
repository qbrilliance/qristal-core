# Simple test cases to check for simple init/validations/regressions

import pytest
import qristal.core

def test_CI_210826_4_init_random_5() :
    print("* With the Aer backend, run a depth 5 circuit, check the length of the results map is >0")
    s = qristal.core.session()
    s.random_circuit_depth = 5
    s.qn = 12
    s.sn = 10
    s.acc = 'aer'
    s.nooptimise = True
    s.run()
    assert (len(s.results) > 0)

def test_CI_210826_5_init_random_3() :
    print("* With the qpp backend, run a depth 3 circuit, check the length of the results map is >0")
    s = qristal.core.session()
    s.random_circuit_depth = 3
    s.qn = 5
    s.sn = 10
    s.acc = 'qpp'
    s.nooptimise = True
    s.run()
    assert (len(s.results) > 0)

def test_CI_210826_6_init_random_0() :
    print("* [Edge condition] check that a ValueError exception occurs")
    s = qristal.core.session()
    s.random_circuit_depth = 0
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_7_init_random_minus_one() :
    print("* [Edge condition] check that a TypeError exception occurs")
    s = qristal.core.session()
    with pytest.raises(TypeError):
        s.random_circuit_depth = -1

def test_CI_210826_8_init_random_256() :
    print("* Run a depth 256 circuit, check the length of the results map is >0")
    s = qristal.core.session()
    s.acc = "aer"
    s.random_circuit_depth = 256
    s.qn = 5
    s.sn = 10
    s.nooptimise = True
    s.run()
    print(s.instring)
    assert (len(s.results) > 0)

def test_CI_210826_9_qn_minus_one() :
    print("* [Edge condition] check that a TypeError exception occurs")
    s = qristal.core.session()
    with pytest.raises(TypeError):
        s.qn = -1

def test_CI_210826_10_qn_0() :
    print("* [Edge condition] check that a ValueError exception occurs")
    s = qristal.core.session()
    s.qn = 0
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_11_init_no_infile_no_instring_no_random() :
    print("* CI_210826_11_init_no_infile_no_instring_no_random:")
    print("* [Edge condition] check that a ValueError exception occurs")
    s = qristal.core.session()
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_12_init_nonexistent_infile() :
    print("* CI_210826_12_init_nonexistent_infile:")
    print("* [Edge condition] check that a ValueError exception occurs")
    s = qristal.core.session()
    s.infile = "nonexist.inc"
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_13_openqasm_index_out_of_range() :
    print("* CI_210826_13_openqasm_index_out_of_range:")
    print("* [XACC exit condition] when an indexing error is detected in OpenQASM")
    s = qristal.core.session()
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
    s = qristal.core.session()
    s.debug = True
    nb_qubits = 12
    circ = qristal.core.Circuit()
    circ.h(0)
    # Entangle all qubits
    for i in range(nb_qubits - 1):
        circ.cnot(i, i + 1)
    circ.measure_all()
    s.instring = circ.openqasm()
    s.sn = 1024
    s.qn = nb_qubits
    s.run()
    print(s.results)
    assert (len(s.results) == 2)

def test_raw_openqasm_file():
    print(" Testing raw OpenQASM file input ")
    import os
    s = qristal.core.session()
    s.debug = True
    dir_path = os.path.dirname(os.path.realpath(__file__))
    s.infile = dir_path + "/test_openqasm.qasm"
    s.sn = 1024
    s.qn = 2
    s.run()
    print(s.results)
    assert (len(s.results) == 2)

def test_raw_openqasm_custom_qreg_names():
    print(" Testing raw OpenQASM input using different qreg variable names")
    s = qristal.core.session()
    s.sn = 1024
    s.qn = 2
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
    print(s.results)
    assert (len(s.results) == 2)

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
    print(s.results)
    assert (len(s.results) == 2)

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
    print(s.results)
    assert (len(s.results) == 2)

# Simple test cases to check for simple init/validations/regressions

import pytest


def test_CI_210826_1_qb12_check_qn_equals_12() :
    print("* CI_210826_1_qb12_check_qn_equals_12:")
    print("* Imports core.core() and calls qb12(), which should give a default setup for 12 qubits.")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    assert (tqb.qn[0][0]) == 12, "[qbos] Failed test: CI_210826_1_qb12_check_qn_equals_12"

def test_CI_210826_2_qb12_check_sn_equals_1024() :
    print("* CI_210826_2_qb12_check_sn_equals_1024:")
    print("* Imports qbemulator Python module and calls qb12(), which should give a default setup for 1024 shots")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    assert (tqb.sn[0][0]) == 1024, "[qbos] Failed test: CI_210826_2_qb12_check_sn_equals_1024"

def test_CI_210826_4_qb12_random_5() :
    print("* CI_210826_4_qb12_random_5:")
    print("* With default qb12 settings, with the Aer backend, run a depth 5 circuit, check the length of the out_count dictionary is >0")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    tqb.random = 5
    tqb.acc = 'aer'
    tqb.run()
    assert (len(tqb.out_count[0][0])) > 0, "[qbos] Failed test: CI_210826_4_qb12_random_5 - Aer"

def test_CI_210826_5_qb12_random_5() :
    print("* CI_210826_5_qb12_random_5:")
    print("* With default qb12 settings, with the qpp backend, run a depth 3 circuit, check the length of the out_count dictionary is >0")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    tqb.random = 3
    tqb.acc = 'qpp'
    tqb.run()
    assert (len(tqb.out_count[0][0])) > 0, "[qbos] Failed test: CI_210826_5_qb12_random_5 - qpp"

def test_CI_210826_6_qb12_random_0() :
    print("* CI_210826_4_qb12_random_0:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    tqb.random = 0
    with pytest.raises(ValueError):
        tqb.run()

def test_CI_210826_7_qb12_random_minus_one() :
    print("* CI_210826_5_qb12_random_minus_one:")
    print("* [Edge condition] check that a TypeError exception occurs")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    with pytest.raises(TypeError):
        tqb.random = -1

def test_CI_210826_8_qb12_random_256() :
    print("* CI_210826_8_qb12_random_256:")
    print("* With default qb12 settings, run a depth 256 circuit, check the length of the out_count dictionary is >0")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    tqb.acc = "aer"
    tqb.random = 256
    tqb.run()
    assert (len(tqb.out_count[0][0])) > 0, "[qbos] failed test: CI_210826_8_qb12_random_256"


def test_CI_210826_9_qn_minus_one() : 
    print("* CI_210826_9_qn_minus_one:")
    print("* [Edge condition] check that a TypeError exception occurs")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    with pytest.raises(TypeError):
        tqb.qn = -1

def test_CI_210826_10_qn_0() :
    print("* CI_210826_10_qn_0:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    tqb.qn = 0
    with pytest.raises(ValueError):
        tqb.run()

def test_CI_210826_11_qb12_no_infile_no_instring_no_random() :
    print("* CI_210826_11_qb12_no_infile_no_instring_no_random:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    with pytest.raises(ValueError):
        tqb.run()
  
def test_CI_210826_12_qb12_nonexistent_infile() :
    print("* CI_210826_12_qb12_nonexistent_infile:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    tqb.infile = "nonexist.inc"
    with pytest.raises(ValueError):
        tqb.run()


def test_CI_210826_13_openqasm_index_out_of_range() :
    print("* CI_210826_13_openqasm_index_out_of_range:")
    print("* [XACC exit condition] when an indexing error is detected in OpenQASM")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    tqb.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0; include "qelib1.inc";
        //
        creg c[4];
        measure q[4] -> c[4];
    }'''
    with pytest.raises((ValueError, RuntimeError)):
        tqb.run()

def test_CI_210826_14_qbtheta_parameters() :
    print("* CI_210826_14_qbtheta_parameters:")
    print("* With default qb12 settings, check the functionality for qbtheta parameter substitution")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    tqb.acc = "aer"
    tqb.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0; 
        include "qelib1.inc";
        creg c[4];
        x q[0];
        x q[2];
        u3(QBTHETA_0, 1.6, QBTHETA_1) q[3];
        measure q[0] -> c[0];
        measure q[1] -> c[1];
        measure q[2] -> c[2];
        measure q[3] -> c[3];
    }'''
    mth = qb.ND()
    mth[0] = 0.05
    mth[1] = -0.7
    tqb.theta[0] = qb.MapND([mth])
    tqb.run()
    assert (tqb.out_transpiled_circuit[0][0] == '\n__qpu__ void QBCIRCUIT(qreg q) {\nOPENQASM 2.0;\ninclude "qelib1.inc";\nu(3.14159, -1.5708, 1.5708) q[0];\nu(3.14159, -1.5708, 1.5708) q[2];\nu(0.05, 1.6, -0.7) q[3];\ncreg c0[1];\nmeasure q[0] -> c0[0];\ncreg c1[1];\nmeasure q[1] -> c1[0];\ncreg c2[1];\nmeasure q[2] -> c2[0];\ncreg c3[1];\nmeasure q[3] -> c3[0];\n\n}\n')


def test_raw_openqasm_str():
    print(" Testing raw OpenQASM string input ")
    import core as qb
    tqb = qb.core()
    tqb.debug = True
    nb_qubits = 12
    circ = qb.Circuit()
    circ.h(0)
    # Entangle all qubits
    for i in range(nb_qubits - 1):
        circ.cnot(i, i + 1)
    circ.measure_all()

    tqb.qb12()
    tqb.instring = circ.openqasm()
    tqb.run()
    print(tqb.out_raw[0])
    assert(len(tqb.out_count[0][0]) == 2)

def test_raw_openqasm_file():
    print(" Testing raw OpenQASM file input ")
    import core as qb
    import os 
    tqb = qb.core()
    tqb.debug = True
    dir_path = os.path.dirname(os.path.realpath(__file__))
    tqb.infile = dir_path + "/test_openqasm.qasm"
    tqb.qb12()
    tqb.run()
    print(tqb.out_raw[0])
    assert(len(tqb.out_count[0][0]) == 2)

def test_raw_openqasm_custom_qreg_names():
    print(" Testing raw OpenQASM input using different qreg variable names")
    import core as qb
    tqb = qb.core()
    tqb.qb12()
    tqb.instring = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg qa1_bb[2];
    creg c[2];
    h qa1_bb[0];
    cx qa1_bb[0],qa1_bb[1];
    measure qa1_bb[0] -> c[0];
    measure qa1_bb[1] -> c[1];
    '''
    tqb.run()
    print(tqb.out_raw[0])
    assert(len(tqb.out_count[0][0]) == 2)

    tqb.instring = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q1[2];
    creg c[2];
    h q1[0];
    cx q1[0],q1[1];
    measure q1[0] -> c[0];
    measure q1[1] -> c[1];
    '''
    tqb.run()
    print(tqb.out_raw[0])
    assert(len(tqb.out_count[0][0]) == 2)

    tqb.instring = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg qb_reg123[2];
    creg c[2];
    h qb_reg123[0];
    cx qb_reg123[0],qb_reg123[1];
    measure qb_reg123[0] -> c[0];
    measure qb_reg123[1] -> c[1];
    '''
    tqb.run()
    print(tqb.out_raw[0])
    assert(len(tqb.out_count[0][0]) == 2)
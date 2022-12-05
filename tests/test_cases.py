# Copyright (c) 2021 Quantum Brilliance Pty Ltd
import os
import pytest
def test_CI_210826_1_qb12_check_qn_equals_12() :
    print("* CI_210826_1_qb12_check_qn_equals_12:")
    print("* Imports qb.core, creates a session and calls qb12(), which should give a default setup for 12 qubits.")
    import qb.core
    s = qb.core.session()
    s.qb12()
    assert (s.qn[0][0]) == 12, "[QB SDK] Failed test: CI_210826_1_qb12_check_qn_equals_12"

def test_CI_210826_2_qb12_check_sn_equals_1024() :
    print("* CI_210826_2_qb12_check_sn_equals_1024:")
    print("* Imports qbemulator Python module and calls qb12(), which should give a default setup for 1024 shots")
    import qb.core
    s = qb.core.session()
    s.qb12()
    assert (s.sn[0][0]) == 1024, "[QB SDK] Failed test: CI_210826_2_qb12_check_sn_equals_1024"

#
# 210826 - Disabled until a backend implementation of rzz works
#
# def test_CI_210826_3_qb12_random_5() :
#     print("* CI_210826_3_qb12_random_5:")
#     print("* With default qb12 settings, with the default (TNQVM-ExaTN-MPS) backend, run a depth 5 circuit, check the length of the out_count dictionary is >0")
#     import qb.core
#     s = qb.core.session()
#     s.qb12()
#     s.random = 5
#     s.run()
#     assert (len(s.out_count[0][0])) > 0, "[QB SDK] Failed test: CI_210826_3_qb12_random_5 - TNQVM-ExaTN-MPS"

def test_CI_210826_4_qb12_random_5() :
    print("* CI_210826_4_qb12_random_5:")
    print("* With default qb12 settings, with the Aer backend, run a depth 5 circuit, check the length of the out_count dictionary is >0")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.random = 5
    s.acc = 'aer'
    s.run()
    assert (len(s.out_count[0][0])) > 0, "[QB SDK] Failed test: CI_210826_4_qb12_random_5 - Aer"

def test_CI_210826_5_qb12_random_5() :
    print("* CI_210826_5_qb12_random_5:")
    print("* With default qb12 settings, with the qpp backend, run a depth 3 circuit, check the length of the out_count dictionary is >0")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.random = 3
    s.acc = 'qpp'
    s.run()
    assert (len(s.out_count[0][0])) > 0, "[QB SDK] Failed test: CI_210826_5_qb12_random_5 - qpp"

def test_CI_210826_6_qb12_random_0() :
    print("* CI_210826_4_qb12_random_0:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.random = 0
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_7_qb12_random_minus_one() :
    print("* CI_210826_5_qb12_random_minus_one:")
    print("* [Edge condition] check that a TypeError exception occurs")
    import qb.core
    s = qb.core.session()
    s.qb12()
    with pytest.raises(TypeError):
        s.random = -1

def test_CI_210826_8_qb12_random_256() :
    print("* CI_210826_8_qb12_random_256:")
    print("* With default qb12 settings, run a depth 256 circuit, check the length of the out_count dictionary is >0")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.acc = "aer"
    s.random = 256
    s.run()
    assert (len(s.out_count[0][0])) > 0, "[QB SDK] failed test: CI_210826_8_qb12_random_256"


def test_CI_210826_9_qn_minus_one() :
    print("* CI_210826_9_qn_minus_one:")
    print("* [Edge condition] check that a TypeError exception occurs")
    import qb.core
    s = qb.core.session()
    s.qb12()
    with pytest.raises(TypeError):
        s.qn = -1

def test_CI_210826_10_qn_0() :
    print("* CI_210826_10_qn_0:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.qn = 0
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_11_qb12_no_infile_no_instring_no_random() :
    print("* CI_210826_11_qb12_no_infile_no_instring_no_random:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import qb.core
    s = qb.core.session()
    s.qb12()
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_12_qb12_nonexistent_infile() :
    print("* CI_210826_12_qb12_nonexistent_infile:")
    print("* [Edge condition] check that a ValueError exception occurs")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.infile = "nonexist.inc"
    with pytest.raises(ValueError):
        s.run()


def test_CI_210826_13_openqasm_index_out_of_range() :
    print("* CI_210826_13_openqasm_index_out_of_range:")
    print("* [XACC exit condition] when an indexing error is detected in OpenQASM")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0; include "qelib1.inc";
        //
        creg c[4];
        measure q[4] -> c[4];
    }'''
    with pytest.raises(ValueError):
        s.run()

def test_CI_210826_14_qbtheta_parameters() :
    print("* CI_210826_14_qbtheta_parameters:")
    print("* With default qb12 settings, check the functionality for qbtheta parameter substitution")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.acc = "aer"
    s.instring = '''
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
    s.theta[0] = qb.MapND([mth])
    s.run()
    assert (s.out_transpiled_circuit[0][0] == '\n__qpu__ void QBCIRCUIT(qreg q) {\nOPENQASM 2.0;\ninclude "qelib1.inc";\nu(3.14159, -1.5708, 1.5708) q[0];\nu(3.14159, -1.5708, 1.5708) q[2];\nu(0.05, 1.6, -0.7) q[3];\ncreg c0[1];\nmeasure q[0] -> c0[0];\ncreg c1[1];\nmeasure q[1] -> c1[0];\ncreg c2[1];\nmeasure q[2] -> c2[0];\ncreg c3[1];\nmeasure q[3] -> c3[0];\n\n}\n')

def test_CI_210826_15_qb_c5_ry_theta() :
    print("* CI_210826_15_qb_c5_ry_theta:")
    print("* With default qb12 settings, check the functionality of QB's custom 5-control Ry(theta) gate")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.acc = "aer"
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[6];
        x q[0];
        x q[1];
        x q[2];
        x q[3];
        x q[4];
        qb_c5_ry(0.2*pi) q[4],q[3],q[2],q[1],q[0],q[5];
        ry(-0.2*pi) q[5];
        measure q[5] -> c[5];
        measure q[4] -> c[4];
        measure q[3] -> c[3];
        measure q[2] -> c[2];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''
    s.run()
    assert (s.out_count[0][0][31] == 1024 )

def test_CI_210826_16_qb_c5_off_ry_theta() :
    print("* CI_210826_16_qb_c5_off_ry_theta:")
    print("* With default qb12 settings, check the functionality of QB's custom 5-control Ry(theta) gate, where 1 control input is |0>")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.acc = "aer"
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[6];
        x q[0];
        // x q[1];
        x q[2];
        x q[3];
        x q[4];
        qb_c5_ry(pi) q[4],q[3],q[2],q[1],q[0],q[5];
        ry(-pi) q[5];
        measure q[5] -> c[5];
        measure q[4] -> c[4];
        measure q[3] -> c[3];
        measure q[2] -> c[2];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''
    s.run()
    assert (s.out_count[0][0][61] == 1024)

def test_CI_210826_17_qb_c5_alloff_ry_theta() :
    print("* CI_210826_17_qb_c5_alloff_ry_theta:")
    print("* With default qb12 settings, check the functionality of QB's custom 5-control Ry(theta) gate, where all control inputs are |0>")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.acc = "aer"
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[6];
        // x q[0];
        // x q[1];
        // x q[2];
        // x q[3];
        // x q[4];
        qb_c5_ry(pi) q[4],q[3],q[2],q[1],q[0],q[5];
        ry(-pi) q[5];
        measure q[5] -> c[5];
        measure q[4] -> c[4];
        measure q[3] -> c[3];
        measure q[2] -> c[2];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''
    s.run()
    assert (s.out_count[0][0][32] == 1024)

def test_CI_210826_18_qb_c7_ry_ry_dag_theta() :
    print("* CI_210826_18_qb_c7_ry_ry_dag_theta:")
    print("* With default qb12 settings, check the functionality of QB's custom 7-control Ry(theta) gate, and it's inverse gate")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.acc = "aer"
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[8];
        x q[0];
        x q[1];
        x q[2];
        x q[3];
        x q[4];
        x q[5];
        x q[6];
        qb_c7_ry(0.1*pi) q[6],q[5],q[4],q[3],q[2],q[1],q[0],q[7];
        qb_c7_ry_dag(0.1*pi) q[6],q[5],q[4],q[3],q[2],q[1],q[0],q[7];
        measure q[7] -> c[7];
        measure q[6] -> c[6];
        measure q[5] -> c[5];
        measure q[4] -> c[4];
        measure q[3] -> c[3];
        measure q[2] -> c[2];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''
    s.run()
    assert (s.out_count[0][0][127] == 1024)


def test_CI_210826_19_qb_c2_x_x_dag() :
    print("* CI_210826_19_qb_c2_x_x_dag:")
    print("* With default qb12 settings, check the functionality of QB's custom Toffoli gate, and it's inverse gate")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.acc = "aer"
    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[3];
        x q[0];
        x q[1];
        qb_c2_x q[1],q[0],q[2];
        qb_c2_x_dag q[1],q[0],q[2];
        measure q[2] -> c[2];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''
    s.run()
    assert (s.out_count[0][0][3] == 1024)

def test_qft4() :
    print("* qft4: Execute 4-qubit Quantum Fourier Transform, noiseless, ExaTN-MPS")

    # Instantiate an object instance
    import qb.core
    s = qb.core.session()
    s.qb12()   # setup defaults = 12 qubits, 1024 shots, tnqvm-exatn-mps back-end

    # Override defaults
    n_qubits = 4
    n_shots = 1024
    s.qn = n_qubits      # We only need 4 qubits here
    s.sn = n_shots       # Explicitly use 1024 shots
    s.xasm = True        # Use XASM circuit format to access XACC's qft()
    s.seed = 123
    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void QBCIRCUIT(qbit q) {
          qft(q, {{"nq",4}});
          Measure(q[3]);
          Measure(q[2]);
          Measure(q[1]);
          Measure(q[0]);
    }
    '''
    s.instring = targetCircuit

    # Run the circuit on the back-end
    s.run()

    # Get the Z-operator expectation value
    expectation_v = s.out_z_op_expect[0][0][0]

    # Test the value against assertions
    out_str = '''* Using %d shots, Z-operator expectation value: %f''' % (n_shots, expectation_v)
    print(out_str)
    assert expectation_v == pytest.approx(0.0, None, 0.2)

def test_aer_matrix_product_state_method() :
    print("* Aer: MPS, QB noise model enabled")

    # Instantiate an object instance
    import qb.core
    import ast
    s=qb.core.session()
    s.qb12()

    # Use the aer accelerator
    s.acc = "aer"
    # Simulation method = MPS
    s.aer_sim_type = "matrix_product_state"
    s.sn = 1024       # Explicitly use 1024 shots
    s.noise = True

    s.instring='''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[2];
        h q[1];
        cx q[1],q[0];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }
    '''

    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    print(res)
    # Expect noisy results: majority is bell pairs (00 and 11), but non-zero error states
    assert all([int(res["00"])>450, int(res["11"])>450, int(res["00"]) + int(res["11"]) < 1024])

def test_aer_matrix_product_state_method_large_circuit() :
    print("* Aer: MPS, QB noise model enabled, large number of qubits")

    # Instantiate an object instance
    import qb.core
    import ast
    s=qb.core.session()
    s.qb12()
    s.nooptimise = True
    s.noplacement = True
    s.output_oqm_enabled = False
    # Use the aer accelerator
    s.acc = "aer"
    # Simulation method = MPS
    s.aer_sim_type = "matrix_product_state"
    s.sn = 1024       # Explicitly use 1024 shots
    s.xasm = True
    s.noise = True

    # Generate DJ Circuit
    def qbdj(qn) :
        bitstr = [1,0]*(qn//2)
        xgates_str=''.join(['X(q['+str(mye[0])+']);' for mye in enumerate(bitstr) if mye[1]==1])

        generator = '''
    __qpu__ void QBCIRCUIT(qreg q) {\n'
    for (int i=0; i<%d; i++) {
      H(q[i]);
    }

    Z(q[%d]);
    // End of init

    // Start of oracle
    %s
    for (int i=0; i<%d; i++) {
      CNOT(q[i], q[%d]);
    }
    %s
    // End of oracle
    for (int i=0; i<%d; i++) {
      H(q[i]);
    }

    // Measurements
    for (int i=0; i<%d; i++) {
      Measure(q[i]);
    }
    }
    ''' % (qn+1,qn,xgates_str,qn,qn,xgates_str,qn,qn)
        return generator

    nb_qubits = 40
    print("DJ -", nb_qubits, ";Total Number of qubits:", nb_qubits + 1)
    s.qn = nb_qubits + 1
    s.instring = qbdj(nb_qubits)
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    print(res)
    expected_bitstring = nb_qubits * '1'
    # Expect noisy results
    assert all([int(res[expected_bitstring])>500, int(res[expected_bitstring])>500 < 1024])

def test_qblib_custom_gates() :
    print("* qblib.inc : include file for custom OpenQASM QB gates")
    print("* With default qb12 settings, check the functionality of QB's custom controlled Ry gate")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.acc = "aer"
    s.include_qb = "../../qblib.inc"

    s.instring = '''
    __qpu__ void QBCIRCUIT(qreg q) {
        OPENQASM 2.0;
        include "qelib1.inc";
        creg c[2];
        x q[0];
        c_ry(0.3*pi) q[0],q[1];
        ry(-0.3*pi) q[1];
        measure q[1] -> c[1];
        measure q[0] -> c[0];
    }'''
    s.run()
    assert (s.out_count[0][0][1] == 1024)

# Deprecated after implementing resampling - 220308
# def test_loopback_qcstack_dummy() :
#    print(" Loopback QCStack dummy count data")
#    import qb.core
#    s = qb.core.session()
#    s.qb12()
#    s.qn = 3
#    s.sn = 8
#    s.acc = "loopback"
#    s.instring = '''
#    __qpu__ void QBCIRCUIT(qreg q) {
#        OPENQASM 2.0;
#        include "qelib1.inc";
#        creg c[3];
#        h q[0];
#        x q[2];
#        measure q[2] -> c[2];
#        measure q[1] -> c[1];
#        measure q[0] -> c[0];
#    }'''
#    s.run()
#    assert (s.out_count[0][0][4] == 4)
#    assert (s.out_count[0][0][5] == 4)

def test_loopback_rx_ry_rz() :
    print(" Loopback QCStack check transpiling into discrete angles")
    import qb.core
    import json
    s = qb.core.session()
    s.qb12()
    s.qn = 1
    s.sn = 2
    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void QBCIRCUIT(qbit q) {
        Rx(q[0], 0.125*pi);
        Ry(q[0], 0.25*pi);
        Rz(q[0], 0.5*pi);
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "loopback"

    # Run the circuit on the back-end
    s.run()
    trj = json.loads(s.out_qbjson[0][0])
    assert (trj['circuit'][0] == "Rx(q[0],0.392699)")
    assert (trj['circuit'][-1] == "Rx(q[0],3.14159)")

def test_qpe():
    print(" Testing Quantum Phase Estimation ")
    import qb.core
    import ast
    s = qb.core.session()
    s.qb12()

    # Oracle
    oracle = qb.Circuit()
    oracle.u1(0, -1.96349540849)

    # 4-bit precision
    nb_bits_precision = 4

    circ = qb.Circuit()
    # State prep: eigen state of the oracle |1>
    circ.x(0)

    # Add QPE
    circ.qpe(oracle, nb_bits_precision)

    # Measure evaluation qubits
    for i in range(nb_bits_precision):
        circ.measure(i + 1)
    # Run:
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    # Expect that the result is "1101" => estimate the phase value of -1.96349540849
    good_count = int(res["1101"])
    # Very high count
    assert (good_count > 1000)

def test_amcu():
    print("Testing Multi-Controlled-U With Ancilla")
    import numpy as np
    import qb.core
    import ast
    s = qb.core.session()
    s.qb12()

    # In this example, we perform an mcx gate on all possible bitstrings
    num_qubits = 5
    control_bits = range(num_qubits-1)
    target_bit = num_qubits-1
    ancilla_bits = range(num_qubits, num_qubits + num_qubits - 2)

    for i in range(2**num_qubits):
        circ = qb.Circuit()

        # Prepare the input state
        bitstring = bin(i)[2:].zfill(5)
        for j in range(num_qubits):
            if bitstring[j] == "1":
                circ.x(j)

        # Add the amcu gate
        U = qb.Circuit()
        U.x(target_bit)
        circ.amcu(U,control_bits,ancilla_bits)

        # Measure and check results
        for j in range(num_qubits):
            circ.measure(j)
        s.ir_target = circ
        s.nooptimise = True
        s.noplacement = True
        s.notiming = True
        s.output_oqm_enabled = False
        s.acc = "qpp"
        s.run()
        result = s.out_raw[0][0]
        res = ast.literal_eval(result)

        if bitstring == "11111":
            assert("11110" in list(res.keys()))
        elif bitstring == "11110":
            assert("11111" in list(res.keys()))
        else:
            assert(bitstring in list(res.keys()))

def test_equality_checker():
    print(" Testing Equality Checker Circuit")
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()

    ###
    # Testing equality checker
    # compare all bitstrings of length 3
    ###

    # Set up inputs
    qubits_a = [0,1,2]
    qubits_b = [3,4,5]
    flag = 6

    # First do the no ancilla version
    for i in range(8):
        for j in range(8):
            circ = qb.Circuit()
            # Prepare input strings
            bin_i = bin(i)[2:].zfill(3)
            bin_j = bin(j)[2:].zfill(3)
            for k in range(3):
                if bin_i[k] == "1":
                    circ.x(qubits_a[k])
                if bin_j[k] == "1":
                    circ.x(qubits_b[k])
            # Add equality checker
            circ.equality_checker(qubits_a, qubits_b, flag)
            # Measure flag
            circ.measure(flag)
            # Run
            s.ir_target = circ
            s.nooptimise = True
            s.noplacement = True
            s.notiming = True
            s.output_oqm_enabled = False
            s.acc = "qpp"
            s.run()
            result = s.out_raw[0][0]
            res = ast.literal_eval(result)
            # Check results
            if i == j:
                assert("1" in list(res.keys()))
            else:
                assert("0" in list(res.keys()))

def test_canonical_ae():
    print(" Testing Canonical Quantum Amplitude Estimation ")
    import qb.core
    import numpy as np
    from qb.core import run_canonical_ae_with_oracle
    import json
    s = qb.core.session()
    s.qb12()
    p = 0.24
    theta_p = 2 * np.arcsin(np.sqrt(p))

    # State prep circuit: (preparing the state that we want to estimate the amplitude)
    state_prep = qb.Circuit()
    state_prep.ry(8, theta_p)

    # In this case, we don't construct the Grover operator by ourselves,
    # instead, just provide the oracle to detect the marked state (|1>)
    oracle = qb.Circuit()
    oracle.z(8)
    bits_precision = 8

    # Execute:
    result = run_canonical_ae_with_oracle(state_prep, oracle, bits_precision,1,1)
    res = json.loads(result)

    amplitude_estimate = float(res["AcceleratorBuffer"]["Information"]["amplitude-estimation"])
    assert (abs(amplitude_estimate - np.sqrt(p)) < 0.1)

def test_controlled_swap():
    print("Testing controlled swap")
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()

    ###
    # Testing controlled swap
    # Define an input string from the alphabet {a,b}
    # Entangle the input string letters to a flag indicating whether the letter is "b"
    # Conditional on this input flag, use controlled swap to move all b's to the end of the string
    # E.g., baba -> aabb
    ###

    # Set up
    qubits_string = [0,1,2,3,4]
    b_flags = [5,6,7,8,9]

    input_string = "babba"
    assert(len(input_string) == len(qubits_string))

    # Prepare input state
    circ = qb.Circuit()
    for i in range(len(input_string)):
        if input_string[i] == "b":
            circ.x(qubits_string[i])

    for i in range(len(input_string)):
        circ.cnot(qubits_string[i], b_flags[i])

    # Perform conditional swaps
    for i in range(len(input_string)):
        k = len(input_string) - 1 - i
        for j in range(k, len(input_string)-1):
            qubits_a = [qubits_string[j]]
            qubits_b = [qubits_string[j+1]]
            flags_on = [b_flags[k]]
            circ.controlled_swap(qubits_a=qubits_a, qubits_b=qubits_b, flags_on=flags_on)

    # Measure
    for i in range(len(input_string)):
        circ.measure(qubits_string[i])

    # Execute circuit
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(len(list(res.keys())) == 1)
    output_measurement = list(res.keys())[0]

    output_string = ""
    for i in range(len(output_measurement)):
        if output_measurement[i] == "0":
            output_string += "a"
        else:
            output_string += "b"

    expected_string = ""
    num_b = 0
    for i in range(len(input_string)):
        if input_string[i] == "b":
            num_b += 1
    for i in range(len(input_string) - num_b):
        expected_string += "a"
    for i in range(num_b):
        expected_string += "b"

    expected_measurement = ""
    for i in range(len(expected_string)):
        if expected_string[i] == "a":
            expected_measurement += "0"
        else:
            expected_measurement += "1"

    assert(output_measurement == expected_measurement)

def test_controlled_addition():
    print("Testing controlled ripple carry adder")
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()

    ###
    # Testing controlled addition.
    # In this example we test adding |10> to |000> conditional on a flag
    # When the flag is off we expect no addition to happen and the output sum to be |000>
    # When the flag is on we expect addition to happen and the output sum to be |100>
    ###

    # Set up input registers
    qubits_adder = [0,1]
    qubits_sum = [2,3,4] # Remember that the sum register needs more qubits than the adder register for overflow
    c_in = 5
    flag = [6]

    ###
    # Test 1: flag off
    ###

    circ1 = qb.Circuit()

    # Prepare initial state
    circ1.x(qubits_adder[0])

    # Perform conditional addition
    circ1.controlled_ripple_carry_adder(qubits_adder, qubits_sum, c_in, flags_on = flag)

    # Measure sum register
    for i in range(len(qubits_sum)):
        circ1.measure(qubits_sum[i])

    s.ir_target = circ1
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.run()
    result1 = s.out_raw[0][0]
    res1 = ast.literal_eval(result1)
    assert(res1["000"] == 1024)

    ###
    # Test 1: flag off
    ###

    circ2 = qb.Circuit()

    # Prepare initial state
    circ2.x(qubits_adder[0])

    # Prepare flag
    circ2.x(flag[0])

    # Perform conditional addition
    circ2.controlled_ripple_carry_adder(qubits_adder, qubits_sum, c_in, flags_on = flag)

    # Measure sum register
    for i in range(len(qubits_sum)):
        circ2.measure(qubits_sum[i])

    s.ir_target = circ2
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.run()
    result2 = s.out_raw[0][0]
    res2 = ast.literal_eval(result2)
    assert(res2["100"] == 1024)

def test_generalised_mcx():
    print("Testing generalised MCX")
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()
    s.sn = 1024

    # In this test we use generalised mcx to
    # perform mcx on all possible 3-qubit bit strings (|000>,...,|111>)
    # with all combinations of control qubit conditions ((on,on),...,(off,off))

    # Expected outcomes:

    # - When the control qubits are required to be (on,on) then we expect
    # |110> -> |111>, |111> -> |110>, and all other bit strings remain unchanged

    # - When the control qubits are required to be (on,off) then we expect
    # |100> -> |101>, |101> -> |100>, and all other bit strings remain unchanged

    # - When the control qubits are required to be (off,on) then we expect
    # |010> -> |011>, |011> -> |010>, and all other bit strings remain unchanged

    # - When the control qubits are required to be (off,off) then we expect
    # |000> -> |001>, |001> -> |000>, and all other bit strings remain unchanged

    control_qubits = [0, 1]
    target_qubit = 2

    conditions = [[0, 0], [0, 1], [1, 0], [1, 1]]
    input_bitstrings = ["000", "001", "010", "011", "100", "101", "110", "111"]
    for condition in conditions:
        for input_bitstring in input_bitstrings:
            circ = qb.Circuit()

            # Prepare the input bitstring
            for i in range(len(input_bitstring)):
                if input_bitstring[i] == "1":
                    circ.x(i)

            # Add the generalised mcx
            controls_on = []
            controls_off = []
            for i in range(len(condition)):
                if condition[i] == 0:
                    controls_off.append(control_qubits[i])
                else:
                    controls_on.append(control_qubits[i])
            circ.generalised_mcx(target_qubit, controls_on, controls_off)

            # Measurements
            circ.measure_all()

            # Run the circuit and check results
            s.ir_target = circ
            s.nooptimise = True
            s.noplacement = True
            s.notiming = True
            s.output_oqm_enabled = False
            s.acc = "qpp"
            s.run()
            result = s.out_raw[0][0]
            res = ast.literal_eval(result)
            expected_output = input_bitstring[0:2]
            if int(input_bitstring[0]) == condition[0] and int(input_bitstring[1]) == condition[1]:
                if input_bitstring[2] == "0":
                    expected_output += "1"
                else:
                    expected_output += "0"
            else:
                expected_output += input_bitstring[2]
            assert(list(res.keys())[0] == expected_output)
            assert(res[expected_output] == 1024)

def test_MLQAE():
    print("Testing Maximum Likelihood Qauntum Amplitude Estimation")
    # Test the example here:
    # https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
    # i.e., estimate the amplitude of the state:
    # sqrt(1-p)|0> + sqrt(p)|1>
    import numpy as np
    import qb.core
    from qb.core import run_MLQAE
    import ast
    s = qb.core.session()
    s.qb12()
    p = 0.24
    theta_p = 2 * np.arcsin(np.sqrt(p))

    # State prep circuit: (preparing the state that we want to estimate the amplitude)
    state_prep = qb.Circuit()
    state_prep.ry(0, theta_p)

    # In this case, we don't construct the Grover operator by ourselves,
    # instead, just provide the oracle to detect the marked state (|1>)
    oracle = qb.Circuit()
    oracle.z(0)
    num_runs = 6
    shots = 100
    def is_in_good_subspace(s,x):
        if int(s[0]) == 1:
            return 1
        else:
            return 0
    total_num_qubits = 1
    score_qubits = [0]

    # Execute:
    result = run_MLQAE(state_prep, oracle, is_in_good_subspace, score_qubits, total_num_qubits, num_runs, shots)
    res = ast.literal_eval(result)
    amplitude_estimate = float(res["AcceleratorBuffer"]["Information"]["amplitude-estimation"])
    assert(abs(amplitude_estimate-np.sqrt(p)) < 0.01)


def test_qaa():
    print(" Testing Quantum Amplitude Amplification (Grover's algorithm) ")
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()
    oracle = qb.Circuit()
    oracle.z(0)

    # For demonstration purposes, we start with a very low population of |1> state:
    # using Ry(epsilon) as the state preparation:
    # Ry(epsilon) |0> ~ epsilon |1> + sqrt(1 - epsilon^2) |1>
    # i.e., the initial population of the marked state is small (~ epsilon)
    epsilon = 0.05
    state_prep = qb.Circuit()
    state_prep.ry(0, epsilon)

    good_state_ampls = []
    # Testing a varying number of iteration
    for i in range(1, 40, 1):
        # Construct full amplitude amplification circuit:
        full_circuit = qb.Circuit()
        # Add amplitude amplification circuit for the above oracle and state preparation sub-circuits.
        full_circuit.amplitude_amplification(oracle, state_prep, i)
        # Add measurement:
        full_circuit.measure_all()

        # Run the full amplitude estimation procedure:
        s.ir_target = full_circuit
        s.nooptimise = True
        s.noplacement = True
        s.notiming = True
        s.output_oqm_enabled = False
        s.acc = "qpp"
        s.run()
        result = s.out_raw[0][0]
        res = ast.literal_eval(result)
        # Calculate the probability of the marked state
        good_count = 0
        # Must check if "1" is present
        # (with a small number of amplification rounds,
        # the amplitude of |1> may still be very small)
        if "1" in list(res.keys()):
            good_count = int(res["1"])

        good_state_ampls.append(good_count/1024)

    # Check the result
    iterations = range(1, 40, 1)
    # Get analytical result for comparison
    probs_theory = [np.sin((2*mm+1)*epsilon/2)**2 for mm in iterations]
    assert(np.allclose(good_state_ampls, probs_theory, atol=0.1))

def test_CI_220207_1_loopback_6s():
    print(" Loopback QCStack check polling interval set from /mnt/qb/share/qb_config.json")
    import qb.core
    import json
    import timeit
    s = qb.core.session()
    s.qb12()
    s.qn = 1
    s.sn = 2
    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void QBCIRCUIT(qbit q) {
        Rx(q[0], 0.125*pi);
        Ry(q[0], 0.25*pi);
        Rz(q[0], 0.5*pi);
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "loopback"

    raw_qpu_config_6s = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95}
    ]
    }
    '''

    raw_qpu_config_2s = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 2, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95}
    ]
    }
    '''

    json_file = open("../../qpu_config_loop_6s.json",'w')
    json_file.write(raw_qpu_config_6s)
    json_file.close()
    s.qpu_config = "../../qpu_config_loop_6s.json"
    # Run the circuit on the back-end
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim > 6.0)

    json_file = open("../../qpu_config_loop_2s.json",'w')
    json_file.write(raw_qpu_config_2s)
    json_file.close()
    s.qpu_config = "../../qpu_config_loop_2s.json"
    # Run the circuit on the back-end
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim < 5.0)

def test_CI_220225_1_init_measure_no_gates() :
    print("When a circuit contains no gates, QB hardware expects a JSON with circuit=[] instead of circuit='null'")
    import qb.core
    import json
    import timeit
    s = qb.core.session()
    s.qb12()

    s.qn = 1
    s.sn = 1
    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void QBCIRCUIT(qbit q) {
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "loopback"

    # Run the circuit on the back-end
    eltim = timeit.timeit(lambda: s.run(), number=1)
    res = json.loads(s.out_qbjson[0][0])
    assert(len(res["circuit"]) == 0)
    targetCircuit = '''
    __qpu__ void QBCIRCUIT(qbit q) {
        H(q[0]);
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    # Run the circuit on the back-end
    eltim = timeit.timeit(lambda: s.run(), number=1)
    res = json.loads(s.out_qbjson[0][0])
    assert(len(res["circuit"]) == 2)

def test_ripple_adder():
    print(" Testing Ripple Carry Adder Circuit ")
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()
    # Testing a simple integer adding, checking the full truth table.

    # Helper to set the input register to an integer value
    def set_integer(circuit, bits, value):
        num_bits = len(bits)
        bit_string = f'{value:0{num_bits}b}'[::-1]
        for idx, bit_val in enumerate(bit_string):
            if bit_val == '1':
                # Apply X to set the bit
                circuit.x(bits[idx])

    # Let's do a 2-qubit adder
    nb_qubits = 2
    # partition the qubit indices:
    # q0: carry in
    # q1, q2: input
    # q3, q4, q5: sum (include the carry over)
    c_in = 0
    a = range(1, nb_qubits + 1)
    # the result register has 3 qubits to catch the carry bit
    b = range(nb_qubits + 1, 2 * (nb_qubits + 1))

    # Testing all the case: 0, 1, 2, 3 (2-qubit)
    for i in range(4):
        for j in range (4):
            circ = qb.Circuit()
            set_integer(circ, a, i)
            set_integer(circ, b, j)
            circ.ripple_add(a, b, c_in)
            # Measure the result register
            for q in b:
                circ.measure(q)
            s.ir_target = circ
            s.nooptimise = True
            s.noplacement = True
            s.notiming = True
            s.output_oqm_enabled = False
            s.acc = "qpp"
            s.run()
            result = s.out_raw[0][0]
            res = ast.literal_eval(result)
            # Expected sum result
            expected = i + j
            expected_bit_string = f'{expected:0{3}b}'[::-1]
            assert(expected_bit_string in list(res.keys()))
            count = int(res[expected_bit_string])
            assert(count == 1024)

def test_ripple_adder_superposition():
    print(" Testing Ripple Carry Adder Circuit for superposition ")
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()
    # Let's do a 2-qubit adder
    nb_qubits = 2
    # partition the qubit indices:
    # q0: carry in
    # q1, q2: input
    # q3, q4, q5: sum (include the carry over)
    c_in = 0
    a = range(1, nb_qubits + 1)
    # the result register has 3 qubits to catch the carry bit
    b = range(nb_qubits + 1, 2 * (nb_qubits + 1))
    # Test adding superposition:
    # (|0> + |1>) + (|0> + |2>)
    # ==> |0> + |1> + |2> + |3>
    circ = qb.Circuit()
    circ.h(a[0])
    circ.h(b[1])
    circ.ripple_add(a, b, c_in)
    # Measure the result register
    for q in b:
        circ.measure(q)
    # Check the circuit:
    # circ.print()
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    # Expected sum result to be a superposition of 4 values:
    assert(len(list(res.keys())) == 4)

def test_comparator():
    print("Testing quantum bit string comparator")
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()

    # Comparator: compare two bitstrings |BestScore> and |TrialScore>
    # within the quantum register |TrialScore>|flag>|BestScore>|ancilla>
    # If TrialScore > BestScore then |flag> will be returned as |1>
    # Otherwise |flag> returned as |0>

    for i in range(0,4):
        for j in range(0,4):
            # BestScore
            BestScore = i

            # number of qubits encoding scores
            num_scoring_qubits = 2

            # set up qubit register
            trial_score_qubits = [1,3]
            flag_qubit = 2
            best_score_qubits = [0,4]
            ancilla_qubits = [5,6,7,8]

            # create circuit
            circ = qb.Circuit()

            # State prep
            TestScore = j
            TestScore_bin = bin(TestScore)[2:].zfill(num_scoring_qubits)
            for k in range(0,num_scoring_qubits):
                if TestScore_bin[k] == "1":
                    circ.x(trial_score_qubits[k])

            # Add comp
            circ.comparator(BestScore, num_scoring_qubits, trial_score_qubits, flag_qubit, best_score_qubits, ancilla_qubits, False)

            # Measure flag qubit
            circ.measure(flag_qubit)

            # Check the circuit:
            #print("OpenQASM:\n", circ.openqasm())

            # Run:
            s.ir_target = circ
            s.nooptimise = True
            s.noplacement = True
            s.notiming = True
            s.output_oqm_enabled = False
            s.acc = "qpp"
            s.run()
            result = s.out_raw[0][0]
            res = ast.literal_eval(result)
            #print("Result:\n", result)

            # Get results
            if j > i:
                expected_bit_string = "1"
            if j <= i:
                expected_bit_string = "0"
            assert(expected_bit_string in list(res.keys()))
            count = int(res[expected_bit_string])
            assert(count == 1024)

def test_efficient_encoding():
    print("Testing Efficient Encoding")
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()

    # Efficient Encoding: given the input |state>|00...0>
    # produces the output |state>|score> where the score for
    # each bitstring is given by a scoring function

    # In this example, we produce the state |000>|000> + |111>|111>

    # scoring function
    def scoring_function(i):
        return i

    # input parameters
    num_state_qubits = 3
    num_scoring_qubits = 3

    # create circuit
    circ = qb.Circuit()

    # State prep: |000>|000> + |111>|0000>
    circ.h(0)
    for k in range(num_state_qubits-1):
        circ.cnot(k,k+1)

    # Add ee
    circ.efficient_encoding(scoring_function, num_state_qubits, num_scoring_qubits)

    # Measure
    circ.measure_all()

    # Check the circuit:
    #print("OpenQASM:\n", circ.openqasm())

    # Run:
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    #print("Result:\n", result)

    # get results
    measurements = list(res.keys())
    allowed_outputs = ['000000', '111111']
    for measurement in measurements:
        assert(measurement in allowed_outputs)

def test_compare_beam_oracle():
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.acc = "aer"
    s.sn = 1024
    s.qb12()

    ##########################################################################
    #Test 1: SA = |0111>, FA = |01>, FB = |01>

    #Inputs
    q0 = 0
    q1 = 1
    q2 = 2
    FA = [3,4]
    FB = [5,6]
    SA = [7,8,9,10]

    circ = qb.Circuit()

    circ.x(FA[1])
    circ.x(FB[1])
    for i in range(1,len(SA)):
      circ.x(SA[i])

    #Add beam comparator
    circ.compare_beam_oracle(q0, q1, q2, FA, FB, SA)

    #Measure flags
    circ.measure(q0)
    circ.measure(q1)
    circ.measure(q2)
    #circ.print()

    #Run
    #result = circ.execute()
    #res = ast.literal_eval(result)
    #print(res)
    #assert("010" in res["AcceleratorBuffer"]["Measurements"])
    s.instring = circ.openqasm()
    s.run()
    #print(s.out_raw[0])
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(res["010"] == 1024)

    ##########################################################################
    #Test 2: SA = |1111>, FA = |01>, FB = |01>

    #Inputs
    q0 = 0
    q1 = 1
    q2 = 2
    FA = [3,4]
    FB = [5,6]
    SA = [7,8,9,10]

    circ = qb.Circuit()

    circ.x(FA[1])
    circ.x(FB[1])
    for i in range(0,len(SA)):
      circ.x(SA[i])

    #Add beam comparator
    circ.compare_beam_oracle(q0, q1, q2, FA, FB, SA)

    #Measure flags
    circ.measure(q0)
    circ.measure(q1)
    circ.measure(q2)

    #Run
    s.instring = circ.openqasm()
    s.run()
    #print(s.out_raw[0])
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(res["111"] == 1024)

def test_multiplication():
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.acc = "qsim"
    s.sn = 1024
    s.qb12()
    s.qn = 9

    ##########################################################################
    # Test 1: qubit_a = 1 = |10>, qubit_b = 3 = |11>,
    # qubit_result = 1 x 3 = 3 = |1100>
    # Inputs
    qubits_a = [0,1]
    qubits_b = [2,3]
    qubits_result = [4,5,6,7]
    qubit_ancilla = 8

    circ = qb.Circuit()

    # Prepare inputs
    circ.x(qubits_a[0])
    circ.x(qubits_b[0])
    circ.x(qubits_b[1])

    circ.multiplication(qubits_a, qubits_b, qubits_result, qubit_ancilla, is_LSB=True)

    # Measure
    for i in range(len(qubits_a)):
        circ.measure(qubits_a[i])
    for i in range(len(qubits_b)):
        circ.measure(qubits_b[i])
    for i in range(len(qubits_result)):
        circ.measure(qubits_result[i])

    #circ.print()

    # Run circuit
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    # print(res)
    assert(res["10111100"] == 1024)

def test_controlled_multiplication():
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.acc = "qsim"
    s.sn = 1024
    s.qb12()
    s.qn = 10

    ##########################################################################
    # Test 1: qubit_a = 1 = |10>, qubit_b = 3 = |11>,
    # qubit_result = 1 x 3 = 3 = |1100>
    # Inputs
    qubits_a = [0,1]
    qubits_b = [2,3]
    qubits_result = [4,5,6,7]
    qubit_ancilla = 8
    controls_on = [9]

    circ = qb.Circuit()

    # Prepare inputs
    circ.x(qubits_a[0])
    circ.x(qubits_b[0])
    circ.x(qubits_b[1])

    circ.x(controls_on[0])

    circ.controlled_multiplication(qubits_a, qubits_b, qubits_result, qubit_ancilla, is_LSB=True, controls_on=controls_on)

    # Measure
    for i in range(len(qubits_a)):
        circ.measure(qubits_a[i])
    for i in range(len(qubits_b)):
        circ.measure(qubits_b[i])
    for i in range(len(qubits_result)):
        circ.measure(qubits_result[i])

    #circ.print()

    # Run circuit
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    #print(res)
    assert(res["10111100"] == 1024)

def test_raw_openqasm_str():
    print(" Testing raw OpenQASM string input ")
    import qb.core
    s = qb.core.session()
    s.debug = True
    nb_qubits = 12
    circ = qb.Circuit()
    circ.h(0)
    # Entangle all qubits
    for i in range(nb_qubits - 1):
        circ.cnot(i, i + 1)
    circ.measure_all()

    s.qb12()
    s.instring = circ.openqasm()
    s.run()
    print(s.out_raw[0])
    assert(len(s.out_count[0][0]) == 2)

def test_raw_openqasm_file():
    print(" Testing raw OpenQASM file input ")
    import qb.core
    import os
    s = qb.core.session()
    s.debug = True
    dir_path = os.path.dirname(os.path.realpath(__file__))
    s.infile = dir_path + "/test_openqasm.qasm"
    s.qb12()
    s.run()
    print(s.out_raw[0])
    assert(len(s.out_count[0][0]) == 2)

def test_raw_openqasm_custom_qreg_names():
    print(" Testing raw OpenQASM input using different qreg variable names")
    import qb.core
    s = qb.core.session()
    s.qb12()
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
    print(s.out_raw[0])
    assert(len(s.out_count[0][0]) == 2)

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
    print(s.out_raw[0])
    assert(len(s.out_count[0][0]) == 2)

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
    print(s.out_raw[0])
    assert(len(s.out_count[0][0]) == 2)

def test_async_run_and_wait():
    print(" Testing asynchronous job runs ")
    import qb.core
    import json
    import numpy as np
    s = qb.core.session()
    s.qb12()
    qpu_configs = {"accs": [{"acc": "aer"}, {"acc": "qpp"}]}
    s.set_parallel_run_config(json.dumps(qpu_configs))
    nb_jobs = 2
    s.qn[0].clear()
    s.sns[0].clear()
    for i in range(nb_jobs):
        # All circuits use 4 qubits
        s.qn[0].append(16)
        s.sns[0].append(1024)

    openQASM = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[16];
    creg c[16];
    h q[0];
    cx q[0],q[1];
    cx q[1],q[2];
    cx q[2],q[3];
    cx q[3],q[4];
    cx q[4],q[5];
    cx q[5],q[6];
    cx q[6],q[7];
    cx q[7],q[8];
    cx q[8],q[9];
    cx q[9],q[10];
    cx q[10],q[11];
    cx q[11],q[12];
    cx q[12],q[13];
    cx q[13],q[14];
    cx q[14],q[15];
    measure q -> c;
    '''
    s.instrings.clear()
    s.xasms[0].clear()
    s.quil1s[0].clear()
    for i in range(nb_jobs):
        s.instrings.append(qb.String())
        s.instrings[i].append(openQASM)
        s.xasms[0].append(False)
        s.quil1s[0].append(False)

    all_handles = []
    for i in range(nb_jobs):
        handle = s.run_async(i, 0)
        all_handles.append(handle)
    print("Complete posting all", nb_jobs, "jobs")

    import time
    all_done = False
    while (not all_done):
        all_done = True
        complete_count = 0
        for handle in all_handles:
            completed = handle.complete()
            if not completed:
                all_done = False
            else:
                complete_count = complete_count + 1
        if not all_done:
            time.sleep(1)

    assert(len(s.out_count) == 2)
    # Bell States
    assert(len(s.out_count[0][0]) == 2)
    assert(len(s.out_count[1][0]) == 2)


def test_normal_request_with_downsampling():
    print("Using loopback to test downsampling")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm=True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=3
    raw_qpu_resample = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": true, "resample_above_percentage": 100},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": true, "resample_above_percentage": 100},
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 1, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": true, "resample_above_percentage": 100}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"

    s.run()
    assert(sum([jj for jj in (s.out_count[0][0]).values()]) == 3)

def test_normal_request_with_upsampling():
    print("Using loopback to test upsampling")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=30
    raw_qpu_resample = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": true, "resample_above_percentage": 100},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": true, "resample_above_percentage": 100},
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 1, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": true, "resample_above_percentage": 100}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"
    s.run()
    assert(sum([jj for jj in (s.out_count[0][0]).values()]) == 30)

def test_normal_request_recursive_with_resampling_above_threshold():
    print("Using loopback to test recursive requests + forced resampling when 50% or above of requested measurements are successful")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=16
    raw_qpu_resample = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": true, "resample": false, "resample_above_percentage": 50},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": true, "resample": false, "resample_above_percentage": 50},
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 1, "poll_retrys": 100, "over_request": 1, "recursive_request": true, "resample": false, "resample_above_percentage": 50}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"
    s.run()
    assert(sum([jj for jj in (s.out_count[0][0]).values()]) == 16)


def test_normal_request_recursive_no_resampling():
    print("Using loopback to test recursive requests + no resampling")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=16
    raw_qpu_resample = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": true, "resample": false, "resample_above_percentage": 100},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": true, "resample": false, "resample_above_percentage": 100},
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 1, "poll_retrys": 100, "over_request": 1, "recursive_request": true, "resample": false, "resample_above_percentage": 100}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"
    s.run()
    assert(sum([jj for jj in (s.out_count[0][0]).values()]) == 16)

def test_over_request_recursive_no_resampling():
    print("Using loopback to test recursive 8x over-requests + no resampling")
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=16
    raw_qpu_resample = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 8, "recursive_request": true, "resample": false, "resample_above_percentage": 100},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 8, "recursive_request": true, "resample": false, "resample_above_percentage": 100},
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 1, "poll_retrys": 100, "over_request": 8, "recursive_request": true, "resample": false, "resample_above_percentage": 100}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"
    s.run()
    assert(sum([jj for jj in (s.out_count[0][0]).values()]) == 16)

def test_over_request_recursive_resampling_qb_safe_limit_shots():
    print("Using loopback to test QB_SAFE_LIMIT_SHOTS")
    import qb.core
    import json
    QB_SAFE_LIMIT_SHOTS = 512
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=1024
    raw_qpu_resample = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 8, "recursive_request": true, "resample": true, "resample_above_percentage": 100},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 8, "recursive_request": true, "resample": true, "resample_above_percentage": 100},
    {"acc": "loopback", "url": "http://127.0.0.1:8000", "poll_secs": 1, "poll_retrys": 100, "over_request": 8, "recursive_request": true, "resample": true, "resample_above_percentage": 100}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"
    s.run()
    tjs = json.loads(s.out_qbjson[0][0])
    assert(tjs['settings']['shots'] == QB_SAFE_LIMIT_SHOTS)


def test_potential_async_deadlock():
    print("Test dispatching a large number of async jobs")
    import qb.core
    import json
    s = qb.core.session()
    s.qb12()
    qpu_configs = {"accs": [{"acc": "aer"}, {"acc": "qpp"}]}
    s.set_parallel_run_config(json.dumps(qpu_configs))
    # Quite a large number of jobs
    nb_jobs = 50
    s.qn[0].clear()
    s.sns[0].clear()
    for i in range(nb_jobs):
        # All circuits use 4 qubits
        s.qn[0].append(4)
        s.sns[0].append(1024)
    openQASM = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[4];
    creg c[4];
    x q[0];
    x q[2];
    h q[0];
    cu1(pi/2) q[1],q[0];
    h q[1];
    cu1(pi/4) q[2],q[0];
    cu1(pi/2) q[2],q[1];
    h q[2];
    cu1(pi/8) q[3],q[0];
    cu1(pi/4) q[3],q[1];
    cu1(pi/2) q[3],q[2];
    h q[3];
    measure q -> c;
    '''
    s.instrings.clear()
    s.xasms[0].clear()
    s.quil1s[0].clear()
    for i in range(nb_jobs):
        s.instrings.append(qb.String())
        s.instrings[i].append(openQASM)
        s.xasms[0].append(False)
        s.quil1s[0].append(False)

    all_handles = []
    for i in range(nb_jobs):
        handle = s.run_async(i, 0)
        all_handles.append(handle)
    print("Complete posting all", nb_jobs, "jobs")

    import time
    all_done = False
    while (not all_done):
        all_done = True
        complete_count = 0
        for handle in all_handles:
            completed = handle.complete()
            if not completed:
                all_done = False
            else:
                complete_count = complete_count + 1
        if not all_done:
            print(complete_count, "/", nb_jobs, "complete.")
            time.sleep(2)
        else:
            print("ALL DONE")

    #print("Result:")
    #for i in range(nb_jobs):
        #print(s.out_raw[i][0])
    count_qpp = 0
    count_aer = 0
    for handle in all_handles:
        if handle.qpu_name() == "qpp":
           count_qpp += 1
        if handle.qpu_name() == "aer":
           count_aer += 1
    assert(count_qpp > 0)
    assert(count_aer > 0)
    assert(count_qpp + count_aer == nb_jobs)

def test_noise_mitigation():
    import qb.core
    import json
    s = qb.core.session()
    s.qb12()
    s.noise = True
    s.nooptimise = True
    s.noplacement = True
    s.sn = 8192
    s.acc = "aer"
    # Simple Bell circuit
    circ = qb.Circuit()
    circ.h(0)
    circ.cnot(0, 1)
    circ.measure_all()

    s.qn = 2
    s.instring = circ.openqasm()
    s.run()
    print("Without mitigation:")
    print(s.out_raw[0][0])
    no_mitigation = json.loads(s.out_raw[0][0])

    s.noise_mitigation = "assignment-error-kernel"
    s.run()
    print("With mitigation:")
    print(s.out_raw[0][0])
    mitigation = json.loads(s.out_raw[0][0])
    assert(int(mitigation["01"]) < int(no_mitigation["01"]))
    assert(int(mitigation["10"]) < int(no_mitigation["10"]))

def test_sparse_sim():
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()
    s.xasm = True
    num_shots = 8192
    s.sn = num_shots
    s.acc = "sparse-sim"

    # Generate DJ Circuit
    def qbdj(qn) :
        import xacc
        import re
        bitstr = [1,0]*(qn//2)
        xgates_str=''.join(['X(q['+str(mye[0])+']);' for mye in enumerate(bitstr) if mye[1]==1])

        generator = '''
    __qpu__ void QBCIRCUIT(qreg q) {\n'
    for (int i=0; i<%d; i++) {
    H(q[i]);
    }

    Z(q[%d]);
    // End of init

    // Start of oracle
    %s
    for (int i=0; i<%d; i++) {
    CNOT(q[i], q[%d]);
    }
    %s
    // End of oracle
    for (int i=0; i<%d; i++) {
    H(q[i]);
    }

    // Measurements
    for (int i=0; i<%d; i++) {
    Measure(q[i]);
    }
    }
    ''' % (qn+1,qn,xgates_str,qn,qn,xgates_str,qn,qn)
        return generator

    nb_qubits = 31
    print("DJ -", nb_qubits, ";Total Number of qubits:", nb_qubits + 1)
    s.qn = nb_qubits + 1
    s.instring = qbdj(nb_qubits)
    s.run()
    results = s.out_raw[0][0]
    res = ast.literal_eval(results)
    expected_bitstring = '1'*nb_qubits
    assert(res[expected_bitstring] == num_shots)

def test_inverse_circuit():
    import qb.core
    import ast

    s = qb.core.session()
    s.qb12()
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.sn = 1024

    qubits = [0,1,2,3]
    circ = qb.Circuit()

    qft = qb.Circuit()
    qft.qft(qubits)

    circ.qft(qubits)
    circ.inverse_circuit(qft)
    for i in qubits:
        circ.measure(i)

    s.ir_target = circ
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(res["0000"] == 1024)

def test_subtraction():
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()
    s.qn = 7

    ###
    # Testing subtraction
    # In this example we perform a 3-qubit subtraction
    ###

    # Set up input registers
    qubits_larger = [0,1,2]
    qubits_smaller = [3,4,5]
    qubit_ancilla = 6

    circ = qb.Circuit()

    i = 6
    j = 4

    # Prepare initial state
    bin_i = bin(i)[2:].zfill(3)
    bin_j = bin(j)[2:].zfill(3)

    for k in range(3):
        if bin_i[k] == '1':
            circ.x(qubits_larger[k])
        if bin_j[k] == '1':
            circ.x(qubits_smaller[k])

    # Perform subtraction
    circ.subtraction(qubits_larger, qubits_smaller, is_LSB = False, qubit_ancilla = qubit_ancilla)

    # Measure
    for k in range(6):
        circ.measure(k)

    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qsim"
    s.run()
    result1 = s.out_raw[0][0]
    res1 = ast.literal_eval(result1)
    # print(res1)

    expected_result = i-j
    expected_result_bin = bin(expected_result)[2:].zfill(3)

    expected_output = expected_result_bin + bin_j

    assert(res1[expected_output] == 1024)

def test_controlled_subtraction():
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()
    s.qn = 8

    ###
    # Testing subtraction
    # In this example we perform a 3-qubit subtraction
    ###

    # Set up input registers
    qubits_larger = [0,1,2]
    qubits_smaller = [3,4,5]
    control_on = [6]
    ancilla = 7

    circ = qb.Circuit()

    i = 6
    j = 4

    # Prepare initial state
    bin_i = bin(i)[2:].zfill(3)
    bin_j = bin(j)[2:].zfill(3)

    for k in range(3):
        if bin_i[k] == '1':
            circ.x(qubits_larger[k])
        if bin_j[k] == '1':
            circ.x(qubits_smaller[k])

    circ.x(control_on[0])

    # Perform subtraction
    circ.controlled_subtraction(qubits_larger, qubits_smaller, is_LSB = False, controls_on = control_on, qubit_ancilla = ancilla)

    # Measure
    for k in range(6):
        circ.measure(k)

    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qsim"
    s.run()
    result1 = s.out_raw[0][0]
    res1 = ast.literal_eval(result1)
    # print(res1)

    expected_result = i-j
    expected_result_bin = bin(expected_result)[2:].zfill(3)

    expected_output = expected_result_bin + bin_j

    assert(res1[expected_output] == 1024)

def test_proper_fraction_division():
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()
    s.qn = 11

    ###
    # Testing proper fraction division
    # In this example we perform every valid 3-qubit proper fraction division with 3 precision bits
    ###

    i = 1
    j = 2
    # Set up input registers
    qubits_numerator = [0,1]
    qubits_denominator = [2,3]
    qubits_fraction = [4,5]
    qubits_ancilla = list(range(6,11))

    circ = qb.Circuit()

    # Prepare initial state
    bin_i = bin(i)[2:].zfill(2)
    bin_j = bin(j)[2:].zfill(2)


    for k in range(2):
        if bin_i[k] == '1':
            circ.x(qubits_numerator[k])
        if bin_j[k] == '1':
            circ.x(qubits_denominator[k])

    # Perform subtraction
    circ.proper_fraction_division(qubits_numerator, qubits_denominator, qubits_fraction, qubits_ancilla, is_LSB = False)

    # Measure
    for k in qubits_numerator:
        circ.measure(k)
    for k in qubits_denominator:
        circ.measure(k)
    for k in qubits_fraction:
        circ.measure(k)
    for k in qubits_ancilla:
        circ.measure(k)

    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    # s.debug = True
    s.acc = "qsim"
    s.run()
    result1 = s.out_raw[0][0]
    res1 = ast.literal_eval(result1)

    # res = circ.execute("sparse-sim", 1024, 34)
    # res1 = ast.literal_eval(res)["AcceleratorBuffer"]["Measurements"]

    print(res1)
    assert(len(res1) == 1)

    if j != 0:
        expected_result = i/j
    else:
        expected_result = 0

    expected_result_bin = ""

    for k in range(1, len(qubits_fraction)+1):
        if expected_result - (1/2**k) >= 0:
            expected_result_bin += "1"
            expected_result -= 1/2**k
        else:
            expected_result_bin += "0"

    expected_output = bin_i + bin_j + expected_result_bin
    for bit in qubits_ancilla:
        expected_output += "0"

    assert(res1[expected_output] == 1024)

def test_controlled_proper_fraction_division():
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.qb12()
    s.qn = 12

    ###
    # Testing proper fraction division
    # In this example we perform every valid 2-qubit proper fraction division with 2 precision bits
    ###

    i = 1
    j = 2
    # Set up input registers
    qubits_numerator = [0,1]
    qubits_denominator = [2,3]
    qubits_fraction = [4,5]
    qubits_ancilla = list(range(6,11)) # = 2k + 1
    controls_on = [11]

    circ = qb.Circuit()

    # Prepare initial state
    bin_i = bin(i)[2:].zfill(2)
    bin_j = bin(j)[2:].zfill(2)

    circ.x(controls_on[0])


    for k in range(2):
        if bin_i[k] == '1':
            circ.x(qubits_numerator[k])
        if bin_j[k] == '1':
            circ.x(qubits_denominator[k])

    # Perform subtraction
    circ.controlled_proper_fraction_division(qubits_numerator, qubits_denominator, qubits_fraction, qubits_ancilla, controls_on, is_LSB = False)

    # Measure
    for k in qubits_numerator:
        circ.measure(k)
    for k in qubits_denominator:
        circ.measure(k)
    for k in qubits_fraction:
        circ.measure(k)
    for k in qubits_ancilla:
        circ.measure(k)

    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qsim"
    s.run()
    result1 = s.out_raw[0][0]
    res1 = ast.literal_eval(result1)
    assert(len(res1) == 1)
    # print(res1)

    if j != 0:
        expected_result = i/j
    else:
        expected_result = 0

    expected_result_bin = ""

    for k in range(1, len(qubits_fraction)+1):
        if expected_result - (1/2**k) >= 0:
            expected_result_bin += "1"
            expected_result -= 1/2**k
        else:
            expected_result_bin += "0"

    expected_output = bin_i + bin_j + expected_result_bin
    for k in range(len(qubits_ancilla)):
        expected_output += "0"

    assert(res1[expected_output] == 1024)

def test_compare_gt():
    import qb.core
    import ast

    s = qb.core.session()
    s.qb12()

    i = 3
    j = 2

    qubits_a = [0,1]
    qubits_b = [2,3]
    qubit_flag = 4
    qubit_ancilla = 5

    circ = qb.Circuit()

    a_bin = bin(i)[2:].zfill(2)
    b_bin = bin(j)[2:].zfill(2)

    for k in range(2):
        ak = a_bin[k]
        bk = b_bin[k]
        if ak == '1':
            circ.x(qubits_a[k])
        if bk == '1':
            circ.x(qubits_b[k])

    circ.compare_gt(qubits_a, qubits_b, qubit_flag, qubit_ancilla, is_LSB = False)

    circ.measure_all()

    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.qn = 6
    s.sn = 1024
    s.acc = "qsim"
    s.run()
    result1 = s.out_raw[0][0]
    res1 = ast.literal_eval(result1)

    expected_output = a_bin + b_bin
    if i > j:
        expected_output += "1"
    else:
        expected_output += "0"
    expected_output += "0"

    assert(res1[expected_output] == 1024)

def test_random_seed():
    # List of accelerators as required by https://qbau.atlassian.net/browse/SEB-97
    accelerators = ["qpp", "aer", "qsim"]
    num_runs = 5
    # Expected repeatable results
    for acc in accelerators:
        previous_result = None
        for i in range(num_runs):
            import xacc
            import qb.core
            s = qb.core.session()
            s.qb12()
            # Override defaults
            n_qubits = 4
            n_shots = 1024
            s.qn = n_qubits      # We only need 4 qubits here
            s.sn = n_shots       # Explicitly use 1024 shots
            s.xasm = True        # Use XASM circuit format to access XACC's qft()
            s.seed = 1234
            s.acc = acc
            # targetCircuit: contains the quantum circuit that will be processed/executed
            targetCircuit = '''
            __qpu__ void QBCIRCUIT(qbit q) {
                    qft(q, {{"nq",4}});
                    Measure(q[3]);
                    Measure(q[2]);
                    Measure(q[1]);
                    Measure(q[0]);
            }
            '''
            s.instring = targetCircuit

            # Run the circuit on the back-end
            s.run()

            # Get the Z-operator expectation value
            expectation_v = s.out_z_op_expect[0][0][0]

            # Test the value against assertions
            out_str = '''* Using %s accelerator with %d shots, Z-operator expectation value: %f''' % (acc, n_shots, expectation_v)
            print(out_str)
            # Verify that the result is deterministic between runs
            if previous_result is None:
                previous_result = expectation_v
            else:
                assert expectation_v == pytest.approx(previous_result, None, 0.01)

def test_random_seed_tnqvm():
    # Need to test TNQVM separately since it is using a global random number generator
    # which is only re-seeded when the seed value is changed.
    # TODO: combine this test with the above test once this ticket is fixed:
    # https://qbau.atlassian.net/browse/SEB-139
    seeds = [123, 456]
    results = {123: [], 456: []}
    num_runs = 5
    for i in range(num_runs):
        for seed in seeds:
            import qb.core
            s = qb.core.session()
            s.qb12()
            # Override defaults
            n_qubits = 4
            n_shots = 1024
            s.qn = n_qubits      # We only need 4 qubits here
            s.sn = n_shots       # Explicitly use 1024 shots
            s.xasm = True        # Use XASM circuit format to access XACC's qft()
            s.seed = seed
            s.acc = "tnqvm"
            # targetCircuit: contains the quantum circuit that will be processed/executed
            targetCircuit = '''
            __qpu__ void QBCIRCUIT(qbit q) {
                    qft(q, {{"nq",4}});
                    Measure(q[3]);
                    Measure(q[2]);
                    Measure(q[1]);
                    Measure(q[0]);
            }
            '''
            s.instring = targetCircuit

            # Run the circuit on the back-end
            s.run()

            # Get the Z-operator expectation value
            expectation_v = s.out_z_op_expect[0][0][0]

            # Test the value against assertions
            out_str = '''* Using %s accelerator with %d shots, Z-operator expectation value: %f''' % ("tnqvm", n_shots, expectation_v)
            print(out_str)
            # Verify that the result is deterministic between runs
            results[seed].append(expectation_v)
    assert len(results) == len(seeds)
    # Check that measurement results for the same seed value are all the same
    for result_for_seed in results.values():
        assert len(result_for_seed) == num_runs
        assert all(x == result_for_seed[0] for x in result_for_seed)


def test_randomized_stats():
    # https://qbau.atlassian.net/browse/SWA-154
    import qb.core
    s = qb.core.session()
    s.qb12()
    s.qn = 2
    s.sn = 1024
    s.instring = '''
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
creg c[2];
x q[0];
h q[1];
measure q[1] -> c[1];
measure q[0] -> c[0];
}'''
    nb_runs = 10
    for _ in range(nb_runs - 1):
        s.acc[0].append("tnqvm")
    s.run()
    ref = s.out_raw[0][0]
    all_equal = True
    print(s.out_raw[0])
    for dist in s.out_raw[0]:
        if dist != ref:
            all_equal = False

    assert all_equal == False

def test_clear_random_seed():
    # https://qbau.atlassian.net/browse/SWA-155
    import qb.core, ast
    s = qb.core.session()
    s.qb12()
    s.qn = 2
    s.sn = 1024
    s.seed = 123
    s.instring = '''
__qpu__ void QBCIRCUIT(qreg q) {
OPENQASM 2.0;
include "qelib1.inc";
creg c[2];
x q[0];
h q[1];
measure q[1] -> c[1];
measure q[0] -> c[0];
}'''
    s.run()
    ref = ast.literal_eval(s.out_raw[0][0])
    nb_runs = 3
    s.seed.clear()
    see_new_random_results = False
    for _ in range(nb_runs):
        s.run()
        new_result = ast.literal_eval(s.out_raw[0][0])
        if new_result != ref:
            see_new_random_results = True

    assert see_new_random_results

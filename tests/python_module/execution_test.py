# Test cases for circuit execution: e.g., qasm compilation and run

import pytest

def test_CI_210826_15_qb_c5_ry_theta() :
    print("* CI_210826_15_qb_c5_ry_theta:")
    print("* With default init settings, check the functionality of QB's custom 5-control Ry(theta) gate")
    import qb.core, ast
    s = qb.core.session()
    s.init()
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
    result = s.out_raw_json[0][0]
    res = ast.literal_eval(result)
    assert (res[''.join(bin(31)[2:].zfill(6))] == 1024)

def test_CI_210826_16_qb_c5_off_ry_theta() :
    print("* CI_210826_16_qb_c5_off_ry_theta:")
    print("* With default init settings, check the functionality of QB's custom 5-control Ry(theta) gate, where 1 control input is |0>")
    import qb.core, ast
    s = qb.core.session()
    s.init()
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
    result = s.out_raw_json[0][0]
    res = ast.literal_eval(result)
    assert (res[''.join(bin(61)[2:].zfill(6))] == 1024)

def test_CI_210826_17_qb_c5_alloff_ry_theta() :
    print("* CI_210826_17_qb_c5_alloff_ry_theta:")
    print("* With default init settings, check the functionality of QB's custom 5-control Ry(theta) gate, where all control inputs are |0>")
    import qb.core, ast
    s = qb.core.session()
    s.init()
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
    result = s.out_raw_json[0][0]
    res = ast.literal_eval(result)
    assert (res[''.join(bin(32)[2:].zfill(6))] == 1024)

def test_CI_210826_18_qb_c7_ry_ry_dag_theta() :
    print("* CI_210826_18_qb_c7_ry_ry_dag_theta:")
    print("* With default init settings, check the functionality of QB's custom 7-control Ry(theta) gate, and it's inverse gate")
    import qb.core, ast
    s = qb.core.session()
    s.init()
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
    result = s.out_raw_json[0][0]
    res = ast.literal_eval(result)
    assert (res[''.join(bin(127)[2:].zfill(8))] == 1024)

def test_CI_210826_19_qb_c2_x_x_dag() :
    print("* CI_210826_19_qb_c2_x_x_dag:")
    print("* With default init settings, check the functionality of QB's custom Toffoli gate, and it's inverse gate")
    import qb.core, ast
    s = qb.core.session()
    s.init()
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
    result = s.out_raw_json[0][0]
    res = ast.literal_eval(result)
    assert (res[''.join(bin(3)[2:].zfill(3))] == 1024)

def test_qft4() :
    print("* qft4: Execute 4-qubit Quantum Fourier Transform, noiseless, ExaTN-MPS")

    # Instantiate an object instance
    import qb.core
    s = qb.core.session()
    s.init()   # setup defaults = 12 qubits, 1024 shots, tnqvm-exatn-mps back-end

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
    s.init()

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
    result = s.out_raw_json[0][0]
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
    s.init()
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
    result = s.out_raw_json[0][0]
    res = ast.literal_eval(result)
    print(res)
    expected_bitstring = nb_qubits * '1'
    # Expect noisy results
    assert all([int(res[expected_bitstring])>500, int(res[expected_bitstring])>500 < 1024])

def test_qblib_custom_gates() :
    print("* qblib.inc : include file for custom OpenQASM QB gates")
    print("* With default init settings, check the functionality of QB's custom controlled Ry gate")
    import qb.core, ast
    s = qb.core.session()
    s.init()
    s.acc = "aer"
    import os
    s.include_qb = os.path.dirname(os.path.abspath(__file__)) + "/qblib.inc"

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
    result = s.out_raw_json[0][0]
    res = ast.literal_eval(result)
    assert (res[''.join(bin(1)[2:].zfill(2))] == 1024)

def test_async_run_and_wait():
    print(" Testing asynchronous job runs ")
    import qb.core, ast
    import json
    import numpy as np
    s = qb.core.session()
    s.init()
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
        s.instrings.append(qb.core.VectorString())
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

    result1 = s.out_raw_json[0][0]
    result2 = s.out_raw_json[1][0]
    res1 = ast.literal_eval(result1)
    res2 = ast.literal_eval(result2)
    # Bell States
    assert(len(list(res1.keys())) == 2)
    assert(len(list(res2.keys())) == 2)

def test_potential_async_deadlock():
    print("Test dispatching a large number of async jobs")
    import qb.core
    import json
    s = qb.core.session()
    s.init()
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
        s.instrings.append(qb.core.VectorString())
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
        #print(s.out_raw_json[i][0])
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
    s.init()
    s.noise = True
    s.nooptimise = True
    s.noplacement = True
    s.sn = 8192
    s.acc = "aer"
    # Simple Bell circuit
    circ = qb.core.Circuit()
    circ.h(0)
    circ.cnot(0, 1)
    circ.measure_all()

    s.qn = 2
    s.instring = circ.openqasm()
    s.run()
    print("Without mitigation:")
    print(s.out_raw_json[0][0])
    no_mitigation = json.loads(s.out_raw_json[0][0])

    s.noise_mitigation = "assignment-error-kernel"
    s.run()
    print("With mitigation:")
    print(s.out_raw_json[0][0])
    mitigation = json.loads(s.out_raw_json[0][0])
    assert(int(mitigation["01"]) < int(no_mitigation["01"]))
    assert(int(mitigation["10"]) < int(no_mitigation["10"]))

def test_sparse_sim():
    import qb.core
    import numpy as np
    import ast
    s = qb.core.session()
    s.init()
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
    results = s.out_raw_json[0][0]
    res = ast.literal_eval(results)
    expected_bitstring = '1'*nb_qubits
    assert(res[expected_bitstring] == num_shots)

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
            s.init()
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
            s.init()
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
    s.init()
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
    ref = s.out_raw_json[0][0]
    all_equal = True
    print(s.out_raw_json[0])
    for dist in s.out_raw_json[0]:
        if dist != ref:
            all_equal = False

    assert all_equal == False

def test_clear_random_seed():
    # https://qbau.atlassian.net/browse/SWA-155
    import qb.core, ast
    s = qb.core.session()
    s.init()
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
    ref = ast.literal_eval(s.out_raw_json[0][0])
    nb_runs = 3
    s.seed.clear()
    see_new_random_results = False
    for _ in range(nb_runs):
        s.run()
        new_result = ast.literal_eval(s.out_raw_json[0][0])
        if new_result != ref:
            see_new_random_results = True

    assert see_new_random_results

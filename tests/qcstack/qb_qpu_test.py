# Test qcstack_server interface with Qristal

import pytest

def test_CI_230208_simple_setters_for_contrast_thresholds():
    print("This test checks the ability to set contrast thresholds.")
    import qristal.core
    import json
    from yaml import safe_load, dump

    init_thresh = 0.01
    qubit_0_thresh = 0.03
    qubit_1_thresh = 0.05

    s = qristal.core.session()
    s.init()
    s.qn = 2
    s.sn = 32
    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void qristal_circuit(qbit q) {
        CZ(q[0], q[1]);
        Ry(q[1], -1.8*pi);
        Measure(q[1]);
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "loopback"

    # Set new options
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["use_default_contrast_settings"] = False
    db["init_contrast_threshold"] = init_thresh
    db["qubit_contrast_thresholds"] = { 0: qubit_0_thresh, 1: qubit_1_thresh }
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    stream.close()
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    # Run the circuit on the backend
    s.run()
    res = json.loads(s.out_qbjson[0][0])
    assert(res['settings'].keys().__contains__('readout_contrast_threshold') == True)
    assert(res['settings']['readout_contrast_threshold']['init'] == init_thresh)
    assert(res['settings']['readout_contrast_threshold']['qubits'][0] == qubit_0_thresh)
    assert(res['settings']['readout_contrast_threshold']['qubits'][1] == qubit_1_thresh)

    # Change the yaml file to just use default contrasts
    db["use_default_contrast_settings"] = True
    stream = open(s.remote_backend_database_path, 'w')
    dump({'loopback': db}, stream)
    stream.close()
    s.run()
    res = json.loads(s.out_qbjson[0][0])
    assert(res['settings'].keys().__contains__('readout_contrast_threshold') == False)

def test_CI_230131_cz_arbitrary_rotation():
    print("Checks CZ and Ry at arbitrary rotation angle.  Verifies that the requested number of shots is actually performed.")
    import qristal.core
    from yaml import safe_load, dump
    s = qristal.core.session()
    s.init()
    s.qn = 2
    s.sn = 64

    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void qristal_circuit(qbit q) {
        CZ(q[0], q[1]);
        Ry(q[1], -1.8*pi);
        Measure(q[1]);
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "loopback"

    # Run the circuit on the back-end
    s.run()
    assert(s.results[0][0].total_counts() == s.sn[0][0])

def test_CI_230131_arbitrary_rotation():
    print("Checks Rx and Ry arbitrary rotation angles.  Verifies that the requested number of shots is actually performed.")
    import qristal.core
    from yaml import safe_load, dump
    s = qristal.core.session()
    s.init()
    s.qn = 2
    s.sn = 64

    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void qristal_circuit(qbit q) {
        Rx(q[0], 0.0625*pi);
        Rz(q[0], -0.33*pi);
        Ry(q[1], -1.8*pi);
        Measure(q[1]);
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "loopback"

    # Run the circuit on the back-end
    s.run()
    assert(s.results[0][0].total_counts() == s.sn[0][0])

def test_CI_230106_1_loopback_6s():
    print("Check 2s and 6s polling interval.")
    import qristal.core
    from yaml import safe_load, dump
    import timeit
    s = qristal.core.session()
    s.init()
    s.qn = 1
    s.sn = 32
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
    s.acc = "loopback"

    # Set new options (including 6s polling)
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["poll_secs"] = 6
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"
    # Run the circuit on the backend
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim > 6.0)

    # Reset polling to 2s
    db["poll_secs"] = 2
    dump({'loopback': db}, stream)
    # Run the circuit on the backend
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim < 150.0)

def test_CI_220225_1_init_measure_no_gates() :
    print("When a circuit contains no gates, QB hardware expects a JSON with circuit=[] instead of circuit='null'")
    import qristal.core
    from yaml import safe_load, dump
    import json

    s = qristal.core.session()
    s.init()

    s.qn = 1
    s.sn = 32
    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void qristal_circuit(qbit q) {
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "loopback"

    # Run the circuit on the backend
    s.run()
    res = json.loads(s.out_qbjson[0][0])
    assert(len(res["circuit"]) == 0)

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void qristal_circuit(qbit q) {
        H(q[0]);
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit

    # Run the circuit on the backend
    s.run()
    res = json.loads(s.out_qbjson[0][0])
    assert(len(res["circuit"]) == 2)

# Note that the reservation is not released after this test, so the qcstack server
# remains in exclusive access mode afterwards -- so this test must run after all
# others that don't use exclusive access mode!
def test_reservation():
    print("Using json web token to reserve qcstack for exclusive access")
    import qristal.core
    from yaml import safe_load, dump
    s = qristal.core.session()
    s.init()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void qristal_circuit(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=16
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["exclusive_access"] = True
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"
    s.run()
    assert(s.results[0][0].total_counts() <= s.sn[0][0])


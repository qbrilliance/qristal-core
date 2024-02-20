# API V1 QC Stack Server integration with Qristal
# The QC Stack Server used here is a software-only instance.

import pytest

def test_CI_230208_simple_setters_for_contrast_thresholds():
    print("This test checks the ability to set contrast thresholds.")
    import qb.core
    import os
    import json
    from yaml import safe_load, dump

    init_thresh = 0.01
    qubit_0_thresh = 0.03
    qubit_1_thresh = 0.05

    s = qb.core.session()
    s.qb12()
    s.qn = 2
    s.sn = 32
    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void QBCIRCUIT(qbit q) {
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
    db["recursive_request"] = False
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
    print("Checks CZ and Ry at arbitrary rotation angle.  Verifies that the requested number of shots is actually performed via recursive requests")
    import qb.core, ast
    from yaml import safe_load, dump
    import os
    s = qb.core.session()
    s.qb12()
    s.qn = 2
    s.sn = 128

    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void QBCIRCUIT(qbit q) {
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
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    # Run the circuit on the back-end
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == s.sn[0][0])

def test_CI_230131_arbitrary_rotation():
    print("Checks Rx and Ry arbitrary rotation angles.  Verifies that the requested number of shots is actually performed via recursive requests")
    import qb.core, ast
    from yaml import safe_load, dump
    import os
    s = qb.core.session()
    s.qb12()
    s.qn = 2
    s.sn = 64

    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void QBCIRCUIT(qbit q) {
        Rx(q[0], 0.0625*pi);
        Rz(q[0], -0.33*pi);
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
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    # Run the circuit on the back-end
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == s.sn[0][0])

def test_CI_230106_1_loopback_6s():
    print("Check 2s and 6s polling interval set from JSON config file")
    import qb.core
    from yaml import safe_load, dump
    import timeit
    import os
    s = qb.core.session()
    s.qb12()
    s.qn = 1
    s.sn = 32
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

    # Set new options (including 6s polling)
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["poll_secs"] = 6
    db["recursive_request"] = False
    db["resample_above_percentage"] = 95
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"
    # Run the circuit on the back-end
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim > 6.0)

    # Reset polling to 2s
    db["poll_secs"] = 2
    dump({'loopback': db}, stream)    
    # Run the circuit on the back-end
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim < 25.0)


def test_CI_220225_1_init_measure_no_gates() :
    print("When a circuit contains no gates, QB hardware expects a JSON with circuit=[] instead of circuit='null'")
    import qb.core
    from yaml import safe_load, dump
    import timeit
    import os
    import json

    s = qb.core.session()
    s.qb12()

    s.qn = 1
    s.sn = 32
    s.xasm = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void QBCIRCUIT(qbit q) {
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "loopback"

    # Set new options
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["recursive_request"] = False
    db["resample_above_percentage"] = 95
    db["over_request"] = 1
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

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

def test_normal_request_with_upsampling():
    print("Using loopback to test upsampling")
    import qb.core, ast
    from yaml import safe_load, dump
    import os
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=30
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["recursive_request"] = False
    db["resample"] = True
    db["over_request"] = 1
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == 30)

def test_over_request_recursive_with_resampling_above_threshold():
    print("Using loopback to test recursive 4x-over-requests + forced resampling when 50% or above of requested measurements are successful")
    import qb.core, ast
    from yaml import safe_load, dump
    import os
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=16
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["over_request"] = 4
    db["resample_above_percentage"] = 95
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == 16)

def test_normal_request_recursive_no_resampling():
    print("Using loopback to test recursive requests + no resampling")
    import qb.core, ast
    from yaml import safe_load, dump
    import os
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=16
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["over_request"] = 1
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == 16)

def test_over_request_recursive_no_resampling():
    print("Using loopback to test recursive 8x over-requests + no resampling")
    import qb.core, ast
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'

    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=16
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == 16)

def test_over_request_recursive_resampling_qb_safe_limit_shots():
    print("Using loopback to test QB_SAFE_LIMIT_SHOTS")
    import qb.core
    from yaml import safe_load, dump
    import os
    import json
    QB_SAFE_LIMIT_SHOTS = 512
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=1024
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["resample"] = True
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"
    s.run()
    tjs = json.loads(s.out_qbjson[0][0])
    assert(tjs['settings']['shots'] == QB_SAFE_LIMIT_SHOTS)

# Note that the reservation is not released after this test, so the qcstack server 
# remains in exclusive access mode afterwards -- so this test must run after all 
# others that don't use exclusive access mode!
def test_reservation():
    print("Using json web token to reserve qcstack for exclusive access")
    import qb.core, ast
    from yaml import safe_load, dump
    import os
    s = qb.core.session()
    s.qb12()
    s.qn=2
    s.acc='loopback'
    s.xasm = True
    s.instring = '''__qpu__ void QBCIRCUIT(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''
    s.sn=16
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["loopback"]
    db["exclusive_access"] = True
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'loopback': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == 16)


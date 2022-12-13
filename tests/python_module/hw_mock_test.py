# Test cases for qcstack loopback
# IMPORTANT: the mock REST server must be running in the background for these tests to work.

import pytest

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

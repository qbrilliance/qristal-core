# Qristal integration test with a software-only QC Stack Server (part of QB Control Team's software stack)
#
# IMPORTANT: 
#    QC Stack Server software-only instances are available as
#    AWS Templates - [Sydney availability region] QB-SDK-2022-JUPYTER-WORKSHOP version 3
#    Note: HTTPS with Basic password auth is set up in the above template
#    that must be used for the URL of the 'loopback' entry.
#        Example: https://<replace-with-username>:<replace-with-password>@473b-3-26-179-186.au.ngrok.io
#
#    These tests will gain access to the URL via:
#       import os
#       qqu = os.environ['QB_QCSTACK_2023_2_1_URL']
#
#    Here the QB_QCSTACK_2023_2_1_URL is provided
#    through GitLab CI variables.

import pytest

def test_CI_220207_1_loopback_6s():
    print("Check 2s and 6s polling interval set from JSON config file")
    import qb.core
    import json
    import timeit
    import os
    qqu = os.environ['QB_QCSTACK_2023_2_1_URL']
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

    raw_qpu_config_6s = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "loopback", "url": "''' + qqu + '''", "poll_secs": 15, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95}
    ]
    }
    '''

    raw_qpu_config_2s = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "loopback", "url": "''' + qqu + '''", "poll_secs": 30, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95}
    ]
    }
    '''

    json_file = open("../../qpu_config_loop_6s.json",'w')
    json_file.write(raw_qpu_config_6s)
    json_file.close()
    s.qpu_config = "../../qpu_config_loop_6s.json"
    # Run the circuit on the back-end
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim > 15.0)

    json_file = open("../../qpu_config_loop_2s.json",'w')
    json_file.write(raw_qpu_config_2s)
    json_file.close()
    s.qpu_config = "../../qpu_config_loop_2s.json"
    # Run the circuit on the back-end
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim < 40.0)

def test_CI_220225_1_init_measure_no_gates() :
    print("When a circuit contains no gates, QB hardware expects a JSON with circuit=[] instead of circuit='null'")
    import qb.core
    import json
    import timeit
    import os
    qqu = os.environ['QB_QCSTACK_2023_2_1_URL']
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

    raw_qpu_config_5s = '''
    {   "accs": [
    {"acc": "dqc_gen1", "url": "https://10.10.10.120:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "qdk_gen1", "url": "https://10.10.10.121:8443", "poll_secs": 6, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95},
    {"acc": "loopback", "url": "''' + qqu + '''", "poll_secs": 5, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": false, "resample_above_percentage": 95}
    ]
    }
    '''
    json_file = open("../../qpu_config_loop_5s.json",'w')
    json_file.write(raw_qpu_config_5s)
    json_file.close()
    s.qpu_config = "../../qpu_config_loop_5s.json"

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
    import os
    qqu = os.environ['QB_QCSTACK_2023_2_1_URL']
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
    {"acc": "loopback", "url": "''' + qqu + '''", "poll_secs": 5, "poll_retrys": 100, "over_request": 1, "recursive_request": false, "resample": true, "resample_above_percentage": 100}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == 30)

def test_over_request_recursive_with_resampling_above_threshold():
    print("Using loopback to test recursive 4x-over-requests + forced resampling when 50% or above of requested measurements are successful")
    import qb.core, ast
    import os
    qqu = os.environ['QB_QCSTACK_2023_2_1_URL']
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
    {"acc": "loopback", "url": "''' + qqu + '''", "poll_secs": 5, "poll_retrys": 100, "over_request": 4, "recursive_request": true, "resample": false, "resample_above_percentage": 50}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == 16)

def test_normal_request_recursive_no_resampling():
    print("Using loopback to test recursive requests + no resampling")
    import qb.core, ast
    import os
    qqu = os.environ['QB_QCSTACK_2023_2_1_URL']
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
    {"acc": "loopback", "url": "''' + qqu + '''", "poll_secs": 5, "poll_retrys": 100, "over_request": 1, "recursive_request": true, "resample": false, "resample_above_percentage": 100}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == 16)

def test_over_request_recursive_no_resampling():
    print("Using loopback to test recursive 8x over-requests + no resampling")
    import qb.core, ast
    import os
    qqu = os.environ['QB_QCSTACK_2023_2_1_URL']
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
    {"acc": "loopback", "url": "''' + qqu + '''", "poll_secs": 5, "poll_retrys": 100, "over_request": 8, "recursive_request": true, "resample": false, "resample_above_percentage": 100}
    ]
    }
    '''
    json_file = open("../../qpu_config_resample.json",'w')
    json_file.write(raw_qpu_resample)
    json_file.close()
    s.qpu_config = "../../qpu_config_resample.json"
    s.run()
    result = s.out_raw[0][0]
    res = ast.literal_eval(result)
    assert(sum([jj for jj in (res).values()]) == 16)

def test_over_request_recursive_resampling_qb_safe_limit_shots():
    print("Using loopback to test QB_SAFE_LIMIT_SHOTS")
    import qb.core
    import json
    import os
    qqu = os.environ['QB_QCSTACK_2023_2_1_URL']
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
    {"acc": "loopback", "url": "''' + qqu + '''", "poll_secs": 5, "poll_retrys": 100, "over_request": 8, "recursive_request": true, "resample": true, "resample_above_percentage": 100}
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

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
    s.qn = 2
    s.sn = 32
    s.input_language = qristal.core.circuit_language.XASM
    s.nooptimise = True

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
    s.acc = "example_hardware_device"

    # Set new options
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["example_hardware_device"]
    db["use_default_contrast_settings"] = False
    db["init_contrast_threshold"] = init_thresh
    db["qubit_contrast_thresholds"] = { 0: qubit_0_thresh, 1: qubit_1_thresh }
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'example_hardware_device': db}, stream)
    stream.close()
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    # Run the circuit on the backend
    s.run()
    res = json.loads(s.qbjson)
    assert(res['settings'].keys().__contains__('readout_contrast_threshold') == True)
    assert(res['settings']['readout_contrast_threshold']['init'] == init_thresh)
    assert(res['settings']['readout_contrast_threshold']['qubits'][0] == qubit_0_thresh)
    assert(res['settings']['readout_contrast_threshold']['qubits'][1] == qubit_1_thresh)

    # Change the yaml file to just use default contrasts
    db["use_default_contrast_settings"] = True
    stream = open(s.remote_backend_database_path, 'w')
    dump({'example_hardware_device': db}, stream)
    stream.close()
    s.run()
    res = json.loads(s.qbjson)
    assert(res['settings'].keys().__contains__('readout_contrast_threshold') == False)

def test_CI_230131_cz_arbitrary_rotation():
    print("Checks CZ and Ry at arbitrary rotation angle.  Verifies that the requested number of shots is actually performed.")
    import qristal.core
    from yaml import safe_load, dump

    s = qristal.core.session()
    s.qn = 2
    s.sn = 64
    s.input_language = qristal.core.circuit_language.XASM
    s.nooptimise = True

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
    s.acc = "example_hardware_device"

    # Run the circuit on the back-end
    s.run()
    assert(s.results.total_counts() == s.sn)

def test_CI_230131_arbitrary_rotation():
    print("Checks Rx and Ry arbitrary rotation angles.  Verifies that the requested number of shots is actually performed.")
    import qristal.core
    from yaml import safe_load, dump

    s = qristal.core.session()
    s.qn = 2
    s.sn = 64
    s.input_language = qristal.core.circuit_language.XASM
    s.nooptimise = True

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
    s.acc = "example_hardware_device"

    # Run the circuit on the back-end
    s.run()
    assert(s.results.total_counts() == s.sn)

def test_CI_230106_1_example_hardware_device_6s():
    print("Check 2s and 6s polling interval.")
    import qristal.core
    from yaml import safe_load, dump
    import timeit

    s = qristal.core.session()
    s.qn = 1
    s.sn = 32
    s.input_language = qristal.core.circuit_language.XASM
    s.nooptimise = True

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
    s.acc = "example_hardware_device"

    # Set new options (including 6s polling)
    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["example_hardware_device"]
    db["poll_secs"] = 6
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'example_hardware_device': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"
    # Run the circuit on the backend
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim > 6.0)

    # Reset polling to 2s
    db["poll_secs"] = 2
    dump({'example_hardware_device': db}, stream)
    # Run the circuit on the backend
    eltim = timeit.timeit(lambda: s.run(), number=1)
    assert (eltim < 150.0)

def test_CI_220225_1_init_measure_no_gates() :
    print("When a circuit contains no gates, QB hardware expects a JSON with circuit=[] instead of circuit='null'")
    import qristal.core
    from yaml import safe_load, dump
    import json

    s = qristal.core.session()
    s.qn = 1
    s.sn = 32
    s.input_language = qristal.core.circuit_language.XASM
    s.nooptimise = True

    # targetCircuit: contains the quantum circuit that will be processed/executed
    targetCircuit = '''
    __qpu__ void qristal_circuit(qbit q) {
        Measure(q[0]);
    }
    '''
    s.instring = targetCircuit
    s.acc = "example_hardware_device"

    # Run the circuit on the backend
    s.run()
    res = json.loads(s.qbjson)
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
    res = json.loads(s.qbjson)
    assert(len(res["circuit"]) == 2)

def test_ACZ_native_gate_execution() :
    print("Using (Rx, Ry, ACZ) native gateset.")
    import qristal.core
    from yaml import safe_load, dump
    my_sim = qristal.core.session()
    my_sim.acc = "example_hardware_device"
    my_sim.qn = 2
    my_sim.sn = 20
    my_sim.instring = '''
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      cx q[1],q[0];
      measure q[0] -> c[0];
      measure q[1] -> c[1];
    }
    '''
    stream = open(my_sim.remote_backend_database_path, 'r')
    db = safe_load(stream)["example_hardware_device"]
    db["model"] = "QB-QDK2-ACZ"
    stream = open(my_sim.remote_backend_database_path + ".temp", 'w')
    dump({'example_hardware_device': db}, stream)
    stream.close()
    my_sim.remote_backend_database_path = my_sim.remote_backend_database_path + ".temp"
    my_sim.run()
    assert(my_sim.results.total_counts() == my_sim.sn)

# Note that the reservation is not released after this test, so the qcstack server
# remains in exclusive access mode afterwards -- so this test must run after all
# others that don't use exclusive access mode!
def test_reservation():
    print("Using json web token to reserve qcstack for exclusive access")
    import qristal.core
    from yaml import safe_load, dump

    s = qristal.core.session()
    s.qn=2
    s.sn=16
    s.acc='example_hardware_device'
    s.input_language = qristal.core.circuit_language.XASM
    s.nooptimise = True
    s.instring = '''__qpu__ void qristal_circuit(qreg q) { X(q[0]); H(q[1]); Measure(q[0]); }'''

    stream = open(s.remote_backend_database_path, 'r')
    db = safe_load(stream)["example_hardware_device"]
    db["exclusive_access"] = True
    stream = open(s.remote_backend_database_path + ".temp", 'w')
    dump({'example_hardware_device': db}, stream)
    s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

    s.run()
    assert(s.results.total_counts() == s.sn)


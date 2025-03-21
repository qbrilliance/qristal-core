# Test cases for benchmark submodule
import pytest
import numpy as np
import math
import qristal.core 
import qristal.core.benchmark as benchmark

def test_SPAMBenchmark_circuit_construction():
    n_qubits = 10
    qubits = {0, 2, 7}

    correct_circuits = []
    for i in range(9):
        cb = qristal.core.Circuit()
        correct_circuits.append(cb)  

    correct_circuits[1].x(0)
    correct_circuits[2].x(2)
    correct_circuits[3].x(0)
    correct_circuits[3].x(2)
    correct_circuits[4].x(7)
    correct_circuits[5].x(0)
    correct_circuits[5].x(7)
    correct_circuits[6].x(2)
    correct_circuits[6].x(7)
    correct_circuits[7].x(0)
    correct_circuits[7].x(2)
    correct_circuits[7].x(7)

    sim = qristal.core.session()
    sim.init()
    sim.acc = "qpp"
    sim.qn = n_qubits

    workflow = benchmark.SPAMBenchmark(qubits, sim)
    circuits = workflow.circuits

    for i in range(len(circuits)):
        assert circuits[i].openqasm() == correct_circuits[i].openqasm()

def test_RotationSweep_circuit_construction():
    n_qubits = 4
    rot_per_qubit = ['I','X', 'Y','Z']
    start_degree = -180
    end_degree = 180
    n_points = 5

    correct_circuits = []
    for i in range(5):
        cb = qristal.core.Circuit()
        correct_circuits.append(cb)

    correct_circuits[0].rx(1, -1.0 * np.pi);
    correct_circuits[0].ry(2, -1.0 * np.pi);
    correct_circuits[0].rz(3, -1.0 * np.pi);
    correct_circuits[1].rx(1, -1.0/2.0 * np.pi);
    correct_circuits[1].ry(2, -1.0/2.0 * np.pi);
    correct_circuits[1].rz(3, -1.0/2.0 * np.pi);
    correct_circuits[2].rx(1, 0.0);
    correct_circuits[2].ry(2, 0.0);
    correct_circuits[2].rz(3, 0.0);
    correct_circuits[3].rx(1, 1.0/2.0 * np.pi);
    correct_circuits[3].ry(2, 1.0/2.0 * np.pi);
    correct_circuits[3].rz(3, 1.0/2.0 * np.pi);
    correct_circuits[4].rx(1, np.pi);
    correct_circuits[4].ry(2, np.pi);
    correct_circuits[4].rz(3, np.pi);

    sim = qristal.core.session()
    sim.init()
    sim.acc='qpp'
    sim.qn = 4

    workflow = benchmark.RotationSweep(
      rot_per_qubit, start_degree, end_degree, n_points, sim
    )
    circuits = workflow.circuits

    for i in range(len(circuits)):
        assert circuits[i].openqasm() == correct_circuits[i].openqasm()

def test_PyGSTiBenchmark_circuit_readin():
    n_qubits = 11

    sim = qristal.core.session(False)
    sim.acc = 'qpp'
    sim.sn = 1000
    sim.qn = n_qubits

    circuit_list = qristal.core.VectorString([
      "{}@(0)",
      "Gxpi2:0@(0)",
      "Gypi2:0@(0)",
      "Gzpi2:0@(0)",
      "Gxpi4:0Gypi4:0Gzpi4:0@(0)",
      "Gn:0@(0)",
      "Gcnot:0:1@(0,1)",
      "Gcz:0:1@(0,1)",
      "Gcphase:0:1@(0,1)",
      "Gxx:0:1@(0,1)",
      "Gyy:0:1@(0,1)",
      "Gzz:0:1@(0,1)",
      "Gxxpi2:0:1Gyypi2:0:1Gzzpi2:0:1@(0,1)",
      "Gxx:9:10Gxpi2:7Gcz:0:2Gzzpi2:4:3@(0,1,2,3,4,5,6,7,8,9,10)"
    ])
    
    correct_circuits = []
    cb = qristal.core.Circuit()
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.rx(0, np.pi/2.0)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.ry(0, np.pi/2.0)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.rz(0, np.pi/2.0)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.rx(0, np.pi/4.0)
    cb.ry(0, np.pi/4.0)
    cb.rz(0, np.pi/4.0)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.rx(0, np.pi/2.0)
    cb.ry(0, math.sqrt(3.0)/2.0)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.cnot(0, 1)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.cz(0, 1)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.cphase(0, 1, np.pi)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.ry(0, np.pi/2.0)
    cb.x(0)
    cb.cz(0,1)
    cb.rx(1,-1.0*np.pi)
    cb.cz(0,1)
    cb.ry(0, np.pi/2.0)
    cb.x(0)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.rx(0, np.pi/2.0)
    cb.rx(1, -1.0*np.pi/2.0)
    cb.ry(1, -1.0*np.pi/2.0)
    cb.cz(0,1)
    cb.rx(1, -1.0*np.pi)
    cb.cz(0,1)
    cb.rx(0, -1.0*np.pi/2.0)
    cb.ry(1, np.pi/2.0)
    cb.rx(1, np.pi/2.0)
    correct_circuits.append(cb)

    cb = qristal.core.Circuit()
    cb.ry(1, np.pi / 2.0)
    cb.x(1)
    cb.cz(0, 1)
    cb.rx(1, -1.0 * np.pi)
    cb.cz(0, 1)
    cb.ry(1, np.pi / 2.0)
    cb.x(1)
    correct_circuits.append(cb)
    
    cb = qristal.core.Circuit()
    cb.ry(0, np.pi / 2.0)
    cb.x(0)
    cb.cz(0, 1) 
    cb.rx(1, np.pi / 2.0)
    cb.cz(0, 1) 
    cb.ry(0, np.pi / 2.0)
    cb.x(0)

    cb.rx(0, np.pi / 2.0)
    cb.rx(1, -1.0 * np.pi / 2.0)
    cb.ry(1, -1.0 * np.pi / 2.0)
    cb.cz(0, 1)
    cb.rx(1, np.pi / 2.0)
    cb.cz(0, 1)
    cb.rx(0, -1.0 * np.pi / 2.0)
    cb.ry(1, np.pi / 2.0)
    cb.rx(1, np.pi / 2.0)

    cb.ry(1, np.pi / 2.0)
    cb.x(1)
    cb.cz(0, 1)
    cb.rx(1, np.pi / 2.0)
    cb.cz(0, 1)
    cb.ry(1, np.pi / 2.0)
    cb.x(1)
    correct_circuits.append(cb)
    
    cb = qristal.core.Circuit()
    cb.ry(9, np.pi / 2.0); 
    cb.x(9)
    cb.cz(9, 10); 
    cb.rx(10, -1.0 * np.pi)
    cb.cz(9, 10); 
    cb.ry(9, np.pi / 2.0)
    cb.x(9)

    cb.rx(7, np.pi / 2.0)

    cb.cz(0, 2)

    cb.ry(3, np.pi / 2.0)
    cb.x(3)
    cb.cz(4, 3)
    cb.rx(3, np.pi / 2.0)
    cb.cz(4, 3)
    cb.ry(3, np.pi / 2.0)
    cb.x(3)
    correct_circuits.append(cb)

    workflow = benchmark.PyGSTiBenchmark(circuit_list, sim)

    assembled_circuits = workflow.circuits

    for correct, assembled in zip(correct_circuits, assembled_circuits):
        assert  correct.openqasm() == assembled.openqasm()

def test_QuantumStateTomography_checkSPAM():
    qubits = {0, 1, 2}

    sim = qristal.core.session(False)
    sim.init()
    sim.acc = "qsim"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.SPAMBenchmark(qubits, sim)
    measure_qst_qubits = {0, 2}
    qstworkflow = benchmark.QuantumStateTomography(workflow, measure_qst_qubits)

    #3 qubit SPAM should generate the following 8 workflow circuits:
    #000, 001, 010, 011, 100, 101, 110, 111
    #measuring the tomography of qubits 0 and 2 should result in the density matrices of states
    #00, 01, 00, 01, 10, 11, 10, 11
    ideal_densities = []
    dim = 2**len(measure_qst_qubits)
    for i in range(2**len(qubits)): 
        ideal_densities.append(np.zeros((dim, dim), dtype=complex))
    ideal_densities[0][0][0] = 1.0
    ideal_densities[1][1][1] = 1.0
    ideal_densities[2][0][0] = 1.0
    ideal_densities[3][1][1] = 1.0
    ideal_densities[4][2][2] = 1.0
    ideal_densities[5][3][3] = 1.0
    ideal_densities[6][2][2] = 1.0
    ideal_densities[7][3][3] = 1.0

    metric = benchmark.QuantumStateDensity(qstworkflow)
    results = metric.evaluate(True)
    _, measured_densities = next(iter(results.items()))
    for ideal, density in zip(ideal_densities, measured_densities): 
        assert np.all(np.isclose(ideal, density, atol=1e-2))

def get_ideal_rotation_density(rotation:chr, angle:float):
    c = np.cos(angle / 2.0)
    s = np.sin(angle / 2.0)
    density = np.zeros((2, 2), dtype=complex)
    if rotation == 'X' or rotation == 'x':
        density[0][0] = c**2
        density[0][1] = complex(0, c*s)
        density[1][0] = complex(0, -1.0*c*s)
        density[1][1] = s**2
    elif rotation == 'Y' or rotation == 'y':
        density[0][0] = c**2
        density[0][1] = c*s
        density[1][0] = c*s
        density[1][1] = s**2
    else: #Z or I 
        density[0][0] = 1.0
    return density

def test_QuantumStateTomography_checkRotationSweep():
    qubits = {0, 1}

    sim = qristal.core.session(False)
    sim.init() 
    sim.acc = "qsim"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.RotationSweep(['X', 'Y'], -90, +90, 6, sim)
    qstworkflow = benchmark.QuantumStateTomography(workflow)

    #assemble ideal densities
    ideal_densities = []
    current_rad = workflow.start_rad()
    while current_rad <= workflow.end_rad(): 
        d = np.ones((1, 1), dtype=complex)
        for rot in workflow.get_rotations_per_qubit():
             d = np.kron(get_ideal_rotation_density(rot, current_rad), d)
        ideal_densities.append(d)
        current_rad += workflow.step()

    #measure densities
    metric = benchmark.QuantumStateDensity(qstworkflow)
    results = metric.evaluate(True)

    #compare 
    _, measured_densities = next(iter(results.items()))
    for ideal, density in zip(ideal_densities, measured_densities): 
        assert np.all(np.isclose(ideal, density, atol=1e-2))

def test_QuantumStateTomography_checkMLE():
    qubits = {0, 1}

    sim = qristal.core.session(False) 
    sim.init()
    sim.acc = "qsim"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.RotationSweep(['Y', 'X'], -45, +45, 6, sim)
    qstworkflow = benchmark.QuantumStateTomography(workflow, True)
    qstworkflow.set_maximum_likelihood_estimation(100, 1e-6)

    #assemble ideal densities
    ideal_densities = []
    current_rad = workflow.start_rad()
    while current_rad <= workflow.end_rad(): 
        d = np.ones((1, 1), dtype=complex)
        for rot in workflow.get_rotations_per_qubit():
             d = np.kron(get_ideal_rotation_density(rot, current_rad), d)
        ideal_densities.append(d)
        current_rad += workflow.step()

    #measure densities
    metric = benchmark.QuantumStateDensity(qstworkflow)
    results = metric.evaluate(True)

    #compare 
    _, measured_densities = next(iter(results.items()))
    for ideal, density in zip(ideal_densities, measured_densities): 
        assert np.all(np.isclose(ideal, density, atol=1e-2))

def test_QuantumProcessTomography_checkSPAM():
    qubits = {0}

    sim = qristal.core.session()
    sim.init()
    sim.acc="qsim"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.SPAMBenchmark(qubits, sim)
    qstworkflow = benchmark.QuantumStateTomography(workflow)
    qptworkflow = benchmark.QuantumProcessTomography(qstworkflow)

    #build ideal processes 
    ideal_processes = []
    p1 = np.zeros((4**len(qubits), 4**len(qubits)), dtype=complex)
    p1[0][0] = 1.0 
    ideal_processes.append(p1)
    p2 = np.zeros((4**len(qubits), 4**len(qubits)), dtype=complex)
    p2[1][1] = 1.0 
    ideal_processes.append(p2)

    #measure processes
    metric = benchmark.QuantumProcessMatrix(qptworkflow)
    results = metric.evaluate(True)

    #compare
    _, measured_processes = next(iter(results.items()))
    for ideal, process in zip(ideal_processes, measured_processes): 
        assert(np.all(np.isclose(ideal, process, atol=1e-2)))

def get_ideal_rotation_process(rotation:chr, angle:float):
    c = np.cos(angle / 2.0)
    s = np.sin(angle / 2.0)
    process = np.zeros((4, 4), dtype=complex)
    index = 0
    if rotation == 'X' or rotation == 'x':
        index = 1
    elif rotation == 'Y' or rotation == 'y':
        index = 2
    elif rotation == 'Z' or rotation == 'z': 
        index = 3 
    process[0][0] = c**2
    process[0][index] = complex(0, c*s)
    process[index][0] = complex(0, -1.0*c*s)
    process[index][index] = s**2
    return process

def test_QuantumProcessTomography_checkRotationSweep(): 
    qubits = {0}

    sim = qristal.core.session(False)
    sim.init() 
    sim.acc = "aer"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.RotationSweep(['X'], -90, +90, 4, sim)
    qstworkflow = benchmark.QuantumStateTomography(workflow)
    qptworkflow = benchmark.QuantumProcessTomography(qstworkflow)

    #assemble ideal processes 
    ideal_processes = []
    current_rad = workflow.start_rad()
    while current_rad <= workflow.end_rad(): 
        p = np.ones((1, 1), dtype=complex)
        for rot in workflow.get_rotations_per_qubit():
             p = np.kron(get_ideal_rotation_process(rot, current_rad), p)
        ideal_processes.append(p)
        current_rad += workflow.step()

    #measure process matrices
    metric = benchmark.QuantumProcessMatrix(qptworkflow)
    results = metric.evaluate(True)

    #compare
    _, measured_processes = next(iter(results.items()))
    for ideal, process in zip(ideal_processes, measured_processes): 
        assert(np.all(np.isclose(ideal, process, atol=1e-2)))

def test_QuantumProcessTomography_checkSimpleCircuitExecution(): 
    qubits = {0, 1}

    sim = qristal.core.session(False)
    sim.init() 
    sim.acc = "qpp"
    sim.sn = 1000000
    sim.qn = len(qubits)

    #define two identical (but differently compiled circuits)
    angle = np.pi 
    #circuit 1
    circuit_native = qristal.core.Circuit() 
    circuit_native.cphase(0, 1, angle)
    #circuit 2
    circuit_transpiled = qristal.core.Circuit() 
    circuit_transpiled.rx(0, np.pi / 2.0)
    circuit_transpiled.ry(0, -1.0 * angle / 2.0)
    circuit_transpiled.rx(0, -1.0 * np.pi / 2.0)
    circuit_transpiled.ry(1, np.pi / 2.0)
    circuit_transpiled.rx(1, np.pi)
    circuit_transpiled.cz(0, 1)
    circuit_transpiled.rx(1, -1.0 * angle / 2.0)
    circuit_transpiled.cz(0, 1)
    l = np.fabs(angle) / 2.0 - np.pi
    if angle < 0.0: 
        l = -1.0 * l
    circuit_transpiled.rx(1, l)
    circuit_transpiled.ry(1, -1.0 * np.pi / 2.0)

    #measure QPT
    workflow = benchmark.SimpleCircuitExecution([circuit_native, circuit_transpiled], sim)
    qstworkflow = benchmark.QuantumStateTomography(workflow)
    qptworkflow = benchmark.QuantumProcessTomography(qstworkflow)

    metric = benchmark.QuantumProcessMatrix(qptworkflow)
    results = metric.evaluate(True)

    #compare
    _, measured_processes = next(iter(results.items()))
    assert(np.all(np.isclose(measured_processes[0], measured_processes[1], atol=1e-2)))

def test_CircuitFidelity_checkSPAM():
    qubits = {0, 1}

    sim = qristal.core.session()
    sim.init()
    sim.acc = "qpp"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.SPAMBenchmark(qubits,sim)

    metric = benchmark.CircuitFidelity(workflow)
    results = metric.evaluate(force_new = True)

    for timestamp,fidelities in results.items():
        for fidelity in fidelities:
            assert fidelity == pytest.approx(1.0,1e-2)

def test_CircuitFidelity_RotationSweep():
    sim = qristal.core.session(False)
    sim.init()
    sim.acc = 'qpp'
    sim.sn= 1000000
    sim.qn = 3

    workflow = benchmark.RotationSweep(['X', 'Y', 'Z'], -90, 90, 9, sim)

    metric = benchmark.CircuitFidelity(workflow)
    results = metric.evaluate(force_new = True)

    for timestamp, fidelities in results.items():
        for fidelity in fidelities:
            assert fidelity == pytest.approx(1.0, 1e-2)

def test_ConfusionMatrix_no_noise():
    n_qubit_list = [1, 2, 3, 4, 5]

    for n_qubits in n_qubit_list:
        dim = 2**n_qubits
        ideal = np.identity(dim)

        sim = qristal.core.session(False)
        sim.init()
        sim.acc = 'qpp'
        sim.sn = 100
        sim.qn = n_qubits

        qubits = set(range(n_qubits))

        workflow = benchmark.SPAMBenchmark(qubits, sim)

        metric = benchmark.ConfusionMatrix(workflow)
        results = metric.evaluate(force_new = True)

        for _, confusion in results.items():
           assert np.all(np.isclose(ideal,confusion, atol=1e-12))

def test_ConfusionMatrix_noisy():
    n_qubits_list = [1, 2]

    for n_qubits in n_qubits_list:
        dim = 2**n_qubits
        ideal = np.identity(dim)

        qubits = set(range(n_qubits))

        p_01 = 0.05
        p_10 = 0.05

        SPAM_error = qristal.core.NoiseModel()

        for q in range(len(qubits)):
            noise_model = qristal.core.NoiseModel()
            ro_error = qristal.core.ReadoutError()
            ro_error.p_01 = 0.05
            ro_error.p_10 = 0.05
            noise_model.set_qubit_readout_error(q, ro_error)

            for qq in range(len(qubits)):
                SPAM_error.add_qubit_connectivity(q,qq)

        sim = qristal.core.session()
        sim.init()
        sim.qn = len(qubits)
        sim.sn = 1000000
        sim.acc = 'aer'
        sim.noise = True
        sim.noise_model = SPAM_error

        workflow = benchmark.SPAMBenchmark(qubits, sim)
        metric = benchmark.ConfusionMatrix(workflow)
        results = metric.evaluate(force_new = True)

        for timestamp, confusion in results.items():
            for i in range(confusion.shape[0]):
                 assert np.sum(confusion, axis=1) == pytest.approx(1.0, 1e-2)

            sim.SPAM_confusion=confusion            
            corrected_results = metric.evaluate(True) 
            _, corrected_confusion = next(iter(corrected_results.items()))
            assert np.all(np.isclose(ideal,confusion, atol=1e-2))

def test_QuantumStateFidelity_checkSPAM():
    qubits = {0, 1}

    sim = qristal.core.session()
    sim.init()
    sim.acc="qpp"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.SPAMBenchmark(qubits, sim)
    qstworkflow = benchmark.QuantumStateTomography(workflow)

    metric = benchmark.QuantumStateFidelity(qstworkflow)
    results = metric.evaluate(True)
    for _, fidelities in results.items():
        for fidelity in fidelities:
            assert fidelity == pytest.approx(1.0, 1e-2)

def test_QuantumStateFidelity_checkRotationSweep():
    qubits = {0, 1}

    sim = qristal.core.session()
    sim.init()
    sim.acc="qpp"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.RotationSweep(['Y','Z'], -90, +90, 6, sim)
    qstworkflow = benchmark.QuantumStateTomography(workflow)

    metric = benchmark.QuantumStateFidelity(qstworkflow)
    results = metric.evaluate(True)
    for _, fidelities in results.items():
        for fidelity in fidelities:
            assert fidelity == pytest.approx(1.0, 1e-2)

def test_QuantumProcessFidelity_checkSPAM():
    qubits = {0}

    sim = qristal.core.session()
    sim.init()
    sim.acc="qsim"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.SPAMBenchmark(qubits, sim)
    qstworkflow = benchmark.QuantumStateTomography(workflow)
    qptworkflow = benchmark.QuantumProcessTomography(qstworkflow)

    metric = benchmark.QuantumProcessFidelity(qptworkflow)
    results = metric.evaluate(True)
    for _, fidelities in results.items():
        for fidelity in fidelities:
            assert fidelity == pytest.approx(1.0, 1e-2)

def test_QuantumProcessFidelity_checkRotationSweep():
    qubits = {0}

    sim = qristal.core.session()
    sim.init()
    sim.acc="qpp"
    sim.sn = 1000000
    sim.qn = len(qubits)

    workflow = benchmark.RotationSweep(['Y'], -90, +90, 2, sim)
    qstworkflow = benchmark.QuantumStateTomography(workflow)
    qptworkflow = benchmark.QuantumProcessTomography(qstworkflow)

    metric = benchmark.QuantumProcessFidelity(qptworkflow)
    results = metric.evaluate(True)
    for _, fidelities in results.items():
        for fidelity in fidelities:
            assert fidelity == pytest.approx(1.0, 1e-2)

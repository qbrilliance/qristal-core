# Copyright (c) Quantum Brilliance Pty Ltd
import os
import pytest
import qristal.core
import numpy as np
import qiskit
from qiskit import QuantumCircuit, QuantumRegister, ClassicalRegister
import ast

def qb_pauli_measurement_circuit(
        op: str,
        qubit: QuantumRegister,
        clbit: ClassicalRegister
) -> QuantumCircuit:
    """Return a qubit Pauli operator measurement circuit.

    Params:
        op: Pauli operator 'X', 'Y', 'Z'.
        qubit: qubit to be measured.
        clbit: clbit for measurement outcome.

    Returns:
        The measurement circuit for the given Pauli.
    """

    circ = QuantumCircuit([qubit, clbit])
    if op == 'X':
        circ.ry(-np.pi/2,qubit)
        circ.measure(qubit, clbit)
    if op == 'Y':
        circ.rx(np.pi/2,qubit)
        circ.measure(qubit, clbit)
    if op == 'Z':
        circ.measure(qubit, clbit)
    return circ

def qb_pauli_preparation_circuit(
        op: str,
        qubit: QuantumRegister
) -> QuantumCircuit:
    """Return a qubit Pauli eigenstate preparation circuit.

    This circuit assumes the qubit is initialized
    in the :math:`Zp` eigenstate :math:`[1, 0]`.

    Params:
        op: Pauli eigenstate 'Zp', 'Zm', 'Xp', 'Xm', 'Yp', or 'Ym'.
        qubit: qubit to be prepared.

    Returns:
        The preparation circuit for the given Pauli eigenstate.
    """

    circ = QuantumCircuit([qubit])
    if op == 'Xp':
        circ.ry(np.pi/2,qubit)
    if op == 'Xm':
        circ.ry(-np.pi/2,qubit)
    if op == 'Yp':
        circ.rx(-np.pi/2,qubit)
    if op == 'Ym':
        circ.ry(np.pi/2,qubit)
    if op == 'Zm':
        circ.ry(np.pi,qubit)
    return circ

def qb_pauli_preparation_matrix(label: str) -> np.array:
    r"""Return the matrix corresponding to a Pauli eigenstate preparation.

    Args:
        label: single-qubit Pauli eigenstate operator label.

    Returns:
        A Numpy array for the Pauli eigenstate. Allowed inputs
            and corresponding returned matrices are:

            'Xp' : :math:`\frac{1}{2}
            \left(\begin{array}{cc}1 & 1\\1 & 1\end{array}\right)`

            'Xm' : :math:`\frac{1}{2}
            \left(\begin{array}{cc}1 & -1\\-1 & 1\end{array}\right)`

            'Yp' : :math:`\frac{1}{2}
            \left(\begin{array}{cc}1 & -i\\i & 1\end{array}\right)`

            'Ym' : :math:`\frac{1}{2}
            \left(\begin{array}{cc}1 & i\\-i & 1\end{array}\right)`

            'Zp' : :math:`\left(\begin{array}
            {cc}1 & 0\\0 & 0\end{array}\right)`

            'Zm' : :math:`\left(\begin{array}
            {cc}0 & 0\\0 & 1\end{array}\right)`
    """
    res = np.array([])
    # Return matrix for allowed label
    if label == 'Xp':
        res = np.array([[0.5, 0.5], [0.5, 0.5]], dtype=complex)
    if label == 'Xm':
        res = np.array([[0.5, -0.5], [-0.5, 0.5]], dtype=complex)
    if label == 'Yp':
        res = np.array([[0.5, -0.5j], [0.5j, 0.5]], dtype=complex)
    if label == 'Ym':
        res = np.array([[0.5, 0.5j], [-0.5j, 0.5]], dtype=complex)
    if label == 'Zp':
        res = np.array([[1, 0], [0, 0]], dtype=complex)
    if label == 'Zm':
        res = np.array([[0, 0], [0, 1]], dtype=complex)
    return res

def qb_pauli_measurement_matrix(label: str, outcome: int) -> np.array:
    r"""Return the matrix corresponding to a Pauli measurement outcome.

    Args:
        label: single-qubit Pauli measurement operator label.
        outcome: measurement outcome.

    Returns:
        A Numpy array for measurement outcome operator.
            Allowed inputs and corresponding returned matrices are:

            'X', 0 : :math:`\frac{1}{2}
            \left(\begin{array}{cc}1 & 1\\1 & 1\end{array}\right)`

            'X', 1 : :math:`\frac{1}{2}
            \left(\begin{array}{cc}1 & -1\\-1 & 1\end{array}\right)`

            'Y', 0 : :math:`\frac{1}{2}
            \left(\begin{array}{cc}1 & -i\\i & 1\end{array}\right)`

            'Y', 1 : :math:`\frac{1}{2}
            \left(\begin{array}{cc}1 & i\\-i & 1\end{array}\right)`

            'Z', 0 : :math:`\left(\begin{array}
            {cc}1 & 0\\0 & 0\end{array}\right)`

            'Z', 1 : :math:`\left(\begin{array}
            {cc}0 & 0\\0 & 1\end{array}\right)`
    """
    res = np.array([])
    # Return matrix
    if label == 'X':
        if outcome in ['0', 0]:
            res = pauli_preparation_matrix('Xp')
        if outcome in ['1', 1]:
            res = pauli_preparation_matrix('Xm')
    if label == 'Y':
        if outcome in ['0', 0]:
            res = pauli_preparation_matrix('Yp')
        if outcome in ['1', 1]:
            res = pauli_preparation_matrix('Ym')
    if label == 'Z':
        if outcome in ['0', 0]:
            res = pauli_preparation_matrix('Zp')
        if outcome in ['1', 1]:
            res = pauli_preparation_matrix('Zm')
    return res

def get_session_from_qiskit_circuits(circuits, backend, no_shots):
    s = qristal.core.session()
    s.sn = no_shots
    s.acc = backend
    s.instring.clear()
    s.noise.clear()
    s.qn.clear()
    s.name_p.clear()
    for circuit in circuits:
        s.qn.append([circuit.num_qubits])
        s.noise.append([True])
        s.instring.append([get_qbqasm_string(circuit)]) # TODO: use Thien's QASM method
        s.name_p.append([circuit.name])
    return s

def get_qbqasm_string(qiskit_circuit):
    '''
    Input: Qiskit circuit object
    Output: qristal_circuit string
    '''
    import re
    return '__qpu__ void qristal_circuit(qreg q) {\n' + re.sub(r"\nqreg [A-Za-z]+[_\dA-za-z]*\[\d+\];", "", qiskit_circuit.qasm()) + '}'

def get_qiskit_experiment_result_data(result, num_qubits):
    '''
    Input: dict of counts for a Qristal experiment
    Output: qiskit.result.models.ExperimentResultData containing counts keyed with hex code
    '''
    hex_keyed_data = dict()
    shots = 0
    for num in result.keys():
        hex_keyed_data[hex(int(bin(num)[2:].zfill(num_qubits)[::-1],2))] = result[num]
        shots = shots + result[num]
    experiment_result_data = qiskit.result.models.ExperimentResultData(counts=hex_keyed_data)
    return experiment_result_data, shots

def get_experiment_result(session):
    '''
    Input: qristal.core.session
    Output: qiskit.result.result.Result
    '''
    experiment_result_data = []
    i = 0
    for row in session.results:
        j = 0
        for result in row:
            num_qubits = session.qn[i][j]
            name = session.name_p[i][j]
            data, shots=get_qiskit_experiment_result_data(result, num_qubits)
            experiment_result_data.append(qiskit.result.models.ExperimentResult(data=data, shots=shots, success=True, header=qiskit.qobj.common.QobjHeader(name = name, memory_slots=num_qubits)))
            j+=1
        i+=1
    experiment_result = qiskit.result.result.Result(backend_name=session.acc[0][0], backend_version=None, qobj_id=None, job_id=None, success=True,results=experiment_result_data)
    return experiment_result

def count_keys(num_qubits):
    return [bin(j)[2:].zfill(num_qubits) for j in range (2**num_qubits)]

def complete_meas_cal(qubit_list=None, qr=None, cr=None, circlabel=''):
    """
    Return a list of measurement calibration circuits for the full
    Hilbert space.

    Each of the 2**n circuits creates a basis state

    Args:
        qubit_list: A list of qubits to perform the measurement correction on,
        if None and qr is given then assumed to be performed over the entire
        qr. The calibration states will be labelled according to this ordering

        qr (QuantumRegister): A quantum register. If none one is created

        cr (ClassicalRegister): A classical register. If none one is created

        circlabel: A string to add to the front of circuit names for
        unique identification

    Returns:
        A list of QuantumCircuit objects containing the calibration circuits

        A list of calibration state labels

    Additional Information:
        The returned circuits are named circlabel+cal_XXX
        where XXX is the basis state,
        e.g., cal_1001

        Pass the results of these circuits to the CompleteMeasurementFitter
        constructor
    """

    if qubit_list is None and qr is None:
        raise QiskitError("Must give one of a qubit_list or a qr")

    # Create the registers if not already done
    if qr is None:
        qr = QuantumRegister(max(qubit_list)+1)

    if qubit_list is None:
        qubit_list = range(len(qr))

    cal_circuits = []
    nqubits = len(qubit_list)

    # create classical bit registers
    if cr is None:
        cr = ClassicalRegister(nqubits)

    # labels for 2**n qubit states
    state_labels = count_keys(nqubits)

    for basis_state in state_labels:
        qc_circuit = QuantumCircuit(qr, cr,
                                    name='%scal_%s' % (circlabel, basis_state))
        for qind, _ in enumerate(basis_state):
            if int(basis_state[nqubits-qind-1]):
                # the index labeling of the label is backwards with
                # the list
                qc_circuit.x(qr[qubit_list[qind]])

            # add measurements
            qc_circuit.measure(qr[qubit_list[qind]], cr[qind])

        cal_circuits.append(qc_circuit)

    return cal_circuits, state_labels

def qiskit_calibration_circuits(num_qubits, qubit_list, experiment_name = 'readout_', quantum_register='q', classical_register='q_c'):
    qr = qiskit.QuantumRegister(num_qubits, quantum_register)
    cr = qiskit.ClassicalRegister(num_qubits, classical_register)
    meas_calibs, state_labels = complete_meas_cal(qubit_list=qubit_list, qr=qr, cr=cr, circlabel=experiment_name)
    return meas_calibs, state_labels

def get_measurement_fitter(num_qubits, qubit_list, experiment_name, no_shots, backend, noise_model, quantum_register='q', classical_register='q_c'):
    meas_calibs, state_labels =  qiskit_calibration_circuits(num_qubits, qubit_list, experiment_name, quantum_register, classical_register)
    s = get_session_from_qiskit_circuits(meas_calibs, backend, no_shots)
    s.noise_model = noise_model
    s.seed = 888
    print('Noise model: ' + s.noise_model[0][0])
    s.run()
    qiskit_result = get_experiment_result(s)
    # Calculate the calibration matrix with the noise model
    from qiskit.utils.mitigation import CompleteMeasFitter
    meas_fitter = CompleteMeasFitter(qiskit_result, state_labels, qubit_list=qubit_list, circlabel=experiment_name)
    return meas_fitter

def test_CI_220915_1_noise_model_1_readout_error():
    num_qubits = 1
    experiment_name = 'readout_nm1'
    qubit_list = [0]
    n_shots = 700
    backend = 'aer'
    noise_model = 'qb-nm1'
    meas_fitter = get_measurement_fitter(num_qubits, qubit_list, experiment_name, n_shots, backend, noise_model)
    print("Calibration Matrix:\n" + str(meas_fitter.cal_matrix))
    print("Average Measurement Fidelity: %f" % meas_fitter.readout_fidelity())
    assert meas_fitter.cal_matrix[0][1] == pytest.approx(0.01, None, 0.01)
    assert meas_fitter.cal_matrix[1][0] == pytest.approx(0.01, None, 0.01)

def test_CI_220915_2_noise_model_2_readout_error():
    num_qubits = 1
    experiment_name = 'readout_nm2'
    qubit_list = [0]
    n_shots = 700
    backend = 'aer'
    noise_model = 'qb-nm2'
    meas_fitter = get_measurement_fitter(num_qubits, qubit_list, experiment_name, n_shots, backend, noise_model)
    print("Calibration Matrix:\n" + str(meas_fitter.cal_matrix))
    print("Average Measurement Fidelity: %f" % meas_fitter.readout_fidelity())
    assert meas_fitter.cal_matrix[0][1] == pytest.approx(0.01, None, 0.01)
    assert meas_fitter.cal_matrix[1][0] == pytest.approx(0.01, None, 0.01)



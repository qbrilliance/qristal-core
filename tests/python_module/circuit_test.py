# Test cases for quantum circuit library

import pytest
import numpy as np

#helper function returning the U3 gate unitary
def U3(theta, phi, lamb):
  c = np.cos(theta / 2.0)
  s = np.sin(theta / 2.0)
  pphi = complex(np.cos(phi), np.sin(phi))
  plambda = complex(np.cos(lamb), np.sin(lamb))
  result = np.array([
    [c, -1.0 * plambda * s],
    [pphi * s, pphi * plambda * c]
  ])
  return result

#helper function returning the CRX gate unitary
def CRX(angle):
  c = np.cos(angle / 2.0)
  s = np.sin(angle / 2.0)
  result = np.array([
    [1, 0, 0, 0],
    [0, c, 0, complex(0, -1.0*s)],
    [0, 0, 1, 0],
    [0, complex(0, -1.0*s), 0, c]
  ])
  return result  

#helper function returning the CRY gate unitary
def CRY(angle):
  c = np.cos(angle / 2.0)
  s = np.sin(angle / 2.0)
  result = np.array([
    [1, 0, 0, 0],
    [0, c, 0, -1.0*s],
    [0, 0, 1, 0],
    [0, s, 0, c]
  ])
  return result   

def test_crx():
  import qristal.core

  #(1) Generate random U3 and CRX angles and calculate ideal state vector 
  U3_0 = 4 * np.pi * np.random.rand(3) - 2 * np.pi
  U3_1 = 4 * np.pi * np.random.rand(3) - 2 * np.pi
  crx_angle = 4 * np.pi * np.random.rand() - 2 * np.pi
  ideal_state = np.zeros(4, dtype=complex)
  ideal_state[0] = 1.0
  ideal_state = np.kron(
    U3(U3_1[0], U3_1[1], U3_1[2]), U3(U3_0[0], U3_0[1], U3_0[2])
  ) @ ideal_state
  ideal_state = CRX(crx_angle) @ ideal_state

  #(2) Construct fixed and parameterized circuit
  circuit_1 = qristal.core.Circuit()
  circuit_1.u3(0, U3_0[0], U3_0[1], U3_0[2])
  circuit_1.u3(1, U3_1[0], U3_1[1], U3_1[2])
  circuit_1.crx(0, 1, crx_angle)
  circuit_1.measure_all()
  circuit_2 = qristal.core.Circuit()
  circuit_2.u3(0, U3_0[0], U3_0[1], U3_0[2])
  circuit_2.u3(1, U3_1[0], U3_1[1], U3_1[2])
  circuit_2.crx(0, 1, "theta")
  circuit_2.measure_all()

  # (3) Obtain simulated state vector from both circuits
  my_sim = qristal.core.session(False)
  my_sim.init()
  my_sim.acc = "qpp"
  my_sim.qn = 2
  my_sim.sn = 1
  my_sim.get_state_vec = True
  # non-parameterized circuit
  my_sim.ir_target = circuit_1
  my_sim.run()
  statevec_1 = my_sim.get_state_vec_raw
  # parameterized circuit 
  my_sim.ir_target = circuit_2
  my_sim.parameter_list = [crx_angle] 
  my_sim.run()
  statevec_2 = my_sim.get_state_vec_raw
  
  # (4) Compare both state vectors to the ideal (correct) one 
  for i in range(len(ideal_state)):
    assert ideal_state[i].real == pytest.approx(statevec_1[i].real, 1e-3)
    assert ideal_state[i].imag == pytest.approx(statevec_1[i].imag, 1e-3)
    assert ideal_state[i].real == pytest.approx(statevec_2[i].real, 1e-3)
    assert ideal_state[i].imag == pytest.approx(statevec_2[i].imag, 1e-3)

def test_cry():
  import qristal.core

  #(1) Generate random U3 and CRX angles and calculate ideal state vector 
  U3_0 = 4 * np.pi * np.random.rand(3) - 2 * np.pi
  U3_1 = 4 * np.pi * np.random.rand(3) - 2 * np.pi
  cry_angle = 4 * np.pi * np.random.rand() - 2 * np.pi
  ideal_state = np.zeros(4, dtype=complex)
  ideal_state[0] = 1.0
  ideal_state = np.kron(
    U3(U3_1[0], U3_1[1], U3_1[2]), U3(U3_0[0], U3_0[1], U3_0[2])
  ) @ ideal_state
  ideal_state = CRY(cry_angle) @ ideal_state

  #(2) Construct fixed and parameterized circuit
  circuit_1 = qristal.core.Circuit()
  circuit_1.u3(0, U3_0[0], U3_0[1], U3_0[2])
  circuit_1.u3(1, U3_1[0], U3_1[1], U3_1[2])
  circuit_1.cry(0, 1, cry_angle)
  circuit_1.measure_all()
  circuit_2 = qristal.core.Circuit()
  circuit_2.u3(0, U3_0[0], U3_0[1], U3_0[2])
  circuit_2.u3(1, U3_1[0], U3_1[1], U3_1[2])
  circuit_2.cry(0, 1, "theta")
  circuit_2.measure_all()

  # (3) Obtain simulated state vector from both circuits
  my_sim = qristal.core.session(False)
  my_sim.init()
  my_sim.acc = "qpp"
  my_sim.qn = 2
  my_sim.sn = 1
  my_sim.get_state_vec = True
  # non-parameterized circuit
  my_sim.ir_target = circuit_1
  my_sim.run()
  statevec_1 = my_sim.get_state_vec_raw
  # parameterized circuit 
  my_sim.ir_target = circuit_2
  my_sim.parameter_list = [cry_angle] 
  my_sim.run()
  statevec_2 = my_sim.get_state_vec_raw
  
  # (4) Compare both state vectors to the ideal (correct) one 
  for i in range(len(ideal_state)):
    assert ideal_state[i].real == pytest.approx(statevec_1[i].real, 1e-3)
    assert ideal_state[i].imag == pytest.approx(statevec_1[i].imag, 1e-3)
    assert ideal_state[i].real == pytest.approx(statevec_2[i].real, 1e-3)
    assert ideal_state[i].imag == pytest.approx(statevec_2[i].imag, 1e-3)

def test_qpe():
    print(" Testing Quantum Phase Estimation ")
    import qristal.core

    s = qristal.core.session()
    s.init()

    # Oracle
    oracle = qristal.core.Circuit()
    oracle.u1(0, -1.96349540849)

    # 4-bit precision
    nb_bits_precision = 4

    circ = qristal.core.Circuit()
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
    # Expect that the result is [1,1,0,1] => estimate the phase value of -1.96349540849
    good_count = s.results[0][0][[1,1,0,1]]
    # Very high count
    assert (good_count > 1000)

def test_amcu():
    print("Testing Multi-Controlled-U With Ancilla")
    import numpy as np
    import qristal.core
    s = qristal.core.session()
    s.init()

    # In this example, we perform an mcx gate on all possible bitstrings
    num_qubits = 5
    control_bits = range(num_qubits-1)
    target_bit = num_qubits-1
    ancilla_bits = range(num_qubits, num_qubits + num_qubits - 2)

    for i in range(2**num_qubits):
        circ = qristal.core.Circuit()

        # Prepare the input state
        bitstring = [int(bit) for bit in bin(i)[2:].zfill(num_qubits)]
        for j in range(num_qubits):
            if bitstring[j] == 1:
                circ.x(j)

        # Add the amcu gate
        U = qristal.core.Circuit()
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
        res = s.results[0][0]

        if bitstring == [1,1,1,1,1]:
            assert([1,1,1,1,0] in res)
        elif bitstring == [1,1,1,1,0]:
            assert([1,1,1,1,1] in res)
        else:
            assert(bitstring in res)

def test_equality_checker():
    print(" Testing Equality Checker Circuit")
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()

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
            circ = qristal.core.Circuit()
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
            res = s.results[0][0]
            # Check results
            assert(len(res) == 1)
            if i == j:
                assert([1] in res)
            else:
                assert([0] in res)

def test_canonical_ae():
    print(" Testing Canonical Quantum Amplitude Estimation ")
    import qristal.core
    import numpy as np
    from qristal.core import run_canonical_ae_with_oracle
    import json
    s = qristal.core.session()
    s.init()
    p = 0.24
    theta_p = 2 * np.arcsin(np.sqrt(p))

    # State prep circuit: (preparing the state that we want to estimate the amplitude)
    state_prep = qristal.core.Circuit()
    state_prep.ry(8, theta_p)

    # In this case, we don't construct the Grover operator by ourselves,
    # instead, just provide the oracle to detect the marked state (|1>)
    oracle = qristal.core.Circuit()
    oracle.z(8)
    bits_precision = 8

    # Execute:
    result = run_canonical_ae_with_oracle(state_prep, oracle, bits_precision,1,1)
    res = json.loads(result)

    amplitude_estimate = float(res["AcceleratorBuffer"]["Information"]["amplitude-estimation"])
    assert (abs(amplitude_estimate - np.sqrt(p)) < 0.1)

def test_controlled_swap():
    print("Testing controlled swap")
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()

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
    circ = qristal.core.Circuit()
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
    res = s.results[0][0]
    assert(len(res) == 1)
    output_measurement = list(next(iter(res)))

    expected_string = ""
    num_b = 0
    for i in range(len(input_string)):
        if input_string[i] == "b":
            num_b += 1
    for i in range(len(input_string) - num_b):
        expected_string += "a"
    for i in range(num_b):
        expected_string += "b"

    expected_measurement = []
    for i in range(len(expected_string)):
        if expected_string[i] == "a":
            expected_measurement.append(0)
        else:
            expected_measurement.append(1)

    assert(output_measurement == expected_measurement)

def test_controlled_addition():
    print("Testing controlled ripple carry adder")
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()

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

    circ1 = qristal.core.Circuit()

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
    res = s.results[0][0]
    assert(res[[0,0,0]] == 1024)

    ###
    # Test 1: flag on
    ###

    circ2 = qristal.core.Circuit()

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
    res = s.results[0][0]
    assert(res[[1,0,0]] == 1024)

def test_generalised_mcx():
    print("Testing generalised MCX")
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()
    s.sn = 1024

    # In this test we use generalised mcx to
    # perform mcx on all possible 3-qubit bit strings (|000>,...,|111>)
    # with all combinations of control qubit conditions ((on,on),...,(off,off))

    # Expected outcomes:

    # - When the control qubits are required to be (on,on) then we expect
    # |011> -> |111>, |111> -> |011>, and all other bit strings remain unchanged

    # - When the control qubits are required to be (on,off) then we expect
    # |001> -> |101>, |101> -> |001>, and all other bit strings remain unchanged

    # - When the control qubits are required to be (off,on) then we expect
    # |010> -> |110>, |110> -> |010>, and all other bit strings remain unchanged

    # - When the control qubits are required to be (off,off) then we expect
    # |000> -> |100>, |100> -> |000>, and all other bit strings remain unchanged

    control_qubits = [0, 1]
    target_qubit = 2

    conditions = [[0, 0], [0, 1], [1, 0], [1, 1]]
    input_bitstrings = ["000", "001", "010", "011", "100", "101", "110", "111"]
    for condition in conditions:
        for input_bitstring in input_bitstrings:
            circ = qristal.core.Circuit()

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
            res = s.results[0][0]
            expected_output = [int(x) for x in input_bitstring[0:2]]
            if int(input_bitstring[0]) == condition[0] and int(input_bitstring[1]) == condition[1]:
                if input_bitstring[2] == "0":
                    expected_output.append(1)
                else:
                    expected_output.append(0)
            else:
                expected_output.append(int(input_bitstring[2]))
            assert(len(res) == 1)
            assert(list(next(iter(res))) == expected_output)
            assert(res[expected_output] == 1024)

def test_MLQAE():
    print("Testing Maximum Likelihood Qauntum Amplitude Estimation")
    # Test the example here:
    # https://qiskit.org/documentation/finance/tutorials/00_amplitude_estimation.html
    # i.e., estimate the amplitude of the state:
    # sqrt(1-p)|0> + sqrt(p)|1>
    import numpy as np
    import qristal.core
    from qristal.core import run_MLQAE
    import ast

    s = qristal.core.session()
    s.init()
    p = 0.24
    theta_p = 2 * np.arcsin(np.sqrt(p))

    # State prep circuit: (preparing the state that we want to estimate the amplitude)
    state_prep = qristal.core.Circuit()
    state_prep.ry(0, theta_p)

    # In this case, we don't construct the Grover operator by ourselves,
    # instead, just provide the oracle to detect the marked state (|1>)
    oracle = qristal.core.Circuit()
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
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()
    oracle = qristal.core.Circuit()
    oracle.z(0)

    # For demonstration purposes, we start with a very low population of |1> state:
    # using Ry(epsilon) as the state preparation:
    # Ry(epsilon) |0> ~ epsilon |1> + sqrt(1 - epsilon^2) |1>
    # i.e., the initial population of the marked state is small (~ epsilon)
    epsilon = 0.05
    state_prep = qristal.core.Circuit()
    state_prep.ry(0, epsilon)

    good_state_ampls = []
    # Testing a varying number of iteration
    for i in range(1, 40, 1):
        # Construct full amplitude amplification circuit:
        full_circuit = qristal.core.Circuit()
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
        res = s.results[0][0]
        # Calculate the probability of the marked state
        good_count = 0
        # Must check if "1" is present
        # (with a small number of amplification rounds,
        # the amplitude of |1> may still be very small)
        if [1] in res:
            good_count = res[[1]]
        good_state_ampls.append(good_count/1024)

    # Check the result
    iterations = range(1, 40, 1)
    # Get analytical result for comparison
    probs_theory = [np.sin((2*mm+1)*epsilon/2)**2 for mm in iterations]
    assert(np.allclose(good_state_ampls, probs_theory, atol=0.1))

def test_ripple_adder():
    print(" Testing Ripple Carry Adder Circuit ")
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()
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
            circ = qristal.core.Circuit()
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
            res = s.results[0][0]
            # Expected sum result
            expected = i + j
            expected_bit_string = [int(x) for x in f'{expected:0{3}b}'[::-1]]
            assert(len(res) == 1)
            assert(expected_bit_string == list(next(iter(res))))
            assert(res[expected_bit_string] == 1024)

def test_ripple_adder_superposition():
    print(" Testing Ripple Carry Adder Circuit for superposition ")
    import qristal.core
    import numpy as np
    s = qristal.core.session()
    s.init()
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
    circ = qristal.core.Circuit()
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
    # Expected sum result to be a superposition of 4 values:
    assert(len(s.results[0][0]) == 4)

def test_comparator():
    print("Testing quantum bit string comparator")
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()

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
            circ = qristal.core.Circuit()

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

            res = s.results[0][0]
            if j > i:
                expected_bit_string = [1]
            if j <= i:
                expected_bit_string = [0]
            assert(len(res) == 1)
            assert(expected_bit_string == list(next(iter(res))))
            assert(res[expected_bit_string] == 1024)

def test_efficient_encoding():
    print("Testing Efficient Encoding")
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()

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
    circ = qristal.core.Circuit()

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

    # Get results
    res = s.results[0][0]
    allowed_outputs = [[0,0,0,0,0,0], [1,1,1,1,1,1]]
    for measurement in res:
        assert(list(measurement) in allowed_outputs)

def test_compare_beam_oracle():
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.acc = "aer"
    s.sn = 1024
    s.init()

    ##########################################################################
    #Test 1: SA = |0111>, FA = |01>, FB = |01>

    #Inputs
    q0 = 0
    q1 = 1
    q2 = 2
    FA = [3,4]
    FB = [5,6]
    SA = [7,8,9,10]

    circ = qristal.core.Circuit()

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

    #Run
    s.instring = circ.openqasm()
    s.run()
    res = s.results[0][0]
    assert(res[[0,1,0]] == 1024)

    ##########################################################################
    #Test 2: SA = |1111>, FA = |01>, FB = |01>

    #Inputs
    q0 = 0
    q1 = 1
    q2 = 2
    FA = [3,4]
    FB = [5,6]
    SA = [7,8,9,10]

    circ = qristal.core.Circuit()

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
    res = s.results[0][0]
    assert(res[[1,1,1]] == 1024)

def test_multiplication():
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.acc = "qsim"
    s.sn = 1024
    s.init()
    s.qn = 9

    ##########################################################################
    # Test 1: qubit_a = 1 = |10>, qubit_b = 3 = |11>,
    # qubit_result = 1 x 3 = 3 = |1100>
    # Inputs
    qubits_a = [0,1]
    qubits_b = [2,3]
    qubits_result = [4,5,6,7]
    qubit_ancilla = 8

    circ = qristal.core.Circuit()

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

    # Run circuit
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.run()
    res = s.results[0][0]
    assert(res[[1,0,1,1,1,1,0,0]] == 1024)

def test_controlled_multiplication():
    import qristal.core
    import numpy as np
    s = qristal.core.session()
    s.acc = "qsim"
    s.sn = 1024
    s.init()
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

    circ = qristal.core.Circuit()

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

    # Run circuit
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.run()
    res = s.results[0][0]
    assert(res[[1,0,1,1,1,1,0,0]] == 1024)

def test_inverse_circuit():
    import qristal.core

    s = qristal.core.session()
    s.init()
    s.nooptimise = True
    s.noplacement = True
    s.notiming = True
    s.output_oqm_enabled = False
    s.acc = "qpp"
    s.sn = 1024

    qubits = [0,1,2,3]
    circ = qristal.core.Circuit()

    qft = qristal.core.Circuit()
    qft.qft(qubits)

    circ.qft(qubits)
    circ.inverse_circuit(qft)
    for i in qubits:
        circ.measure(i)

    s.ir_target = circ
    s.run()
    res = s.results[0][0]
    assert(res[[0,0,0,0]] == 1024)

def test_subtraction():
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()
    s.qn = 7

    ###
    # Testing subtraction
    # In this example we perform a 3-qubit subtraction
    ###

    # Set up input registers
    qubits_larger = [0,1,2]
    qubits_smaller = [3,4,5]
    qubit_ancilla = 6

    circ = qristal.core.Circuit()

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
    res = s.results[0][0]

    expected_result = i-j
    expected_result_bin = bin(expected_result)[2:].zfill(3)
    expected_output = [int(x) for x in (expected_result_bin + bin_j)]
    assert(res[expected_output] == 1024)

def test_controlled_subtraction():
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()
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

    circ = qristal.core.Circuit()

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
    res = s.results[0][0]

    expected_result = i-j
    expected_result_bin = bin(expected_result)[2:].zfill(3)
    expected_output = [int(x) for x in (expected_result_bin + bin_j)]
    assert(res[expected_output] == 1024)

def test_proper_fraction_division():
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()
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

    circ = qristal.core.Circuit()

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
    s.acc = "qsim"
    s.run()
    res = s.results[0][0]
    assert(len(res) == 1)

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

    assert(res[[int(x) for x in expected_output]] == 1024)

def test_controlled_proper_fraction_division():
    import qristal.core
    import numpy as np

    s = qristal.core.session()
    s.init()
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

    circ = qristal.core.Circuit()

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
    res = s.results[0][0]
    assert(len(res) == 1)

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

    assert(res[[int(x) for x in expected_output]] == 1024)

def test_compare_gt():
    import qristal.core

    s = qristal.core.session()
    s.init()

    i = 3
    j = 2

    qubits_a = [0,1]
    qubits_b = [2,3]
    qubit_flag = 4
    qubit_ancilla = 5

    circ = qristal.core.Circuit()

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
    res = s.results[0][0]

    expected_output = a_bin + b_bin
    if i > j:
        expected_output += "1"
    else:
        expected_output += "0"
    expected_output += "0"

    assert(res[[int(x) for x in expected_output]] == 1024)


import qbos as qb
import numpy as np
import ast

# Comparator: compare two bitstrings |BestScore> and |TrialScore>
# within the quantum register |TrialScore>|flag>|BestScore>|ancilla>
# If TrialScore > BestScore then |flag> will be returned as |1>
# Otherwise |flag> returned as |0>

for i in range(0,4):
    for j in range(0,4):
        # number of qubits encoding scores
        num_scoring_qubits = 2

        # BestScore
        BestScore = i
        BestScore_bin = bin(BestScore)[2:].zfill(num_scoring_qubits)
        
        # create circuit
        circ = qb.Circuit()

        # State prep
        TestScore = j
        TestScore_bin = bin(TestScore)[2:].zfill(num_scoring_qubits)
        # TestScore_bin_LSB = [TestScore_bin[-i] for i in range(1,num_scoring_qubits+1)]
        for k in range(0,num_scoring_qubits):
            #if TestScore_bin_LSB[k] == "1":
            if TestScore_bin[k] == "1":
                circ.x(k)
        Input_State = TestScore_bin + "0" + BestScore_bin + "00000"

        # Add comp
        circ.comparator_as_oracle(BestScore, num_scoring_qubits, is_LSB=False)

        # Measure flag qubit
        circ.measure_all()

        # Check the circuit:
        #print("OpenQASM:\n", circ.openqasm())

        # Run:
        # tqb.ir_target = circ
        # tqb.nooptimise = True
        # tqb.noplacement = True
        # tqb.notiming = True
        # tqb.output_oqm_enabled = False
        # tqb.acc = "qpp"
        # tqb.run()

        # Get results
        # result = tqb.out_raw[0][0]
        result = circ.execute()
        res = ast.literal_eval(result)["AcceleratorBuffer"]["Measurements"]
        if j > i:
            expected_bit_string = TestScore_bin  + BestScore_bin + "000000"
        if j <= i:
            expected_bit_string = TestScore_bin + BestScore_bin + "000000"
        assert(expected_bit_string in list(res.keys()))
        count = int(res[expected_bit_string])
        assert(count == 1024)

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
        tqb = qb.core()
        tqb.qb12()
        tqb.ir_target = circ
        tqb.nooptimise = True
        tqb.noplacement = True
        tqb.notiming = True
        tqb.output_oqm_enabled = False
        tqb.acc = "qpp"
        tqb.run()

        # Get results
        result = tqb.out_raw[0][0]
        res = ast.literal_eval(result)
        if j > i:
            expected_bit_string = "1"
        if j <= i:
            expected_bit_string = "0" 
        assert(expected_bit_string in list(res.keys()))
        count = int(res[expected_bit_string])
        assert(count == 1024)
import qb.core
import numpy as np
s = qb.core.session()
s.qb12()

# Quantum Phase Estimation: Oracle(|State>) = exp(i*Phase)*|State>
# and we need to estimate that Phase value.
# The Oracle in this case is a general U1 rotation
# i.e. U1(theta)|1> = exp(i*theta)|1>
# Test value: -5pi/8

# 4-bit precision
nb_bits_precision = 4
trial_qubits = [2]
evaluation_qubits = [0,1,3,4]

# Oracle
oracle = qb.core.Circuit()
#oracle.u1(0, -1.96349540849)
oracle.u1(trial_qubits[0], -1.96349540849)

circ = qb.core.Circuit()

# State prep: eigen state of the oracle |1>
circ.x(trial_qubits[0])
#circ.x(0)

# Add QPE
circ.qpe(oracle, nb_bits_precision, trial_qubits, evaluation_qubits)

# Measure evaluation qubits
for i in range(nb_bits_precision):
    circ.measure(evaluation_qubits[i])
    #circ.measure(1+i)

# Check the circuit:
print("OpenQASM:\n", circ.openqasm())

# Run:
s.ir_target = circ
s.nooptimise = True
s.noplacement = True
s.notiming = True
s.output_oqm_enabled = False
s.acc = "qpp"
s.run()
result = s.out_raw[0][0]
print("Result:\n", result)


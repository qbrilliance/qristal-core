# Import the core of the QB SDK
import qb.core

# A circuit on two qubits that is extremely long!!!
circ = qb.core.Circuit()
# Repeat a random circuits...
# there are 5 x 3 = 15 CNOT gates
for i in range(5):
    circ.h(0)
    circ.rx(1, 0.123)
    circ.cnot(1, 0)
    circ.x(1)
    circ.ry(1, 2.345)
    circ.ry(0, 0.456)
    circ.cnot(0, 1)
    circ.rz(1, 0.345)
    circ.rx(0, 0.876)
    circ.cnot(0, 1)
    circ.h(0)
    circ.h(1)
circ.measure_all()

print("Original circuit:")
circ.print()

# Create a quantum computing session using the QB SDK
my_sim = qb.core.session(True)

# Set up meaningful defaults for session parameters
my_sim.init()

# Choose a simulator backend
my_sim.acc = "qpp"

# Choose how many qubits to simulate
my_sim.qn = 2

# Choose how many 'shots' to run through the circuit
my_sim.sn = 100

# Set the quantum program to run
my_sim.ir_target = circ

# Set the circuit optimization pipeline:
# I want to apply two-qubit squash then remove any redundant gates!
my_sim.circuit_optimization = [qb.core.two_qubit_squash(), qb.core.redundancy_removal()]
# Make sure we run those passes..
my_sim.nooptimise = False
# Run the circuit 100 times and count up the results in each of the classical registers
print("About to run quantum program...")
my_sim.run()
print("Ran successfully!")

# Print the cumulative results in each of the classical registers
print("Results:\n", my_sim.out_raw_json[0][0])


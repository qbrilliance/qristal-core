import sys
sys.path.append("lib")
print("Executing Python demo...")

# Import the core of the QB SDK
import qb.core

# Create a quantum computing session using the QB SDK
my_sim = qb.core.session()

# Set up meaningful defaults for session parameters
my_sim.qb12()

# Choose a simulator backend
my_sim.acc = "qb-lambda"

# Choose how many qubits to simulate
my_sim.qn = 18

# Choose how many 'shots' to run through the circuit
my_sim.sn = 100

# Instead of writing a kernel, auto-generate a random one, 20 gates deep
my_sim.random = 20

# Run the circuit 100 times and count up the results in each of the classical registers
print("About to run quantum program...")
my_sim.run()
print("Ran successfully!")

# Print the cumulative results in each of the classical registers
print("Results:\n", my_sim.out_raw[0][0])


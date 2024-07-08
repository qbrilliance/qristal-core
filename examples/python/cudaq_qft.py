# Using CUDA Quantum simulator backend to run a Qristal circuit
print("Executing Python demo...")

# Import the core of the QB SDK
import qb.core

# Create a quantum computing session using the QB SDK
my_sim = qb.core.session()

# Set up meaningful defaults for session parameters
my_sim.init()

# Choose a simulator backend from CUDA Quantum,
# e.g., custatevec_fp32 (single-precision)
my_sim.acc = "cudaq:custatevec_fp32"

# Choose how many qubits to simulate
num_qubits = 10
my_sim.qn = num_qubits

# Choose how many 'shots' to run through the circuit
my_sim.sn = 1024

# Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit')
# Use Qristal circuit builder to construct a QFT circuit.
circ = qb.core.Circuit()
circ.qft(range(num_qubits))
circ.measure_all()
# Set the circuit
my_sim.ir_target = circ
# Run the circuit 'shots' times and count up the results in each of the classical registers
print("About to run quantum program...")
my_sim.run()
print("Ran successfully!")

# Print the cumulative results in each of the classical registers
print("Results:\n", my_sim.out_raw_json[0][0])


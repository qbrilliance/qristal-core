# Using CUDA Quantum simulator backend to run a Qristal circuit
print("Executing Python demo...")

# Import the core of Qristal
import qristal.core

# Create a quantum computing session using Qristal
my_sim = qristal.core.session()

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
circ = qristal.core.Circuit()
circ.qft(range(num_qubits))
circ.measure_all()
# Set the circuit
my_sim.irtarget = circ
# Run the circuit 'shots' times and count up the results in each of the classical registers
print("About to run quantum program...")
my_sim.run()
print("Ran successfully!")

# Print the cumulative results in each of the classical registers
print("Results:")
print(my_sim.results)


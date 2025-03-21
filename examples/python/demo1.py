print("Executing Python demo...")

# Import the core of Qristal
import qristal.core

# Create a quantum computing session using Qristal
my_sim = qristal.core.session()

# Set up meaningful defaults for session parameters
my_sim.init()

# Choose a simulator backend
my_sim.acc = "qpp"

# Choose how many qubits to simulate
my_sim.qn = 2

# Choose how many 'shots' to run through the circuit
my_sim.sn = 100

# Define the quantum program to run (aka 'quantum kernel' aka 'quantum circuit')
my_sim.instring = '''
__qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
{
  OPENQASM 2.0;
  include "qelib1.inc";
  creg c[2];
  h q[0];
  x q[1];
  measure q[0] -> c[0];
  measure q[1] -> c[1];
}
'''

# Run the circuit 100 times and count up the results in each of the classical registers
print("About to run quantum program...")
my_sim.run()
print("Ran successfully!")

# Print the cumulative results in each of the classical registers
print("Results:")
print(my_sim.results[0][0])


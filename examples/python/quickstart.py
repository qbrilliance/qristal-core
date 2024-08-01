# Quickstart
# To learn how to use the SDK, let's run a simple Python example to create a Bell state.

# Import the Qristal core:
import qristal.core

# Create a quantum computing session using Qristal:
my_sim = qristal.core.session()

# Choose some default session parameters:
my_sim.init()

# Set the number of shots to run through the circuit:
my_sim.sn = 1024

# Set the number of qubits:
my_sim.qn = 2

# Choose the simulator backend:
my_sim.acc = "qpp"

# Create the $\ket{\Phi^+}$ component of the Bell state using Hadamard and CNOT gates:
my_sim.instring = '''
__qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
{
  OPENQASM 2.0;
  include "qelib1.inc";
  creg c[2];
  h q[0];
  cx q[0], q[1];
  measure q[0] -> c[0];
  measure q[1] -> c[1];
}
'''

# Run the circuit:
my_sim.run()

# Print the cumulative results in each of the classical registers:
print("Bell state |Phi+>: ")
print(my_sim.results[0][0])


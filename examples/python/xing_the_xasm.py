# Import the core of Qristal
import qristal.core

# Create a quantum computing session using Qristal
my_sim = qristal.core.session()

# Set up meaningful defaults for session parameters
my_sim.init()

# Tell Qristal not to actually execute the circuit
my_sim.execute_circuit = False

# Choose a QB hardware backend
my_sim.acc = "example_hardware_device"

# indicate how many qubits in the input circuit
my_sim.qn = 2

# Give Qristal your circuit in OpenQASM
my_sim.instring = '''
__qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
{
  OPENQASM 2.0;
  include "qelib1.inc";
  creg c[2];
  h q[0];
  cx q[1],q[0];
  measure q[0] -> c[0];
  measure q[1] -> c[1];
}
'''

# Run
my_sim.run()

# Get it back in XASM
import json
print("Circuit in XASM: ", json.loads(my_sim.out_qbjson[0][0])["circuit"])

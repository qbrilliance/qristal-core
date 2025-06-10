from yaml import safe_load, dump

# Import the core of Qristal
import qristal.core

# In this example, we demonstrate how noise-aware placement will be
# applied on circuits to be submitted to hardware backend (e.g., Rigetti) on AWS Braket.
circ = qristal.core.Circuit()
circ.h(0)
circ.cnot(0, 1)
circ.measure_all()
print("Original circuit:")
circ.print()

# Create a session object
s = qristal.core.session()
s.sn = 100

# Set the circuit
s.irtarget = circ
s.qn = circ.num_qubits()
# Use qristal noise-aware placement
s.noplacement = False
s.noise = True
s.placement = "noise-aware"

# Use AWS Rigetti backend (hardware)
# !Important!: make sure AWS credentials have been set on the computer,
# e.g., using AWS CLI tool to cache the API key.
s.acc = "aws-braket"
stream = open(s.remote_backend_database_path, 'r')
db = safe_load(stream)["aws-braket"]
db["device"] = "Rigetti"
stream = open(s.remote_backend_database_path + ".temp", 'w')
dump({'aws-braket': db}, stream)
s.remote_backend_database_path = s.remote_backend_database_path + ".temp"

# Don't submit the circuit to AWS for execution (via the `execute_circuit` config)
# Note: there is no charge when querying backend information during circuit placement run.
# Actual circuit execution, on the other hand, will incur a cost.
s.execute_circuit = False
s.run()
print("Placed circuit (for Rigetti device):")
circ.print()

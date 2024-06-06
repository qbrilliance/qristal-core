# Import the core of the QB SDK
import qb.core

# In this example, we demonstrate how noise-aware placement will be 
# applied on circuits to be submitted to hardware backend (e.g., Rigetti) on AWS Braket.
circ = qb.core.Circuit()
circ.h(0)
circ.cnot(0, 1)
circ.measure_all()
print("Original circuit:")
circ.print()

# Create a session object
s = qb.core.session()
s.qb12()

# Set the circuit
s.ir_target = circ
# Use qb "noise-aware" placement 
s.noplacement = False
s.placement = "noise-aware"

# Use AWS Rigetti backend (hardware)
# !Important!: make sure AWS credentials have been set on the computer,
# e.g., using AWS CLI tool to cache the API key.
s.acc = "aws-braket"
s.aws_device = "Rigetti"

# Don't submit the circuit to AWS for execution (via the `nosim` config)
# Note: there is no charge when querying backend information during circuit placement run.
# Actual circuit execution, on the other hand, will incur a cost.
s.nosim = True
s.run()
print("Placed circuit (for Rigetti device):")
circ.print() 

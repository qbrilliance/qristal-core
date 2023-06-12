# Import the core of the QB SDK
import qb.core

# In this example, we demonstrate simple circuit placement based on device topology.
nb_qubits = 4

ghz = qb.core.Circuit()
ghz.h(0)
for i in range(nb_qubits - 1):
    ghz.cnot(i, i + 1)
print("Original circuit:")
ghz.print()

# Create a ring/square topology
#  0 - 1
#  |   |
#  2 - 3
nb_qubits = 4
connectivity = [(0, 1), (0, 2), (1, 3), (2, 3)]

# Construct a swap-based placement pass for this topology
placement = qb.core.swap_placement_pass(connectivity)
# Apply the pass over the input circuit
placement.apply(ghz)
print("After placement:")
ghz.print()
# Could you spot a SWAP gate?
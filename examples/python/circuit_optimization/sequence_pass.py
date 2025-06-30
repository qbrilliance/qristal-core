# Import the core of Qristal
import qristal.core

# A circuit demonstrating that the order of passes affects the outcome.
circ = qristal.core.Circuit()

circ.h(0)
circ.h(0)
circ.x(0)
circ.cnot(0,1)
circ.x(1)

circ_copy = circ.copy()

print("Original circuit:")
circ.print()

# Apply two passes in different orders
# Create a circuit optimisation sequence and apply it to the circuit
pass_list = qristal.core.VectorString(["redundancy-removal", "simplify-initial"])
sequence_pass = qristal.core.sequence_pass(pass_list)
sequence_pass.apply(circ)

print("Optimized circuit [redundancy removal, simplify initial]:")
circ.print()

# Create another circuit optimisation sequence in the opposite order and apply it to the original circuit
pass_list = qristal.core.VectorString(["simplify-initial","redundancy-removal"])
sequence_pass = qristal.core.sequence_pass(pass_list)
sequence_pass.apply(circ_copy)

print("Optimized circuit [simplify initial, redundancy removal]:")
circ_copy.print()

# Import the core of Qristal
import qristal.core

# Circuit
circ = qristal.core.Circuit()
circ.h(0)
circ.s(0)
circ.h(0)
circ.cnot(0, 1)
circ.s(1)
circ.sdg(1)
circ.h(1)
circ.h(1)
circ.cnot(0, 1)

print("Original circuit:")
circ.print()

# Apply Clifford simplification:
# This pass cancels or combines redundant Clifford gates
opt_pass = qristal.core.optimise_cliffords()
opt_pass.apply(circ)

# Print the result
print("\nOptimised circuit:")
circ.print()

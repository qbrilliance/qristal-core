# Import the core of Qristal
import qristal.core

# Circuit
circ = qristal.core.Circuit()
circ.sdg(0)
circ.h(0)
circ.t(0)

print("Original circuit:")
circ.print()

# Rebase to Clifford gates
# decomposing gates into sequences of Clifford operations
opt_pass = qristal.core.rebase_to_clifford()
opt_pass.apply(circ)

# Print the result
print("\nRebased circuit:")
circ.print()

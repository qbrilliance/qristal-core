# Import the core of Qristal
import qristal.core

# Circuit
circ = qristal.core.Circuit()
circ.h(0)
circ.s(0)
circ.t(0)

print("Original circuit:")
circ.print()

# Rebase the circuit into Rz and Rx rotations
# This pass rewrites all gates into only Rz and Rx gates
opt_pass = qristal.core.rebase_to_rzrx()
opt_pass.apply(circ)

# Print the result
print("\nRebased circuit:")
circ.print()

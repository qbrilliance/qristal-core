# Import the core of Qristal
import qristal.core

circ = qristal.core.Circuit()
circ.swap(0,1)
circ.swap(2,3)

print("Original circuit commands:")
circ.print()

# Apply the decompose_swap pass:
# This replaces each SWAP with 3 CNOTs
opt_pass = qristal.core.decompose_swap()
# Apply decompose swap pass
opt_pass.apply(circ)

# Print the result
print("\nCircuit commands after decompose swap pass:")
circ.print()

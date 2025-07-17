# Import the core of Qristal
import qristal.core

# Circuit
circ = qristal.core.Circuit()
circ.rz(0, 0.5)
circ.cnot(0,1)
circ.t(1)
circ.cnot(0,1)
circ.z(0)

print("Original circuit:")
circ.print()

# Apply the commute-through-multis optimization:
# This pass moves single-qubit gates (like Rz, Z, T) through multi-qubit 
# gates (like CNOT) when allowed by quantum gate commutation rules
opt_pass = qristal.core.commute_through_multis()
# Apply commute throuh multis pass
opt_pass.apply(circ)

# Print the result
print("\nFinal circuit:")
circ.print()

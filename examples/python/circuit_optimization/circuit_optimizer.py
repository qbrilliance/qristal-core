# Import the core of Qristal
import qristal.core

# A circuit with a optimizable pattern
circ = qristal.core.Circuit()
# This is a CZ gate!!!
circ.h(1)
circ.cnot(0, 1)
circ.h(1)

print("Original circuit:")
circ.print()

# Get the circuit_optimizer pass
opt_pass = qristal.core.circuit_optimizer()
# Apply it on the input circuit
opt_pass.apply(circ)

# Print it out
print("Optimized circuit:")
circ.print()

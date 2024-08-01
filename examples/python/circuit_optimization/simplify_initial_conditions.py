# Import the core of Qristal
import qristal.core

# A circuit with a optimizable pattern
circ = qristal.core.Circuit()
# A rather costly circuit to create |11> state (CNOT is expensive!!!)
circ.x(0)
circ.cnot(0, 1)

print("Original circuit:")
circ.print()

# Get the simplify initial pass
opt_pass = qristal.core.simplify_initial()
# Apply it on the input circuit
opt_pass.apply(circ)

# Print it out
print("Optimized circuit (based on initial conditions):")
circ.print()
# Two X gates should achieve the same goal (final state == |11>)!!!

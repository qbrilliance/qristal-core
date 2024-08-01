# Import the core of Qristal
import qristal.core

# A circuit with lots of redundancies
circ = qristal.core.Circuit()
circ.h(0)
circ.x(1)
circ.h(0)
circ.x(1)
circ.rx(2, 0.123)
circ.rx(2, 0.456)
print("Original circuit:")
circ.print()

# Get the redundancy removal pass
opt_pass = qristal.core.redundancy_removal()
# Apply it on the input circuit
opt_pass.apply(circ)

# Print it out (single Rx remains)
print("Optimized circuit:")
circ.print()

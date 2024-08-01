# Import the core of Qristal
import qristal.core

# A circuit on two qubits that are extremely long!!!
circ = qristal.core.Circuit()
# Repeat a random circuits...
# there are 5 x 3 = 15 CNOT gates
for i in range(5):
    circ.h(0)
    circ.rx(1, 0.123)
    circ.cnot(1, 0)
    circ.x(1)
    circ.ry(1, 2.345)
    circ.ry(0, 0.456)
    circ.cnot(0, 1)
    circ.rz(1, 0.345)
    circ.rx(0, 0.876)
    circ.cnot(0, 1)
    circ.h(0)
    circ.h(1)


print("Original circuit:")
circ.print()

# Get the two-qubit squash pass
opt_pass = qristal.core. two_qubit_squash()
# Apply it on the input circuit
opt_pass.apply(circ)

# Print it out
print("Optimized circuit:")
# There should only be a couple of CNOT gates remain (e.g., 3)
circ.print()
# Check that the total unitary is equivalent. I dare you!!!

# Import the core of Qristal
import qristal.core

# Circuit
circ = qristal.core.Circuit()

# Pauli gate redundancies 
circ.x(0)
circ.x(0)          # X X = I
circ.h(1)
circ.h(1)          # H H = I

# Rotation cancellation & merges
circ.rz(0, 0.3)
circ.rz(0, 0.7)
circ.rz(0, -1.0)   # sum to zero

circ.rx(1, 0.5)
circ.rx(1, -0.5)   # inverse cancellation

circ.ry(2, 0.4)
circ.ry(2, 0.6)
circ.ry(2, -1.0)   # sum to zero

# T gate fusion and cancellation
circ.t(0)
circ.t(0)
circ.t(0)
circ.t(0)          # T^4 = Z
circ.tdg(0)
circ.tdg(0)        

# Swap gate pair
circ.swap(1, 2)
circ.swap(1, 2)   

# Multi-CX sequence 
circ.cnot(0, 1)
circ.cnot(0, 1)
circ.cnot(0, 1)         

# Rotation gate sequence
circ.rz(0, 0.3)
circ.rx(0, 0.4)
circ.rz(0, 0.2)

# Print original circuit
print("Original Circuit:")
circ.print()

# Apply full peephole optimization
opt = qristal.core.peephole_optimisation()
opt.apply(circ)

# Print optimized circuit
print("Optimized Circuit:")
circ.print()

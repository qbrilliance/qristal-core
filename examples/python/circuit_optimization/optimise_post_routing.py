# Import the core of Qristal
import qristal.core

# Circuit
circ = qristal.core.Circuit()
circ.swap(0,1)
circ.cnot(0,2)
circ.swap(0,2)

print("Original circuit:")
circ.print()

# Apply SWAP placement for linear connectivity: 0–1–2
connectivity = [(0, 1), (1, 2)]
placement_pass = qristal.core.swap_placement_pass(connectivity)
placement_pass.apply(circ)

print("\nAfter swap placement:")
circ.print()

# Apply post-routing optimization:
# removes redundant gates and simplifies the circuit after qubit routing
post_opt = qristal.core.optimise_post_routing()
post_opt.apply(circ)

print("\nAfter optimise_post_routing (post-routing):")
circ.print()

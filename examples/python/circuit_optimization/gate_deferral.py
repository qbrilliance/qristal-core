import qristal.core
# The gate deferral technique defers gates that create superpositions, allowing the state to remain sparse for
# as long as possible.
# In this example, the gate that creates superposition is the Hadamard gate. The deferral technique shifts
# the H gate pass the SWAP gate and acts H on qubit 1: H(0), SWAP(0, 1) -> SWAP(0, 1), H(1).
circ = qristal.core.Circuit()
circ.h(0)
circ.swap(0, 1)
circ.measure_all()

print("Original circuit:")
circ.print()

# Apply the gate deferral on circuit
gate_deffered_circ = qristal.core.gate_deferral_pass(circ)
# Print the new circuit
print("Gate-deferred circuit:")
gate_deffered_circ.print()

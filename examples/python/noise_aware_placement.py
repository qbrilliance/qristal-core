# Import the core of Qristal
import qristal.core

# Simple Bell circuit
circ = qristal.core.Circuit()
circ.h(0)
circ.cnot(0, 1)
circ.measure_all()
print("Original circuit:")
circ.print()

# Create a dummy device with simple characteristics for noise-aware placement testing:
# 4 qubits with a line topology:
# 0 - 1 - 2 - 3
nb_qubits = 4
dummy_device = qristal.core.noise_aware_placement_config()
for i in range(nb_qubits - 1):
    # (i) and (i + 1) qubits are connected
    dummy_device.connectivity.append((i, i + 1))
    # set fidelity for qubit-qubit pairs to 90% (error = 10%)
    dummy_device.two_qubit_gate_errors[(i, i + 1)] = 0.1

# Uniform single-qubit fidelity and readout of 99%
for i in range(nb_qubits):
    dummy_device.single_qubit_gate_errors[i] = 0.01
    dummy_device.readout_errors[i] = 0.01

# Override two-qubit gate error rate b/w qubit 2 and 3 to be very small.
# This will make these two qubits preferable during the noise-aware mapping.
dummy_device.two_qubit_gate_errors[(2, 3)] = 0.02

# Get the qristal noise-aware placement pass
placement = qristal.core.noise_aware_placement_pass(dummy_device)
# Apply the pass over the input circuit
placement.apply(circ)
print("After noise-aware placement:")
circ.print()
# It'll map the circuit to qubits 2 and 3 rather than using 0 and 1 as written!

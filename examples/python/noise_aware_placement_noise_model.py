# Import the core of Qristal
import qristal.core

# Simple Bell circuit
circ = qristal.core.Circuit()
circ.h(0)
circ.cnot(0, 1)
circ.measure_all()
print("Original circuit:")
circ.print()

# Create a ring noise model of 3 qubits, uniform noise except qubit 0 is very **bad** (strong noise)
# Make an empty noise model
noise_model = qristal.core.NoiseModel()
nb_qubits = 3
# Name the model whatever you like
noise_model.name = "ring_noise_model"

# Define the gate fidelities (errors are 1 - fidelity)
u1_error = 1e-4
u2_error = 1e-3
u3_error = 1e-3
cx_error = 1e-2

# Define the readout errors
ro_error = qristal.core.ReadoutError()
ro_error.p_01 = 1e-2
ro_error.p_10 = 5e-3

# Loop over the qubits
for qId in range(nb_qubits):
    # Set the readout errors
    noise_model.set_qubit_readout_error(qId, ro_error)

    # Set the single-qubit gate fidelities
    if qId == 0:
        # Gate noises (depolarizing) on Q0 is much stronger than the others
        noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, 10*u1_error), "u1", [qId])
        noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, 10*u2_error), "u2", [qId])
        noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, 10*u3_error), "u3", [qId])
    else:
        noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, u1_error), "u1", [qId])
        noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, u2_error), "u2", [qId])
        noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, u3_error), "u3", [qId])

    # Set the qubit connections to form a ring
    qId2 = 0 if qId == nb_qubits - 1 else qId + 1
    noise_model.add_qubit_connectivity(qId, qId2)

    # Set the corresponding two-qubit gate fidelities
    noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, qId2, cx_error), "cx", [qId, qId2])
    noise_model.add_gate_error(qristal.core.DepolarizingChannel.Create(qId, qId2, cx_error), "cx", [qId2, qId])


# Get the qristal noise-aware placement pass (constructed with the above noise model)
placement = qristal.core.noise_aware_placement_pass(noise_model)
# Apply the pass over the input circuit
placement.apply(circ)
print("After noise-aware placement:")
circ.print()
# It'll map the circuit to qubits 1 and 2 rather than using 0 and 1 as written since qubit 0 is very noisy!

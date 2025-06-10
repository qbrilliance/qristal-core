# Import the core of Qristal
import qristal.core

# Define a Qristal session
n_qubits = 2
my_sim = qristal.core.session()
my_sim.acc = "aer"
my_sim.qn = n_qubits
my_sim.sn = 100

# Add a custom noise model including readout errors only
read_out_error = qristal.core.ReadoutError()
read_out_error.p_01 = 0.05
read_out_error.p_10 = 0.05
device_properties = qristal.core.NoiseProperties()
qubit_topology_list = []
for q in range(n_qubits):
    device_properties.readout_errors[q] = read_out_error
    for qq in range(q+1, n_qubits):
        qubit_topology_list.append((q, qq))
device_properties.qubit_topology = qubit_topology_list
nm = qristal.core.NoiseModel(device_properties)
my_sim.noise = True
my_sim.noise_model = nm

# Define a Bell circuit to run
circ = qristal.core.Circuit()
circ.h(0)
circ.cnot(0, 1)
circ.measure_all()

# Hand the kernel over to the my_sim object
my_sim.irtarget = circ

# Automatically measure a SPAM benchmark for 1000 shots, and enable automatic SPAM correction
# then run the Bell circuit for the requested 100 shots
print("About to run quantum program...")
my_sim.run_with_SPAM(1000)
print("Ran successfully!")
print("The following SPAM correction matrix was used:\n", my_sim.SPAM_correction_matrix)

# Print the cumulative results in each of the classical registers
print("Native Results:")
print(my_sim.results_native)
print("SPAM-corrected Results:")
print(my_sim.results)

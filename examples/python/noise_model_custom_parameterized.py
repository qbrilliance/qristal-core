# Import the core of the QB SDK
import qb.core

# Create a dummy device using noise parameters.
# 4 qubits with a line topology:
# 0 - 1 - 2 - 3 
nb_qubits = 4

# Qubit topology
qubit_topology_list = []
for i in range(nb_qubits - 1):
    qubit_topology_list.append((i, i + 1))

# Qubit t1 and t2 (microseconds)
t1 = 1e6 # Relaxation time
t2 = 1e3 # Dephasing time

# Gate time (microseconds)
u1_time = 1.0
u2_time = 1.0
u3_time = 1.0
cx_time = 2.0

# Gate error (microseconds)
u1_error = 1e-4
u2_error = 1e-3
u3_error = 1e-3
cx_error = 5e-2

# Read-out errors
read_out_error = qb.core.ReadoutError()
read_out_error.p_01 = 1e-2
read_out_error.p_10 = 1e-2

device_properties = qb.core.NoiseProperties()
device_properties.gate_time_us["u1"] = {}
device_properties.gate_time_us["u2"] = {}
device_properties.gate_time_us["u3"] = {}
device_properties.gate_time_us["cx"] = {}

device_properties.gate_pauli_errors["u1"] = {}
device_properties.gate_pauli_errors["u2"] = {}
device_properties.gate_pauli_errors["u3"] = {}
device_properties.gate_pauli_errors["cx"] = {}

# 1-qubit properties
for i in range(nb_qubits):
    device_properties.t1_us[i] = t1
    device_properties.t2_us[i] = t2
    device_properties.readout_errors[i] = read_out_error
    device_properties.gate_time_us["u1"][[i]] = u1_time
    device_properties.gate_time_us["u2"][[i]] = u2_time
    device_properties.gate_time_us["u3"][[i]] = u3_time
    device_properties.gate_pauli_errors["u1"][[i]] = u1_error
    device_properties.gate_pauli_errors["u2"][[i]] = u2_error
    device_properties.gate_pauli_errors["u3"][[i]] = u3_error

# 2-qubit properties
for i in range(nb_qubits - 1):
    device_properties.gate_time_us["cx"][[i, i + 1]] = cx_time
    device_properties.gate_pauli_errors["cx"][[i, i + 1]] = cx_error

# Qubit topology
device_properties.qubit_topology = qubit_topology_list

# Create noise model using device noise properties
nm = qb.core.NoiseModel(device_properties)

# Simple Bell circuit
circ = qb.core.Circuit()
circ.h(0)
circ.cnot(0, 1)
circ.measure_all()

s = qb.core.session()
s.init()
s.qn = nb_qubits
s.noise = True
s.noise_model = nm
s.ir_target = circ
s.acc = "aer"
s.run()
result = s.out_raw_json[0][0]
print("Output = " + result)

print("Executing parametrized circuit demo...")

# Import the core of Qristal
import qristal.core

# Define a circuit. Add some gates and measurements.
circuit = qristal.core.Circuit()
circuit.rx(0, "alpha")
circuit.ry(0, "beta")
circuit.ry(1, "gamma")
circuit.rz(0, 2.5)
circuit.cnot(0, 1)
circuit.measure_all(circuit.num_qubits())
circuit.print()

# Define the runtime parameters for the above circuit
params_map = {"alpha": 1.0, "beta": 1.5, "gamma": 2.0}
params_list = circuit.param_dict_to_list(params_map)

# Create the executor object
my_sim = qristal.core.session()
my_sim.qn = circuit.num_qubits()
my_sim.sn = 1000
my_sim.circuit_parameters = params_list
my_sim.calc_gradients = True
my_sim.irtarget = circuit

print("About to run circuit...")
my_sim.run()
print("Ran successfully!")

# Print the counts of the executed circuit
print("Direct results:")
print(my_sim.results)
print("Results via all_bitstring_counts:")
import itertools
for bits in itertools.product([0, 1], repeat=circuit.num_qubits()):
  reversed_bitstring = ''.join([str(x) for x in reversed(bits)])
  print(f'{reversed_bitstring}: {my_sim.all_bitstring_counts[my_sim.bitstring_index(bits)]}')

# Print the probability jacobian as well
print("Probability gradients:\n", my_sim.all_bitstring_probability_gradients)

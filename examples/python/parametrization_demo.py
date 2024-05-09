print("Executing parametrized circuit demo...")

# Import the core of the QB SDK
import qb.core

# Define a circuit
circuit = qb.core.Circuit()

# Add some gates
circuit.rx(0, "alpha")
circuit.ry(0, "beta")
circuit.ry(1, "gamma")
circuit.rz(0, 2.5)
circuit.cnot(0, 1)

circuit.measure_all(2)
circuit.print()
# Define the runtime parameters for the above circuit
params_map = {"alpha": 1.0, "beta": 1.5, "gamma": 2.0}
params_list = circuit.param_dict_to_list(params_map)
# Create the executor object
my_sim = qb.core.session()
#Define settings
my_sim.qb12()
my_sim.qn = 2
my_sim.sn = 1000
my_sim.parameter_list = params_list
my_sim.calc_jacobian = True
my_sim.ir_target = circuit

print("About to run circuit...")
my_sim.run()
print("Ran successfully!")

# Print the counts of the executed circuit
print("Results:")
for x in ["00", "10", "01", "11"]:
  print(x, ": ", my_sim.out_counts[0][0][my_sim.bitstring_index(x, 0, 0)])


# Print the probability jacobian as well
print("Jacobian:\n", my_sim.out_prob_jacobians[0][0])
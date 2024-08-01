# Parametrized Quantum Circuits

The central part of all variational quantum algorithms are parametrized quantum circuits (PQCs), which are circuits with rotation gates without defined values prior to runtime, such that they can have their parameters changed for every run. This allows for linking with both gradient-based and gradient-free optimizers, to minimize an objective function based e.g. on the probability output or some other cost function.

## Example in Qristal

### Defining a parametrized circuit

Qristal can be used to define both parametrized and non-parametrized quantum gates using the `qristal.core.Circuit()` object in Python. Here is a sample circuit definition:

```python
# Import the core of Qristal
import qristal.core

# Define a circuit
circuit = qristal.core.Circuit()

# Add some gates
circuit.rx(0, "alpha")
circuit.ry(0, "beta")
circuit.ry(1, "gamma")
circuit.rz(0, 2.5)
circuit.cnot(0, 1)

circuit.measure_all(2)
```

While it may be self-evident what the above circuit will look like, we can ensure that it follows the defined parametrization by running `circuit.print()`.

```
Rx(alpha) q0
Ry(beta) q0
Ry(gamma) q1
Rz(2.5) q0
CNOT q0,q1
Measure q0
Measure q1
```

### Executing the circuit

Now that the circuit is defined, we must define runtime values and an executor for the circuit. Given that we have 3 distinct parameters, we define these accordingly. One can define a parameter map that takes the form of a dictionary to assign values to free parameters. This can be converted to a list that respects the order that the parameters are defined in within the circuit, then passed to the `session` class along with some additional relevant settings.

```python
# Define the runtime parameters for the above circuit
params_map = {"alpha": 1.0, "beta": 1.5, "gamma": 2.0}
params_list = circuit.param_dict_to_list(params_map)
# Create the executor object
my_sim = qristal.core.session()

#Define settings
my_sim.init()
my_sim.qn = 2
my_sim.sn = 1000
my_sim.parameter_list = params_list
my_sim.calc_jacobian = True
my_sim.ir_target = circuit

print("About to run circuit...")
my_sim.run()
print("Ran successfully!")
```

In the above example, we assign the runtime values of `params` to the `parameter_list` property of the executor, and tell it to calculate gradients as well by setting the `calc_jacobian` property to `True`.

**NB**: This will set the properties for all circuits being simulated with this `session` object, so when setting the parameters and enabling/disabling gradient calculations for multiple circuits in C++, ensure it is passed in the correct format using the properties `parameter_vectors` and `calc_jacobians`.

Finally, we execute the circuit by invoking `my_sim.run()`. This populates the output fields of the executor; the ones we are interested in for now are the `out_counts` and `out_prob_jacobians`, which contain the output counts for each bitstring and the output probability jacobian (with respect to the runtime parameters) respectively.

### Obtaining results

One can obtain the results simply by retrieving the aforementioned objects.

```python
# Print the statistics of the executed circuit
print("Results:")
for x in ["00", "01", "10", "11"]:
  print(x, ": ", my_sim.out_counts[0][0][my_sim.bitstring_index(x, 0, 0)])

# Print the probability jacobian as well
print("Jacobian:\n", my_sim.out_prob_jacobians[0][0])
```

This will give the following terminal output showing both the probabilities and jacobian of the circuit when executed with the defined parameters:


    About to run circuit...
    Ran successfully!
    Results:
    00 :  156
    01 :  343
    10 :  333
    11 :  168
    Jacobian:
     [[-0.012499999999999997, 0.016500000000000015, -0.004999999999999977, 0.0010000000000000009], [-0.067, 0.1905, -0.20550000000000002, 0.082], [-0.244, 0.22499999999999998, 0.22999999999999998, -0.211], [0.0, 0.0, 0.0, 0.0]]


The above code can be found at `examples/python/parametrization_demo.py`. For a C++-based example, `examples/cpp/parametrization/parametrization_demo.cpp` also describes the same process but with 2 different circuits using the same executor.

# v0 Python Plugin

The Grover algorithm was originally a method for finding a given entry in a database [*reference*] but it has since been generalised to finding extremal values of functions. This in turn has a multitude of applications, including finding optimal strings of characters given a measure, which we later illustrate in the *quantum_decoder*.

To perform a Grover iteration the *grover_iteration*() method is called: 

```python
import grover as gr
compare_vector = gr.comparison_vector(qubits_of_interest,initial_vector,compare_value,comparison)
grovered_state = gr.grover_iteration(initial_vector,qubits_of_interest,compare_vector)
```

Note that the Grover iteration is designed to return a state vector for which the desirable components have amplitudes much greater than all the other components combined so that an appropriate result is overwhelmingly likely to be obtained upon measurement. This is not guaranteed. 

The effectiveness of the Grover iteration is also affected by the distribution of the measures and of the probability amplitudes within the initial state.

The input parameter compare_vector specifies which components obey the criteria for being desirable, such as having a measure greater than *compare_value* (see the *comparator* application above). The Grover iteration magnifies the probability amplitude of the desirable components at the expense of the undesirable components.
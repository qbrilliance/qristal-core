# v0 Python Plugin

Some algorithms, such as the exponential search and other applications of the Grover iteration below, need to compare a quantum register to a given value. The user specifies if they want registers whose value is greater or lesser than this value, as well as the state vector being compared, the qubits-of-interest, and the value being compared against.  This has been implemented in version zero so as to provide a vector that yields a positive one if the condition is obeyed and a negative one if it is not. The currently implemented comparisons are "greater than" and "less than".

```python
import grover as gr
compare_vector = gr.comparison_vector(qubits_of_interest,initial_vector,compare_value,comparison)
```
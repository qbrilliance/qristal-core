# v0 Python Plugin

The decoder is an algorithm that, given a string of symbols such as letters of the alphabet, and a probability table or some other form of the sampler, will predict the subsequent symbols. The applications for such an algorithm range from text autofill to autonomous pathfinding. 

The quantum decoder encodes a superposition of all possible strings with a measure, typically the probability of the string, and then uses an exponential search algorithm to manipulate the probability amplitudes so that the highest measure string is the one most likely returned upon measurement.

It currently uses a classical sampler but a quantum version will be needed eventually. The easiest way to use the quantum decoder is to instantiate a *quantum_decoder* object from the quantum_decoder.py module:

```python
import quantum_decoder as qd
decoder = qd.quantum_decoder(S,N_string,total_length,table,precision,Alphabet,
				grammar,Starting_String = '!', search_cutoff = 0.9, Encoding = "Bitstring")

decoded_state = decoder.get_decoder_state()
optimal_string = es.measurement(qubits_of_interest, decoded_state**)    #Almost all the time**
```

The quantum decoder is based on the Grover iteration and returns a state for which the most suitable string has an amplitude much greater than all the other components combined so that the correct string is overwhelmingly likely, though not guaranteed, to be obtained upon measurement.
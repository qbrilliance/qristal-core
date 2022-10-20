# QAOA extensions in qbOS

# Recursive QAOA

## Quickstart example for Recursive QAOA

```python
print("* Runs recursive QAOA on built-in Nelder-Mead. Uses a classical/diagonal 5-qubit Hamiltonian. Uses qpp and QAOA-depth = 3 with extended parameter set. Assert checks the optimum eigenstate.")

import qbos_op
qa = qbos_op.QaoaRecursive()
qa.qaoa_step = 3
qa.sn = 1024   
qa.extended_param = True

# specify the number of qubits
qa.qn = 5

# specify the threshold number of variables (terms in final Hamiltonian)
qa.n_c = 3

qa.ham = "0.5 Z0 Z2 + 0.5 Z1 Z2 + 0.5 Z2 Z3 + 0.5 Z1 Z4 + 0.5 Z2 Z4 + 0.5 Z3 Z4"
qa.maxeval = 800
    
qa.run()
assert qa.out_eigenstate[0][0] == "10100" or qa.out_eigenstate[0][0] == "01011", "[qbos_op.QaoaRecursive()] Failed eigenstate test: 10100 or 01011"
```

## About Recursive QAOA

Recursive QAOA iteratively reduces the problem size and outperforms QAOA on certain forms of Ising Hamiltonians.

## Usage options different from standard QAOA

- `theta` parameter is not required: appropriate set of parameters will be generated inside the algorithm in accordance with specified `extended_param` parameter.
- `n_c` defines the threshold number of variables (terms in final Hamiltonian) to stop the recursion.

# Warm-started QAOA

## Quickstart example for Warm-started QAOA

```python
print("* Runs warm-started QAOA on built-in Nelder-Mead. Uses a classical/diagonal 3-qubit Hamiltonian. Uses qpp and QAOA-depth = 1 with standard parameter set. Assert checks the optimum eigenstate.")

import qbos_op
qa = qbos_op.QaoaWarmStart()

qa.qaoa_step = 1
qa.sn = 1024   
qa.extended_param = False
qa.qn = 3
qa.ham = "0.5 Z0 Z1 + 0.5 Z0 Z2 + 0.5 Z1 Z2"

#specify a good cut
qa.good_cut = "010"
qa.maxeval = 200

qa.theta = qbos_op.ND()
for ii in range(qa.qaoa_step[0][0]*2) :
    qa.theta[0][0][ii] = 0.25

qa.run()
assert qa.out_eigenstate[0][0] == "010", "[qbos_op.QaoaWarmStart()] Failed eigenstate test: 010"
```

## About Warm-started QAOA

Warm-starting is an approach in classical computing to speed up the search for an optimal solution, which is commonly used in the domains of machine learning and optimization. The general idea of warm-starting is to use the knowledge of previous solutions or solutions for related or relaxed problems to facilitate the search for better solutions. Warm-starting does not guarantee finding the best solution efficiently but may speed up the search for a good result.

## Usage options different from standard QAOA

- `good_cut` is a classically pre-computed good cut.
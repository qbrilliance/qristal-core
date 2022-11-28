```{attention} Placeholder only 

Copied from legacy qbOS documentation, need revision...
```

# 1. Quickstart

Using a small experimental design, let's run some circuits. 



## **Step 1 of 3**: Start your qbOS environment

Once you have completed the login to your VM host, use your Web browser to open: [http://localhost:8889](http://localhost:8889) 


## **Step 2 of 3**: Python code that calls qbOS and executes all (four) corner conditions of the experiment

```python
import qbos
tqb = qbos.core()  # This object has access to the core API methods for circuit simulation
tqb.qb12()         # Set up some sensible defaults
```

> Setup conditions that apply to all experiment corners:
> 

```python
tqb.xasm = True    # Use XASM circuit format to access XACC's qft()    
tqb.sn = 1024      # Explicitly use 1024 shots
tqb.acc = "aer"    # Use the aer state-vector simulator
```

> Setup rows of the experiment table:
> 

```python
tqb.qn.clear()
tqb.qn.append(qbos.N([2]))  # 2-qubits for the top row
tqb.qn.append(qbos.N([4]))  # 4-qubits for the bottom row

qpu_kernel_qft_2 = '''
__qpu__ void QBCIRCUIT(qreg q) 
{
qft(q, {{"nq",2}});
Measure(q[1]);
Measure(q[0]);
}
'''

qpu_kernel_qft_4 = '''
__qpu__ void QBCIRCUIT(qreg q) 
{
qft(q, {{"nq",4}});
Measure(q[3]);
Measure(q[2]);
Measure(q[1]);
Measure(q[0]);
}
'''

tqb.instring.clear()
tqb.instring.append(qbos.String([qpu_kernel_qft_2]))   # QPU Kernel for the top row
tqb.instring.append(qbos.String([qpu_kernel_qft_4]))   # QPU Kernel for the bottom row
```

> Enable the noise model
> 

```python
tqb.noise.clear()
tqb.noise.append(qbos.Bool([False, True]))  # noise is False (disabled) for the left (index 0) column, True (enabled) for the right column (index 1)
```

> Run the simulation in all experiment corners
> 

```python
tqb.run()    
```

## **Step 3 of 3**: Inspect the results (shot counts)

After the code from Step 2 has finished executing, you can inspect shot counts (in each state and two easy-to-understand formats) by calling: `tqb.out_raw[0]` or `tqb.out_count[0]` for the first experiment and `tqb.out_raw[1]` or `tqb.out_count[1]` for the second one. More details on how to use Python codes to inspect settings or results, as configured for experiments above, are listed in the tables below:



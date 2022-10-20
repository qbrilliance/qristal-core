# Testing T1 decay of the noise simulation.
# Set up qubit in |1> state, use Identity gate to model wait time and observing the decay of |1> population.
import qbos
tqb = qbos.core(True)  # This object has access to the core API methods for circuit simulation
tqb.qb12()         # Set up some sensible defaults

tqb.xasm = True    
tqb.sn = 1024      # Explicitly use 1024 shots
tqb.acc = "aws_acc"    # Using AWS
tqb.nooptimise = True
tqb.noplacement = True

# Setup rows in the experiment table
tqb.qn.clear()

all_kernels = []
for i in range(0, 100, 4):
    tqb.qn.append(qbos.N([1])) 
    qpu_kernel = '''
__qpu__ void QBCIRCUIT(qreg q) 
{
X(q[0]);
for (int i = 0; i < ''' + str(i) + '''; i++) {
    I(q[0]);
}
Measure(q[0]);
}
'''
    all_kernels.append(qpu_kernel)

tqb.instrings.clear()
for kernel_str in all_kernels:
    tqb.instrings.append(qbos.String([kernel_str]))
# Setup columns in the experiment table
tqb.noise = True
tqb.run()
print(tqb.out_counts)

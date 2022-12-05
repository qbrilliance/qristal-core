# Testing T1 decay of the noise simulation.
# Set up qubit in |1> state, use Identity gate to model wait time and observing the decay of |1> population.
import qb.core
s = qb.core.session(True)  # This object has access to the core API methods for circuit simulation
s.qb12()         # Set up some sensible defaults

s.xasm = True
s.sn = 1024      # Explicitly use 1024 shots
s.acc = "aws_acc"    # Using AWS
s.nooptimise = True
s.noplacement = True

# Setup rows in the experiment table
s.qn.clear()

all_kernels = []
for i in range(0, 100, 4):
    s.qn.append(qb.core.N([1]))
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

s.instrings.clear()
for kernel_str in all_kernels:
    s.instrings.append(qb.core.String([kernel_str]))
# Setup columns in the experiment table
s.noise = True
s.run()
print(s.out_counts)

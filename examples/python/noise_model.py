import qb.core
my_sim = qb.core.session()
my_sim.qb12()
my_sim.qn = 2
my_sim.acc = "aer"
my_sim.noise = True
my_sim.noise_model = "default"
my_sim.instring = '''
    OPENQASM 2.0;
    include "qelib1.inc";
    creg c[2];
    h q[0];
    cx q[0],q[1];
    measure q[1] -> c[1];
    measure q[0] -> c[0];
'''
my_sim.run()
print(my_sim.out_raws[0])

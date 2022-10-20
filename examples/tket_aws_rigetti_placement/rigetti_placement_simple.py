import qbos
tqb = qbos.core()
tqb.qb12()
tqb.acc = "aws_acc"
tqb.aws_device = "Rigetti"
tqb.sn = 1024
tqb.placement = "tket"
tqb.nooptimise = True
tqb.nosim = True
tqb.notiming = True
tqb.output_oqm_enabled = False

circ = qbos.Circuit()
circ.h(0)
circ.cnot(0, 1)

# measure
circ.measure_all()

# run the circuit and check results
tqb.ir_target = circ

print("Before placement:")
tqb.ir_target[0][0].print()
# Note: Since nosim is set, we don't actually submit the circuit to Rigetti
tqb.run()

print("After placement:")
tqb.ir_target[0][0].print()
import qb.core
s = qb.core.session()
s.qb12()
s.acc = "aws_acc"
s.aws_device = "Rigetti"
s.sn = 1024
s.placement = "tket"
s.nooptimise = True
s.nosim = True
s.notiming = True
s.output_oqm_enabled = False

circ = qb.core.Circuit()
circ.qft(range(5))
circ.measure_all()


# run the circuit and check results
s.ir_target = circ

print("Before placement:")
s.ir_target[0][0].print()
# Note: Since nosim is set, we don't actually submit the circuit to Rigetti
s.run()

print("After placement:")
s.ir_target[0][0].print()

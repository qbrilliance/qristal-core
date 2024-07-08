import qb.core
import numpy as np
s = qb.core.session()
s.init()

circ = qb.core.Circuit()
# QFT
circ.qft(range(5))
#IQFT
circ.iqft(range(5))

circ.measure_all()

# Check the circuit:
circ.print()

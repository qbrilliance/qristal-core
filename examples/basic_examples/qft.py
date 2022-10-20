import qbos as qb
import numpy as np
tqb = qb.core()
tqb.qb12()

circ = qb.Circuit()
# QFT
circ.qft(range(5))
#IQFT
circ.iqft(range(5))

circ.measure_all()

# Check the circuit:
circ.print()
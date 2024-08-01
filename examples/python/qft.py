import qristal.core
import numpy as np
s = qristal.core.session()
s.init()

circ = qristal.core.Circuit()
# QFT
circ.qft(range(5))
#IQFT
circ.iqft(range(5))

circ.measure_all()

# Check the circuit:
circ.print()

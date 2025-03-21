import qristal.core
import math
import random

from qiskit import QuantumCircuit
from qiskit import QuantumRegister, ClassicalRegister
from qiskit import transpile

import xacc

# Random angle to use as the phase
theta = random.uniform(-2.0, 2.0)*math.pi
# print("theta:", theta)

q = QuantumRegister(2, 'q')
c = ClassicalRegister(2, 'c')
circ_aer = QuantumCircuit(q, c)

# Get transpiled circuit from AER
circ_aer.cp(theta, 0, 1, label=None, ctrl_state=None)
transpiled_circ_aer = transpile(circ_aer, basis_gates =["rx","ry","cz"], optimization_level=3)

# Get transpiled circuit from qb_visitor
my_sim = qristal.core.session()
my_sim.init()
my_sim.qn = 2
my_sim.acc = "qpp"
my_sim.noplacement = True

circ_qristal = qristal.core.Circuit()
circ_qristal.cphase(0, 1, theta)
my_sim.ir_target = circ_qristal
my_sim.run()
transpiled_circ_qristal = my_sim.out_transpiled_circuit[0][0]

# Check that both transpiled circuits are the same
compiler = xacc.getCompiler("staq")
ir_aer = compiler.compile(transpiled_circ_aer.qasm()).getComposites()[0]
ir_qristal = compiler.compile(transpiled_circ_qristal).getComposites()[0]
assert(ir_aer.nInstructions() == ir_qristal.nInstructions())

for i in range(ir_aer.nInstructions()):
   if ir_aer.getInstruction(i).name() == "Rx" or ir_aer.getInstruction(i).name() == "Ry":
      assert(ir_aer.getInstruction(i).name() == ir_qristal.getInstruction(i).name())
      assert(ir_aer.getInstruction(i).bits() == ir_qristal.getInstruction(i).bits())
      assert(math.isclose(ir_aer.getInstruction(i).getParameter(0), ir_qristal.getInstruction(i).getParameter(0), abs_tol=1.0e-6))
   elif ir_aer.getInstruction(i).name() == "CZ":
      assert(ir_aer.getInstruction(i).name() == ir_qristal.getInstruction(i).name())
      assert(ir_aer.getInstruction(i).bits() == ir_qristal.getInstruction(i).bits())

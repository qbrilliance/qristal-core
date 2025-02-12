# Test cases for quantum circuit library
import pytest

def test_cz_optimization():
    print(" Testing CZ optimization ")
    import qristal.core
    s = qristal.core.session()
    s.init()
    circ = qristal.core.Circuit()
    # Check this circuit is optimized to a single CZ gate
    # Check for this pattern
    # -------------
    #       |
    #       |
    # H-----+-----H
    circ.h(1)
    circ.cnot(0, 1)
    circ.h(1)
    s.ir_target = circ
    s.nooptimise = False
    s.noplacement = True
    s.notiming = True
    s.execute_circuit = False
    s.run()
    print(s.out_transpiled_circuit[0][0])
    # Use xacc to recompile the transpiled qasm for validation
    import xacc
    compiler = xacc.getCompiler("staq")
    ir = compiler.compile(s.out_transpiled_circuit[0][0]).getComposites()[0]
    assert(ir.nInstructions() == 1)
    assert(ir.getInstruction(0).name() == "CZ")

def test_cz_placement():
    print(" Testing placement of CZ gate ")
    import qristal.core
    s = qristal.core.session()
    s.init()
    circ = qristal.core.Circuit()
    circ.cz(0, 1)
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = False
    s.notiming = True
    s.execute_circuit = False
    s.run()
    print(s.out_transpiled_circuit[0][0])
    # Use xacc to recompile the transpiled qasm for validation
    import xacc
    compiler = xacc.getCompiler("staq")
    ir = compiler.compile(s.out_transpiled_circuit[0][0]).getComposites()[0]
    assert(ir.nInstructions() == 1)
    assert(ir.getInstruction(0).name() == "CZ")

def test_cphase_simple():
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
    my_sim.execute_circuit = False
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

def test_xasm_output():
    print(" Testing QB machine code (XASM) output.")
    import qristal.core
    import json
    my_sim = qristal.core.session()
    my_sim.init()
    my_sim.execute_circuit = False
    my_sim.acc = "example_hardware_device"
    my_sim.qn = 2
    my_sim.instring = '''
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      cx q[1],q[0];
      measure q[0] -> c[0];
      measure q[1] -> c[1];
    }
    '''
    my_sim.run()
    assert(json.loads(my_sim.out_qbjson[0][0])["circuit"] == ['Ry(q[0],1.570796)', 'Rx(q[0],3.141593)', 'Ry(q[0],1.570796)', 'Rx(q[0],3.141593)', 'CZ(q[1],q[0])', 'Ry(q[0],1.570796)', 'Rx(q[0],3.141593)'])

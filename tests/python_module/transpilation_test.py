# Test cases for quantum circuit library

import pytest


def test_cz_optimization():
    print(" Testing CZ optimization ")
    import qb.core
    s = qb.core.session()
    s.qb12()
    circ = qb.core.Circuit()
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
    import qb.core
    s = qb.core.session()
    s.qb12()
    circ = qb.core.Circuit()
    circ.cz(0, 1)
    s.ir_target = circ
    s.nooptimise = True
    s.noplacement = False
    s.notiming = True
    s.run()
    print(s.out_transpiled_circuit[0][0])
    # Use xacc to recompile the transpiled qasm for validation
    import xacc
    compiler = xacc.getCompiler("staq")
    ir = compiler.compile(s.out_transpiled_circuit[0][0]).getComposites()[0]
    assert(ir.nInstructions() == 1)
    assert(ir.getInstruction(0).name() == "CZ")
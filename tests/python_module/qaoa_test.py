# Test cases for qaoa

import qristal.core
import qristal.core.optimization as qbOpt
import pytest

def qaoa_helper(
        pauliString,
        nOptVars,
        nQaoaSteps = 2,
        nShots = 256,
        nOptEvalsMax = 1,
        optTol = 1e-6,
        accelerator = 'qpp',
        initTheta = 0.1,
        extParams = False,
    ):

    nPaulis = pauliString.count('+') + pauliString.count('-') - 1

    qa = qbOpt.qaoa_QaoaSimple()

    qa.ham = pauliString
    qa.qn = nOptVars
    qa.qaoa_step = nQaoaSteps
    qa.sn = nShots
    qa.maxeval = nOptEvalsMax
    qa.functol[0][0][0] = optTol
    qa.acc = accelerator
    qa.extended_param = extParams

    if (qa.extended_param[0][0]):
        nThetas = (nOptVars+nPaulis)*nQaoaSteps
    else:
        nThetas = 2*nQaoaSteps

    qa.theta = qristal.core.MapIntDouble()
    for ii in range(nThetas) :
        qa.theta[0][0][ii] = initTheta

    qa.run()

    return qa.out_eigenstate[0][0]

def test_qaoa_linear() :
    solution = "001"
    problem = "1.0 + 3.5 Z0 - 5.5 Z1 - 5.9 Z2"

    qrystalOutput = qaoa_helper(problem, 3, 2, 1024, 300)

    assert(qrystalOutput == solution)

def test_qaoa_kite() :
    solution1 = "10100"
    solution2 = "01011"
    problem = "+ 0.0 + 0.5 Z0 Z2 + 0.5 Z1 Z2 + 0.5 Z2 Z3 + 0.5 Z1 Z4 + 0.5 Z2 Z4 + 0.5 Z3 Z4"

    qrystalOutput = qaoa_helper(problem, 5, 2, 100000, 1000, 1e-6, 'qpp', True)

    assert((qrystalOutput == solution1) or (qrystalOutput == solution2))


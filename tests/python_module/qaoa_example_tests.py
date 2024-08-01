import qristal.core
import qristal.core.optimization as qbOpt

def test_qaoa_simple() :
    print("* qaoa_simple:")
    print("* Runs QAOA on built-in Nelder-Mead. Uses a classical/diagonal 3-qubit Hamiltonian.  Uses qpp and QAOA-depth = 2.  Assert checks the optimum eigenstate.")
    qa = qbOpt.qaoa_QaoaSimple()
    qa.qaoa_step = 2
    qa.sn = 1024
    qa.extended_param = False
    qa.qn = 3
    qa.ham = "1.0 + 3.5 Z0 + -5.5 Z1 + -5.9 Z2"
    qa.ham[0].append("1.0 + -3.5 Z0 + -5.5 Z1 + -5.9 Z2")
    qa.maxeval = 300
    qa.theta = qristal.core.MapIntDouble()
    for ii in range(qa.qaoa_step[0][0]*2) :
        qa.theta[0][0][ii] = 0.1

    qa.run()
    print("eigenstate: " + str(qa.out_eigenstate[0][0]))

    assert qa.out_eigenstate[0][0] == "001", "Failed eigenstate test: qaoa_simple_expect_001"
    assert qa.out_eigenstate[0][1] == "000", "Failed eigenstate test: qaoa_simple_expect_000"
    print("test_qaoa_simple passed")

def test_qaoa_simple_kite() :
    print("\n\n* qaoa_simple_kite:")
    print("* Runs QAOA on built-in Nelder-Mead. Uses a classical/diagonal 5-qubit Hamiltonian.  Uses qpp and QAOA-depth = 1.  Assert checks the optimum eigenstate.")
    nQubits = 5
    nQaoaSteps = 1
    pauliString = "+ 0.0 + 0.5 Z0 Z2 + 0.5 Z1 Z2 + 0.5 Z2 Z3 + 0.5 Z1 Z4 + 0.5 Z2 Z4 + 0.5 Z3 Z4"
    nPaulis = pauliString.count('+') + pauliString.count('-') -1

    qa = qbOpt.qaoa_QaoaSimple()
    qa.qaoa_step = nQaoaSteps
    qa.sn = 10000
    qa.extended_param = True
    qa.qn = nQubits
    qa.ham = pauliString
    qa.maxeval = 1000
    qa.functol[0][0][0] = 1e-6
    qa.theta = qristal.core.MapIntDouble()
    for ii in range((nQubits+nPaulis)*nQaoaSteps) :
        qa.theta[0][0][ii] = 0.25

    qa.run()
    print("eigenstate: " + str(qa.out_eigenstate[0][0]))

    assert qa.out_eigenstate[0][0] == "10100" or qa.out_eigenstate[0][0] == "01011", "Failed eigenstate test: qaoa_simple_kite"
    print("test_qaoa_simple passed")

def test_qaoa_recursive_kite() :
    print("\n\n* test_qaoa_recursive_kite:")
    print("* Runs recursive QAOA on built-in Nelder-Mead. Uses a classical/diagonal 5-qubit Hamiltonian.  Uses qpp and QAOA-depth = 3.  Assert checks the optimum eigenstate.")
    nQubits = 5
    nQaoaSteps = 1
    pauliString = "+ 0.0 + 0.5 Z0 Z2 + 0.5 Z1 Z2 + 0.5 Z2 Z3 + 0.5 Z1 Z4 + 0.5 Z2 Z4 + 0.5 Z3 Z4"
    nPaulis = pauliString.count('+') + pauliString.count('-') -1

    qa = qbOpt.qaoa_QaoaRecursive()
    qa.qaoa_step = nQaoaSteps
    qa.sn = 10000
    qa.extended_param = True
    qa.n_c = 3
    qa.qn = nQubits
    qa.ham = pauliString
    qa.maxeval = 1000
    qa.functol[0][0][0] = 1e-6

    qa.run()
    print("eigenstate: " + str(qa.out_eigenstate[0][0]))

    assert qa.out_eigenstate[0][0] == "10100" or qa.out_eigenstate[0][0] == "01011", "Failed eigenstate test: CI_220711_rqaoa_kite"
    print("test_qaoa_recursive_kite passed")

def test_qaoa_warm_start_kite() :
    print("\n\n* test_qaoa_warm_start_kite:")
    print("* Runs warm-started QAOA on built-in Nelder-Mead. Uses a classical/diagonal 5-qubit Hamiltonian.  Uses qpp and QAOA-depth = 3.  Assert checks the optimum eigenstate.")
    nQubits = 5
    nQaoaSteps = 1
    pauliString = "+ 0.0 + 0.5 Z0 Z2 + 0.5 Z1 Z2 + 0.5 Z2 Z3 + 0.5 Z1 Z4 + 0.5 Z2 Z4 + 0.5 Z3 Z4"
    nPaulis = pauliString.count('+') + pauliString.count('-') -1
    extendedParams = False

    qa = qbOpt.qaoa_QaoaWarmStart()
    qa.qaoa_step = nQaoaSteps
    qa.sn = 10000
    qa.extended_param = extendedParams
    qa.qn = nQubits
    qa.ham = pauliString
    qa.good_cut = "010"
    qa.maxeval = 1000
    qa.functol[0][0][0] = 1e-6

    if (extendedParams):
        nThetas = (1+nPaulis)*nQaoaSteps
    else:
        nThetas = 2*nQaoaSteps

    qa.theta = qristal.core.MapIntDouble()
    for ii in range(nThetas) :
        qa.theta[0][0][ii] = 0.25

    qa.run()
    print("eigenstate: " + str(qa.out_eigenstate[0][0]))

    assert qa.out_eigenstate[0][0] == "01011", "Failed eigenstate test: qaoa_warm_start_kite"
    print("test_qaoa_warm_start_kite passed")

test_qaoa_simple()
test_qaoa_simple_kite()
test_qaoa_recursive_kite()
test_qaoa_warm_start_kite()

// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/optimization/qaoa/qaoa.hpp"

int main () {
    xacc::Initialize();;
    xacc::external::load_external_language_plugins();
    xacc::set_verbose(true);
    xacc::ScopeTimer timer_for_cpu("Walltime in ms", false);

    std::string pauliString {"+ 1.0 + 3.5 Z0 - 5.5 Z1 - 5.9 Z2"};

    std::size_t nPaulis = -1 + std::count_if(pauliString.begin(), pauliString.end(),
                            [](char c) {return ((c=='+') || (c=='-'));});

    std::cout << "pauli string (" << nPaulis << "): " << pauliString << std::endl;

    std::size_t nOptVars = 3;
    std::size_t nQaoaSteps = 2;
    qb::op::QaoaSimple qaoa{false};
    qaoa.set_ham(pauliString);
    qaoa.set_qn(nOptVars);
    qaoa.set_acc("qpp");
    qaoa.set_functol({{0,1.0e-6}});
    qaoa.set_maxeval(300);
    qaoa.set_qaoa_step(nQaoaSteps);
    qb::VectorMapND thetas{{{}}};

    bool extendedParams = false;
    std::size_t nThetas{};
    if (extendedParams){
        nThetas = (nOptVars+nPaulis)*nQaoaSteps;
    }else{
        nThetas =                  2*nQaoaSteps;
    }
    qaoa.set_extended_param(extendedParams);
    for (int idx=0; idx<(int)(nThetas); idx++){
        thetas[0][0][idx] = 0.1;
    }
    qaoa.set_thetas(thetas);

    qaoa.run(0,0);

    const qb::VectorString eigenstates{qaoa.get_out_eigenstates()};
    const qb::VectorMapND energies{qaoa.get_out_energys()};
    const std::map<int,double> cost{energies[0][0]};
    const std::string eigenstate{eigenstates[0][0]};

    std::cout   << "cost: "           <<  cost.at(0)
                << ", eigenstate: "   <<  eigenstate << std::endl;
    
    if  ( eigenstate == "001" ) { // are equal?
        std::cout << "test passed!" << std::endl;
    } else {
        std::cout << "test failed!" << std::endl;
    }
    
    xacc::Finalize();

    return 0;
}

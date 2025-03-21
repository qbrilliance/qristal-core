// Copyright (c) Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include "qristal/core/optimization/qaoa/qaoa.hpp"

TEST(qaoaTester, check_simple) {
    std::string pauliString {"+ 1.0 + 3.5 Z0 - 5.5 Z1 - 5.9 Z2"};

    std::size_t nPaulis = -1 + std::count_if(pauliString.begin(), pauliString.end(), 
                            [](char c) {return ((c=='+') || (c=='-'));});

    std::size_t nOptVars = 3;
    std::size_t nQaoaSteps = 2; 
    qristal::op::QaoaSimple qaoa{false};
    qaoa.set_ham(pauliString);
    qaoa.set_qn(nOptVars);
    qaoa.set_acc("qpp");
    qaoa.set_functol({{0,1.0e-6}});
    qaoa.set_maxeval(300);
    qaoa.set_qaoa_step(nQaoaSteps);
    qristal::Table2d<std::map<int,double>> thetas{{{}}};

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

    //const auto      nIters = params.energies.size();
    //const double  cpu_ms = timer_for_cpu.getDurationMs();
    const qristal::Table2d<std::string> eigenstates{qaoa.get_out_eigenstates()}; //[0][0]
    const qristal::Table2d<std::map<int,double>> energies{qaoa.get_out_energys()}; //[0][0][0]
    const std::map<int,double> cost{energies[0][0]};

    EXPECT_TRUE(eigenstates[0][0] == "001");
    std::cout << "qaoa test finished.\n";
}

/* // future test once poor scaling of XACC QAOA is fixed upstream
TEST(qaoaTester, check_simple_QAP) {
    std::string pauliString {
        " + 400 Z0 Z1 + 400 Z0 Z2 + 400 Z0 Z3 +  80 Z0 Z4 + 150 Z0 Z5 + 400 Z0 Z6 +  32 Z0 Z7 +  60 Z0 Z8"
        " + 400 Z1 Z2 +  80 Z1 Z3 + 400 Z1 Z4 + 130 Z1 Z5 +  32 Z1 Z6 + 400 Z1 Z7 +  52 Z1 Z8"
        " + 150 Z2 Z3 + 130 Z2 Z4 + 400 Z2 Z5 +  60 Z2 Z6 +  52 Z2 Z7 + 400 Z2 Z8"
        " + 400 Z3 Z4 + 400 Z3 Z5 + 400 Z3 Z6 +  48 Z3 Z7 +  90 Z3 Z8"
        " + 400 Z4 Z5 +  48 Z4 Z6 + 400 Z4 Z7 +  78 Z4 Z8"
        " +  90 Z5 Z6 +  78 Z5 Z7 + 400 Z5 Z8"
        " + 400 Z6 Z7 + 400 Z6 Z8"
        " + 400 Z7 Z8"
        " - 2400"}; // " - 2400" or " + 4800 - 800 Z0 - 800 Z1 - 800 Z2 - 800 Z3 - 800 Z4 - 800 Z5 - 800 Z6 - 800 Z7 - 800 Z8" 

    std::size_t nPaulis = -1 + std::count_if(pauliString.begin(), pauliString.end(), 
                            [](char c) {return ((c=='+') || (c=='-'));});

    std::size_t nOptVars = 9; // matrixSize^2
    std::size_t nQaoaSteps = 10; 
    qristal::op::QaoaSimple qaoa{false};
    qaoa.set_ham(pauliString);
    qaoa.set_qn(nOptVars);
    qaoa.set_acc("qpp");
    qaoa.set_functol({{0,1.0e-5}});
    qaoa.set_maxeval(100); // 800
    qaoa.set_qaoa_step(nQaoaSteps);
    qristal::Table2d<std::map<int,double>> thetas{{{}}};

    bool extendedParams = false;
    std::size_t nThetas{}; 
    if (extendedParams){
        nThetas = (nOptVars+nPaulis)*nQaoaSteps;
    }else{
        nThetas =                  2*nQaoaSteps;
    }
    qaoa.set_extended_param(extendedParams);
    for (int idx=0; idx<(int)(nThetas); idx++){
        thetas[0][0][idx] = 0.25;
    }
    qaoa.set_thetas(thetas);

    qaoa.run(0,0);

    //const auto      nIters = params.energies.size();
    //const double  cpu_ms = timer_for_cpu.getDurationMs();
    const qristal::Table2d<std::string> eigenstates{qaoa.get_out_eigenstates()}; //[0][0]
    const qristal::Table2d<std::map<int,double>> energies{qaoa.get_out_energys()}; //[0][0][0]
    const std::map<int,double> cost{energies[0][0]};

    EXPECT_TRUE(eigenstates[0][0] == "100010001");
    std::cout << "qaoa test finished.\n";
}
*/

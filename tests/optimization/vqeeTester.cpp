// Copyright (c) Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include "qristal/core/optimization/vqee/vqee.hpp"

TEST(vqeeTester, checkH2_UCCSD) {
    xacc::external::load_external_language_plugins();
    xacc::set_verbose(false);
    xacc::ScopeTimer timer_for_cpu("Walltime in ms", false);

    const bool isRoot = GetRank() == 0;        
    qristal::vqee::Params params{qristal::vqee::makeJob(qristal::vqee::JobID::H2_UCCSD)}; // has all inputs for VQE

    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    qristal::vqee::VQEE vqe{params};
    vqe.optimize();

    const auto nIters = params.energies.size();
    const double  cpu_ms = timer_for_cpu.getDurationMs(); 

    double exactEnergy{-1.137275943617};
    // Be aware that pyscf Pauli produces exact energy but qiskit pauli does not include core-core interaction and must add 1/1.4 to true energy, i.e. EXPECT_NEAR(params.optimalValue, exactEnergy -1.0/1.4, 1e-3);
    EXPECT_NEAR(params.optimalValue, exactEnergy, 1e-3);
    std::cout << "vqee test finished.\n";
}


TEST(vqeeTester, checkGeometryToPauli) {
    xacc::ScopeTimer timer_for_cpu("Walltime in ms", false);

    const bool isRoot = GetRank() == 0;        

    // start with default object
    qristal::vqee::Params params{};
    params.nQubits=4;
    params.maxIters=100;

    // modify the pauli terms
    std::string geometry = qristal::vqee::hydrogenChainGeometry(2); // gives in Angstrom: "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486"
    std::cout << geometry << std::endl;
    params.pauliString = qristal::vqee::pauliStringFromGeometry(geometry, "sto-3g");
    std::cout << params.pauliString << std::endl;

    // set ansatz again
    std::size_t nOptParams = qristal::vqee::setAnsatz(params, qristal::vqee::AnsatzID::UCCSD, params.nQubits, params.nQubits/2);
    params.theta = std::vector<double>(nOptParams, 0.1);

    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    qristal::vqee::VQEE vqe{params};
    vqe.optimize();

    const auto      nIters = params.energies.size();
    const double  cpu_ms = timer_for_cpu.getDurationMs(); 
    
    double exactEnergy{-1.137275943617};
    EXPECT_NEAR(params.optimalValue, exactEnergy, 1e-3); // pyscf Pauli produces exact energy
    std::cout << "vqee test finished.\n";
}

TEST(vqeeTester, check_direct_expectation) {
    const bool isRoot = GetRank() == 0;        
    qristal::vqee::Params params{};
    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    params.nQubits = 4;
    params.maxIters = 30;
    params.acceleratorName = "qpp";

    // modify the pauli terms
    std::string geometry = qristal::vqee::hydrogenChainGeometry(2); // gives in Angstrom: "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486"
    std::cout << geometry << std::endl;
    params.pauliString = qristal::vqee::pauliStringFromGeometry(geometry, "sto-3g");
    std::cout << params.pauliString << std::endl;
    double exactEnergy{-1.137275943617};

    // set ansatz 
    std::size_t nOptParams = qristal::vqee::setAnsatz(params, qristal::vqee::AnsatzID::UCCSD, params.nQubits, params.nQubits/2);
    
    // Execute standard VQE (expectation from shot sampling)
    params.theta = std::vector<double>(nOptParams, 0.1);
    params.isDeterministic = false;
    params.nShots = 10000;

    xacc::ScopeTimer sample_timer_for_cpu("Sampling expectation - Walltime in ms", false);
    qristal::vqee::VQEE vqe{params};
    vqe.optimize();
    const double sample_cpu_ms = sample_timer_for_cpu.getDurationMs();

    // Execute VQE with direct expectation
    params.theta = std::vector<double>(nOptParams, 0.1);
    params.isDeterministic = true;
    
    xacc::ScopeTimer direct_timer_for_cpu("Direct expectation - Walltime in ms", false);
    qristal::vqee::VQEE vqe_direct{params};
    vqe_direct.optimize();
    const double direct_cpu_ms = direct_timer_for_cpu.getDurationMs();

    EXPECT_NEAR(params.optimalValue, exactEnergy, 1e-3);
    ASSERT_LT(direct_cpu_ms, sample_cpu_ms);
}

TEST(vqeeTester, check_nelder_mead_stopval) {
    const bool isRoot = GetRank() == 0;        
    qristal::vqee::Params params{};
    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    params.nQubits = 4;
    params.maxIters = 1024;
    params.acceleratorName = "qpp";

    // modify the pauli terms
    std::string geometry = qristal::vqee::hydrogenChainGeometry(2); // gives in Angstrom: "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486"
    params.pauliString = qristal::vqee::pauliStringFromGeometry(geometry, "sto-3g");
    double stopEnergy{-1.05};

    // set ansatz 
    std::size_t nOptParams = qristal::vqee::setAnsatz(params, qristal::vqee::AnsatzID::UCCSD, params.nQubits, params.nQubits/2);
    
    // set Nelder-Mead with stopval
    params.algorithm = "nelder-mead";
    params.extraOptions = "stopval: -1.05";

    // Execute standard VQE (expectation from shot sampling)
    params.theta = std::vector<double>(nOptParams, 0.1);
    params.isDeterministic = false;
    params.nShots = 10000;
    
    qristal::vqee::VQEE vqe{params};
    vqe.optimize();
    EXPECT_NEAR(params.optimalValue, stopEnergy, 5e-2);
}

TEST(vqeeTester, check_nelder_mead_theta_lowerb) {
    const bool isRoot = GetRank() == 0;        
    qristal::vqee::Params params{};
    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    params.nQubits = 4;
    params.maxIters = 64;
    params.acceleratorName = "qpp";

    // modify the pauli terms
    std::string geometry = qristal::vqee::hydrogenChainGeometry(2); // gives in Angstrom: "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486"
    params.pauliString = qristal::vqee::pauliStringFromGeometry(geometry, "sto-3g");

    // set ansatz 
    std::size_t nOptParams = qristal::vqee::setAnsatz(params, qristal::vqee::AnsatzID::UCCSD, params.nQubits, params.nQubits/2);
    
    // set Nelder-Mead
    params.algorithm = "nelder-mead";
    params.extraOptions = "lowerbounds: [0.0, 0.0, 0.0]";

    // Execute standard VQE (expectation from shot sampling)
    params.theta = std::vector<double>(nOptParams, 0.1);
    params.isDeterministic = false;
    params.nShots = 10000;
    
    qristal::vqee::VQEE vqe{params};
    vqe.optimize();
    ASSERT_LE(0.0, params.theta.at(0));
    ASSERT_LE(0.0, params.theta.at(1));
    ASSERT_LE(0.0, params.theta.at(2));
}

TEST(vqeeTester, check_nelder_mead_theta_upperb) {
    const bool isRoot = GetRank() == 0;        
    qristal::vqee::Params params{};
    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    params.nQubits = 4;
    params.maxIters = 64;
    params.acceleratorName = "qpp";

    // modify the pauli terms
    std::string geometry = qristal::vqee::hydrogenChainGeometry(2); // gives in Angstrom: "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486"
    params.pauliString = qristal::vqee::pauliStringFromGeometry(geometry, "sto-3g");

    // set ansatz 
    std::size_t nOptParams = qristal::vqee::setAnsatz(params, qristal::vqee::AnsatzID::UCCSD, params.nQubits, params.nQubits/2);
    
    // set Nelder-Mead
    params.algorithm = "nelder-mead";
    params.extraOptions = "upperbounds: [0.02, 0.02, 0.02]";

    // Execute standard VQE (expectation from shot sampling)
    params.theta = std::vector<double>(nOptParams, 0.001);
    params.isDeterministic = false;
    params.nShots = 10000;
    
    qristal::vqee::VQEE vqe{params};
    vqe.optimize();
    ASSERT_LE(params.theta.at(0), 0.02);
    ASSERT_LE(params.theta.at(1), 0.02);
    ASSERT_LE(params.theta.at(2), 0.02);
}

TEST(vqeeTester, adam_checkH2_UCCSD) {
    const bool isRoot = GetRank() == 0;        
    qristal::vqee::Params params{qristal::vqee::makeJob(qristal::vqee::JobID::H2_UCCSD)}; // has all inputs for VQE

    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    // set ADAM
    params.algorithm = "adam";
    params.extraOptions = "{stepsize: 0.1, beta1: 0.67, beta2: 0.9, momentum: 0.11, exactobjective: true}";

    qristal::vqee::VQEE vqe{params};
    vqe.optimize();

    double exactEnergy{-1.137275943617};
    EXPECT_NEAR(params.optimalValue, exactEnergy, 1e-3);
}

TEST(vqeeTester, lbfgs_checkH2_UCCSD) {
    const bool isRoot = GetRank() == 0;        
    qristal::vqee::Params params{qristal::vqee::makeJob(qristal::vqee::JobID::H2_UCCSD)}; // has all inputs for VQE

    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    // set L-BFGS
    params.algorithm = "l-bfgs";
    params.isDeterministic = true;
    qristal::vqee::VQEE vqe{params};
    vqe.optimize();

    double exactEnergy{-1.137275943617};
    EXPECT_NEAR(params.optimalValue, exactEnergy, 1e-3);
}

TEST(vqeeTester, cmaes_checkH2_UCCSD) {
    const bool isRoot = GetRank() == 0;        
    qristal::vqee::Params params{qristal::vqee::makeJob(qristal::vqee::JobID::H2_UCCSD)}; // has all inputs for VQE

    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    // set L-BFGS
    params.algorithm = "cmaes";
    // Reverse upper and lower : see https://github.com/eclipse/xacc/issues/574
    params.extraOptions = "{upper: -10.0, lower: 10.0}";

    qristal::vqee::VQEE vqe{params};
    vqe.optimize();

    double exactEnergy{-1.137275943617};
    EXPECT_NEAR(params.optimalValue, exactEnergy, 1e-3);    
}

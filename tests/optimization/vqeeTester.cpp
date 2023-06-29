// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include "qb/core/optimization/vqee/vqee.hpp"

TEST(vqeeTester, checkH2_UCCSD) {
    // needs to call xacc::Initialize(); to allow single testing. Must be adapted in all tests. Cannot be called twice without xacc::Finalize(); in between
    xacc::Initialize(); //xacc::Initialize(argc, argv);
    xacc::external::load_external_language_plugins();
    xacc::set_verbose(false);
    xacc::ScopeTimer timer_for_cpu("Walltime in ms", false);

    const bool isRoot = GetRank() == 0;        
    qb::vqee::Params params{qb::vqee::makeJob(qb::vqee::JobID::H2_UCCSD)}; // has all inputs for VQE

    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    qb::vqee::VQEE vqe{params};
    vqe.optimize();

    const auto 	    nIters = params.energies.size();
    const double 	cpu_ms = timer_for_cpu.getDurationMs(); 

    double exactEnergy{-1.137275943617};
    // Be aware that pyscf Pauli produces exact energy but qiskit pauli does not include core-core interaction and must add 1/1.4 to true energy, i.e. EXPECT_NEAR(params.optimalValue, exactEnergy -1.0/1.4, 1e-3);
    EXPECT_NEAR(params.optimalValue, exactEnergy, 1e-3);
    std::cout << "vqee test finished.\n";
    // this would actually need to call xacc::Finalize(); to allow single testing. Must be adapted in all tests
}


TEST(vqeeTester, checkGeometryToPauli) {
    // this would actually need to call xacc::Initialize(); to allow single testing. Must be adapted in all tests
    xacc::ScopeTimer timer_for_cpu("Walltime in ms", false);

    const bool isRoot = GetRank() == 0;        

    // start with default object
    qb::vqee::Params params{};
    params.nQubits=4;
    params.maxIters=100;

    // modify the pauli terms
    std::string geometry = qb::vqee::hydrogenChainGeometry(2); // gives in Angstrom: "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486"
    std::cout << geometry << std::endl;
    params.pauliString = qb::vqee::pauliStringFromGeometry(geometry, "sto-3g");
    std::cout << params.pauliString << std::endl;

    // set ansatz again
    std::size_t nOptParams = qb::vqee::setAnsatz(params, qb::vqee::AnsatzID::UCCSD, params.nQubits, params.nQubits/2);
    params.theta = std::vector<double>(nOptParams, 0.1);

    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    qb::vqee::VQEE vqe{params};
    vqe.optimize();

    const auto 	    nIters = params.energies.size();
    const double 	cpu_ms = timer_for_cpu.getDurationMs(); 
    
    double exactEnergy{-1.137275943617};
    EXPECT_NEAR(params.optimalValue, exactEnergy, 1e-3); // pyscf Pauli produces exact energy
    std::cout << "vqee test finished.\n";
    // this would actually need to call xacc::Finalize(); to allow single testing. Must be adapted in all tests
}

TEST(vqeeTester, check_direct_expectation) {
    const bool isRoot = GetRank() == 0;        
    qb::vqee::Params params{};
    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;

    params.nQubits = 4;
    params.maxIters = 30;
    params.acceleratorName = "qpp";

    // modify the pauli terms
    std::string geometry = qb::vqee::hydrogenChainGeometry(2); // gives in Angstrom: "H 0.0 0.0 0.0; H 0.0 0.0 0.7408481486"
    std::cout << geometry << std::endl;
    params.pauliString = qb::vqee::pauliStringFromGeometry(geometry, "sto-3g");
    std::cout << params.pauliString << std::endl;
    double exactEnergy{-1.137275943617};

    // set ansatz 
    std::size_t nOptParams = qb::vqee::setAnsatz(params, qb::vqee::AnsatzID::UCCSD, params.nQubits, params.nQubits/2);
    
    // Execute standard VQE (expectation from shot sampling)
    params.theta = std::vector<double>(nOptParams, 0.1);
    params.isDeterministic = false;
    params.nShots = 10000;

    xacc::ScopeTimer sample_timer_for_cpu("Sampling expectation - Walltime in ms", false);
    qb::vqee::VQEE vqe{params};
    vqe.optimize();
    const double sample_cpu_ms = sample_timer_for_cpu.getDurationMs();

    // Execute VQE with direct expectation
    params.theta = std::vector<double>(nOptParams, 0.1);
    params.isDeterministic = true;
    
    xacc::ScopeTimer direct_timer_for_cpu("Direct expectation - Walltime in ms", false);
    qb::vqee::VQEE vqe_direct{params};
    vqe_direct.optimize();
    const double direct_cpu_ms = direct_timer_for_cpu.getDurationMs();

    EXPECT_NEAR(params.optimalValue, exactEnergy, 1e-3);
    ASSERT_LT(direct_cpu_ms, sample_cpu_ms);
}

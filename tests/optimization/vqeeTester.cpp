// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include "qb/core/optimization/vqee/vqee.hpp"

TEST(vqeeTester, checkH2_UCCSD) {
    xacc::Initialize(); //xacc::Initialize(argc, argv);
    xacc::external::load_external_language_plugins();
    xacc::set_verbose(false);
    xacc::ScopeTimer timer_for_cpu("Walltime in ms", false);

    const bool isRoot = GetRank() == 0;        
    qb::vqee::Params params{qb::vqee::makeJob(qb::vqee::JobID::H2_UCCSD)}; // has all inputs for VQE

    params.nWorker = GetSize();
    params.nThreadsPerWorker = 1;
    //params.partitioned = true; // enable for cases with many Pauli-terms

    qb::vqee::VQEE vqe{params};
    vqe.optimize();

    const auto 	    nIters = params.energies.size();
    const double 	cpu_ms = timer_for_cpu.getDurationMs(); 
/*  
    if (isRoot){  
        std::cout << "theta: " << params.theta << ", energy: " << params.optimalValue << ", iterations: " << nIters << ", CPU wall-time: " << cpu_ms << " ms" << std::endl;
    }
*/  
    EXPECT_NEAR(params.optimalValue, -1.851, 1e-3);
    std::cout << "vqee test finished.\n";
    //xacc::Finalize();
}

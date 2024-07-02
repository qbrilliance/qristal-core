// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <iostream>
#include <filesystem>
#include <sstream>

#include "qb/core/session.hpp"
#include "qb/core/noise_model/noise_model.hpp"
#include "qb/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qb/core/benchmark/workflows/RotationSweep.hpp"
#include "qb/core/benchmark/metrics/CircuitFidelity.hpp"

using namespace qb::benchmark;


TEST(CircuitFidelityTester, checkSPAM) {
    const std::vector<size_t> qubits{0, 1};

    //define session  
    qb::session sim(false); 
    sim.qb12();
    sim.set_acc("qpp");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow 
    SPAMBenchmark workflow(qubits, sim);

    //evaluate metric 
    CircuitFidelity<SPAMBenchmark> metric(workflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 1e-2);
        }
    }
}

TEST(CircuitFidelityTester, checkRotationSweep) {
    const std::vector<size_t> qubits{0, 1, 2};

    //define session  
    qb::session sim(false); 
    sim.qb12();
    sim.set_acc("qpp");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow 
    RotationSweep workflow(
        std::vector<char>{'Z', 'X', 'Y'}, 
        -90,
        +90,
        9, 
        sim
    );

    //evaluate metric 
    CircuitFidelity<RotationSweep> metric(workflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 1e-2);
        }
    }
}
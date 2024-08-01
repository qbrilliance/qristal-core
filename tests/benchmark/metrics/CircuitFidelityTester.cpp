// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <iostream>
#include <filesystem>
#include <sstream>

#include "qristal/core/session.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/workflows/RotationSweep.hpp"
#include "qristal/core/benchmark/metrics/CircuitFidelity.hpp"

using namespace qristal::benchmark;


TEST(CircuitFidelityTester, checkSPAM) {
    const std::set<size_t> qubits{0, 1};

    //define session
    qristal::session sim(false);
    sim.init();
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
    //define session
    qristal::session sim(false);
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(1000000);
    sim.set_qn(3);

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

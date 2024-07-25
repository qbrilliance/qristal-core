// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <iostream>
#include <filesystem>
#include <sstream>

#include "qb/core/session.hpp"
#include "qb/core/noise_model/noise_model.hpp"
#include "qb/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qb/core/benchmark/workflows/RotationSweep.hpp"
#include "qb/core/benchmark/workflows/QuantumStateTomography.hpp"
#include "qb/core/benchmark/metrics/QuantumStateFidelity.hpp"

using namespace qb::benchmark;

TEST(QuantumStateFidelityTester, checkSPAM) {
    const std::set<size_t> qubits{0, 1};

    //define session
    qb::session sim(false);
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow
    SPAMBenchmark workflow(qubits, sim);
    QuantumStateTomography<SPAMBenchmark> qstworkflow(workflow);

    //evaluate metric
    QuantumStateFidelity<QuantumStateTomography<SPAMBenchmark>> metric(qstworkflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 1e-2);
        }
    }
}

TEST(QuantumStateFidelityTester, checkRotationSweep) {
    const std::set<size_t> qubits{0, 1};

    //define session
    qb::session sim(false);
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow
    RotationSweep workflow(
        std::vector<char>{'Y', 'Z'},
        -90,
        +90,
        6,
        sim
    );
    QuantumStateTomography<RotationSweep> qstworkflow(workflow, qubits);

    //evaluate metric
    QuantumStateFidelity<QuantumStateTomography<RotationSweep>> metric(qstworkflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 1e-2);
        }
    }
}

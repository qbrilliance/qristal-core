// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include <qristal/core/session.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/PreOrAppendWorkflow.hpp>
#include <qristal/core/benchmark/metrics/BitstringCounts.hpp>

// range v3
#include <range/v3/view/zip.hpp>

// Gtest
#include <gtest/gtest.h>

using namespace qristal::benchmark;


TEST(BitstringCountsTester, checkSPAM) {
    const std::set<size_t> qubits{0, 1};
    const size_t n_shots = 1000;

    //define session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = n_shots;
    sim.qn = qubits.size();

    //define workflow
    SPAMBenchmark workflow(qubits, sim);

    //evaluate metric and check against ideal 
    std::vector<std::vector<bool>> bitstrings {{0,0}, {1,0}, {0,1}, {1,1}};
    BitstringCounts<SPAMBenchmark> metric(workflow);
    std::map<std::time_t, std::vector<std::map<std::vector<bool>, int>>> results = metric.evaluate(true); //optional bool will force new execution
    for (const auto& [counts, ideal_bitstring] : ::ranges::views::zip(results.begin()->second, bitstrings)) {
        EXPECT_EQ(counts[ideal_bitstring], n_shots);
    }
}

TEST(BitstringCountsTester, checkRotationSweep) {
    const size_t n_shots = 1000;

    //define session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = n_shots;
    sim.qn = 1;

    //define workflow for x rotation sweep pre and appended by ry(pi/2)
    RotationSweep workflow(
        std::vector<char>{'X'},
        -180,
        +180,
        9,
        sim
    );
    std::vector<qristal::Pauli> paulis{qristal::Pauli::Symbol::X};
    PreOrAppendWorkflow<RotationSweep> prepended_workflow(workflow, paulis, Placement::Prepend);
    PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>> final_workflow(prepended_workflow, paulis, Placement::Append);

    //evaluate metric
    //expected behavior: Ry(pi/2) moves 1-qubit state to X-eigenstate, Rx(theta) only applies a global phase, Ry(pi/2) moves the state further to |1> 
    BitstringCounts<PreOrAppendWorkflow<PreOrAppendWorkflow<RotationSweep>>> metric(final_workflow);
    std::map<std::time_t, std::vector<std::map<std::vector<bool>, int>>> results = metric.evaluate(true); //optional bool will force new execution
    for (const auto& counts : results.begin()->second) {
        EXPECT_EQ(counts.at({true}), n_shots);
    }
}

// Copyright (c) Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <iostream>
#include <filesystem>
#include <sstream>

#include "qristal/core/session.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/workflows/RotationSweep.hpp"
#include "qristal/core/benchmark/metrics/QuantumProcessFidelity.hpp"

using namespace qristal::benchmark;

TEST(QuantumProcessFidelityTester, checkSPAM) {
    const std::set<size_t> qubits{0};

    //define session  
    qristal::session sim(false); 
    sim.init();
    sim.set_acc("qsim");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow 
    SPAMBenchmark workflow(qubits, sim);
    using QST = QuantumStateTomography<SPAMBenchmark>;
    QST qstworkflow(workflow); //wrap circuit execution into QST object
    using QPT = QuantumProcessTomography<QST>;
    QPT qptworkflow(qstworkflow); //wrap QST into QPT object

    //evaluate metric 
    QuantumProcessFidelity<QPT> metric(qptworkflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 1e-2);
        }
    }
}

TEST(QuantumProcessFidelityTester, checkRotationSweep) {
    const std::set<size_t> qubits{0, 1};

    //define session  
    qristal::session sim(false); 
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow 
    RotationSweep workflow(
        std::vector<char>{'Y', 'X'}, 
        -90,
        +90,
        2, 
        sim
    );
    using QST = QuantumStateTomography<RotationSweep>;
    QST qstworkflow(workflow, qubits); //wrap circuit execution into QST object
    using QPT = QuantumProcessTomography<QST>;
    QPT qptworkflow(qstworkflow); //wrap QST into QPT object

    //evaluate metric 
    QuantumProcessFidelity<QPT> metric(qptworkflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 1e-2);
        }
    }
}
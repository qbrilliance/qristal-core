// Copyright (c) Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <unistd.h>

#include <qristal/core/session.hpp>

#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/QuantumStateTomography.hpp>
#include <qristal/core/benchmark/workflows/QuantumProcessTomography.hpp>
#include <qristal/core/benchmark/workflows/RuntimeAnalyzer.hpp>

#include <qristal/core/benchmark/metrics/CircuitFidelity.hpp>
#include <qristal/core/benchmark/metrics/QuantumStateDensity.hpp>
#include <qristal/core/benchmark/metrics/QuantumProcessMatrix.hpp>

using namespace qristal::benchmark;

TEST(RuntimeAnalyzerTester, checkcppuprofileinstantiate) {
    std::stringstream ss;
    ss << "cppuprofile_test.out";
    const size_t interval = 500; //in milliseconds
    const size_t profiling_time = 3; // in seconds

    EXPECT_NO_THROW(uprofile::start(ss.str().c_str()));
    EXPECT_NO_THROW(uprofile::startProcessMemoryMonitoring(interval));
    EXPECT_NO_THROW(uprofile::startSystemMemoryMonitoring(interval));
    EXPECT_NO_THROW(uprofile::startCPUUsageMonitoring(interval));
    #ifdef GPU_MONITOR_NVIDIA
        EXPECT_NO_THROW(uprofile::addGPUMonitor(new uprofile::NvidiaMonitor));
        EXPECT_NO_THROW(uprofile::startGPUMemoryMonitoring(interval));
        EXPECT_NO_THROW(uprofile::startGPUUsageMonitoring(interval));
    #endif()

    sleep(profiling_time); //dummy "workload"

    EXPECT_NO_THROW(uprofile::stop());
}

TEST(RuntimeAnalyzerTester, checkSPAM) {
    const std::set<size_t> qubits{0, 1};

    //define session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = 1000000;
    sim.qn = qubits.size();

    //define workflow
    SPAMBenchmark workflow(qubits, sim);
    // wrap into RuntimeAnalyzer and set profiling interval
    const size_t profiling_interval_ms = 500;
    RuntimeAnalyzer<SPAMBenchmark> wrapped_workflow(workflow, profiling_interval_ms);

    //evaluate metric
    CircuitFidelity<RuntimeAnalyzer<SPAMBenchmark>> metric(wrapped_workflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 1e-3);
        }
    }
}

TEST(RuntimeAnalyzerTester, checkRotationSweep) {
    const std::vector<size_t> qubits{0, 1, 2};

    //define session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = 1000000;
    sim.qn = qubits.size();

    //define workflow
    RotationSweep workflow(
        std::vector<char>{'Z', 'X', 'Y'},
        -90,
        +90,
        9,
        sim
    );
    // wrap into RuntimeAnalyzer and set profiling interval
    const size_t profiling_interval_ms = 500;
    RuntimeAnalyzer<RotationSweep> wrapped_workflow(workflow, profiling_interval_ms);

    //evaluate metric
    CircuitFidelity<RuntimeAnalyzer<RotationSweep>> metric(wrapped_workflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 1e-3);
        }
    }
}

TEST(RuntimeAnalyzerTester, checkQST) {
    Eigen::MatrixXcd ideal_density(4, 4); //ideal Bell state density
    ideal_density << 0.5, 0.0, 0.0, 0.5,
                     0.0, 0.0, 0.0, 0.0,
                     0.0, 0.0, 0.0, 0.0,
                     0.5, 0.0, 0.0, 0.5;

    const size_t n_qubits = 2;
    const size_t n_shots = 1000000;

    // define session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = n_shots;
    sim.qn = n_qubits;

    // define workflow
    qristal::CircuitBuilder circuit;
    circuit.H(0);
    circuit.CNOT(0, 1);
    SimpleCircuitExecution workflow(
        std::vector<qristal::CircuitBuilder>{circuit},
        sim
    );
    QuantumStateTomography<SimpleCircuitExecution> qstworkflow(workflow); //wrap into QST object
    // wrap into RuntimeAnalyzer and set profiling interval
    const size_t profiling_interval_ms = 500;
    RuntimeAnalyzer<QuantumStateTomography<SimpleCircuitExecution>> wrapped_workflow(qstworkflow, profiling_interval_ms);

    //(3) pass to metric
    QuantumStateDensity<RuntimeAnalyzer<QuantumStateTomography<SimpleCircuitExecution>>> metric(wrapped_workflow);

    //(4) evaluate and check
    auto density = metric.evaluate(true).begin()->second.front(); //optional bool will enforce new execution
    EXPECT_TRUE(density.isApprox(ideal_density, 1e-2));
}

TEST(RuntimeAnalyzerTester, checkQPT) {
    /*
    assemble ideal CNOT process:
    CNOT = 1/2 ( II + IX + ZI - ZX ) * rho * 1/2 ( II + IX + ZI - ZX )

    = 1/4 * (
        IIrII + IIrIX + IIrZI - IIrZX +
        IXrII + IXrIX + IXrZI - IXrZX +
        ZIrII + ZIrIX + ZIrZI - ZIrZX +
        -ZXrII - ZXrIX - ZXrZI + ZXrZX
    )

    with indices: II : 0; IX : 1; ZI: 12; ZX: 13
    */
    Eigen::MatrixXcd ideal_process = Eigen::MatrixXcd::Zero(16, 16);
    ideal_process(0,0) = 0.25;
    ideal_process(0,1) = 0.25;
    ideal_process(0,12) = 0.25;
    ideal_process(0,13) = -0.25;

    ideal_process(1,0) = 0.25;
    ideal_process(1,1) = 0.25;
    ideal_process(1,12) = 0.25;
    ideal_process(1,13) = -0.25;

    ideal_process(12,0) = 0.25;
    ideal_process(12,1) = 0.25;
    ideal_process(12,12) = 0.25;
    ideal_process(12,13) = -0.25;

    ideal_process(13,0) = -0.25;
    ideal_process(13,1) = -0.25;
    ideal_process(13,12) = -0.25;
    ideal_process(13,13) = 0.25;

    const size_t n_qubits = 2;
    const size_t n_shots = 1000000;

    //(1) define session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = n_shots;
    sim.qn = n_qubits;

    //(2) define workflow
    qristal::CircuitBuilder circuit;
    circuit.CNOT(1, 0); //right to left odering of qubit indices!
    SimpleCircuitExecution workflow(
        std::vector<qristal::CircuitBuilder>{circuit},
        sim
    );
    using QST = QuantumStateTomography<SimpleCircuitExecution>;
    QST qstworkflow(workflow); //wrap circuit execution into QST object
    using QPT = QuantumProcessTomography<QST>;
    QPT qptworkflow(qstworkflow); //wrap QST into QPT object
    // wrap into RuntimeAnalyzer and set profiling interval
    const size_t profiling_interval_ms = 500;
    RuntimeAnalyzer<QPT> wrapped_workflow(qptworkflow, profiling_interval_ms);

    //(3) pass to metric
    QuantumProcessMatrix<RuntimeAnalyzer<QPT>> metric(wrapped_workflow);

    //(4) evaluate and check
    auto process = metric.evaluate(true).begin()->second.front(); //optional bool will enforce new execution
    EXPECT_TRUE(process.isApprox(ideal_process, 1e-2));
}

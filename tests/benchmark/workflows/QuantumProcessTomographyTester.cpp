// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <iostream>

#include "qb/core/session.hpp"
#include "qb/core/noise_model/noise_model.hpp"
#include "qb/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qb/core/benchmark/workflows/RotationSweep.hpp"
#include "qb/core/benchmark/workflows/QuantumProcessTomography.hpp"
#include "qb/core/benchmark/workflows/SimpleCircuitExecution.hpp"
#include "qb/core/benchmark/DataLoaderGenerator.hpp"

using namespace qb::benchmark;

TEST(QuantumProcessTomographyTester, checkSPAM) {

    //create folder for intermediate benchmark results (required because DataLoaderGenerator is not used here!)
    if ( std::filesystem::exists(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME)) == false ){
        std::filesystem::create_directory(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME));
    }

    const std::set<size_t> qubits = {0};

    //define session  
    qb::session sim(false); 
    sim.init();
    sim.set_acc("qsim");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow and execute
    SPAMBenchmark workflow(qubits, sim);
    using QST = QuantumStateTomography<SPAMBenchmark>;
    QST qstworkflow(workflow);
    using QPT = QuantumProcessTomography<QST>;
    QPT qptworkflow(qstworkflow);
    std::time_t t = qptworkflow.execute(std::vector<Task>{Task::MeasureCounts, Task::IdealProcess}); //let QPT store all measurement results and all ideal processes

    //since data generation and loading are completely separated in qb::benchmark, 
    //a DataLoaderGenerator is required to load in the measured counts
    DataLoaderGenerator dlg(qptworkflow.get_identifier(), std::vector<Task>{Task::MeasureCounts, Task::IdealProcess});
    dlg.set_timestamps(std::vector<std::time_t>{t}); //manually load in correct timestamp
    auto counts = dlg.obtain_measured_counts()[0]; //and obtain measurements + ideal processes (the 0 indexing is required because we only need one timestamp!)
    auto ideal_processes = dlg.obtain_ideal_processes()[0];
    
    //assemble the densities
    auto measured_densities = qptworkflow.get_qst().assemble_densities(counts);
    //assemble processes using the densities
    auto measured_processes = qptworkflow.assemble_processes(measured_densities);

    //test if measured processes match the ideal ones 
    for (size_t i = 0; i < measured_processes.size(); ++i) {
        EXPECT_TRUE(ideal_processes[i].isApprox(measured_processes[i], 1e-2));
    }
}

TEST(QuantumProcessTomographyTester, checkRotationSweep) {

    //create folder for intermediate benchmark results (required because DataLoaderGenerator is not used here!)
    if ( std::filesystem::exists(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME)) == false ){
        std::filesystem::create_directory(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME));
    }

    const std::set<size_t> qubits{0};

    //define session  
    qb::session sim(false); 
    sim.init();
    sim.set_acc("aer");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow 
    RotationSweep workflow(
        std::vector<char>({'X'}), //specific rotations applied
        -90, //start (deg)
        90,  //end (deg)
        4,   //points
        sim
    );
    using QST = QuantumStateTomography<RotationSweep>;
    QST qstworkflow(workflow, qubits);
    using QPT = QuantumProcessTomography<QST>;
    QPT qptworkflow(qstworkflow);
    std::time_t t = qptworkflow.execute(std::vector<Task>{Task::MeasureCounts, Task::IdealProcess}); //let QPT store all measurement results and all ideal processes

    //since data generation and loading are completely separated in qb::benchmark, 
    //a DataLoaderGenerator is required to load in the measured counts
    DataLoaderGenerator dlg(qptworkflow.get_identifier(), std::vector<Task>{Task::MeasureCounts, Task::IdealProcess});
    dlg.set_timestamps(std::vector<std::time_t>{t}); //manually load in correct timestamp
    auto counts = dlg.obtain_measured_counts()[0]; //and obtain measurements + ideal processes (the 0 indexing is required because we only need one timestamp!)
    auto ideal_processes = dlg.obtain_ideal_processes()[0];
    
    //assemble the densities
    auto measured_densities = qptworkflow.get_qst().assemble_densities(counts);
    //assemble processes using the densities
    auto measured_processes = qptworkflow.assemble_processes(measured_densities);

    //test if measured processes match the ideal ones 
    for (size_t i = 0; i < measured_processes.size(); ++i) {
        EXPECT_TRUE(ideal_processes[i].isApprox(measured_processes[i], 1e-2));
    }
}


TEST(QuantumProcessTomographyTester, checkSimpleCircuitExecution) {
    //purpose of the test: compute process matrices for two equivalent circuits (here Controlled-phase gate vs. 
    //transpiled to the Rx, Ry, CZ gate set) and check for equivalence. 
    //create folder for intermediate benchmark results (required because DataLoaderGenerator is not used here!)
    if ( std::filesystem::exists(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME)) == false ){
        std::filesystem::create_directory(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME));
    }

    const std::set<size_t> qubits{0, 1};

    //define session  
    qb::session sim(false); 
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define circuits 
    const double angle = std::numbers::pi;
    qb::CircuitBuilder circuit_native_CP, circuit_transpiled_CP; 
    //(1) build native CP circuit
    circuit_native_CP.CPhase(0, 1, angle);
    //(2) build transpiled CP circuit (to native gate set)
    circuit_transpiled_CP.RX(0, std::numbers::pi / 2.0);
    circuit_transpiled_CP.RY(0, -1.0 * angle / 2.0);
    circuit_transpiled_CP.RX(0, -1.0 * std::numbers::pi / 2.0);
    circuit_transpiled_CP.RY(1, std::numbers::pi / 2.0);
    circuit_transpiled_CP.RX(1, std::numbers::pi);
    circuit_transpiled_CP.CZ(0, 1);
    circuit_transpiled_CP.RX(1, -1.0 * angle / 2.0);
    circuit_transpiled_CP.CZ(0, 1);
    double lambda = (angle < 0.0 ? -1.0 : 1.0) * ( std::fabs(angle) / 2.0 - std::numbers::pi );
    circuit_transpiled_CP.RX(1, lambda);
    circuit_transpiled_CP.RY(1, -1.0 * std::numbers::pi / 2.0);

    //define workflow (wrap circuit in SimpleCircuitExecution object) 
    SimpleCircuitExecution workflow(
        std::vector<qb::CircuitBuilder>{circuit_native_CP, circuit_transpiled_CP},
        sim
    );
    using QST = QuantumStateTomography<SimpleCircuitExecution>;
    QST qstworkflow(workflow);
    using QPT = QuantumProcessTomography<QST>;
    QPT qptworkflow(qstworkflow);
    std::time_t t = qptworkflow.execute(std::vector<Task>{Task::MeasureCounts}); //let QPT store all measurement results and all ideal processes

    //since data generation and loading are completely separated in qb::benchmark, 
    //a DataLoaderGenerator is required to load in the measured counts
    DataLoaderGenerator dlg(qptworkflow.get_identifier(), std::vector<Task>{Task::MeasureCounts});
    dlg.set_timestamps(std::vector<std::time_t>{t}); //manually load in correct timestamp
    auto counts = dlg.obtain_measured_counts()[0]; //and obtain measurements + ideal processes (the 0 indexing is required because we only need one timestamp!)
    
    //assemble the densities
    auto measured_densities = qptworkflow.get_qst().assemble_densities(counts);
    //assemble processes using the densities
    auto measured_processes = qptworkflow.assemble_processes(measured_densities);

    EXPECT_TRUE(measured_processes[0].isApprox(measured_processes[1], 1e-2));

}
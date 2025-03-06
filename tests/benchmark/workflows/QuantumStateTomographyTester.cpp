// Copyright (c) Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <iostream>

#include "qristal/core/session.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/workflows/RotationSweep.hpp"
#include "qristal/core/benchmark/workflows/QuantumStateTomography.hpp"
#include "qristal/core/benchmark/DataLoaderGenerator.hpp"

using namespace qristal::benchmark;

TEST(QuantumStateTomographyTester, checkSPAM) {
    //create folder for intermediate benchmark results (required because DataLoaderGenerator is not used here!)
    if ( std::filesystem::exists(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME)) == false ){
        std::filesystem::create_directory(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME));
    }

    const std::set<size_t> qubits = {0, 1, 2};

    //define session
    qristal::session sim(false);
    sim.init();
    sim.set_acc("qsim");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow
    SPAMBenchmark workflow(qubits, sim);

    //wrap into QST workflow
    const std::set<size_t> measure_QST_qubits{0, 2};
    QuantumStateTomography<SPAMBenchmark> qst(workflow, measure_QST_qubits);
    std::time_t t = qst.execute(std::vector<Task>{Task::MeasureCounts});

    //since data generation and loading are completely separated in qristal::benchmark,
    //a DataLoaderGenerator is required to load in the measured counts
    DataLoaderGenerator dlg(qst.get_identifier(), std::vector<Task>{Task::MeasureCounts});
    dlg.set_timestamps(std::vector<std::time_t>{t}); //manually load in correct timestamp
    auto counts = dlg.obtain_measured_counts(); //and obtain measurements

    //3 qubit SPAM should generate the following 8 workflow circuits:
    //000, 001, 010, 011, 100, 101, 110, 111
    //measuring the tomography of qubits 0 and 2 should result in the density matrices of states
    //00, 01, 00, 01, 10, 11, 10, 11

    //check that this is indeed the case
    std::vector<ComplexMatrix> exact_densities(8, ComplexMatrix::Zero(4, 4));
    exact_densities[0](0, 0) = 1.0;
    exact_densities[1](1, 1) = 1.0;
    exact_densities[2](0, 0) = 1.0;
    exact_densities[3](1, 1) = 1.0;
    exact_densities[4](2, 2) = 1.0;
    exact_densities[5](3, 3) = 1.0;
    exact_densities[6](2, 2) = 1.0;
    exact_densities[7](3, 3) = 1.0;

    auto measured_densities = qst.assemble_densities(counts[0]);

    for (size_t i = 0; i < exact_densities.size(); ++i) {
        EXPECT_TRUE(exact_densities[i].isApprox(measured_densities[i], 1e-2));
    }
}

TEST(QuantumStateTomographyTester, checkRotationSweep) {
    //create folder for intermediate benchmark results (required because DataLoaderGenerator is not used here!)
    if ( std::filesystem::exists(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME)) == false ){
        std::filesystem::create_directory(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME));
    }

    const std::set<size_t> qubits = {0, 1, 2};

    //define session
    qristal::session sim(false);
    sim.init();
    sim.set_acc("qsim");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow
    RotationSweep workflow(
        std::vector<char>({'X', 'Y', 'Z'}), //specific rotations applied
        -90, //start (deg)
        90,  //end (deg)
        6,   //points
        sim
    );

    QuantumStateTomography<RotationSweep> qst(workflow);
    std::time_t t = qst.execute(std::vector<Task>{Task::MeasureCounts, Task::IdealDensity});

    DataLoaderGenerator dlg(qst.get_identifier(), std::vector<Task>{Task::MeasureCounts, Task::IdealDensity});
    dlg.set_timestamps(std::vector<std::time_t>{t}); //manually load in correct timestamp

    auto ideal_densities = dlg.obtain_ideal_densities()[0];
    auto counts = dlg.obtain_measured_counts(); //and obtain measurements
    auto measured_densities = qst.assemble_densities(counts[0]);

    for (size_t i = 0; i < ideal_densities.size(); ++i) {
        EXPECT_TRUE(ideal_densities[i].isApprox(measured_densities[i], 1e-2));
    }
}

TEST(QuantumStateTomographyTester, checkMLE) {
    //create folder for intermediate benchmark results (required because DataLoaderGenerator is not used here!)
    if ( std::filesystem::exists(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME)) == false ){
        std::filesystem::create_directory(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME));
    }

    const std::set<size_t> qubits = {0, 1, 2};

    //define session
    qristal::session sim(false);
    sim.init();
    sim.set_acc("qsim");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //define workflow
    RotationSweep workflow(
        std::vector<char>({'Z', 'X', 'Y'}), //specific rotations applied
        -45, //start (deg)
        45,  //end (deg)
        6,   //points
        sim
    );

    QuantumStateTomography<RotationSweep> qst(workflow, true);
    qst.set_maximum_likelihood_estimation(100, 1e-6);
    std::time_t t = qst.execute(std::vector<Task>{Task::MeasureCounts, Task::IdealDensity});

    DataLoaderGenerator dlg(qst.get_identifier(), std::vector<Task>{Task::MeasureCounts, Task::IdealDensity});
    dlg.set_timestamps(std::vector<std::time_t>{t}); //manually load in correct timestamp

    auto ideal_densities = dlg.obtain_ideal_densities()[0];
    auto counts = dlg.obtain_measured_counts(); //and obtain measurements
    auto measured_densities = qst.assemble_densities(counts[0]);

    for (size_t i = 0; i < ideal_densities.size(); ++i) {
        EXPECT_TRUE(ideal_densities[i].isApprox(measured_densities[i], 1e-2));
    }
}

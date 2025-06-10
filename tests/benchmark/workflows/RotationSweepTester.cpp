// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include <qristal/core/session.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/noise_model/noise_model.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/DataLoaderGenerator.hpp>

// STL
#include <iostream>

// range v3
#include <range/v3/view/zip.hpp>

// Gtest
#include <gtest/gtest.h>

using namespace qristal::benchmark;

TEST(RotationSweepTester, check_circuit_construction) {
    const size_t n_qubits = 4;
    const std::vector<char> rot_per_qubit{'I', 'X', 'Y', 'Z'};
    const int start_degree = -180;
    const int end_degree = 180;
    const size_t n_points = 5;

    //construct expected circuits
    std::vector<qristal::CircuitBuilder> correct_circuits;
    for (size_t i = 0; i < 5; ++i) {
        qristal::CircuitBuilder cb;
        correct_circuits.push_back(cb);
    }
    correct_circuits[0].RX(1, -1.0 * std::numbers::pi);
    correct_circuits[0].RY(2, -1.0 * std::numbers::pi);
    correct_circuits[0].RZ(3, -1.0 * std::numbers::pi);
    correct_circuits[1].RX(1, -1.0/2.0 * std::numbers::pi);
    correct_circuits[1].RY(2, -1.0/2.0 * std::numbers::pi);
    correct_circuits[1].RZ(3, -1.0/2.0 * std::numbers::pi);
    correct_circuits[2].RX(1, 0.0);
    correct_circuits[2].RY(2, 0.0);
    correct_circuits[2].RZ(3, 0.0);
    correct_circuits[3].RX(1, 1.0/2.0 * std::numbers::pi);
    correct_circuits[3].RY(2, 1.0/2.0 * std::numbers::pi);
    correct_circuits[3].RZ(3, 1.0/2.0 * std::numbers::pi);
    correct_circuits[4].RX(1, std::numbers::pi);
    correct_circuits[4].RY(2, std::numbers::pi);
    correct_circuits[4].RZ(3, std::numbers::pi);

    //define session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = 1000;
    sim.qn = 4;

    //generate circuits through SPAMBenchmark
    RotationSweep workflow(rot_per_qubit, start_degree, end_degree, n_points, sim);
    auto circuits = workflow.get_circuits();

    //compare to test
    for (size_t i = 0; i < circuits.size(); ++i) {
        EXPECT_EQ(circuits[i].get()->toString(), correct_circuits[i].get()->toString());
    }
}

TEST(RotationSweepTester, check_serialization) {
    //create folder for intermediate benchmark results (required because DataLoaderGenerator is not used here!)
    if ( std::filesystem::exists(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME)) == false ){
        std::filesystem::create_directory(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME));
    }

    //define serializable objects
    //(1) session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = 1000;
    sim.qn = 3;
    sim.noise_mitigation = "assignment-error-kernel";
    sim.noise_model = std::make_shared<qristal::NoiseModel>("default", sim.qn);

    //(2) bit string counts
    std::vector<std::map<std::vector<bool>, int>> counts {{{{0,0,0}, 1}},
                                                          {{{0,0,1}, 2}},
                                                          {{{0,1,0}, 3}},
                                                          {{{0,1,1}, 4}},
                                                          {{{1,0,0}, 5}},
                                                          {{{1,0,1}, 6}},
                                                          {{{1,1,0}, 7}},
                                                          {{{1,1,1}, 8}}};

    //(3) complex matrices
    ComplexMatrix mat_zero = ComplexMatrix::Zero(8, 8);
    ComplexMatrix mat_ones = ComplexMatrix::Ones(8, 8);
    std::vector<ComplexMatrix> mats{mat_zero, mat_ones};

    //define RotationSweep and call serialization
    RotationSweep workflow(std::vector<char>{'X', 'Y', 'Z'}, -180, 180, 3, sim);
    std::time_t t = std::time(nullptr);
    workflow.serialize_session_infos(t);
    workflow.serialize_ideal_counts(counts, t);
    workflow.serialize_measured_counts(counts, t);
    workflow.serialize_ideal_densities(mats, t);
    workflow.serialize_ideal_processes(mats, t);

    //Load data using DataLoaderGenerator and compare to test
    DataLoaderGenerator dlg(workflow.get_identifier(), std::vector<Task>{Task::MeasureCounts, Task::IdealCounts, Task::Session, Task::IdealDensity, Task::IdealProcess});
    dlg.set_timestamps(std::vector<std::time_t>{t});

    auto session_info = dlg.obtain_session_infos()[0];
    EXPECT_EQ(session_info.acc, sim.acc);
    EXPECT_EQ(session_info.noise_mitigation, sim.noise_mitigation);
    EXPECT_EQ(session_info.noise_model, sim.noise_model->to_json());
    EXPECT_EQ(session_info.qn, sim.qn);
    EXPECT_EQ(session_info.sn, sim.sn);

    auto measured_counts = dlg.obtain_measured_counts()[0];
    EXPECT_EQ(measured_counts, counts);

    auto ideal_counts = dlg.obtain_ideal_counts()[0];
    EXPECT_EQ(ideal_counts, counts);

    auto ideal_densities = dlg.obtain_ideal_densities()[0];
    for (const auto& [mat, ideal_density] : ::ranges::views::zip(mats, ideal_densities)) {
        for (size_t row = 0; row < mat.rows(); ++row) {
            for (size_t col = 0; col < mat.cols(); ++col) {
                EXPECT_EQ(mat(row, col), ideal_density(row, col));
            }
        }
    }

    auto ideal_processes = dlg.obtain_ideal_processes()[0];
    for (const auto& [mat, ideal_process] : ::ranges::views::zip(mats, ideal_processes)) {
        for (size_t row = 0; row < mat.rows(); ++row) {
            for (size_t col = 0; col < mat.cols(); ++col) {
                EXPECT_EQ(mat(row, col), ideal_process(row, col));
            }
        }
    }
}

//Circuit execution is tested in QuantumStateTomographyTester and QuantumProcessTomographyTester

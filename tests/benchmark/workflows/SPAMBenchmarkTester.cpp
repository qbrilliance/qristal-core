// Copyright (c) Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <iostream>

#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qristal/core/tools/zip_tool.hpp"
#include "qristal/core/session.hpp"
#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/DataLoaderGenerator.hpp"

using namespace qristal::benchmark;

TEST(SPAMBenchmarkTester, check_circuit_construction) {
    const size_t n_qubits = 10;
    const std::set<size_t> qubits{0, 2, 7};

    //construct expected circuits
    std::vector<qristal::CircuitBuilder> correct_circuits;
    for (size_t i = 0; i < 9; ++i) {
        qristal::CircuitBuilder cb;
        correct_circuits.push_back(cb);
    }
    correct_circuits[1].X(0);
    correct_circuits[2].X(2);
    correct_circuits[3].X(0);
    correct_circuits[3].X(2);
    correct_circuits[4].X(7);
    correct_circuits[5].X(0);
    correct_circuits[5].X(7);
    correct_circuits[6].X(2);
    correct_circuits[6].X(7);
    correct_circuits[7].X(0);
    correct_circuits[7].X(2);
    correct_circuits[7].X(7);

    //define session
    qristal::session sim(false);
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(1000);
    sim.set_qn(n_qubits);

    //generate circuits through SPAMBenchmark
    SPAMBenchmark workflow(qubits, sim);
    auto circuits = workflow.get_circuits();

    //compare to test
    for (size_t i = 0; i < circuits.size(); ++i) {
        EXPECT_EQ(circuits[i].get()->toString(), correct_circuits[i].get()->toString());
    }
}

TEST(SPAMBenchmarkTester, check_serialization) {
    //create folder for intermediate benchmark results (required because DataLoaderGenerator is not used here!)
    if ( std::filesystem::exists(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME)) == false ){
        std::filesystem::create_directory(std::filesystem::path(SerializerConstants::INTERMEDIATE_RESULTS_FOLDER_NAME));
    }

    //define serializable objects
    //(1) session
    qristal::session sim(false);
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(1000);
    sim.set_qn(3);
    sim.set_noise_mitigations(qristal::Table2d<std::string>{std::vector<std::string>{"assignment-error-kernel", "rich-extrap"}, std::vector<std::string>{"ro-error"}});
    qristal::NoiseModel nm("default");
    sim.set_noise_model(nm);

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

    //define SPAMBenchmark and call serialization
    SPAMBenchmark workflow(std::set<size_t>{0, 1, 2}, sim);
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
    EXPECT_EQ(session_info.accs_, sim.get_accs());
    EXPECT_EQ(session_info.noise_mitigations_, sim.get_noise_mitigations());
    auto nms = sim.get_noise_models()[0][0]->to_json();
    EXPECT_EQ(session_info.noise_models_, std::vector<std::vector<std::string>>{std::vector<std::string>{nms}});
    EXPECT_EQ(session_info.qns_, sim.get_qns());
    EXPECT_EQ(session_info.sns_, sim.get_sns());

    auto measured_counts = dlg.obtain_measured_counts()[0];
    EXPECT_EQ(measured_counts, counts);

    auto ideal_counts = dlg.obtain_ideal_counts()[0];
    EXPECT_EQ(ideal_counts, counts);

    auto ideal_densities = dlg.obtain_ideal_densities()[0];
    for (const auto& [mat, ideal_density] : std::ranges::views::zip(mats, ideal_densities)) {
        for (size_t row = 0; row < mat.rows(); ++row) {
            for (size_t col = 0; col < mat.cols(); ++col) {
                EXPECT_EQ(mat(row, col), ideal_density(row, col));
            }
        }
    }

    auto ideal_processes = dlg.obtain_ideal_processes()[0];
    for (const auto& [mat, ideal_process] : std::ranges::views::zip(mats, ideal_processes)) {
        for (size_t row = 0; row < mat.rows(); ++row) {
            for (size_t col = 0; col < mat.cols(); ++col) {
                EXPECT_EQ(mat(row, col), ideal_process(row, col));
            }
        }
    }
}

//Circuit execution is tested in QuantumStateTomographyTester and QuantumProcessTomographyTester

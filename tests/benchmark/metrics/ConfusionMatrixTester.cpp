// Copyright (c) 2024 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <iostream>

#include "qristal/core/session.hpp"
#include "qristal/core/noise_model/noise_model.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/metrics/ConfusionMatrix.hpp"

using namespace qristal::benchmark;

TEST(ConfusionMatrixTester, check_no_noise) {
    const std::vector<size_t> n_qubits_list{1, 2, 3, 4, 5};

    for (auto const & n_qubits : n_qubits_list) {
        //ideal confusion matrix is just the identity 
        size_t dim = std::pow(2, n_qubits);
        Eigen::MatrixXd ideal = Eigen::MatrixXd::Identity(dim, dim);

        //define session
        qristal::session sim(false);
        sim.init();
        sim.set_acc("qpp");
        sim.set_sn(100);
        sim.set_qn(n_qubits);

        //define workflow
        std::set<size_t> qubits;
        for (size_t q = 0; q < n_qubits; ++q) {
            qubits.insert(q);
        }
        SPAMBenchmark workflow(qubits, sim);

        //evaluate metric
        ConfusionMatrix<SPAMBenchmark> metric(workflow);
        std::map<std::time_t, Eigen::MatrixXd> results = metric.evaluate(true); //optional bool will force new execution
        for (auto const & [_, confusion] : results) {
          EXPECT_TRUE(ideal.isApprox(confusion, 1e-12));
        }
    }
}

TEST(ConfusionMatrixTester, check_noisy) {
    std::vector<size_t> n_qubits_list{1, 2};

    for (auto const & n_qubits : n_qubits_list) {
        //(0) ideal confusion matrix is just the identity 
        size_t dim = std::pow(2, n_qubits);
        Eigen::MatrixXd ideal = Eigen::MatrixXd::Identity(dim, dim);

        //(1) generate qubit set
        std::set<size_t> qubits;
        for (size_t q = 0; q < n_qubits; ++q) {
            qubits.insert(q);
        }

        //(2) Build noise model using fixed readout errors only
        const double p_01 = 0.05; 
        const double p_10 = 0.05;
        qristal::NoiseModel SPAM_error;
        for (size_t q = 0; q < qubits.size(); ++q) {
          SPAM_error.set_qubit_readout_error(q, qristal::ReadoutError(p_01, p_10));
          for (size_t qq = q+1; qq < qubits.size(); ++qq) {
            SPAM_error.add_qubit_connectivity(q, qq);
          }
        }

        //(3) define session
        auto sim = qristal::session();
        sim.init();
        sim.set_qn(qubits.size());
        sim.set_sn(1e6);
        sim.set_acc("aer");
        sim.set_noise(true);
        sim.set_noise_model(SPAM_error);

        //(3) define workflow
        SPAMBenchmark workflow(qubits, sim);

        //(4) evaluate metric
        ConfusionMatrix<SPAMBenchmark> metric(workflow);
        std::map<std::time_t, Eigen::MatrixXd> results = metric.evaluate(true); //optional bool will force new execution
        for (auto const & [_, confusion] : results) {
            //trivial check: all rows should sum to exactly 1.0
            for (Eigen::Index i = 0; i < confusion.rows(); ++i) {
              EXPECT_NEAR(confusion.row(i).sum(), 1.0, 1e-12);
            }

            //enable automatic SPAM correction, rerun, and check against identity
            sim.set_SPAM_confusion_matrix(confusion);
            auto corrected_confusion = metric.evaluate(true).begin()->second;
            EXPECT_TRUE(ideal.isApprox(corrected_confusion, 1e-2));
        }
    }
}
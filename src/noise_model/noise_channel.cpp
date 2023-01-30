// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/noise_model/noise_channel.hpp"
#include <Eigen/Dense>
#include <unsupported/Eigen/KroneckerProduct>

namespace qb
{

    NoiseChannel AmplitudeDampingChannel::Create(size_t q, double gamma)
    {
        KrausOperator::Matrix mat1{{1.0, 0.0}, {0.0, std::sqrt(1 - gamma)}};
        KrausOperator::Matrix mat2{{0.0, std::sqrt(gamma)}, {0.0, 0.0}};
        return {{mat1, {q}}, {mat2, {q}}};
    }

    NoiseChannel PhaseDampingChannel::Create(size_t q, double gamma)
    {
        KrausOperator::Matrix mat1{{1.0, 0.0}, {0.0, std::sqrt(1 - gamma)}};
        KrausOperator::Matrix mat2{{0.0, 0.0}, {0.0, std::sqrt(gamma)}};
        return {{mat1, {q}}, {mat2, {q}}};
    }

    NoiseChannel DepolarizingChannel::Create(size_t q, double p)
    {
        const double p1 = std::sqrt(1 - p);
        const double p2 = std::sqrt(p / 3);
        KrausOperator::Matrix mat_Id{{p1, 0.0}, {0.0, p1}};
        KrausOperator::Matrix mat_X{{0, p2}, {p2, 0}};
        KrausOperator::Matrix mat_Y{{0, -p2 * std::complex<double>(0, 1)}, {p2 * std::complex<double>(0, 1), 0}};
        KrausOperator::Matrix mat_Z{{p2, 0}, {0, -p2}};

        return {{mat_Id, {q}}, {mat_X, {q}}, {mat_Y, {q}}, {mat_Z, {q}}};
    }

    NoiseChannel DepolarizingChannel::Create(size_t q1, size_t q2, double p)
    {
        constexpr size_t num_terms = 16;
        constexpr double max_param = num_terms / (num_terms - 1.0);
        const double coeff_iden = std::sqrt(1 - p / max_param);
        const double coeff_pauli = std::sqrt(p / num_terms);

        static const std::unordered_map<char, Eigen::MatrixXcd> pauli_op_map = []()
        {
            Eigen::MatrixXcd Id2{Eigen::MatrixXcd::Identity(2, 2)};
            Eigen::MatrixXcd X{Eigen::MatrixXcd::Zero(2, 2)}; ///< Pauli Sigma-X gate
            Eigen::MatrixXcd Y{Eigen::MatrixXcd::Zero(2, 2)}; ///< Pauli Sigma-Y gate
            Eigen::MatrixXcd Z{Eigen::MatrixXcd::Zero(2, 2)}; ///< Pauli Sigma-Z gate
            X << 0, 1, 1, 0;
            Y << 0, -std::complex<double>(0, 1), std::complex<double>(0, 1), 0;
            Z << 1, 0, 0, -1;
            return std::unordered_map<char, Eigen::MatrixXcd>{{'I', Id2}, {'X', X}, {'Y', Y}, {'Z', Z}};
        }();

        const auto build_kraus_op = [q1, q2](const std::string &pauli_str, double coeff) -> qb::KrausOperator
        {
            assert(pauli_str.size() == 2);
            const auto first_mat = pauli_op_map.find(pauli_str[0])->second;
            const auto second_mat = pauli_op_map.find(pauli_str[1])->second;
//            Eigen::MatrixXcd kron_mat = Eigen::kroneckerProduct(first_mat, second_mat);
//            kron_mat = coeff * kron_mat;
            Eigen::MatrixXcd kron_mat = coeff * Eigen::kroneckerProduct(first_mat, second_mat);
            // std::cout << pauli_str << ":\n" << kron_mat << "\n";
            assert(kron_mat.rows() == 4);
            assert(kron_mat.cols() == 4);
            KrausOperator::Matrix mat(kron_mat.rows());
            for (int row = 0; row < kron_mat.rows(); ++row)
            {
                std::vector<std::complex<double>> row_vec(kron_mat.cols());
                for (int col = 0; col < kron_mat.cols(); ++col)
                {
                    row_vec[col] = kron_mat(row, col);
                }
                mat[row] = row_vec;
            }

            qb::KrausOperator kraus_op;
            kraus_op.matrix = mat;
            kraus_op.qubits = {q1, q2};
            return kraus_op;
        };

        NoiseChannel kraus_ops;
        const std::vector<std::pair<std::string, double>> pauli_kraus_ops{
            {"II", coeff_iden},
            {"IX", coeff_pauli},
            {"IY", coeff_pauli},
            {"IZ", coeff_pauli},
            {"XI", coeff_pauli},
            {"XX", coeff_pauli},
            {"XY", coeff_pauli},
            {"XZ", coeff_pauli},
            {"YI", coeff_pauli},
            {"YX", coeff_pauli},
            {"YY", coeff_pauli},
            {"YZ", coeff_pauli},
            {"ZI", coeff_pauli},
            {"ZX", coeff_pauli},
            {"ZY", coeff_pauli},
            {"ZZ", coeff_pauli}};
        for (const auto &[op_label, coeff] : pauli_kraus_ops)
        {
            kraus_ops.emplace_back(build_kraus_op(op_label, coeff));
        }
        return kraus_ops;
    }

    NoiseChannel GeneralizedPhaseAmplitudeDampingChannel::Create(
        size_t q, double excited_state_population, double param_amp, double param_phase)
    {
        const auto c0 = std::sqrt(1 - excited_state_population);
        const auto c1 = std::sqrt(excited_state_population);
        const auto param = 1 - param_amp - param_phase;
        // Damping ops to 0 state
        KrausOperator::Matrix A0 = {{c0, 0}, {0, c0 * std::sqrt(param)}};
        KrausOperator::Matrix A1 = {{0, c0 * std::sqrt(param_amp)}, {0, 0}};
        KrausOperator::Matrix A2 = {{0, 0}, {0, c0 * std::sqrt(param_phase)}};
        // Damping ops to 1 state
        KrausOperator::Matrix B0 = {{c1 * std::sqrt(param), 0}, {0, c1}};
        KrausOperator::Matrix B1 = {{0, 0}, {c1 * std::sqrt(param_amp), 0}};
        KrausOperator::Matrix B2 = {{c1 * std::sqrt(param_phase), 0}, {0, 0}};

        const std::vector<KrausOperator::Matrix> all_ops{A0, A1, A2, B0, B1, B2};
        // Select non-zero ops
        const auto is_non_zero = [](const KrausOperator::Matrix &mat)
        {
            constexpr double tol = 1e-12;
            for (const auto &row : mat)
            {
                for (const auto &el : row)
                {
                    if (std::abs(el) > tol)
                    {
                        return true;
                    }
                }
            }
            return false;
        };

        std::vector<KrausOperator::Matrix> non_zero_ops;
        std::copy_if(all_ops.begin(), all_ops.end(), std::back_inserter(non_zero_ops), is_non_zero);
        NoiseChannel channel;
        for (const auto &kraus_mat : non_zero_ops)
        {
            KrausOperator op{kraus_mat, {q}};
            channel.emplace_back(op);
        }
        return channel;
    }

    NoiseChannel GeneralizedAmplitudeDampingChannel::Create(
        size_t q, double excited_state_population, double gamma)
    {
        return GeneralizedPhaseAmplitudeDampingChannel::Create(q, excited_state_population, gamma, /*param_phase*/ 0.0);
    }

}
// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/noise_model/noise_channel.hpp"
#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qb/core/tools/zip_tool.hpp"

#include <numeric>
#include <unsupported/Eigen/KroneckerProduct>
#include "qb/core/primitives.hpp"

namespace 
{
    using eigen_cmat = Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>;
    using eigen_cvec = Eigen::Vector<std::complex<double>, Eigen::Dynamic>;

    /// Convert an STL-based matrix to an eigen matrix
    eigen_cmat matrix_to_eigen(const qb::KrausOperator::Matrix &mat) {
      assert(!mat.empty());
      const size_t rows = mat.size();
      const size_t cols = mat.at(0).size();
      eigen_cmat eigen_mat(rows, cols);
      for (size_t i = 0; i < rows; ++i) {
        eigen_mat.row(i) = eigen_cvec::Map(&mat[i][0], mat[i].size());
      }
      return eigen_mat;
    }

    /// Convert an eigen matrix to an STL-based matrix 
    qb::KrausOperator::Matrix eigen_to_matrix(const eigen_cmat &eigen_mat) {
      qb::KrausOperator::Matrix mat;
      for (Eigen::Index i = 0; i < eigen_mat.rows(); ++i) {
        eigen_cvec eigen_row = eigen_mat.row(i);
        std::vector<std::complex<double>> row(
            eigen_row.data(), eigen_row.data() + eigen_row.size());
        mat.emplace_back(row);
      }
      return mat;
    }
}
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
            Eigen::MatrixXcd kron_mat = coeff * Eigen::kroneckerProduct(first_mat, second_mat);
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

    /// Internal method to convert noise channel from Kraus -> Choi representation.
    /// A CPTP noise channel represented by a list of Kraus operators (matrices)
    /// can be converted into a single Choi matrix representing that map.
    /// Note: Kraus matrices acting on N qubits have dimension (2^N, 2^N);
    /// the corresponding Choi matrix has dimension (4^N, 4^N).
    eigen_cmat kraus_to_choi(const std::vector<eigen_cmat> &kraus_mats) 
    {
      const std::size_t input_dim = kraus_mats[0].cols();
      const std::size_t output_dim = kraus_mats[0].rows();

      // construct the un-normalized maximally entangled state
      eigen_cmat max_entangled_state =
          eigen_cmat::Zero(input_dim * input_dim, 1);
      for (size_t i = 0; i < input_dim; ++i) {
        max_entangled_state(i * input_dim + i, 0) = 1;
      }
      eigen_cmat max_entangled_state_adj = max_entangled_state.adjoint();
      eigen_cmat Omega = max_entangled_state * max_entangled_state_adj;

      eigen_cmat choi_mat =
          eigen_cmat::Zero(input_dim * output_dim, input_dim * output_dim);

      for (const auto &kraus_mat : kraus_mats) {
        choi_mat +=
            Eigen::kroneckerProduct(eigen_cmat::Identity(input_dim, input_dim),
                                    kraus_mat) *
            Omega *
            ((Eigen::kroneckerProduct(
                  eigen_cmat::Identity(input_dim, input_dim), kraus_mat))
                 .adjoint());
      }
      return choi_mat;
    }

    /// Helper to convert a noise channel into a list of Kraus ops as Eigen matrices
    static std::vector<eigen_cmat> noise_channel_to_eigen(const NoiseChannel &noise_channel) 
    {
      assert(!noise_channel.empty());
      std::vector<eigen_cmat> kraus_mats;
      for (const auto &chan : noise_channel) {
        kraus_mats.emplace_back(matrix_to_eigen(chan.matrix));
      }
      return kraus_mats;
    }

    /// Apply sqrt to singular values of a matrix.
    /// i.e., returns U * sqrt(S) * V^T
    /// where S is the diagonal matrix of singular values
    static eigen_cmat sqrt_svd_transform(const eigen_cmat &matrix) 
    {
      Eigen::JacobiSVD<eigen_cmat> svd(matrix, Eigen::ComputeFullU | Eigen::ComputeFullV);
      eigen_cmat unitary1 = svd.matrixU();
      eigen_cmat unitary2 = svd.matrixV();
      auto singular_values = svd.singularValues();
      // apply sqrt
      auto sqrt_singular_values = singular_values.cwiseSqrt();
      return unitary1 * sqrt_singular_values.asDiagonal() * unitary2.transpose();
    }

    /// Compute the fidelity of two normalized CPTP maps 
    /// Returns a value in the range of [0.0, 1.0], assuming that these matrices have been normalized.
    static double compute_fidelity(const eigen_cmat &mat1, const eigen_cmat &mat2) {
      /// Ref: https://qiskit.org/documentation/stubs/qiskit.quantum_info.process_fidelity.html#qiskit.quantum_info.process_fidelity
      auto s1sq = sqrt_svd_transform(mat1);
      auto s2sq = sqrt_svd_transform(mat2);
      const auto compute_nuclear_norm = [](const eigen_cmat &mat) -> double {
        // Sum of eigen values
        Eigen::JacobiSVD<eigen_cmat> svd(mat, Eigen::ComputeFullU | Eigen::ComputeFullV);
        return svd.singularValues().sum();
      };
      return std::pow(compute_nuclear_norm(s1sq * s2sq), 2.0);
    }

    KrausOperator::Matrix kraus_to_choi(const NoiseChannel &noise_channel) {
      return eigen_to_matrix(kraus_to_choi(noise_channel_to_eigen(noise_channel)));
    }

    KrausOperator::Matrix process_to_choi(const KrausOperator::Matrix& process_matrix) {
      return eigen_to_matrix(process_to_choi(matrix_to_eigen(process_matrix)));
    }

    Eigen::MatrixXcd get_computational_to_pauli_transform(const size_t n_qubits) {
      std::vector<qb::Pauli> basis{
        qb::Pauli::Symbol::I,
        qb::Pauli::Symbol::X,
        qb::Pauli::Symbol::Y,
        qb::Pauli::Symbol::Z 
      };
      const size_t dim = std::pow(basis.size(), n_qubits);
      Eigen::MatrixXcd conversion_mat = Eigen::MatrixXcd::Zero(dim, dim);
      for (size_t i = 0; i < dim; ++i) {
        Eigen::VectorXcd pauli = Eigen::VectorXcd{qb::build_up_matrix_by_Kronecker_product(i, basis, n_qubits).transpose().reshaped()};
        conversion_mat.row(i) = pauli; //set row-wise
      }
      return conversion_mat;
    }

    Eigen::MatrixXcd process_to_choi(const Eigen::MatrixXcd& process_matrix) {
      //transform process matrix in Pauli basis (II..I, II..X, ..., ZZ..Z) to computational basis (|0..0><0..0|, ..., |1..1><1..1|)
      const size_t n_qubits = std::log2(process_matrix.rows()) / 2;
      auto T = get_computational_to_pauli_transform(n_qubits);
      return T.adjoint() * process_matrix * T; //return transformed process matrix
    }

    NoiseChannel choi_to_kraus(const KrausOperator::Matrix& choi_matrix) {
      return eigen_to_noisechannel(choi_to_kraus(matrix_to_eigen(choi_matrix)));
    }

    std::vector<Eigen::MatrixXcd> choi_to_kraus(const Eigen::MatrixXcd& choi_matrix) {
      std::vector<Eigen::MatrixXcd> result;
      const size_t n_qubits = std::log2(choi_matrix.rows()) / 2;
      const size_t dim = std::pow(2, n_qubits);
      Eigen::ComplexEigenSolver<eigen_cmat> solver(choi_matrix); 
      for (Eigen::Index i = 0; i < solver.eigenvalues().size(); ++i) {
        if (std::abs(solver.eigenvalues()[i]) > 1e-14) { //only add non-zero channels
          Eigen::MatrixXcd eigenvec = solver.eigenvectors().col(i);
          eigenvec.resize(dim, dim);
          result.push_back(std::sqrt(solver.eigenvalues()[i]) * eigenvec);
        }
      }
      return result;
    }
    
    NoiseChannel eigen_to_noisechannel(const std::vector<Eigen::MatrixXcd>& kraus_mats) {
      NoiseChannel result; 
      for (const auto & kraus_mat : kraus_mats) {
        const size_t n_qubits = std::log2(kraus_mat.rows());
        std::vector<size_t> qubits(n_qubits);
        std::iota(qubits.begin(), qubits.end(), 0);
        KrausOperator ko; 
        ko.matrix = eigen_to_matrix(kraus_mat);
        ko.qubits = qubits;
        result.push_back(ko);
      }
      return result;
    }

    NoiseChannel process_to_kraus(const KrausOperator::Matrix& process_matrix) {
      return eigen_to_noisechannel(choi_to_kraus(process_to_choi(matrix_to_eigen(process_matrix))));
    }

    std::vector<Eigen::MatrixXcd> process_to_kraus(const Eigen::MatrixXcd& process_matrix) {
      return choi_to_kraus(process_to_choi(process_matrix));
    }

    /// Compute the process fidelity of a noise channel as compared to a no-noise (Identity) channel.
    /// e.g., a value of 0.9 means that this channel is equivalent to a ~10% gate noise (as measured by tomography).
    double process_fidelity(const NoiseChannel &noise_channel) {
      const size_t input_dim = 1u << (noise_channel[0].qubits.size());
      auto id_mat = eigen_cmat::Identity(input_dim, input_dim);
      eigen_cmat choi_id = kraus_to_choi({id_mat});
      eigen_cmat choi_chan =kraus_to_choi(noise_channel_to_eigen(noise_channel));
      // normalized by the system dimension
      // ref: https://qiskit.org/documentation/stubs/qiskit.quantum_info.process_fidelity.html#qiskit.quantum_info.process_fidelity
      return compute_fidelity(choi_id / (double)input_dim,
                              choi_chan / (double)input_dim);
    }
} // namespace qb
   
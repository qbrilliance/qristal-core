// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/noise_model/noise_channel.hpp"
#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qristal/core/tools/zip_tool.hpp"

#include <numeric>
#include <random>
#include <unsupported/Eigen/KroneckerProduct>
#include "qristal/core/primitives.hpp"

namespace 
{
    using eigen_cmat = Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>;
    using eigen_cvec = Eigen::Vector<std::complex<double>, Eigen::Dynamic>;

    /// Convert an STL-based matrix to an eigen matrix
    eigen_cmat matrix_to_eigen(const qristal::KrausOperator::Matrix &mat) {
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
    qristal::KrausOperator::Matrix eigen_to_matrix(const eigen_cmat &eigen_mat) {
      qristal::KrausOperator::Matrix mat;
      for (Eigen::Index i = 0; i < eigen_mat.rows(); ++i) {
        eigen_cvec eigen_row = eigen_mat.row(i);
        std::vector<std::complex<double>> row(
            eigen_row.data(), eigen_row.data() + eigen_row.size());
        mat.emplace_back(row);
      }
      return mat;
    }
}

namespace qristal
{

    NoiseChannel AmplitudeDampingChannel::Create(size_t q, double gamma)
    {
        const double p1 = 1 - gamma;
        const double p2 = gamma;
        KrausOperator::Matrix mat1{{1.0, 0.0}, {0.0, std::sqrt(p1)}};
        KrausOperator::Matrix mat2{{0.0, std::sqrt(gamma)}, {0.0, 0.0}};
        return {{mat1, {q}, p1}, {mat2, {q}, p2}};
    }

    NoiseChannel PhaseDampingChannel::Create(size_t q, double gamma)
    {
        const double p1 = 1 - gamma;
        const double p2 = gamma;
        KrausOperator::Matrix mat1{{1.0, 0.0}, {0.0, std::sqrt(p1)}};
        KrausOperator::Matrix mat2{{0.0, 0.0}, {0.0, std::sqrt(gamma)}};
        return {{mat1, {q}, p1}, {mat2, {q}, p2}};
    }

    NoiseChannel DepolarizingChannel::Create(size_t q, double p)
    {
        const double p1 = 1 - p;
        const double p2 = p / 3;
        KrausOperator::Matrix mat_Id{{std::sqrt(p1), 0.0}, {0.0, std::sqrt(p1)}};
        KrausOperator::Matrix mat_X{{0, std::sqrt(p2)}, {std::sqrt(p2), 0}};
        KrausOperator::Matrix mat_Y{{0, -std::sqrt(p2) * std::complex<double>(0, 1)}, {std::sqrt(p2) * std::complex<double>(0, 1), 0}};
        KrausOperator::Matrix mat_Z{{std::sqrt(p2), 0}, {0, -std::sqrt(p2)}};

        return {{mat_Id, {q}, p1}, {mat_X, {q}, p2}, {mat_Y, {q}, p2}, {mat_Z, {q}, p2}};
    }

    NoiseChannel DepolarizingChannel::Create(size_t q1, size_t q2, double p)
    {
        constexpr size_t num_terms = 16;
        constexpr double max_param = num_terms / (num_terms - 1.0);
        const double prob_iden = 1 - p / max_param;
        const double prob_pauli = p / num_terms;
        const double coeff_iden = std::sqrt(prob_iden);
        const double coeff_pauli = std::sqrt(prob_pauli);

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

        const auto build_kraus_op = [q1, q2](const std::string &pauli_str, double coeff) -> qristal::KrausOperator
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

            qristal::KrausOperator kraus_op;
            kraus_op.matrix = mat;
            kraus_op.qubits = {q1, q2};
            kraus_op.prob = std::pow(coeff, 2);
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
        const double pA0 = (1 - excited_state_population) * param;
        const double pA1 = (1 - excited_state_population) * param_amp;
        const double pA2 = (1 - excited_state_population) * param_phase;
        const double pB0 = excited_state_population * param;
        const double pB1 = excited_state_population * param_amp;
        const double pB2 = excited_state_population * param_phase;
        // Damping ops to 0 state
        KrausOperator::Matrix A0 = {{c0, 0}, {0, c0 * std::sqrt(param)}};
        KrausOperator::Matrix A1 = {{0, c0 * std::sqrt(param_amp)}, {0, 0}};
        KrausOperator::Matrix A2 = {{0, 0}, {0, c0 * std::sqrt(param_phase)}};
        // Damping ops to 1 state
        KrausOperator::Matrix B0 = {{c1 * std::sqrt(param), 0}, {0, c1}};
        KrausOperator::Matrix B1 = {{0, 0}, {c1 * std::sqrt(param_amp), 0}};
        KrausOperator::Matrix B2 = {{c1 * std::sqrt(param_phase), 0}, {0, 0}};

        const std::vector<KrausOperator::Matrix> all_ops{A0, A1, A2, B0, B1, B2};
        const std::vector<double> all_probs{pA0, pA1, pA2, pB0, pB1, pB2};
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
        NoiseChannel channel;
        for (size_t i = 0; i < all_ops.size(); i++) {
          if (is_non_zero(all_ops[i])) {
            KrausOperator op{all_ops[i], {q}, all_probs[i]};
            channel.emplace_back(op);
          }
        }

        return channel;
    }

    NoiseChannel GeneralizedAmplitudeDampingChannel::Create(
        size_t q, double excited_state_population, double gamma)
    {
        return GeneralizedPhaseAmplitudeDampingChannel::Create(q, excited_state_population, gamma, /*param_phase*/ 0.0);
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
    

    Eigen::MatrixXcd get_computational_to_pauli_transform(const size_t n_qubits) {
      std::vector<qristal::Pauli> basis{
        qristal::Pauli::Symbol::I,
        qristal::Pauli::Symbol::X,
        qristal::Pauli::Symbol::Y,
        qristal::Pauli::Symbol::Z 
      };
      const size_t dim = std::pow(basis.size(), n_qubits);
      Eigen::MatrixXcd conversion_mat = Eigen::MatrixXcd::Zero(dim, dim);
      for (size_t i = 0; i < dim; ++i) {
        Eigen::VectorXcd pauli = Eigen::VectorXcd{qristal::build_up_matrix_by_Kronecker_product(i, basis, n_qubits).transpose().reshaped()};
        conversion_mat.row(i) = pauli; //set row-wise
      }
      return conversion_mat;
    }

    //============================================ Quantum Process Matrix Transformations ============================================
    //---------------------------------------------- Transformations from process matrix ---------------------------------------------

    Eigen::MatrixXcd process_to_choi(const Eigen::MatrixXcd& process_matrix) {
      //transform process matrix in Pauli basis (II..I, II..X, ..., ZZ..Z) to computational basis (|0..0><0..0|, ..., |1..1><1..1|)
      const size_t n_qubits = std::log2(process_matrix.rows()) / 2;
      auto T = get_computational_to_pauli_transform(n_qubits);
      return T.adjoint() * process_matrix * T; //return transformed process matrix
    }

    KrausOperator::Matrix process_to_choi(const KrausOperator::Matrix& process_matrix) {
      return eigen_to_matrix(process_to_choi(matrix_to_eigen(process_matrix)));
    }

    Eigen::MatrixXcd process_to_superoperator(const Eigen::MatrixXcd& process_matrix) {
      return choi_to_superoperator(process_to_choi(process_matrix));
    }

    KrausOperator::Matrix process_to_superoperator(const KrausOperator::Matrix& process_matrix) {
      return eigen_to_matrix(process_to_superoperator(matrix_to_eigen(process_matrix)));
    }

    std::vector<Eigen::MatrixXcd> process_to_kraus(const Eigen::MatrixXcd& process_matrix) {
      return choi_to_kraus(process_to_choi(process_matrix));
    }

    NoiseChannel process_to_kraus(const KrausOperator::Matrix& process_matrix) {
      return eigen_to_noisechannel(choi_to_kraus(process_to_choi(matrix_to_eigen(process_matrix))));
    }

    //----------------------------------------------- Transformations from Choi matrix -----------------------------------------------

    Eigen::MatrixXcd choi_to_superoperator(const Eigen::MatrixXcd& choi_matrix) {
      size_t dim = std::pow(2, log2(choi_matrix.rows())/2); //retrieve density matrix dimensions from choi_matrix
      Eigen::MatrixXcd result = Eigen::MatrixXcd::Zero(choi_matrix.rows(), choi_matrix.cols());
      for (Eigen::Index row = 0; row < choi_matrix.rows(); ++row) {
        for (Eigen::Index col = 0; col < choi_matrix.cols(); ++col) {
          result((col % dim)*dim + (row % dim), (col / dim)*dim + (row / dim)) = choi_matrix(row, col);
        }
      }
      return result;
    }

    KrausOperator::Matrix choi_to_superoperator(const KrausOperator::Matrix& choi_matrix) {
      return eigen_to_matrix(choi_to_superoperator(matrix_to_eigen(choi_matrix)));
    }

    std::vector<Eigen::MatrixXcd> choi_to_kraus(const Eigen::MatrixXcd& choi_matrix) {
      std::vector<Eigen::MatrixXcd> result;
      const size_t n_qubits = std::log2(choi_matrix.rows()) / 2;
      const size_t dim = std::pow(2, n_qubits);
      Eigen::ComplexEigenSolver<Eigen::MatrixXcd> solver(choi_matrix); 
      for (Eigen::Index i = 0; i < solver.eigenvalues().size(); ++i) {
        if (std::abs(solver.eigenvalues()[i]) > 1e-14) { //only add non-zero channels
          Eigen::MatrixXcd eigenvec = solver.eigenvectors().col(i);
          eigenvec.resize(dim, dim);
          result.push_back(std::sqrt(solver.eigenvalues()[i]) * eigenvec);
        }
      }
      return result;
    }

    NoiseChannel choi_to_kraus(const KrausOperator::Matrix& choi_matrix) {
      return eigen_to_noisechannel(choi_to_kraus(matrix_to_eigen(choi_matrix)));
    }

    //------------------------------------------- Transformations from superoperator matrix ------------------------------------------
    
    Eigen::MatrixXcd superoperator_to_choi(const Eigen::MatrixXcd& superop) {
      size_t dim = std::pow(2, log2(superop.rows())/2); //retrieve density matrix dimensions from superoperator matrix
      Eigen::MatrixXcd result = Eigen::MatrixXcd::Zero(superop.rows(), superop.cols());
      for (Eigen::Index row = 0; row < superop.rows(); ++row) {
        for (Eigen::Index col = 0; col < superop.cols(); ++col) {
          result((row % dim) + dim * (col % dim), (row / dim) + dim * (col / dim)) = superop(row, col);
        }
      }
      return result;
    }

    KrausOperator::Matrix superoperator_to_choi(const KrausOperator::Matrix& superop) {
      return eigen_to_matrix(superoperator_to_choi(matrix_to_eigen(superop)));
    }

    std::vector<Eigen::MatrixXcd> superoperator_to_kraus(const Eigen::MatrixXcd& superop) {
      return choi_to_kraus(superoperator_to_choi(superop));
    }

    NoiseChannel superoperator_to_kraus(const KrausOperator::Matrix& superop) {
      return eigen_to_noisechannel(superoperator_to_kraus(matrix_to_eigen(superop)));
    }

    //------------------------------------------- Transformations from Kraus representation ------------------------------------------

    /// Internal method to convert noise channel from Kraus -> Choi representation.
    /// A CPTP noise channel represented by a list of Kraus operators (matrices)
    /// can be converted into a single Choi matrix representing that map.
    /// Note: Kraus matrices acting on N qubits have dimension (2^N, 2^N);
    /// the corresponding Choi matrix has dimension (4^N, 4^N).
    Eigen::MatrixXcd kraus_to_choi(const std::vector<Eigen::MatrixXcd> &kraus_mats) 
    {
      const std::size_t input_dim = kraus_mats[0].cols();
      const std::size_t output_dim = kraus_mats[0].rows();

      // construct the un-normalized maximally entangled state
      Eigen::MatrixXcd max_entangled_state =
          Eigen::MatrixXcd::Zero(input_dim * input_dim, 1);
      for (size_t i = 0; i < input_dim; ++i) {
        max_entangled_state(i * input_dim + i, 0) = 1;
      }
      Eigen::MatrixXcd max_entangled_state_adj = max_entangled_state.adjoint();
      Eigen::MatrixXcd Omega = max_entangled_state * max_entangled_state_adj;

      Eigen::MatrixXcd choi_mat =
          Eigen::MatrixXcd::Zero(input_dim * output_dim, input_dim * output_dim);

      for (const auto &kraus_mat : kraus_mats) {
        choi_mat +=
            Eigen::kroneckerProduct(Eigen::MatrixXcd::Identity(input_dim, input_dim),
                                    kraus_mat) *
            Omega *
            ((Eigen::kroneckerProduct(
                  Eigen::MatrixXcd::Identity(input_dim, input_dim), kraus_mat))
                 .adjoint());
      }
      return choi_mat;
    }

    KrausOperator::Matrix kraus_to_choi(const NoiseChannel &noise_channel) {
      return eigen_to_matrix(kraus_to_choi(noise_channel_to_eigen(noise_channel)));
    }

    Eigen::MatrixXcd kraus_to_superoperator(const std::vector<Eigen::MatrixXcd> &kraus_mats) {
      return choi_to_superoperator(kraus_to_choi(kraus_mats));
    }

    KrausOperator::Matrix kraus_to_superoperator(const NoiseChannel& noise_channel) {
      return eigen_to_matrix(kraus_to_superoperator(noise_channel_to_eigen(noise_channel)));
    }
    
    Eigen::MatrixXcd choi_to_process(const Eigen::MatrixXcd &choi){
      //transform choi matrix in computational basis (|0..0><0..0|, ..., |1..1><1..1|) to Pauli basis (II..I, II..X, ..., ZZ..Z)
      const size_t n_qubits = std::log2(choi.rows()) / 2;
      auto T = get_computational_to_pauli_transform(n_qubits);
      return 1.0 / std::pow(4, n_qubits) * T * choi * T.adjoint(); //return transformed process matrix
    }

    Eigen::MatrixXcd superoperator_to_process(const Eigen::MatrixXcd &superop){
        return choi_to_process(superoperator_to_choi(superop));
    }
    //================================================================================================================================

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

    NoiseChannel krausOpToChannel::Create(std::vector<size_t> qubits, std::vector<Eigen::MatrixXcd> kraus_ops_eigen,
        std::optional<std::vector<double>> kraus_probs) {
      NoiseChannel kraus_ops;
      for (size_t i = 0; i < kraus_ops_eigen.size(); i++) {
        Eigen::MatrixXcd kraus_op_mat = kraus_ops_eigen[i];
        if (qubits.size() == 1) {
          assert(kraus_op_mat.rows() == 2);
          assert(kraus_op_mat.cols() == 2);
        } else {
          assert(kraus_op_mat.rows() == 4);
          assert(kraus_op_mat.cols() == 4);
        }

        qristal::KrausOperator kraus_op;
        kraus_op.matrix = eigen_to_matrix(kraus_op_mat);
        kraus_op.qubits = qubits;
        kraus_op.prob = kraus_probs ? kraus_probs.value()[i] : 0.0;
        kraus_ops.emplace_back(kraus_op);
      }

      return kraus_ops;
    }

    //============================================ Process matrix interpolation methods ============================================

    Eigen::MatrixXcd createIdealU3ProcessMatrix(double theta, double phi, double lambda) {
      // Calculate I, X, Y, and Z components of U3 gate
      std::complex<double> U_I = 0.5 * cos(theta/2) * (1.0 + exp(std::complex(0.0, phi + lambda)));
      std::complex<double> U_X = 0.5 * sin(theta/2) * (exp(std::complex(0.0, phi)) - exp(std::complex(0.0, lambda)));
      std::complex<double> U_Y = std::complex(0.0, -0.5) * sin(theta/2) * (exp(std::complex(0.0, phi)) + exp(std::complex(0.0, lambda)));
      std::complex<double> U_Z = 0.5 * cos(theta/2) * (1.0 - exp(std::complex(0.0, phi + lambda)));

      // Build 1-qubit process matrix from components.
      Eigen::MatrixXcd U(4, 4);
      U << U_I * std::conj(U_I), U_I * std::conj(U_X), U_I * std::conj(U_Y), U_I * std::conj(U_Z),
           U_X * std::conj(U_I), U_X * std::conj(U_X), U_X * std::conj(U_Y), U_X * std::conj(U_Z),
           U_Y * std::conj(U_I), U_Y * std::conj(U_X), U_Y * std::conj(U_Y), U_Y * std::conj(U_Z),
           U_Z * std::conj(U_I), U_Z * std::conj(U_X), U_Z * std::conj(U_Y), U_Z * std::conj(U_Z);

      return U;
    }

    Eigen::MatrixXcd create1QubitNoisyProcessMatrix(const double& theta, const double& phi, const double& lambda,
        const std::vector<noiseChannelSymbol>& channel_list, const Eigen::VectorXd& channel_params) {
      // Create ideal process matrix
      Eigen::MatrixXcd ideal_process_mat = qristal::createIdealU3ProcessMatrix(theta, phi, lambda);
      // Convert process matrix to superoperator representation
      Eigen::MatrixXcd ideal_process_mat_super = qristal::process_to_superoperator(ideal_process_mat);

      // The Kraus matrices in the noise channels are independent of the qubit index, so we can directly create
      // one channel, say for qubit 0, and apply them to all qubits.
      std::vector<Eigen::MatrixXcd> channel_kraus_matrices =
          qristal::setChannelMatrices(channel_list, channel_params);

      // Convert noise channel matrices to superoperator representation
      Eigen::MatrixXcd channel_kraus_matrices_super = qristal::kraus_to_superoperator(channel_kraus_matrices);
      // Apply noise channel on ideal process matrix
      Eigen::MatrixXcd noisy_process_mat_super = channel_kraus_matrices_super * ideal_process_mat_super;

      return noisy_process_mat_super;
    }

    Eigen::MatrixXcd createNQubitNoisyProcessMatrix(const size_t nb_qubits,
        const std::vector<double>& theta, const std::vector<double>& phi, const std::vector<double>& lambda,
        const std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
            qristal::vector_hash<std::vector<size_t>>>& channel_list,
        const Eigen::VectorXd& channel_params) {
      // Generate noisy process matrix by creating an ideal process matrix and acting the noise channel
      // matrices on it.
      Eigen::MatrixXcd process_matrix = Eigen::MatrixXcd::Identity(std::pow(2, 2*nb_qubits), std::pow(2, 2*nb_qubits));
      size_t num_previous_params = 0;
      for (const auto &[qubits, channels] : channel_list) {
        // Calculate the number of total parameters for all channels in channel_list
        size_t nb_params = std::accumulate(channels.begin(), channels.end(), 0,
            [](size_t sum, const noiseChannelSymbol& channel) {
              return sum + getNumberOfNoiseChannelParams(channel);
            }
        );

        // Initialize empty Eigen vector for that particular qubit
        Eigen::VectorXd channel_param_1qubit = Eigen::VectorXd::Zero(nb_params);
        // Fill with the respective channel parameters
        size_t p_idx = 0;
        for (auto const& channel : channels) {
          for (size_t local_idx = 0; local_idx < getNumberOfNoiseChannelParams(channel); local_idx++) {
            channel_param_1qubit(p_idx + local_idx) = channel_params(num_previous_params + p_idx + local_idx);
          }
          p_idx += getNumberOfNoiseChannelParams(channel);
        }
        num_previous_params += p_idx;

        if (qubits.size() == 1) {
          // Create noisy 1-qubit process matrix
          Eigen::MatrixXcd noisy_process_mat_super = create1QubitNoisyProcessMatrix(
              theta[qubits[0]], phi[qubits[0]], lambda[qubits[0]], channels, channel_param_1qubit);

          // Expand the vector space of the 1-qubit process matrix to N-qubit vector space
          Eigen::MatrixXcd process_matrix_Nqubit = expandProcessMatrixSpace(qubits, nb_qubits, noisy_process_mat_super);
          process_matrix *= process_matrix_Nqubit;
        } else if (qubits.size() == 2) {
          // 2-qubit gates are angle-independent, hence a 2-qubit system's noiseless process matrix is just
          // a 4^N x 4^N identity matrix. The corresponding noisy process matrix can then be obtained by acting
          // the 2-qubit noise channel on that process matrix, i.e.
          // noisy_process_mat = noise_channel * identity = noise_channel.

          // Get 2-qubit depolarization channel as an N-qubit process matrix
          Eigen::MatrixXcd depol_2qubit_process_mat = create2QubitDepolProcessMatrix(qubits, nb_qubits, channel_param_1qubit(0));
          Eigen::MatrixXcd depol_2qubit_process_mat_super = process_to_superoperator(depol_2qubit_process_mat);
          process_matrix *= depol_2qubit_process_mat;
        }
      }

      return process_matrix;
    }

    Eigen::VectorXd processMatrixSolverInternal(const size_t& nb_qubits,
        const std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
            qristal::vector_hash<std::vector<size_t>>>& channel_list,
        const size_t& nb_params, size_t max_iter,
        Eigen::LevenbergMarquardt<Eigen::NumericalDiff<qristal::LMFunctorNoisy>, double>& lm,
        std::optional<Eigen::VectorXd> guess_params) {
      static std::random_device rd;
      static std::mt19937 gen(rd());
      // Random values to perturb the solved parameters
      static std::uniform_real_distribution<double> dist_perturb(-0.1, 0.1);

      // Solve for guess parameters x's.
      lm.fnorm = std::numeric_limits<double>::signaling_NaN();
      double sum_fvec = std::numeric_limits<double>::signaling_NaN();
      int info = 10;
      Eigen::VectorXd x(nb_params);
      for (size_t i = 0; i < max_iter; i++) {
        if (std::isnan(lm.fnorm) || std::isnan(sum_fvec) || info > 5) {
          if (guess_params) { // Use guess parameters if provided
            x = guess_params.value();
          } else { // Use random guess parameters
            std::vector<double> guess_rate_vec = generateRandomChannels(nb_qubits, channel_list);
            Eigen::VectorXd guess_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(guess_rate_vec.data(), guess_rate_vec.size());
            x << guess_rate;
          }

          info = lm.minimize(x);
          sum_fvec = std::accumulate(lm.fvec.begin(), lm.fvec.end(), 0.0);
        } else {
          break;
        }
      }

      // Do the subsequent iterations to improve x if needed.
      bool x_precision_tolerance = (x.array() > 1e-8).all(); // Check whether x contains precision violating-valued elements
      bool x_is_positive_valued = (x.array() > 0).all(); // Check whether x contains negative-valued elements
      if (lm.fnorm > 1e-4 || sum_fvec > 1e-6 || info > 5 || !x_is_positive_valued || !x_precision_tolerance) {
        Eigen::VectorXd x_prev = x;
        double sum_fvec_prev = sum_fvec;
        double fnorm_prev = lm.fnorm;

        for (size_t i = 0; i < max_iter; i++) {
          int info = lm.minimize(x);
          double sum_fvec = std::accumulate(lm.fvec.begin(), lm.fvec.end(), 0.0);
          x_precision_tolerance = (x.array() > 1e-8).all(); // Check whether x contains precision violating-valued elements
          x_is_positive_valued = (x.array() > 0).all(); // Check whether x contains negative-valued elements

          // Perturb small value if the norm of current vector function (fnorm) is not converged to < 1e-4
          // The tolerance value of 1e-4 is selected from the reconstruction of the process matrix (reconstructed_mat)
          // gives an acceptable difference relative to the initial process matrix (process_mat_noisy).
          if (lm.fnorm > 1e-4 || sum_fvec > 1e-6 || info > 5 || !x_is_positive_valued || !x_precision_tolerance) {
            if (sum_fvec > sum_fvec_prev) {
              x = x_prev;
            } else {
              sum_fvec_prev = sum_fvec;
              x_prev = x;
            }

            // Check whether elements of x are smaller than solver's precision tolerance of 1e-8. If there are,
            // then reassign it a random value.
            if (!x_precision_tolerance) {
              x = (x.array() < 1e-8).select(dist_perturb(gen), x);
            }
            // Check whether elements of x are negative-values. If there are, then negate the negative elements.
            if (!x_is_positive_valued) {
              x = (x.array() < 0).select(-x, x);
            }
            // Perturb elements of x.
            Eigen::VectorXd randVec = Eigen::VectorXd::NullaryExpr(x.size(), [&](){return dist_perturb(gen);}); // Create Eigen vector of random numbers using dist_perturb().
            x = x + x.cwiseProduct(randVec); // Perturb all elements of x.
          } else {
            break;
          }
        }
      }

      return x;
    }

    Eigen::VectorXd processMatrixSolver1Qubit(Eigen::MatrixXcd& process_matrix,
        const double& theta, const double& phi, const double& lambda,
        const std::vector<noiseChannelSymbol>& channel_list, const size_t& nb_params,
        size_t max_iter, size_t maxfev, double xtol, double ftol, double gtol) {
      // To use Eigen's numerical differentiation, we need to convert the complex matrix into a complex vector.
      Eigen::VectorXcd process_vec = Eigen::Map<Eigen::VectorXcd>(process_matrix.data(), process_matrix.rows() * process_matrix.cols());

      std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
          qristal::vector_hash<std::vector<size_t>>> channel_list_map;
      channel_list_map[{0}] = channel_list;

      qristal::LMFunctorNoisy functor;
      functor.input_vec = process_vec;
      functor.m = process_vec.size();
      functor.n = nb_params;
      functor.nb_qubits = 1;
      functor.channel_list = channel_list_map;
      functor.theta = {theta};
      functor.phi = {phi};
      functor.lambda = {lambda};

      Eigen::NumericalDiff<qristal::LMFunctorNoisy> numDiff(functor);
      Eigen::LevenbergMarquardt<Eigen::NumericalDiff<qristal::LMFunctorNoisy>, double> lm(numDiff);
      lm.parameters.maxfev = maxfev;
      lm.parameters.xtol = xtol;
      lm.parameters.ftol = ftol;
      lm.parameters.gtol = gtol;
      // Solve for guess parameters x's.
      Eigen::VectorXd x = qristal::processMatrixSolverInternal(1, channel_list_map, nb_params, max_iter, lm);

      return x;
    }

    Eigen::VectorXd processMatrixSolverNQubit(std::vector<Eigen::MatrixXcd>& process_matrix_1qubit,
        Eigen::MatrixXcd& process_matrix_Nqubit, const size_t& nb_qubits,
        const std::vector<double>& theta, const std::vector<double>& phi, const std::vector<double>& lambda,
        const std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
            qristal::vector_hash<std::vector<size_t>>>& channel_list,
        const std::vector<size_t>& nb_params, size_t max_iter, size_t maxfev, double xtol, double ftol,
        double gtol) {
      //--------------------------------- Solve the 1-qubit process matrices ---------------------------------
      // Solve the 1-qubit process matrices and use their solutions as a guess to the N-qubit process matrix
      assert(nb_qubits == process_matrix_1qubit.size());
      std::vector<double> x_vec; // Vector to store 1-qubit solutions
      size_t channel_ctr = 0;
      for (const auto &[qubits, channels] : channel_list) {
        if (qubits.size() == 1) {
          Eigen::VectorXd x = qristal::processMatrixSolver1Qubit(process_matrix_1qubit[qubits[0]],
              theta[qubits[0]], phi[qubits[0]], lambda[qubits[0]], channels, nb_params[channel_ctr], max_iter);
          // Collect solutions to be used as guess values for the N-qubit process matrix solver
          for (size_t j = 0; j < x.size(); j++) {
            x_vec.emplace_back(x(j));
          }
        } else if (qubits.size() == 2) {
          // The guess solutions obtained from solving the 1-qubit process matrices do not contain
          // guesses for 2-qubit channels. So we add them to x_guess here as a small number.
          static std::random_device rd;
          static std::mt19937 gen(rd());
          static std::uniform_real_distribution<double> dist_depol2(1e-8, 2e-4);
          x_vec.emplace_back(dist_depol2(gen));
        }

        channel_ctr++;
      }
      Eigen::VectorXd x_guess = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(x_vec.data(), x_vec.size());

      // --------------------------------- Solve the N-qubit process matrix ---------------------------------
      // To use Eigen's numerical differentiation, we need to convert the complex matrix into a complex vector.
      Eigen::VectorXcd process_vec = Eigen::Map<Eigen::VectorXcd>(process_matrix_Nqubit.data(), process_matrix_Nqubit.rows() * process_matrix_Nqubit.cols());
      size_t sum_nb_params = std::accumulate(nb_params.begin(), nb_params.end(), 0);
      qristal::LMFunctorNoisy functor;
      functor.input_vec = process_vec;
      functor.m = process_vec.size();
      functor.n = sum_nb_params;
      functor.nb_qubits = nb_qubits;
      functor.channel_list = channel_list;
      functor.theta = theta;
      functor.phi = phi;
      functor.lambda = lambda;

      Eigen::NumericalDiff<qristal::LMFunctorNoisy> numDiff(functor);
      Eigen::LevenbergMarquardt<Eigen::NumericalDiff<qristal::LMFunctorNoisy>, double> lm(numDiff);
      lm.parameters.maxfev = maxfev;
      lm.parameters.xtol = xtol;
      lm.parameters.ftol = ftol;
      lm.parameters.gtol = gtol;

      // Solve for guess parameters x's.
      Eigen::VectorXd x = qristal::processMatrixSolverInternal(nb_qubits, channel_list, sum_nb_params,
                                                               max_iter, lm, x_guess);
      return x;
    }

    void InterpolationModel::validateInputs() const  {
      switch (type_) {
        case Type::Polynomial: {
          if (!polynomial_degree_.has_value()) {
            throw std::invalid_argument("Polynomial interpolation requires a polynomial degree!");
          }
          break;
        }
        default: {
          if (polynomial_degree_.has_value()) {
            std::cerr << "Interpolator warning! Polynomial degree supplied but a different interpolation model was selected!" << std::endl;
          }
        }
      }
    }

    NoiseChannelInterpolator::NoiseChannelInterpolator(
      const std::vector<Eigen::VectorXd>& noise_params, 
      const std::vector<U3Angle>& rotation_angles, 
      const std::vector<InterpolationModel>& models
    ) {
      //for each interpolation model 
      for (size_t i = 0; i < models.size(); ++i) {
        switch (models[i].type()) {
          case InterpolationModel::Type::Average: {
            //just compute the average of all i-th noise channel parameters 
            double average = 0.0; 
            for (Eigen::Index a = 0; a < rotation_angles.size(); ++a) {
              average += noise_params[a](i);
            }
            average /= static_cast<double>(rotation_angles.size());
            //and push lambda  
            interpolationFunctions_.push_back(
              [average, i](const U3Angle& target, Eigen::VectorXd& params) {
                params(i) = average; 
              }
            );
            break;
          }
          case InterpolationModel::Type::Linear:
          case InterpolationModel::Type::Polynomial: {
            //(1) build design matrix of all rotation angles and noise channel param targets
            //first compute the number of terms in the polynomial basis 
            size_t num_terms = 0; 
            const size_t polynomial_degree = (models[i].type() == InterpolationModel::Type::Linear ? 1 : models[i].polynomial_degree().value());
            for (size_t n = 0; n <= polynomial_degree; ++n) {
              num_terms += (n + 1) * (n + 2) / 2; //number of terms for degree n 
            }
            Eigen::MatrixXd X = Eigen::MatrixXd::Zero(rotation_angles.size(), num_terms); 
            Eigen::VectorXd f = Eigen::VectorXd::Zero(rotation_angles.size());
            for (size_t angle = 0; angle < rotation_angles.size(); ++angle) {
              f(angle) = noise_params[angle](i);
              double theta = std::get<0>(rotation_angles[angle]); 
              double phi = std::get<1>(rotation_angles[angle]);
              double lamb = std::get<2>(rotation_angles[angle]);
              size_t col = 0; 
              //for all possible degrees up to maximum polynomial_degree
              for (int n = 0; n <= polynomial_degree; ++n) {
                //generate all polynomial terms theta^j phi^k lamb^l such that j + k + l = n
                for (int j = 0; j <= n; ++j) { //theta^j
                  for (int k = 0; k <= n - j; ++k) { //phi^k 
                    int l = n - j - k; //lamb^k
                    X(angle, col) = std::pow(theta, j) * std::pow(phi, k) * std::pow(lamb, l);
                    ++col;
                  }
                }
              }
            }
            
            //(2) Fit the model 
            Eigen::MatrixXd XtX = X.transpose() * X;
            //use pseudo inverse to circumvent numerical instabilities for singular matrices
            Eigen::VectorXd coeffs = XtX.completeOrthogonalDecomposition().pseudoInverse() * X.transpose() * f; 

            //(3) and push lambda passing the coefficients 
            interpolationFunctions_.push_back(
              [coeffs, i, polynomial_degree](const U3Angle& target, Eigen::VectorXd& params) {
                //(3.1) compute the representation of target in polynomial basis
                double theta = std::get<0>(target);
                double phi = std::get<1>(target);
                double lamb = std::get<2>(target); 
                size_t col = 0; 
                Eigen::VectorXd a(coeffs.size());
                for (int n = 0; n <= polynomial_degree; ++n) {
                  for (int j = 0; j <= n; ++j) { //theta^j
                    for (int k = 0; k <= n - j; ++k) { //phi^k 
                      int l = n - j - k; //lamb^k
                      a(col) = std::pow(theta, j) * std::pow(phi, k) * std::pow(lamb, l);
                      ++col;
                    }
                  }
                }
                //(3.2) contract with coefficients to get the interpolated parameter  
                params(i) = coeffs.transpose() * a;
              }
            );
            break;
          }
          case InterpolationModel::Type::Exponential: {
            //(1) Set up design matrix for log target 
            Eigen::MatrixXd X = Eigen::MatrixXd::Zero(rotation_angles.size(), 4); 
            Eigen::VectorXd f = Eigen::VectorXd::Zero(rotation_angles.size());
            for (size_t angle = 0; angle < rotation_angles.size(); ++angle) {
              if (noise_params[angle](i) <= 0.0) {
                throw std::runtime_error("Exponential interpolation is only possible for positive noise channel parameters!");
              }
              f(angle) = std::log(noise_params[angle](i));
              X(angle, 0) = 1.0; 
              X(angle, 1) = std::get<0>(rotation_angles[angle]); 
              X(angle, 2) = std::get<1>(rotation_angles[angle]);
              X(angle, 3) = std::get<2>(rotation_angles[angle]);
            }

            //(2) Fit the model 
            Eigen::MatrixXd XtX = X.transpose() * X;
            //use pseudo inverse to circumvent numerical instabilities for singular matrices
            Eigen::VectorXd coeffs = XtX.completeOrthogonalDecomposition().pseudoInverse() * X.transpose() * f; 

            //(3) and push lambda passing the coefficients 
            interpolationFunctions_.push_back(
              [coeffs, i](const U3Angle& target, Eigen::VectorXd& params) {
                params(i) = std::exp(coeffs[0]) * std::exp(
                    coeffs[1] * std::get<0>(target) + 
                    coeffs[2] * std::get<1>(target) +
                    coeffs[3] * std::get<2>(target)
                );
              }
            );
            break;
          }
        }
      } 
    }

    std::vector<Eigen::MatrixXcd> setChannelMatrices(const std::vector<noiseChannelSymbol>& channel_list,
        const Eigen::VectorXd& channel_params) {
      std::vector<Eigen::MatrixXcd> channel_kraus_matrices = {};
      size_t param_index = 0;
      qristal::NoiseChannel noise_channel;
      for (auto const& channel : channel_list) {
        switch (channel) {
          case noiseChannelSymbol::depolarization_1qubit:
            noise_channel = qristal::DepolarizingChannel::Create(0, channel_params[param_index]);
            ++param_index;
            break;
          case noiseChannelSymbol::depolarization_2qubit:
            noise_channel = qristal::DepolarizingChannel::Create(0, 1, channel_params[param_index]);
            ++param_index;
            break;
          case noiseChannelSymbol::generalized_phase_amplitude_damping:
            noise_channel = qristal::GeneralizedPhaseAmplitudeDampingChannel::Create(
                0, 0.0, channel_params[param_index], channel_params[param_index + 1]);
            ++(++param_index);
            break;
          case noiseChannelSymbol::generalized_amplitude_damping:
            noise_channel = qristal::GeneralizedAmplitudeDampingChannel::Create(
                0, 0.0, channel_params[param_index]);
            ++param_index;
            break;
          case noiseChannelSymbol::amplitude_damping:
            noise_channel = qristal::AmplitudeDampingChannel::Create(0, channel_params[param_index]);
            ++param_index;
            break;
          case noiseChannelSymbol::phase_damping:
            noise_channel = qristal::PhaseDampingChannel::Create(0, channel_params[param_index]);
            ++param_index;
            break;
        }

        for (const auto &m : noise_channel) {
          channel_kraus_matrices.emplace_back(matrix_to_eigen(m.matrix));
        }
      }

      return channel_kraus_matrices;
    }

    std::vector<double> generateRandomChannels(const size_t& nb_qubits,
        const std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
            qristal::vector_hash<std::vector<size_t>>>& channel_list) {
      static std::random_device rd;
      static std::mt19937 gen(rd());
      // Choose some physically meaningful random values as guess values
      static std::uniform_real_distribution<double> dist_amp_damp(1e-8, 2e-6);
      static std::uniform_real_distribution<double> dist_phase_damp(1e-8, 2e-3);
      static std::uniform_real_distribution<double> dist_depol1(1e-8, 2e-5);
      static std::uniform_real_distribution<double> dist_depol2(1e-8, 2e-4);

      std::vector<double> params;
      for (auto const& [qubits, channels] : channel_list) {
        for (size_t i = 0; i < channels.size(); i++) {
          qristal::noiseChannelSymbol channel = channels[i];
          switch (channel) {
            case noiseChannelSymbol::depolarization_1qubit:
              params.push_back(dist_depol1(gen));
              break;
            case noiseChannelSymbol::depolarization_2qubit:
              params.push_back(dist_depol2(gen));
              break;
            case noiseChannelSymbol::generalized_phase_amplitude_damping:
              params.push_back(dist_amp_damp(gen));
              params.push_back(dist_phase_damp(gen));
              break;
            case noiseChannelSymbol::generalized_amplitude_damping:
              params.push_back(dist_amp_damp(gen));
              break;
            case noiseChannelSymbol::amplitude_damping:
              params.push_back(dist_amp_damp(gen));
              break;
            case noiseChannelSymbol::phase_damping:
              params.push_back(dist_phase_damp(gen));
              break;
          }
        }
      }

      return params;
    }

    Eigen::MatrixXcd expandProcessMatrixSpace(const std::vector<size_t>& qubit_idx, const size_t& nb_qubits,
        const Eigen::MatrixXcd& process_matrix_1qubit) {
      Eigen::MatrixXcd temp = qristal::superoperator_to_process(process_matrix_1qubit);
      // Expand the vector space of the 1-qubit process matrix to N-qubit vector space
      Eigen::MatrixXcd process_matrix_Nqubit;
      size_t num_left_qubits = qubit_idx[0];
      size_t num_right_qubits = 0;
      if (qubit_idx.size() == 1) {
        num_right_qubits = nb_qubits - qubit_idx[0] - 1;
      } else if (qubit_idx.size() == 2) {
        num_right_qubits = nb_qubits - qubit_idx[1] - 1;
      }

      Eigen::MatrixXcd Id_left = Eigen::MatrixXcd::Zero(std::pow(2, 2*num_left_qubits), std::pow(2, 2*num_left_qubits));
      Id_left(0, 0) = 1.0;
      Eigen::MatrixXcd Id_right = Eigen::MatrixXcd::Zero(std::pow(2, 2*num_right_qubits), std::pow(2, 2*num_right_qubits));
      Id_right(0, 0) = 1.0;
      process_matrix_Nqubit = Eigen::kroneckerProduct(Id_left, temp).eval();
      process_matrix_Nqubit = Eigen::kroneckerProduct(process_matrix_Nqubit, Id_right).eval();

      return qristal::process_to_superoperator(process_matrix_Nqubit);
    }

    Eigen::MatrixXcd create2QubitDepolProcessMatrix(const std::vector<size_t>& depol_qubits, const size_t& nb_qubits,
        const double& p) {
      // Find min and max element in depol_qubits
      auto [min_it, max_it] = std::minmax_element(depol_qubits.begin(), depol_qubits.end());
      // Evaluate how many qubits need to be injected before min and between min and max
      size_t pre = *min_it;
      size_t mid = *max_it - *min_it - 1;
      // Compute identity and pauli coefficients
      const double coeff_iden = std::sqrt(1.0 - 15.0 * p / 16.0);
      const double coeff_pauli = std::sqrt(p / 16.0);
      //build process matrix diagonal
      Eigen::VectorXd process_mat_diagonal = Eigen::VectorXd::Zero(std::pow(4, nb_qubits));
      process_mat_diagonal(0) = coeff_iden;
      for (size_t i = 1; i < 16; ++i) {
        auto s = qristal::convert_decimal(i, 4, 2); // Convert decimal numbers i to a number of base 4 with length 2
        // Convert to N-qubit process matrix index
        size_t index = s[0] * std::pow(4, nb_qubits - pre - 1) + s[1] * std::pow(4, nb_qubits - pre - mid - 2);
        // Set element in diagonal
        process_mat_diagonal(index) = coeff_pauli;
      }
      // Build up full matrix and return
      return process_mat_diagonal.asDiagonal();
    }

  //----------------------------------------------- Partial trace --------------------------------------------------

  size_t get_index(const size_t n_qubits, const std::set<size_t>& indices, size_t index){
    size_t result = 0;
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
      result += std::pow(4, n_qubits - *it - 1) *  (index % 4);
      index /= 4;
    }
    return result;
  }

  //internal partial trace function called by the exposed partialTraceProcessMatrixKeep and partialTraceProcessMatrixRemove
  Eigen::MatrixXcd partialTraceProcessMatrix(
      const Eigen::MatrixXcd& full, 
      const std::set<size_t>& kept_indices, 
      const std::set<size_t>& removed_indices
  ) {
    //initialize result matrix
    size_t new_size = std::pow(2, 2*kept_indices.size());
    size_t n_qubits = std::log2(full.rows())/2; 
    Eigen::MatrixXcd result = Eigen::MatrixXcd::Zero(new_size, new_size);

    //perform partial trace
    size_t sum_size = full.rows() / new_size;
    for (size_t i = 0; i < result.rows(); ++i) {
      size_t index_i = get_index(n_qubits, kept_indices, i);
      for (size_t j = 0; j < result.cols(); ++j) {
        size_t index_j = get_index(n_qubits, kept_indices, j);
        for (size_t k = 0; k < sum_size; ++k) { //summation index
          size_t index_out = get_index(n_qubits, removed_indices, k);
          size_t left = index_i + index_out;
          size_t right = index_j + index_out;
          result(i,j) += full(left, right); 
        } 
      }
    }
    return result;
  }

  std::set<size_t> getComplementarySet(const size_t n, const std::set<size_t>& s) {
    std::set<size_t> complement;
    for (size_t i = 0; i < n; ++i) {
      if (s.find(i) == s.end()) {
        complement.insert(i);
      }
    }
    return complement;
  }

  Eigen::MatrixXcd partialTraceProcessMatrixKeep(const Eigen::MatrixXcd& full, const std::set<size_t>& indices) {    
    size_t n_qubits = std::log2(full.rows())/2; 
    return partialTraceProcessMatrix(full, indices, getComplementarySet(n_qubits, indices));
  }

  Eigen::MatrixXcd partialTraceProcessMatrixRemove(const Eigen::MatrixXcd& full, const std::set<size_t>& indices) {
    size_t n_qubits = std::log2(full.rows())/2; 
    return partialTraceProcessMatrix(full, getComplementarySet(n_qubits, indices), indices);
  }

}

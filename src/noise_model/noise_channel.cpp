// Copyright (c) 2022 Quantum Brilliance Pty Ltd

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

    NoiseChannel krausOpToChannel::Create(std::vector<size_t> qubits, std::vector<Eigen::MatrixXcd> kraus_ops_eigen) {
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

    Eigen::MatrixXcd create1QubitNoisyProcessMatrix(double theta, double phi, double lambda,
        Eigen::VectorXd channel_params) {
      // Create ideal process matrix
      Eigen::MatrixXcd ideal_process_mat = qristal::createIdealU3ProcessMatrix(theta, phi, lambda);
      // Convert process matrix to superoperator representation
      Eigen::MatrixXcd ideal_process_mat_super = qristal::process_to_superoperator(ideal_process_mat);

      // The Kraus matrices in the noise channels are independent of the qubit index, so we can directly create
      // one channel, say for qubit 0, and apply them to all qubits.
      double gen_amp_damp_rate = channel_params(0);
      double gen_phase_damp_rate = channel_params(1);
      double depol_1qubit_rate = channel_params(2);
      qristal::NoiseChannel gen_phase_amp_damp =
          qristal::GeneralizedPhaseAmplitudeDampingChannel::Create(0, 0.0, gen_amp_damp_rate, gen_phase_damp_rate);
      qristal::NoiseChannel depol_1qubit = qristal::DepolarizingChannel::Create(0, depol_1qubit_rate);

      std::vector<Eigen::MatrixXcd> channel_kraus_matrices;
      for (const auto &x : gen_phase_amp_damp) {
        channel_kraus_matrices.emplace_back(matrix_to_eigen(x.matrix));
      }
      for (const auto &x : depol_1qubit) {
        channel_kraus_matrices.emplace_back(matrix_to_eigen(x.matrix));
      }

      // Convert noise channel matrices to superoperator representation
      Eigen::MatrixXcd channel_kraus_matrices_super = qristal::kraus_to_superoperator(channel_kraus_matrices);
      // Apply noise channel on ideal process matrix
      Eigen::MatrixXcd noisy_process_mat_super = channel_kraus_matrices_super * ideal_process_mat_super;

      return noisy_process_mat_super;
    }

    Eigen::MatrixXcd createNQubitNoisyProcessMatrix(size_t nb_qubits,
        std::vector<double> theta, std::vector<double> phi, std::vector<double> lambda,
        Eigen::VectorXd channel_params) {
      // Generate noisy process matrix by creating an ideal process matrix and acting the noise channel
      // matrices on it.
      Eigen::MatrixXcd process_matrix = Eigen::MatrixXcd::Zero(std::pow(2, 2*nb_qubits), std::pow(2, 2*nb_qubits));
      for (size_t i = 0; i < nb_qubits; i++) {
        // Create noisy 1-qubit process matrix
        double gen_amp_damp_rate = channel_params(i);
        double gen_phase_damp_rate = channel_params(i + nb_qubits);
        double depol_1qubit_rate = channel_params(i + 2*nb_qubits);
        Eigen::VectorXd channel_params_1qubit(3);
        channel_params_1qubit << gen_amp_damp_rate, gen_phase_damp_rate, depol_1qubit_rate;
        Eigen::MatrixXcd noisy_process_mat_super =  qristal::create1QubitNoisyProcessMatrix(
            theta[i], phi[i], lambda[i], channel_params_1qubit);

        // Expand the vector space of the 1-qubit process matrix N-qubit vector space
        Eigen::MatrixXcd process_mat_tmp;
        for (size_t qId = 0; qId < nb_qubits; qId++) {
          Eigen::MatrixXcd Id_left = Eigen::MatrixXcd::Identity(std::pow(2, 2*qId), std::pow(2, 2*qId));
          Eigen::MatrixXcd Id_right = Eigen::MatrixXcd::Identity(std::pow(2, 2*(nb_qubits - qId - 1)), std::pow(2, 2*(nb_qubits - qId - 1)));
          process_mat_tmp = Eigen::kroneckerProduct(Id_left, noisy_process_mat_super).eval();
          process_mat_tmp = Eigen::kroneckerProduct(process_mat_tmp, Id_right).eval();
        }
        process_matrix += process_mat_tmp;
      }

      return process_matrix;
    }

    Eigen::VectorXd processMatrixSolverInternal(size_t nb_qubits, size_t nb_params, size_t max_iter,
        Eigen::LevenbergMarquardt<Eigen::NumericalDiff<qristal::LMFunctorNoisy>, double> lm,
        std::optional<Eigen::VectorXd> guess_params) {
      static std::random_device rd;
      static std::mt19937 gen(rd());
      // Choose some physically meaningful random values as guess values
      static std::uniform_real_distribution<double> dist_amp_damp(0.0, 2e-6);
      static std::uniform_real_distribution<double> dist_phase_damp(0.0, 2e-3);
      static std::uniform_real_distribution<double> dist_depol1(0.0, 2e-5);
      // Random values to perturb the solved parameters
      static std::uniform_real_distribution<double> dist_perturb(-0.1, 0.1);

      // Solve for guess parameters x's.
      // Do one some rough iterations first to get a converged solution and to get some "previous"
      // output to be fed into subsequent iterations
      lm.fnorm = std::numeric_limits<double>::signaling_NaN();
      double sum_fvec = std::numeric_limits<double>::signaling_NaN();
      int info = 10;
      Eigen::VectorXd x(nb_params);
      bool x_is_positive_valued = 0;
      for (size_t i = 0; i < max_iter; i++) {
        if (std::isnan(lm.fnorm) || std::isnan(sum_fvec) || info > 5 || !x_is_positive_valued) {
          if (guess_params) { // Use guess parameters if provided
            x = guess_params.value();
          } else { // Use random guess parameters
            std::vector<double> gen_amp_damp_rate_vec, gen_phase_damp_rate_vec, depol_1qubit_rate_vec;
            for (size_t i = 0; i < nb_qubits; i++) {
              gen_amp_damp_rate_vec.emplace_back(dist_amp_damp(gen));
              gen_phase_damp_rate_vec.emplace_back(dist_phase_damp(gen));
              depol_1qubit_rate_vec.emplace_back(dist_depol1(gen));
            }
            Eigen::VectorXd gen_amp_damp_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_amp_damp_rate_vec.data(), gen_amp_damp_rate_vec.size());
            Eigen::VectorXd gen_phase_damp_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_phase_damp_rate_vec.data(), gen_phase_damp_rate_vec.size());
            Eigen::VectorXd depol_1qubit_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(depol_1qubit_rate_vec.data(), depol_1qubit_rate_vec.size());
            x << gen_amp_damp_rate, gen_phase_damp_rate, depol_1qubit_rate;
          }

          info = lm.minimize(x);
          sum_fvec = std::accumulate(lm.fvec.begin(), lm.fvec.end(), 0.0);
          for (size_t i = 0; i < x.size(); i++) {
            if (x[i] < 0) {
              x_is_positive_valued = 0;
              break;
            } else {
              x_is_positive_valued = 1;
            }
          }
        } else {
          break;
        }
      }
      Eigen::VectorXd x_prev = x;
      double sum_fvec_prev = sum_fvec;
      double fnorm_prev = lm.fnorm;

      // Now do the subsequent iterations for convergence
      for (size_t i = 0; i < max_iter; i++) {
        int info = lm.minimize(x);
        double sum_fvec = std::accumulate(lm.fvec.begin(), lm.fvec.end(), 0.0);
        bool x_is_positive_valued = 1;
        for (size_t i = 0; i < x.size(); i++) {
          if (x[i] < 0) {
            x_is_positive_valued = 0;
            break;
          }
        }

        // Perturb small angles if the norm of current vector function (fnorm) is not converged to < 1e-4
        // The tolerance value of 1e-4 is selected from the reconstruction of the process matrix (reconstructed_mat)
        // gives an acceptable difference relative to the initial process matrix (process_mat_noisy).
        if (lm.fnorm > 1e-4 || sum_fvec > 1e-6 || info > 5 || !x_is_positive_valued) {
          if (sum_fvec > sum_fvec_prev) {
            x = x_prev;
          } else {
            sum_fvec_prev = sum_fvec;
            x_prev = x;
          }

          for (size_t i = 0; i < x.size(); i++) {
            x[i] = (x[i] < 0) ? x[i] * -1.0 : x[i];
            assert(x[i] >= 0);
            x[i] += dist_perturb(gen) * x[i];
          }
        } else {
          break;
        }
      }

      return x;
    }

    Eigen::VectorXd processMatrixSolver1Qubit(Eigen::MatrixXcd process_matrix,
        double theta, double phi, double lambda, size_t max_iter,
        size_t maxfev, double xtol, double ftol, double gtol) {
      size_t nb_params = 3; // 3 parameters for the 2 noise channels

      // To use Eigen's numerical differentiation, we need to convert the complex matrix into a complex vector.
      Eigen::VectorXcd process_vec = Eigen::Map<Eigen::VectorXcd>(process_matrix.data(), process_matrix.rows() * process_matrix.cols());

      qristal::LMFunctorNoisy functor;
      functor.input_vec = process_vec;
      functor.m = process_vec.size();
      functor.n = nb_params;
      functor.nb_qubits = 1;
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
      Eigen::VectorXd x = qristal::processMatrixSolverInternal(1, nb_params, max_iter, lm);

      return x;
    }

    Eigen::VectorXd processMatrixSolverNQubit(std::vector<Eigen::MatrixXcd> process_matrix_1qubit,
        Eigen::MatrixXcd process_matrix_Nqubit, size_t nb_qubits,
        std::vector<double> theta, std::vector<double> phi, std::vector<double> lambda, size_t max_iter,
        size_t maxfev, double xtol, double ftol, double gtol) {
      //--------------------------------- Solve the 1-qubit process matrices ---------------------------------
      // Solve the 1-qubit process matrices and use their solutions as a guess to the N-qubit process matrix
      assert(nb_qubits == process_matrix_1qubit.size());
      std::vector<double> gen_amp_damp_rate_vec, gen_phase_damp_rate_vec, depol_1qubit_rate_vec; // Vectors so store 1-qubit solutions
      for (size_t i = 0; i < nb_qubits; i++) {
        Eigen::VectorXd x = qristal::processMatrixSolver1Qubit(process_matrix_1qubit[i], theta[i], phi[i], lambda[i], max_iter);
        // Collect solutions to be used as guess values for the N-qubit process matrix solver
        gen_amp_damp_rate_vec.emplace_back(x(0));
        gen_phase_damp_rate_vec.emplace_back(x(1));
        depol_1qubit_rate_vec.emplace_back(x(2));
      }

      //--------------------------------- Solve the N-qubit process matrix ---------------------------------
      size_t nb_params = 3 * nb_qubits; // 3 parameters for the 2 noise channels, per qubit
      Eigen::VectorXd gen_amp_damp_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_amp_damp_rate_vec.data(), gen_amp_damp_rate_vec.size());
      Eigen::VectorXd gen_phase_damp_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_phase_damp_rate_vec.data(), gen_phase_damp_rate_vec.size());
      Eigen::VectorXd depol_1qubit_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(depol_1qubit_rate_vec.data(), depol_1qubit_rate_vec.size());
      Eigen::VectorXd x_guess(nb_params);
      x_guess << gen_amp_damp_rate, gen_phase_damp_rate, depol_1qubit_rate;

      // To use Eigen's numerical differentiation, we need to convert the complex matrix into a complex vector.
      Eigen::VectorXcd process_vec = Eigen::Map<Eigen::VectorXcd>(process_matrix_Nqubit.data(), process_matrix_Nqubit.rows() * process_matrix_Nqubit.cols());

      qristal::LMFunctorNoisy functor;
      functor.input_vec = process_vec;
      functor.m = process_vec.size();
      functor.n = nb_params;
      functor.nb_qubits = nb_qubits;
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
      Eigen::VectorXd x = qristal::processMatrixSolverInternal(nb_qubits, nb_params, max_iter, lm, x_guess);

      return x;
    }

    Eigen::MatrixXcd processMatrixInterpolator(size_t nb_qubits,
        std::vector<Eigen::MatrixXcd> process_matrix_1qubit_1, std::vector<Eigen::MatrixXcd> process_matrix_1qubit_2,
        Eigen::MatrixXcd process_matrix_Nqubit_1, Eigen::MatrixXcd process_matrix_Nqubit_2,
        std::vector<double> theta1, std::vector<double> phi1, std::vector<double> lambda1,
        std::vector<double> theta2, std::vector<double> phi2, std::vector<double> lambda2,
        double theta_target, double phi_target, double lambda_target, size_t max_iter, size_t maxfev,
        double xtol, double ftol, double gtol) {

      // Solve for the noise channel parameters at angle {tx1, ty1, tz1}
      Eigen::VectorXd params1 = qristal::processMatrixSolverNQubit(
          process_matrix_1qubit_1, process_matrix_Nqubit_1, nb_qubits, theta1, phi1, lambda1,
          max_iter, maxfev, xtol, ftol, gtol);
      // Solve for the noise channel parameters at angle {tx2, ty2, tz2}
      Eigen::VectorXd params2 = qristal::processMatrixSolverNQubit(
          process_matrix_1qubit_2, process_matrix_Nqubit_2, nb_qubits, theta2, phi2, lambda2,
          max_iter, maxfev, xtol, ftol, gtol);

      // Calculate average of each param in param1 and param2
      Eigen::VectorXd params_avg = (params1 + params2) / 2;

      // Create output process matrix using average noise channel parameters of both
      // input process matrices
      std::vector<double> theta_target_vec, phi_target_vec, lambda_target_vec;
      for (size_t i = 0; i < nb_qubits; i++) {
        theta_target_vec.emplace_back(theta_target);
        phi_target_vec.emplace_back(phi_target);
        lambda_target_vec.emplace_back(lambda_target);
      }
      Eigen::MatrixXcd output_process_matrix = qristal::createNQubitNoisyProcessMatrix(
          nb_qubits, theta_target_vec, phi_target_vec, lambda_target_vec, params_avg);

      return output_process_matrix;
    }
}

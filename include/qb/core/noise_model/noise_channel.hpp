// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#ifndef _QB_NOISE_CHANNEL_
#define _QB_NOISE_CHANNEL_

#include <complex>
#include <vector>
#include <Eigen/Dense>

namespace qb
{
    struct KrausOperator
    {
        using Matrix = std::vector<std::vector<std::complex<double>>>;

        /**
         * @brief Kraus matrix
         *
         */
        Matrix matrix;

        /**
         * @brief Qubits that this Kraus operator acts on.
         *
         */
        std::vector<size_t> qubits;
    };

    /**
     * @brief Noise channel is a list of Kraus operators
     *
     */
    using NoiseChannel = std::vector<KrausOperator>;

    /**
     * @brief Obtain basis transformation matrix from the computational to the Pauli basis.
     * 
     * Arguments: 
     * @param n_qubits the unsigned integer number of qubits. The basis transformation may be applied to.
     *
     * @return Eigen::MatrixXcd the basis transformation matrix. 
     * 
     * @details This function generates transformation matrices for the standard computational basis 
     * (|0..0><0..0|, |0..0><0..1|, ..., |1..1><1..0|, |1..1><1..1|) to the standard Pauli basis 
     * (II..I, II..X, ... ZZ..Y, ZZ..Z) for an arbitrary number of qubits.
     */
    Eigen::MatrixXcd get_computational_to_pauli_transform(const size_t n_qubits);

    //============================================ Quantum Process Matrix Transformations ============================================
    //---------------------------------------------- Transformations from process matrix ---------------------------------------------

    /**
     * @brief Convert an Eigen-based process matrix to its Eigen-based Choi matrix representation.
     * 
     * Arguments: 
     * @param process_matrix the Eigen-based (Eigen::MatrixXcd) process matrix in the 
     * standard Pauli basis ordered from II..I, II..X, ... ZZ..Y, ZZ..Z
     *
     * @return Choi matrix in the computational basis ordered in ascending bit string order 
     * (|0..0><0..0|, |0..0><0..1|, ..., |1..1><1..0|, |1..1><1..1|)
     * 
     * @details This function may be used to convert arbitrary Eigen-based quantum process 
     * matrices to their Choi representation by applying a basis transformation obtained via
     * get_computational_to_pauli_transform.
     */
    Eigen::MatrixXcd process_to_choi(const Eigen::MatrixXcd& process_matrix);
    /**
     * @brief Convert an STL-based process matrix to its STL-based Choi matrix representation.
     * 
     * Arguments: 
     * @param process_matrix the STL-based (std::vector<std::vector<T>>) process matrix in the 
     * standard Pauli basis ordered from II..I, II..X, ... ZZ..Y, ZZ..Z
     *
     * @return Choi matrix in the computational basis ordered in ascending bit string order 
     * (|0..0><0..0|, |0..0><0..1|, ..., |1..1><1..0|, |1..1><1..1|)
     * 
     * @details This function may be used to convert arbitrary STL-based quantum process 
     * matrices to their Choi representation by delegating the transformation to the Eigen-based 
     * implementation.
     */
    KrausOperator::Matrix process_to_choi(const KrausOperator::Matrix& process_matrix);

    /**
     * @brief Convert an Eigen-based process matrix to its Eigen-based superoperator matrix representation.
     * 
     * Arguments: 
     * @param process_matrix the Eigen-based (Eigen::MatrixXcd) process matrix in the 
     * standard Pauli basis ordered from II..I, II..X, ... ZZ..Y, ZZ..Z
     *
     * @return Eigen::MatrixXcd superoperator matrix representation.
     * 
     * @details This function transforms arbitrary Eigen-based process matrices to their superoperator 
     * representation by (i) calling process_to_choi(const Eigen::MatrixXcd&), and (ii) calling 
     * choi_to_superoperato(const Eigen::MatrixXcd&).
     */
    Eigen::MatrixXcd process_to_superoperator(const Eigen::MatrixXcd& process_matrix);
    /**
     * @brief Convert an STL-based process matrix to its STL-based superoperator matrix representation.
     * 
     * Arguments: 
     * @param process_matrix the STL-based (std::vector<std::vector<T>>) process matrix in the 
     * standard Pauli basis ordered from II..I, II..X, ... ZZ..Y, ZZ..Z
     *
     * @return std::vector<std::vector<T>> superoperator matrix representation.
     * 
     * @details This function transforms arbitrary STL-based process matrices to their superoperator 
     * representation by delegating the transformation to the Eigen-based implementation.
     */
    KrausOperator::Matrix process_to_superoperator(const KrausOperator::Matrix& process_matrix);

    /**
     * @brief Convert an Eigen-based process matrix to a std::vector of Kraus matrices.
     * 
     * Arguments: 
     * @param process_matrix the Eigen-based process matrix of the quantum process.
     *
     * @return std::vector<Eigen::MatrixXcd> a vector containing all complex Kraus operator matrices.
     * 
     * @details This function transforms arbitrary Eigen-based process matrices to their Kraus 
     * representation by (i) transforming the process matrix to its Choi representation and (ii) 
     * transforming the Choi matrix to individual Kraus operators. 
     */
    std::vector<Eigen::MatrixXcd> process_to_kraus(const Eigen::MatrixXcd& process_matrix);

    /**
     * @brief Convert an STL-based process matrix to a NoiseChannel of Kraus matrices.
     * 
     * Arguments: 
     * @param process_matrix the STL-based process matrix of the quantum process.
     *
     * @return NoiseChannel a noise channel containing all complex Kraus operator matrices.
     * 
     * @details This function transforms arbitrary STL-based process matrices to a NoiseChannel 
     * object by (i) converting the STL-based process matrix to an Eigen matrix, (ii) calling 
     * process_to_kraus(const Eigen::MatrixXcd&), and (iii) calling eigen_to_noisechannel.
     */
    NoiseChannel process_to_kraus(const KrausOperator::Matrix& process_matrix);

    //----------------------------------------------- Transformations from Choi matrix -----------------------------------------------

    /**
     * @brief Convert an Eigen-based Choi matrix to its Eigen-based superoperator matrix representation.
     * 
     * Arguments: 
     * @param choi_matrix the Eigen-based (Eigen::MatrixXcd) Choi matrix in the computational basis 
     * in ascending bit string order (|0..0><0..0|, |0..0><0..1|, ..., |1..1><1..0|, |1..1><1..1|).
     *
     * @return Eigen::MatrixXcd superoperator matrix representation.
     * 
     * @details This function may be used to convert arbitrary Eigen-based quantum process 
     * matrices in the Choi representation to their superoperator representation by applying 
     * a matrix reshuffling.
     */
    Eigen::MatrixXcd choi_to_superoperator(const Eigen::MatrixXcd& choi_matrix);

    /**
     * @brief Convert an STL-based Choi matrix to its STL-based superoperator matrix representation.
     * 
     * Arguments: 
     * @param choi_matrix the STL-based (std::vector<std::vector<T>>) Choi matrix in the computational basis 
     * in ascending bit string order (|0..0><0..0|, |0..0><0..1|, ..., |1..1><1..0|, |1..1><1..1|).
     *
     * @return std::vector<std::vector<T>> superoperator matrix representation.
     * 
     * @details This function transforms arbitrary STL-based Choi matrices to their superoperator 
     * representation by delegating the transformation to the Eigen-based implementation.
     */
    KrausOperator::Matrix choi_to_superoperator(const KrausOperator::Matrix& choi_matrix);

    /**
     * @brief Convert an Eigen-based Choi matrix to a std::vector of Kraus matrices.
     * 
     * Arguments: 
     * @param choi_matrix the Eigen-based Choi matrix of the quantum process.
     *
     * @return std::vector<Eigen::MatrixXcd> a vector containing all complex Kraus operator matrices.
     * 
     * @details This function will convert arbitrary Eigen-based Choi matrices to their Kraus 
     * representation by (i) obtaining eigenvalues and eigenvectors of the Choi matrix, and 
     * (ii) building Kraus matrics via sqrt(l)*unvec(v) for eigenvalues |l| > 1e-14 and 
     * matrix-reshaped eigenvectors v. 
     */
    std::vector<Eigen::MatrixXcd> choi_to_kraus(const Eigen::MatrixXcd& choi_matrix);

    /**
     * @brief Convert an STL-based Choi matrix to a NoiseChannel of Kraus matrices.
     * 
     * Arguments: 
     * @param choi_matrix the STL-based Choi matrix of the quantum process.
     *
     * @return NoiseChannel the noise channel containing all complex Kraus operator matrices.
     * 
     * @details This function will convert the STL-based Choi matrix to a complex-valued 
     * Eigen matrix and delegate the conversion to choi_to_kraus(const Eigen::MatrixXcd&).
     */
    NoiseChannel choi_to_kraus(const KrausOperator::Matrix& choi_matrix);

    //------------------------------------------- Transformations from superoperator matrix ------------------------------------------

    /**
     * @brief Convert an Eigen-based superoperator matrix to its Eigen-based Choi representation.
     * 
     * Arguments: 
     * @param superop the Eigen-based (Eigen::MatrixXcd) superoperator matrix representation 
     * of the quantum process.
     *
     * @return Eigen::MatrixXcd Choi matrix representation.
     * 
     * @details This function may be used to convert arbitrary Eigen-based quantum process 
     * matrices in the superoperator representation to their Choi representation by applying 
     * a matrix reshuffling.
     */
    Eigen::MatrixXcd superoperator_to_choi(const Eigen::MatrixXcd& superop);

    /**
     * @brief Convert an STL-based superoperator matrix to its STL-based Choi matrix representation.
     * 
     * Arguments: 
     * @param superop the STL-based (std::vector<std::vector<T>>) superoperator matrix representation
     * of the quantum process.
     *
     * @return std::vector<std::vector<T>> Choi matrix representation.
     * 
     * @details This function transforms arbitrary STL-based superoperator matrices to their Choi 
     * representation by delegating the transformation to the Eigen-based implementation.
     */
    KrausOperator::Matrix superoperator_to_choi(const KrausOperator::Matrix& superop);

    /**
     * @brief Convert an Eigen-based superoperator matrix to a std::vector of Kraus matrices.
     * 
     * Arguments: 
     * @param superop the Eigen-based (Eigen::MatrixXcd) superoperator matrix representation 
     * of the quantum process.
     *
     * @return std::vector<Eigen::MatrixXcd> a vector containing all complex Kraus operator matrices.
     * 
     * @details This function will convert arbitrary Eigen-based superoperator matrices to their Kraus 
     * representation by (i) transforming from superoperator to Choi representation, (ii) obtaining 
     * eigenvalues and eigenvectors of the Choi matrix, and (iii) building Kraus matrics via 
     * sqrt(l)*unvec(v) for eigenvalues |l| > 1e-14 and matrix-reshaped eigenvectors v. 
     */
    std::vector<Eigen::MatrixXcd> superoperator_to_kraus(const Eigen::MatrixXcd& superop);

    /**
     * @brief Convert an STL-based superoperator matrix to a NoiseChannel of Kraus matrices.
     * 
     * Arguments: 
     * @param superop the STL-based (std::vector<std::vector<T>>) superoperator matrix representation
     * of the quantum process.
     *
     * @return NoiseChannel the noise channel containing all complex Kraus operator matrices.
     * 
     * @details This function will convert the STL-based superoperator matrix to a complex-valued 
     * Eigen matrix and delegate the conversion to superoperator_to_kraus(const Eigen::MatrixXcd&).
     */
    NoiseChannel superoperator_to_kraus(const KrausOperator::Matrix& superop);

    //------------------------------------------- Transformations from Kraus representation ------------------------------------------

    /**
     * @brief Convert a vector of Eigen-based Kraus operator matrices into their Choi matrix representation.
     * 
     * Arguments: 
     * @param kraus_mats a std::vector of Eigen-based, complex-valued Kraus matrices.
     *
     * @return Eigen::MatrixXcd the Choi representation.
     */
    Eigen::MatrixXcd kraus_to_choi(const std::vector<Eigen::MatrixXcd> &kraus_mats);

    /**
     * @brief Convert a noise channel (list of STL-based Kraus operator matrices) into their Choi matrix representation.
     * 
     * Arguments: 
     * @param noise_channel the noise channel composed of STL-based Kraus matrices.
     *
     * @return KrausOperator::Matrix the STL-based Choi matrix representation.
     */
    KrausOperator::Matrix kraus_to_choi(const NoiseChannel& noise_channel);

    /**
     * @brief Convert a vector of Eigen-based Kraus operator matrices into their Eigen-based superoperator 
     * matrix representation.
     * 
     * Arguments: 
     * @param kraus_mats a std::vector of Eigen-based, complex-valued Kraus matrices.
     *
     * @return Eigen::MatrixXcd the superoperator representation.
     */
    Eigen::MatrixXcd kraus_to_superoperator(const std::vector<Eigen::MatrixXcd> &kraus_mats);

    /**
     * @brief Convert a noise channel (list of STL-based Kraus operator matrices) into their STL-based superoperator
     *  matrix representation.
     * 
     * Arguments: 
     * @param noise_channel the noise channel composed of STL-based Kraus matrices.
     *
     * @return KrausOperator::Matrix the STL-based superoperator matrix representation.
     */
    KrausOperator::Matrix kraus_to_superoperator(const NoiseChannel& noise_channel);

    //================================================================================================================================

    /**
     * @brief Convert a std::vector of Eigen-based Kraus matrices to a NoiseChannel object.
     * 
     * Arguments: 
     * @param kraus_mats a std::vector of complex-valued Kraus matrices.
     *
     * @return NoiseChannel the noise channel containing all complex Kraus operator matrices.
     */
    NoiseChannel eigen_to_noisechannel(const std::vector<Eigen::MatrixXcd>& kraus_mats);

    /**
     * @brief Compute the process fidelity of a noisy quantum channel.
     *
     * @param noise_channel Quantum noise channel
     * @return The process fidelity (as compared to an identity channel, aka no
     * noise)
     */
    double process_fidelity(const NoiseChannel &noise_channel);

    /**
     * @brief Amplitude damping channel factory
     *
     */
    struct AmplitudeDampingChannel
    {
        static constexpr char name[] = "amplitude_damp";

        static NoiseChannel Create(size_t q, double gamma);
    };

    /**
     * @brief Phase damping channel factory
     *
     */
    struct PhaseDampingChannel
    {
        static constexpr char name[] = "phase_damp";

        static NoiseChannel Create(size_t q, double gamma);
    };

    /**
     * @brief Depolarizing channel factory
     *
     */
    struct DepolarizingChannel
    {
        static constexpr char name[] = "depolarize";

        /**
         * @brief Create single-qubit depolarizing channel (balanced/symmetric)
         *
         * @param q Qubit index
         * @param p Total depolarizing probability
         * @return NoiseChannel
         */
        static NoiseChannel Create(size_t q, double p);

        /**
         * @brief Create two-qubit depolarizing channel (balanced/symmetric)
         *
         * @param q1 First qubit
         * @param q2 Second qubit
         * @param p Total depolarizing probability
         * @return NoiseChannel
         */
        static NoiseChannel Create(size_t q1, size_t q2, double p);
    };

    /**
     * @brief Generalized Single-qubit combined phase and amplitude damping quantum error channel
     *
     */
    struct GeneralizedPhaseAmplitudeDampingChannel
    {
        static constexpr char name[] = "generalized_phase_amplitude_damp";
        /**
         * @brief Create a generalized amplitude and phase damping channel
         *
         * @param q Qubit
         * @param excited_state_population Excited state population
         * @param param_amp Amplitude damping parameter
         * @param param_phase Phase damping parameter
         * @return NoiseChannel List of Kraus op
         */
        static NoiseChannel Create(
            size_t q, double excited_state_population, double param_amp, double param_phase);
    };

    /**
     * @brief Generalized amplitude damping channel factory
     *
     */
    struct GeneralizedAmplitudeDampingChannel
    {
        static constexpr char name[] = "generalized_amplitude_damp";
        /**
         * @brief Create a generalized amplitude damping channel
         *
         * @param q Qubit
         * @param excited_state_population Excited state population
         * @param gamma Amplitude damping parameter
         * @return List of Kraus op
         */
        static NoiseChannel Create(
            size_t q, double excited_state_population, double gamma);
    };

    /**
     * @brief Convert input Kraus operator matrix to noise channel
     * 
     */
    struct krausOpToChannel {
      static constexpr char name[] = "custom_kraus";
      static NoiseChannel Create(std::vector<size_t> qubits, std::vector<Eigen::MatrixXcd> kraus_ops_eigen);
    };
}

#endif
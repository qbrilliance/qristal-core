// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#ifndef _QB_NOISE_CHANNEL_
#define _QB_NOISE_CHANNEL_

#include <complex>
#include <vector>
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>
#include <optional>
#include <iostream>

namespace qristal
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

        /**
         * @brief Probability of each Kraus matrix.
         * 
         */
        double prob;
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
     * @param qubits Vector of qubit indices that the Kraus operators act on
     * @param kraus_ops_eigen Vector of Kraus operators in Eigen matrix type
     * @param kraus_probs (Optional) Vector of Kraus operator probability. Each probability
     * corresponds to the contribution of each Kraus matrix to the noise channel. Providing
     * this will help speed up the qsim state vector-based backend simulation.
     */
    struct krausOpToChannel {
      static constexpr char name[] = "custom_kraus";
      static NoiseChannel Create(std::vector<size_t> qubits, std::vector<Eigen::MatrixXcd> kraus_ops_eigen,
          std::optional<std::vector<double>> kraus_probs = std::nullopt);
    };

    //============================================ Process matrix interpolation methods ============================================

    /**
     * @brief Create a 1-qubit process matrix from a U3 rotation gate. The U3 gate is a generic 1-qubit rotation
     * gate with 3 Euler angles; U3(theta, phi, lambda). 1-qubit rotation gates Rx(theta_x), Ry(theta_y) and
     * Rz(theta_z) can be generated from the U3 gate as follows:
     * Rx(theta_x) = U3(theta_x, -pi/2, pi/2)
     * Ry(theta_y) = U3(theta_y, 0, 0)
     * Rz(theta_z) = U3(0, 0, theta_z)
     * See https://docs.quantum.ibm.com/api/qiskit/qiskit.circuit.library.U3Gate for more details.
     *  
     * @param theta Euler angle \theta
     * @param phi Euler angle \phi
     * @param lambda Euler angle \lambda
     * @return 1-qubit process matrix
     */
    Eigen::MatrixXcd createIdealU3ProcessMatrix(double theta, double phi, double lambda);

    /**
     * @brief Create a noisy N-qubit process matrix
     * 
     * @param nb_qubits number of qubits
     * @param theta vector containing the Euler rotation angle \theta of all qubits
     * @param phi vector containing the Euler rotation angle \phi of all qubits
     * @param lambda vector containing the Euler rotation angle \lambda of all qubits
     * @param channel_params vector containing noise channel parameters of all qubits
     * @return Output process matrix 
     */
    Eigen::MatrixXcd createNQubitNoisyProcessMatrix(size_t nb_qubits,
        std::vector<double> theta, std::vector<double> phi, std::vector<double> lambda,
        Eigen::VectorXd channel_params);

    /**
     * @brief Create a noisy 1-qubit process matrix
     * 
     * @param theta qubit's Euler rotation angle \theta
     * @param phi qubit's Euler rotation angle \phi
     * @param lambda qubit's Euler rotation angle \lambda
     * @param channel_params vector containing noise channel damping parameters of qubit
     * @return Output process matrix 
     */
    Eigen::MatrixXcd create1QubitNoisyProcessMatrix(double theta, double phi, double lambda,
        Eigen::VectorXd channel_params);

    /**
     * @brief Generic functor (function vector) to use Eigen's numerical differentiator Eigen::NumericalDiff
     * 
     * @param Scalar_ Scalar function to solve
     * @param NX Number of inputs
     * @param NY Value of inputs
     */
    template <typename Scalar_, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
    struct EigenNumericalDiffFunctor {
      // Information that tells the caller the numeric type (eg. double) and size (input / output dim)
      typedef Scalar_ Scalar;
      enum { InputsAtCompileTime = NX, ValuesAtCompileTime = NY }; // Required by numerical differentiation module.
      // Tell the caller the matrix sizes associated with the input, output, and jacobian
      typedef Eigen::Matrix<Scalar, InputsAtCompileTime, 1> InputType;
      typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, 1> ValueType;
      typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, InputsAtCompileTime> JacobianType;

      // Local copy of the number of inputs
      const int m_inputs, m_values;

      // Constructors:
      EigenNumericalDiffFunctor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
      EigenNumericalDiffFunctor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

      // Get methods for users to determine function input and output dimensions
      int inputs() const { return m_inputs; }
      int values() const { return m_values; }
    };

    /**
     * @brief Functor (function vector) to use Eigen's Levenberg-Marquardt (Eigen::LevenbergMarquardt)
     * nonlinear solver to solve for process matrix.
     * 
     * @param input_vec Process matrix flatted to a Eigen::VectorXcd
     * @param nb_qubits Number of qubits
     * @param theta vector containing the Euler rotation angles \theta of all qubits
     * @param phi vector containing the Euler rotation angles \phi all qubits
     * @param lambda vector containing the Euler rotation angles \lambda of all qubits
     * @param m Size of input_vector
     * @param n Number of parameters to solve for
     */
    struct LMFunctorNoisy : qristal::EigenNumericalDiffFunctor<double> {
      LMFunctorNoisy(void): qristal::EigenNumericalDiffFunctor<double>(m = 0, n = 0) {}
      int operator()(const Eigen::VectorXd &x, Eigen::VectorXd &fvec) const {
        // Create noisy process matrix with input angles and noise channel parameters x.
        Eigen::MatrixXcd guess_mat;
        if (nb_qubits == 1) {
          guess_mat = qristal::create1QubitNoisyProcessMatrix(theta[0], phi[0], lambda[0], x);
        } else {
          guess_mat = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta, phi, lambda, x);
        }
        // To use Eigen's numerical differentiation, we need to convert the complex matrix into a complex vector.
        Eigen::VectorXcd guess_vec = Eigen::Map<Eigen::VectorXcd>(guess_mat.data(), guess_mat.rows() * guess_mat.cols());
        // Solve for x's by minimizing the difference between the input process matrix and the generated noisy process matrix
        fvec = (input_vec - guess_vec).cwiseAbs();

        return 0;
      }

      Eigen::VectorXcd input_vec; // Input process matrix
      size_t nb_qubits; // Number of qubits
      std::vector<double> theta; // Euler rotation angle \theta
      std::vector<double> phi; // Euler rotation angle \phi
      std::vector<double> lambda; // Euler rotation angle \lambda
      int m; // Number of data points, i.e. values.
      int n; // The number of parameters, i.e. inputs.
      int values() const { return m; } // Returns 'm', the number of values.
      int inputs() const { return n; } // Returns 'n', the number of inputs.
    };

    /**
     * @brief Solves noise channel parameters for an input N-qubit process matrix. Current noise channels:
     *  - generalized phase and amplitude damping
     *  - 1-qubit depolarization
     * 
     * @param process_matrix_1qubit vector of 1-qubit process matrices
     * @param process_matrix_Nqubit N-qubit process matrix
     * @param nb_qubits Number of qubits
     * @param theta vector containing the Euler rotation angle \theta of all qubits
     * @param phi vector containing the Euler rotation angle \phi of all qubits
     * @param lambda vector containing the Euler rotation angle \lambda of all qubits
     * @param max_iter Maximum number of solver iterations (default = 1000)
     * @param maxfev Maximum number of function evaluation (default = 1000)
     * @param xtol tolerance for the norm of the solution vector (default = 1e-8)
     * @param ftol tolerance for the norm of the vector function (default = 1e-8)
     * @param gtol tolerance for the norm of the gradient of the error vector (default = 1e-8)
     * 
     * @return Vector containing solved noise channel parameters
     */
    Eigen::VectorXd processMatrixSolverNQubit(std::vector<Eigen::MatrixXcd> process_matrix_1qubit,
        Eigen::MatrixXcd process_matrix_Nqubit, size_t nb_qubits,
        std::vector<double> theta, std::vector<double> phi, std::vector<double> lambda, size_t max_iter = 1000,
        size_t maxfev = 1000, double xtol = 1e-8, double ftol = 1e-8, double gtol = 1e-8);

    /**
     * @brief Solves noise channel parameters for an input 1-qubit process matrix. Current noise channels:
     *  - generalized phase and amplitude damping
     *  - 1-qubit depolarization
     * 
     * @param process_matrix_1qubit 1-qubit process matrices
     * @param nb_qubits Number of qubits
     * @param theta qubit's Euler rotation angle \theta
     * @param phi qubit's Euler rotation angle \phi
     * @param lambda qubit's Euler rotation angle \lambda
     * @param max_iter Maximum number of solver iterations (default = 1000)
     * @param maxfev Maximum number of function evaluation (default = 1000)
     * @param xtol tolerance for the norm of the solution vector (default = 1e-8)
     * @param ftol tolerance for the norm of the vector function (default = 1e-8)
     * @param gtol tolerance for the norm of the gradient of the error vector (default = 1e-8)
     * 
     * @return Vector containing solved noise channel parameters
     */
    Eigen::VectorXd processMatrixSolver1Qubit(Eigen::MatrixXcd process_matrix,
        double theta, double phi, double lambda, size_t max_iter = 1000,
        size_t maxfev = 1000, double xtol = 1e-8, double ftol = 1e-8, double gtol = 1e-8);

    /**
     * @brief Internal functionality for process matrix solver. Contains the 2 loops: first loop is a solve
     * for the rough channel parameters. The second loops takes the roughly-solved values as input and
     * does a few fine solving to improve accuracy. 
     * 
     * @param nb_qubits Number of qubits
     * @param nb_params Number of parameters in the solution vector (3 for current available noise channels)
     * @param lm Eigen::LevenbergMarquardt solver object
     * @param guess_params Input guess vector (optional). Generates random guess value if not provided
     * 
     * @return Vector containing solved noise channel parameters
     */
    Eigen::VectorXd processMatrixSolverInternal(size_t nb_qubits, size_t nb_params, size_t max_iter,
        Eigen::LevenbergMarquardt<Eigen::NumericalDiff<qristal::LMFunctorNoisy>, double> lm,
        std::optional<Eigen::VectorXd> guess_params = std::nullopt);

    /**
     * @brief Creates an interpolated process matrix at angle {theta, phi, lambda} using
     * the average noise channel damping parameters of 2 input process matrices
     * 
     * @param nb_qubits Number of qubits
     * @param process_matrix_1qubit_1 vector of 1-qubit process matrices at Euler angle {theta1, phi1, lambda1}
     * @param process_matrix_1qubit_2 vector of 1-qubit process matrices at Euler angle {theta2, phi2, lambda2}
     * @param process_matrix_Nqubit_1 N-qubit process matrix at Euler angle {theta1, phi1, lambda1}
     * @param process_matrix_Nqubit_2 N-qubit process matrix at Euler angle {theta2, phi2, lambda2}
     * @param theta1 vector containing the x-rotation angle of all qubits for the set of Euler angles {theta1, phi1, lambda1}
     * @param phi1 vector containing the y-rotation angle of all qubits for the set of Euler angles {theta1, phi1, lambda1}
     * @param lambda1 vector containing the z-rotation angle of all qubits for the set of Euler angles {theta1, phi1, lambda1}
     * @param theta2 vector containing the x-rotation angle of all qubits for the set of Euler angles {theta2, phi2, lambda2}
     * @param phi2 vector containing the y-rotation angle of all qubits for the set of Euler angles {theta2, phi2, lambda2}
     * @param lambda2 vector containing the z-rotation angle of all qubits for the set of Euler angles {theta2, phi2, lambda2}
     * @param theta_target \theta target angle of output process matrix
     * @param phi_target \phi target angle of output process matrix
     * @param lambda_target \lambda target angle of output process matrix
     * @param max_iter Maximum number of solver iterations (default = 1000)
     * @param maxfev Maximum number of function evaluation (default = 1000)
     * @param xtol Tolerance for the norm of the solution vector (default = 1e-8)
     * @param ftol tolerance for the norm of the vector function (default = 1e-8)
     * @param gtol Tolerance for the norm of the gradient of the error vector (default = 1e-8)
     *
     * @return Output process matrix
     */
    Eigen::MatrixXcd processMatrixInterpolator(size_t nb_qubits,
        std::vector<Eigen::MatrixXcd> process_matrix_1qubit_1, std::vector<Eigen::MatrixXcd> process_matrix_1qubit_2,
        Eigen::MatrixXcd process_matrix_Nqubit_1, Eigen::MatrixXcd process_matrix_Nqubit_2,
        std::vector<double> theta1, std::vector<double> phi1, std::vector<double> lambda1,
        std::vector<double> theta2, std::vector<double> phi2, std::vector<double> lambda2,
        double theta_target, double phi_target, double lambda_target,
        size_t max_iter = 1000, size_t maxfev = 1000, double xtol = 1e-8, double ftol = 1e-8, double gtol = 1e-8);
}

#endif
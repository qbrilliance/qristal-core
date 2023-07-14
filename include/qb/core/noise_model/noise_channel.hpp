// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#pragma once
#include <complex>
#include <vector>

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
     * @brief Convert a noise channel (list of Kraus operator matrices) into a Choi matrix representation
     *
     * @return Choi matrix
     */
    KrausOperator::Matrix kraus_to_choi(const NoiseChannel& noise_channel);

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
}
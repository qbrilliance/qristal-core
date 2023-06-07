// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#pragma once

#include <unordered_map>
#include "json_complex_convert.hpp"
#include "noise_channel.hpp"
#include "noise_properties.hpp"
#include "readout_error.hpp"

namespace qb
{
    /**
     * @brief Noise Model class
     * This allows specification noise parameters for
     * each quantum gate.
     */
    class NoiseModel
    {
    public:
        NoiseModel() = default;

        /**
         * @brief Construct a new Noise Model object from JSON
         *
         * @param js JSON object contains noise model parameters
         */
        NoiseModel(const nlohmann::json &js);

        /**
         * @brief Construct a new Noise Model object from noise properties
         *
         * @param noise_props Device noise properties
         */
        NoiseModel(const NoiseProperties &noise_props);

        /**
         * @brief Convert noise model to json string
         *
         * @return JSON string
         */
        std::string to_json() const;

        /**
         * @brief Get the connectivity (pairs of connected qubits)
         * 
         * @return Connectivity as list of qubit index pairs
         */
        std::vector<std::pair<int, int>> get_connectivity() const;

        /**
         * @brief Add a gate error channel for a gate operation
         *
         * @param noise_channel Noise channel to be associated with the gate
         * @param gate_name Name of the gates
         * @param qubits Qubit operands of the gate.
         */
        void add_gate_error(const NoiseChannel &noise_channel, const std::string &gate_name, const std::vector<size_t> &qubits);

        /**
         * @brief Added connected qubit pair to the topology model
         * 
         * @param q1 First qubit index
         * @param q2 Second qubit index
         */
        void add_qubit_connectivity(int q1, int q2);
        
        /**
         * @brief Set the qubit readout error 
         * 
         * @param qubitIdx Qubit to set
         * @param ro_error Readout error
         */
        void set_qubit_readout_error(size_t qubitIdx, const ReadoutError& ro_error);
        
        /**
         * @brief Type of qubit connectivity
         *
         */
        enum class QubitConnectivity
        {
            AllToAll,
            Custom
        };

        /// @brief  Retrieve the name of the QObj compiler to use with the AER
        /// simulator
        /// @return Name of the QObj compiler
        virtual std::string get_qobj_compiler() const;

      protected:
        /**
         * @brief Calculate component of Pauli error caused by decoherence on a single qubit
         *
         * @param t1 T1 time
         * @param tphi Tphi time
         * @param gate_time Duration of the gate affected by this error.
         * @return Calculated Pauli error resulting from decoherence.
         */
        double decoherence_pauli_error(double t1, double tphi, double gate_time);

    protected:
        /// @brief Gate noise channel registry
        /// Map from gate name -> a map of qubit operands -> noise channel
        /// If the noise is uniform (qubit independent), use empty vector for qubit operands.
        std::unordered_map<std::string, std::map<std::vector<size_t>, std::vector<NoiseChannel>>> m_noise_channels;
        /// @brief Readout errors
        std::unordered_map<size_t, ReadoutError> m_readout_errors;
        /// @brief Qubit connectivity 
        std::vector<std::pair<int, int>> m_qubit_topology;
        /// @brief Noise model Json conforming to IBM Qiskit QObj schema if provided.
        nlohmann::json m_qobj_noise_model;
    };
}

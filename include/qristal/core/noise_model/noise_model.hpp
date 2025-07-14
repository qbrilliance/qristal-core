// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once

#include <unordered_map>
#include <optional>
#include <functional>

#include <qristal/core/noise_model/json_complex_convert.hpp>
#include <qristal/core/noise_model/noise_channel.hpp>
#include <qristal/core/noise_model/noise_properties.hpp>
#include <qristal/core/noise_model/readout_error.hpp>
#include <qristal/core/passes/noise_aware_placement_config.hpp>

namespace qristal
{

    /**
     * @brief Noise Model class
     * This allows specification noise parameters for
     * each quantum gate.
     */
    class NoiseModel
    {

      public:

        /**
         * @brief Type of qubit connectivity
         *
         */
        enum class QubitConnectivity
        {
            AllToAll,
            Custom
        };

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
         * @brief Build and return a registered noise model.
         * Optionally allows for customization, e.g., number of qubits, if supported.
         *
         * @param name The name of the registered noise model.
         * @param nb_qubits Number of qubits
         * @param connectivity [Optional] Connectivity pattern
         * @param connected_pairs [Optional] Custom connectivity topology (if not AllToAll)
         * @return Noise model object
         *
         * IMPORTANT NOTE: The ability to build and return some noise models requires the Qristal Emulator library libqristal_emulator.so to be found at runtime.
         * To dynamically load libqristal_emulator.so requires:
         *  (1) For Python users: libqristal_emulator.so is installed in the lib/ directory of the Qristal core package (i.e., the same location as the core Python package itself)
         *  (2) For C++ users: if building an executable linking against libqristal_core, you need to set the rpath of your executable target to include the directory where libqristal_emulator.so located.
         * In both cases, LD_LIBRARY_PATH can **also** be used to point the dynamic linker to the location of libqristal_emulator.so (if setting rpath fails).
         */
        NoiseModel(const std::string& name,
                   size_t nb_qubits,
                   std::optional<QubitConnectivity> connectivity = std::nullopt,
                   std::optional<std::reference_wrapper<const std::vector<std::pair<size_t, size_t>>>> connected_pairs = std::nullopt);

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
         * @brief Get the readout errors
         *
         * @return Map from qubit index to readout errors
        */
        const std::unordered_map<size_t, ReadoutError>& get_readout_errors() const;

        /**
         * @brief Get all the gate noise channels
         *
         * @return Map from (gate name + qubit operands) to noise channels
        */
        const std::unordered_map<std::string, std::map<std::vector<size_t>, std::vector<NoiseChannel>>>& get_noise_channels() const;

        /**
         * @brief Add a gate error channel for a gate operation
         *
         * @param noise_channel Noise channel to be associated with the gate
         * @param gate_name Name of the gates
         * @param qubits Qubit indices of the gate.
         */
        void add_gate_error(const NoiseChannel &noise_channel, const std::string &gate_name, const std::vector<size_t> &qubits);

        /**
         * @brief Add a connected qubit pair to the topology model
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

        /// @brief  Retrieve the name of the QObj compiler to use with the AER
        /// simulator
        /// @return Name of the QObj compiler
        std::string get_qobj_compiler() const;

        /// @brief  Set the name of the QObj compiler to use with the AER
        /// simulator
        /// Throw if the compiler name is invalid.
        void set_qobj_compiler(const std::string& qobj_compiler);

        /// @brief Return the list of basis gates that the AER QObj will be
        /// referring to.
        /// @return List of basis gates.
        /// Note: This corresponds to which QObj compiler being set.
        /// The AER simulator will only look for gate noise associated with
        /// these gates.
        std::vector<std::string> get_qobj_basis_gates() const;

        /// The colloquial name of the noise model
        std::string name;

        /// Conversion into a noise_aware_placement_config
        operator noise_aware_placement_config() const noexcept {
          return to_noise_aware_placement_config();
        }

      protected:

        /**
         * @brief Build and return the default noise model.
         * Optionally allows for customization, e.g., number of qubits, if supported.
         *
         * @param make_from_this The NoiseModel instance to build into the default noise model.
         * @param nb_qubits Number of qubits
         * @param connectivity [Optional] Connectivity pattern
         * @param connected_pairs [Optional] Custom connectivity topology (if not AllToAll)
         * @return Noise model object
         */
        static void make_default(NoiseModel& make_from_this,
                                 size_t nb_qubits,
                                 QubitConnectivity connectivity = QubitConnectivity::AllToAll,
                                 const std::vector<std::pair<size_t, size_t>> &connected_pairs = {});

        /**
         * @brief Calculate component of Pauli error caused by decoherence on a single qubit
         *
         * @param t1 T1 time
         * @param tphi Tphi time
         * @param gate_time Duration of the gate affected by this error.
         * @return Calculated Pauli error resulting from decoherence.
         */
        double decoherence_pauli_error(double t1, double tphi, double gate_time);

        /**
         * @brief Helper method to convert this NoiseModel into a noise aware placement configuration
         *
         * @return noise_aware_placement_config
         */
        noise_aware_placement_config to_noise_aware_placement_config() const;

        /// @brief Gate noise channel registry
        /// Map from gate name -> a map of qubit operands -> noise channel
        /// If the noise is uniform (qubit independent), use empty vector for qubit operands.
        std::unordered_map<std::string, std::map<std::vector<size_t>, std::vector<NoiseChannel>>> m_noise_channels;

        /// @brief Create qristal noise properties and channels from type nlohmann::json
        void initialise_noise_properties_from_json(const nlohmann::json &js);

        /// @brief Readout errors
        std::unordered_map<size_t, ReadoutError> m_readout_errors;

        /// @brief Qubit connectivity
        std::vector<std::pair<int, int>> m_qubit_topology;

        /// @brief Noise model Json conforming to IBM Qiskit QObj schema if provided.
        nlohmann::json m_qobj_noise_model;

        /// @brief Name of the QObj compiler to use with the AER simulator
        // The default "xacc-qobj" compiler from XACC
        std::string m_qobj_compiler = "xacc-qobj";
    };
}

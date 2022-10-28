// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#pragma once

#include "noise_model.hpp"

namespace qb
{
    /**
     * @brief Abstract class for Noise Model factory object
     * 
     */
    struct NoiseModelFactory
    {
        /**
         * @brief Create a concrete noise model
         * Optionally allows for customization, e.g., number of qubits, if supported.
         *
         * @param nb_qubits [Optional] Number of qubits
         * @param connectivity [Optional] Connectivity pattern
         * @param connected_pairs [Optional] Custom connectivity topology (if not AllToAll)
         * @return Noise model object
         */
        virtual NoiseModel Create(size_t nb_qubits = 0, NoiseModel::QubitConnectivity connectivity = NoiseModel::QubitConnectivity::AllToAll, const std::vector<std::pair<size_t, size_t>> &connected_pairs = {}) = 0;
    };

    /**
     * @brief Get the noise model factory object
     *
     * @param name Name of the noise model factory 
     * Must be a valid one.
     * @return Corresponding noise model factory if present, null otherwise.
     * e.g., requesting custom Quantum Brilliance noise model without a proper Emulator installation.
     */
    std::shared_ptr<NoiseModelFactory> get_noise_model_factory(const std::string &name);
}
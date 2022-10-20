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
        virtual NoiseModel Create(size_t nb_qubits, NoiseModel::QubitConnectivity connectivity = NoiseModel::QubitConnectivity::AllToAll, const std::vector<std::pair<size_t, size_t>> &connected_pairs = {}) = 0;
    };

    const std::vector<std::string> VALID_NOISE_MODEL_FACTORIES = {
        "default",
        "custom",
#ifdef _HAS_QB_EMULATOR
        // If Emulator is present
        "qb-gen1",
        "qb-gen2",
#endif
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
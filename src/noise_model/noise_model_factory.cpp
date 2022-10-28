// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/noise_model/noise_model_factory.hpp"
#include <dlfcn.h>
#include <iostream>
namespace qb
{
    /**
     * @brief Simple factory wrapper of a provided noise model.
     * This doesn't allow for customization of the generated noise model.
     * 
     */
    struct NoiseModelWrapperFactory : public NoiseModelFactory
    {
        NoiseModel noise_model;
        NoiseModelWrapperFactory(NoiseModel *nm) : noise_model(*nm) {}
        virtual NoiseModel Create(
            size_t, NoiseModel::QubitConnectivity,
            const std::vector<std::pair<size_t, size_t>> &) override
        {
            return noise_model;
        }
    };

    std::shared_ptr<NoiseModelFactory> load_emulator_noise_model_factory(const std::string &name)
    {
        // Emulator noise model retrieval function signature
        using func_type = NoiseModel *(const char *);
        /**
         * IMPORTANT NOTE: The ability to dynamically load libqbemulator.so (externally provided) requires:
         * (1) For Python users: libqbemulator.so is installed in the lib/ directory of the QB's core package (i.e., the same location as the core Python package itself)
         * (2) For C++ users: if building an executable linking against libqbcore, need to set rpath of the executable target to include the directory where libqbemulator.so located.
         * In both cases, LD_LIBRARY_PATH can **also** be used to point the dynamic linker to the location of libqbemulator.so (if setting rpath failed)
         */
        static const char *EMULATOR_NOISE_MODEL_LIB_NAME = "libqbemulator.so";
        void *handle = dlopen(EMULATOR_NOISE_MODEL_LIB_NAME, RTLD_LOCAL | RTLD_LAZY);
        if (!handle)
        {
            char *error_msg = dlerror();
            throw std::runtime_error("Failed to load noise modeling library from QB emulator installation. " + (error_msg ? std::string(error_msg) : ""));
        }

        // Clear all errors
        dlerror();
        auto *get_emulator_noise_model = reinterpret_cast<func_type *>(dlsym(handle, "get_emulator_noise_model"));
        char *error_msg = dlerror();
        // Encounter an error:
        if (error_msg)
        {
            throw std::runtime_error("Failed to load noise model: " + std::string(error_msg));
        }

        return std::make_shared<NoiseModelWrapperFactory>(get_emulator_noise_model(name.c_str()));
    }

    /**
     * @brief Dummy default Noise Model generator
     * Generate a simple noise model of 99.9% gate fidelity
     *
     */
    struct DefaultNoiseModelFactory : public NoiseModelFactory
    {
        virtual NoiseModel Create(size_t nb_qubits, NoiseModel::QubitConnectivity connectivity, const std::vector<std::pair<size_t, size_t>> &connected_pairs) override
        {
            NoiseModel noise_model;
            constexpr double GATE_FIDELITY = 0.999; // 99.9%
            // Uniform readout error of 1%
            NoiseProperties::ReadoutError ro_error;
            ro_error.p_01 = 0.01;
            ro_error.p_10 = 0.01;
            // Set qubit readout errors
            for (size_t qId = 0; qId < nb_qubits; ++qId)
            {
                noise_model.set_qubit_readout_error(qId, ro_error);
            }

            // Single-qubit gates : universal standard gate set: { u1, u2, u3 }
            for (const auto &gate_name : {"u1", "u2", "u3"})
            {
                for (size_t qId = 0; qId < nb_qubits; ++qId)
                {
                    noise_model.add_gate_error(DepolarizingChannel::Create(qId, 1.0 - GATE_FIDELITY), gate_name, {qId});
                }
            }

            // Two-qubit gate
            if (connectivity == NoiseModel::QubitConnectivity::AllToAll)
            {
                for (size_t qId1 = 0; qId1 < nb_qubits; ++qId1)
                {
                    for (size_t qId2 = qId1 + 1; qId2 < nb_qubits; ++qId2)
                    {
                        noise_model.add_gate_error(DepolarizingChannel::Create(qId1, qId2, 1.0 - GATE_FIDELITY), "cx", {qId1, qId2});
                        noise_model.add_gate_error(DepolarizingChannel::Create(qId1, qId2, 1.0 - GATE_FIDELITY), "cx", {qId2, qId1});
                        noise_model.add_qubit_connectivity(qId1, qId2);
                    }
                }
            }
            else
            {
                // Connectivity topology is provided
                for (const auto &[qId1, qId2] : connected_pairs)
                {
                    noise_model.add_gate_error(DepolarizingChannel::Create(qId1, qId2, 1.0 - GATE_FIDELITY), "cx", {qId1, qId2});
                    noise_model.add_gate_error(DepolarizingChannel::Create(qId1, qId2, 1.0 - GATE_FIDELITY), "cx", {qId2, qId1});
                    noise_model.add_qubit_connectivity(qId1, qId2);
                }
            }

            return noise_model;
        }
    };

    std::shared_ptr<NoiseModelFactory> get_noise_model_factory(const std::string &name)
    {
        // Handles builtin noise models:
        if (name == "default")
        {
            return std::make_shared<DefaultNoiseModelFactory>();
        }

        // Requesting externally-provided noise models from the QB Emulator package.
        // Try to load it.
        return load_emulator_noise_model_factory(name);
    }
}
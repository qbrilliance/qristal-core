// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/noise_model/noise_model_factory.hpp"

namespace qb
{
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
        if (std::find(VALID_NOISE_MODEL_FACTORIES.begin(), VALID_NOISE_MODEL_FACTORIES.end(), name) == VALID_NOISE_MODEL_FACTORIES.end())
        {
            throw std::runtime_error("Invalid noise model factory named '" + name + "' requested!");
        }

        if (name == "default")
        {
            return std::make_shared<DefaultNoiseModelFactory>();
        }

        return nullptr;
    }
}
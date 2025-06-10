// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/noise_model/noise_model.hpp>

namespace qristal
{
   
    // Build and return an instance of the default noise model
    void NoiseModel::make_default(NoiseModel& noise_model, 
                                  size_t nb_qubits, 
                                  QubitConnectivity connectivity, 
                                  const std::vector<std::pair<size_t, size_t>> &connected_pairs)
    {
        noise_model.name = "default";
        constexpr double GATE_FIDELITY = 0.999; // 99.9%
        // Uniform readout error of 1%
        ReadoutError ro_error;
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
        if (connectivity == QubitConnectivity::AllToAll)
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
    }

}

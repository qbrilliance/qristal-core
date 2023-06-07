// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/noise_model/noise_model.hpp"

namespace qb
{
    NoiseModel::NoiseModel(const nlohmann::json &js) : m_qobj_noise_model(js)
    {
        // TODO: parse qubit connectivity topology from qObj
    }

    NoiseModel::NoiseModel(const NoiseProperties &noise_props)
    {
        // Get qubit connectivity topology
        m_qubit_topology = noise_props.qubit_topology;
        
        // Add readout errors
        m_readout_errors = noise_props.readout_errors;

        // Thermal noise:
        for (const auto &[gate_name, operands_to_durations] : noise_props.gate_time_us)
        {
            for (const auto &[qubits, gate_duration] : operands_to_durations)
            {
                // Retrieve gate error (e.g., from randomized benchmarking) if any
                const double rb_pauli_rate = [&noise_props](const std::string &gate_name, const std::vector<size_t> &qubits)
                {
                    const auto iter1 = noise_props.gate_pauli_errors.find(gate_name);
                    if (iter1 != noise_props.gate_pauli_errors.end())
                    {
                        auto &operands_to_error_rates = iter1->second;
                        const auto iter2 = operands_to_error_rates.find(qubits);
                        if (iter2 != operands_to_error_rates.end())
                        {
                            return iter2->second;
                        }
                    }
                    return 0.0;
                }(gate_name, qubits);

                for (const auto &qubit : qubits)
                {
                    const auto t1_iter = noise_props.t1_us.find(qubit);
                    const auto t2_iter = noise_props.t2_us.find(qubit);
                    assert(t1_iter != noise_props.t1_us.end());
                    assert(t2_iter != noise_props.t2_us.end());
                    const auto t1 = t1_iter->second;
                    const auto t2 = t2_iter->second;
                    const double amp_damp_rate = 1.0 - std::exp(-gate_duration / t1);
                    const double phase_damp_rate = 1.0 - std::exp(-gate_duration / t2);
                    auto thermal_relaxation = GeneralizedPhaseAmplitudeDampingChannel::Create(qubit, 0.0, amp_damp_rate, phase_damp_rate);
                    const double equiv_pauli_rate = decoherence_pauli_error(t1, t2, gate_duration);
                    // std::cout << "equiv_pauli_rate = " << equiv_pauli_rate << "\n";
                    add_gate_error(thermal_relaxation, gate_name, qubits);
                    // Adds depolarizing noise if needed.
                    if (rb_pauli_rate > equiv_pauli_rate)
                    {
                        const double p_depol = rb_pauli_rate - equiv_pauli_rate;
                        // std::cout << "Adding depol rate of: " << p_depol << " on top of thermal noises.\n";
                        add_gate_error(DepolarizingChannel::Create(qubit, p_depol), gate_name, {qubit});
                    }
                }
            }
        }
    }

    std::string NoiseModel::get_qobj_compiler() const {
      // The default "qobj" compiler from XACC
      return "qobj";
    }
    
    double NoiseModel::decoherence_pauli_error(double t1, double tphi, double gate_time)
    {
        // Formula:
        // (1/4) * [1 - e^(-t/T1)] + (1/2) * [1 - e^(-t/(2*T1) - t/Tphi]
        const auto gamma_2 = (1 / (2 * t1)) + 1 / tphi;
        const auto exp1 = std::exp(-gate_time / t1);
        const auto exp2 = std::exp(-gate_time * gamma_2);
        const auto px = 0.25 * (1 - exp1);
        const auto py = px;
        const auto pz = 0.5 * (1 - exp2) - px;
        return px + py + pz;
    }

    void NoiseModel::add_gate_error(const NoiseChannel &noise_channel, const std::string &gate_name, const std::vector<size_t> &qubits)
    {
        auto iter = m_noise_channels.find(gate_name);
        if (iter != m_noise_channels.end())
        {
            auto operands_to_noise_map = iter->second;
            auto iter2 = operands_to_noise_map.find(qubits);
            if (iter2 != operands_to_noise_map.end())
            {
                auto noise_channels =  iter2->second;
                noise_channels.emplace_back(noise_channel);
                m_noise_channels[gate_name][qubits] = noise_channels;
            }
            else
            {
                m_noise_channels[gate_name][qubits] = {noise_channel};
            }
        }
        else
        {

            std::map<std::vector<size_t>, std::vector<NoiseChannel>> gate_noise_map;
            gate_noise_map[qubits] = {noise_channel};
            // First time seeing this gate
            m_noise_channels.emplace(gate_name, std::move(gate_noise_map));
        }
    }

    void NoiseModel::set_qubit_readout_error(size_t qubitIdx, const ReadoutError &ro_error)
    {
        m_readout_errors[qubitIdx] = ro_error;
    }

    std::string NoiseModel::to_json() const
    {
        if (!m_qobj_noise_model.empty())
        {
            // If the noise model was provided as a json object, e.g., externally built by qiskit,
            // just use it as is.
            return m_qobj_noise_model.dump(6);
        }

        // Build the noise model QObj Json
        nlohmann::json noiseModel;
        std::vector<nlohmann::json> noiseElements;
        // Readout errors
        for (const auto &[qIdx, ro_error] : m_readout_errors)
        {
            const auto meas0Prep1 = ro_error.p_01;
            const auto meas1Prep0 = ro_error.p_10;

            const std::vector<std::vector<double>> probs{
                {1 - meas1Prep0, meas1Prep0}, {meas0Prep1, 1 - meas0Prep1}};
            nlohmann::json element;
            element["type"] = "roerror";
            element["operations"] = std::vector<std::string>{"measure"};
            element["probabilities"] = probs;
            element["gate_qubits"] = std::vector<std::vector<std::size_t>>{{qIdx}};
            noiseElements.push_back(element);
        }

        // Gate errors
        for (const auto &[gateName, channel_map] : m_noise_channels)
        {
            for (const auto &[qubits, noise_channels] : channel_map)
            {
                nlohmann::json element;
                element["type"] = "qerror";
                element["operations"] = std::vector<std::string>{gateName};
                element["gate_qubits"] = std::vector<std::vector<std::size_t>>{qubits};
                std::vector<nlohmann::json> instructions;
                for (const auto &noise_channel : noise_channels)
                {
                    nlohmann::json instruction;
                    instruction["name"] = "kraus";
                    auto kraus_mats = nlohmann::json::array();
                    for (const auto &kraus_op : noise_channel)
                    {
                        kraus_mats.push_back(kraus_op.matrix);
                        std::vector<int> kraus_qubits(kraus_op.qubits.size());
                        std::iota(kraus_qubits.begin(), kraus_qubits.end(), 0);
                        instruction["qubits"] = kraus_qubits;
                    }
                    instruction["params"] = kraus_mats;
                    instructions.emplace_back(instruction);
                }
                assert(instructions.size() == noise_channels.size());
                element["instructions"] =
                    std::vector<std::vector<nlohmann::json>>{instructions};
                element["probabilities"] = std::vector<double>(1, 1.0);
                noiseElements.push_back(element);
            }
        }
        noiseModel["errors"] = noiseElements;
        return noiseModel.dump(6);
    }

    std::vector<std::pair<int, int>> NoiseModel::get_connectivity() const
    {
        return m_qubit_topology;
    }

    void NoiseModel::add_qubit_connectivity(int q1, int q2)
    {
        m_qubit_topology.emplace_back(std::make_pair(q1, q2));
    }
}

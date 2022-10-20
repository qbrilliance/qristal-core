// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#pragma once
#include <map>
#include <unordered_map>
#include <vector>
namespace qb
{
    /**
     * @brief Noise-defining properties for a quantum device
     * This can be used to generate a noise model
     */
    struct NoiseProperties
    {
        struct ReadoutError
        {
            /// @brief  P(read |0> from |1>)
            double p_01;
            /// @brief P(read |1> from |0>)
            double p_10;
        };

        std::unordered_map<size_t, double> t1_us;
        std::unordered_map<size_t, double> t2_us;
        std::unordered_map<size_t, ReadoutError> readout_errors;
        std::unordered_map<std::string, std::map<std::vector<size_t>, double>> gate_time_us;
        std::unordered_map<std::string, std::map<std::vector<size_t>, double>> gate_pauli_errors;
        std::vector<std::pair<int, int>> qubit_topology;
    };
}
// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <map>
#include <unordered_map>
#include <vector>
#include <qristal/core/noise_model/readout_error.hpp>

namespace qristal
{
    /**
    * @brief Use NoiseProperties to accept user input parameters for custom noise models.  There are 3 types of inputs used for constructing a custom noise model:
    *  - Qubit topology
    *  - Time duration of quantum gate operations
    *  - Parameters for quantum noise channels and classical errors
    */
    struct NoiseProperties
    {

        /**
        * @brief **T1** is the **qubit relaxation time**.  For a qubit register, with individual qubits zero-indexed by i; `t1_us` is a map from qubit[i] -> T1[i].
        * Unit: microseconds
        *
        *     // C++ code example: 4 qubits all with T1 = 1.5us
        *     qristal::NoiseProperties t_qbnp;
        *     t_qbnp.t1_us{{0, 1.5}, {1, 1.5}, {2, 1.5}, {3, 1.5}};
        */
        std::unordered_map<size_t, double> t1_us;
        
        /**
        * @brief **T2** is the **qubit dephasing time**.  For a qubit register, with individual qubits zero-indexed by i, `t2_us` is a map from qubit[i] -> T2[i].
        * Unit: microseconds
        *
        *     // C++ code example: 4 qubits all with T2 = 0.15us
        *     qristal::NoiseProperties t_qbnp;
        *     t_qbnp.t2_us{{0, 0.15}, {1, 0.15}, {2, 0.15}, {3, 0.15}};
        */
        std::unordered_map<size_t, double> t2_us;

        /**
        * @brief **readout_errors** is the **classical readout error (off-diagonal elements of the confusion matrix)**.  For a qubit register, with individual qubits zero-indexed by i, `readout_errors` is a map from qubit[i] -> qristal::ReadoutError[i].
        * Unit: none (quantities are probabilities).
        *
        *     // C++ code example: 2 qubits with p(0|1) = p(1|0) = 0.05, 2 qubits with p(0|1) = 0.1 and p(1|0) = 0.08
        *     qristal::NoiseProperties t_qbnp;
        *     qristal::ReadoutError t_qbnpro_balanced, t_qbnpro_asym;
        *     t_qbnpro_balanced.p_01 = 0.05;
        *     t_qbnpro_balanced.p_10 = 0.05;
        *     t_qbnpro_asym.p_01 = 0.10;
        *     t_qbnpro_asym.p_10 = 0.08;
        *     t_qbnp.readout_errors{{0, t_qbnpro_balanced}, {1, t_qbnpro_balanced}, {2, t_qbnpro_asym}, {3, t_qbnpro_asym}};
        */       
        std::unordered_map<size_t, ReadoutError> readout_errors;

        /**
        * @brief **gate_time_us** is the duration for a quantum gate operation when applied at a target set of qubits.
        * Unit: microseconds
        *
        *     // C++ code example: "u3" single-qubit gate, uniform duration 5.2us, 4 qubits
        *     qristal::NoiseProperties t_qbnp;
        *     t_qbnp.gate_time_us{{"u3",{{0},5.2}}, {"u3",{{1},5.2}}, {"u3",{{2},5.2}}, {"u3",{{3},5.2}}};
        */
        std::unordered_map<std::string, std::map<std::vector<size_t>, double>> gate_time_us;

        /**
        * @brief **gate_pauli_errors** is the parameter for gate error derived from randomized benchmarking of a quantum gate operation that is applied at a target set of qubits.
        * Unit: none
        *
        *     // C++ code example: "u3" single-qubit gate, gate error parameter = 0.03, uniform for 4 qubits
        *     qristal::NoiseProperties t_qbnp;
        *     t_qbnp.gate_pauli_errors{{"u3",{{0},0.03}}, {"u3",{{1},0.03}}, {"u3",{{2},0.03}}, {"u3",{{3},0.03}}};
        */
        std::unordered_map<std::string, std::map<std::vector<size_t>, double>> gate_pauli_errors;

        /**
        * @brief **qubit_topology** is a graph comprised of directed edges {control qubit, target qubit} with control qubit as the source of the edge -> target qubit as the destination of the edge.
        *
        *     // C++ code example: "cx" symmetrical two-qubit gate with 4 qubits in the topology below:
        *     //    q0 <--cx--> q1
        *     //     ^           ^
        *     //     |           |
        *     //     cx          cx
        *     //     |           |
        *     //     v           v
        *     //    q3 <--cx--> q2
        *     qristal::NoiseProperties t_qbnp;
        *     t_qbnp.qubit_topology{{0,1},{1,2},{2,3},{3,0}};
        */
        std::vector<std::pair<int, int>> qubit_topology;
    };
}

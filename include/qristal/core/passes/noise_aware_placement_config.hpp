// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <unordered_map>
#include <vector>

namespace qristal {

  /// @brief Hash function for std::pair
  struct pair_hash {
    template <class T1, class T2>
    /// Compute the hash value
    std::size_t operator()(const std::pair<T1, T2> &pair) const {
      return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
  };

  /// @brief Data-structure captures information requires for noise-aware
  /// placement pass.
  struct noise_aware_placement_config {
    // Typedefs for device info needed by this pass
    /// Mapping from qubit indicies to average single-qubit gate errors
    using single_qubit_gate_errors_t = std::unordered_map<std::size_t, double>;
    /// Mapping from qubit indicies to average measurement/readout errors
    using readout_errors_t = std::unordered_map<std::size_t, double>;
    /// Mapping from qubit index pairs to average single-qubit gate errors between
    /// qubits in the pair
    using two_qubit_gate_errors_t =
        std::unordered_map<std::pair<std::size_t, std::size_t>, double,
                           pair_hash>;
    /// List of qubit index pairs representing the device topology
    using device_topology_t = std::vector<std::pair<std::size_t, std::size_t>>;
    /// Qubit connectivity topology
    device_topology_t qubit_connectivity;
    /// Average single-qubit gate errors
    single_qubit_gate_errors_t avg_single_qubit_gate_errors;
    /// Average error rate for a two-qubit gate between two qubits
    two_qubit_gate_errors_t avg_two_qubit_gate_errors;
    /// Average readout/measurement error
    readout_errors_t avg_qubit_readout_errors;
  };

}

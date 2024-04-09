// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

// QB
#include "qb/core/typedefs.hpp"
#include "qb/core/noise_model/noise_model.hpp"

// STL
#include <optional>
#include <string>

namespace qb
{

  /// Supported types of the input source string defining a quantum kernel
  enum class source_string_type
  {
    XASM,
    Quil,
    OpenQASM
  };

  /// Indexed (i, j) run configurations of a session
  struct run_i_j_config
  {
    /// Number of measurement shots (sns_)
    int num_shots;
    /// Number of qubits (qns_)
    int num_qubits;
    /// Number of repetitions (rns_)
    int num_repetitions;
    /// Enable post-execution transpilation and resource estimation
    bool oqm_enabled;
    /// Name of the backend accelerator
    std::string acc_name;
    /// Full path to the OpenQASM include file where custom QB gates are defined
    std::string openqasm_qb_include_filepath;
    /// Full path to the remote backend database file
    std::string remote_backend_database_filepath;
    /// Type (assembly dialect) of the input source string
    source_string_type source_type;
    /// Disable placement if true
    bool no_placement;
    /// Disable circuit optimisation if true
    bool no_optimise;
    /// Disable simulation (accelerator execution) if true
    /// e.g., just want to run transpilation for resource estimation.
    bool no_sim;
    /// Enable noisy simulation/emulation
    bool noise;
    /// Noise model
    NoiseModel noise_model;
    /// Noise mitigation strategy (empty if none)
    std::string noise_mitigation;
    /// Random seed value for the simulator (if set)
    std::optional<int> simulator_seed;
    /// *
    /// Backend-specific configurations
    /// *
    /// [TNQVM] Max MPS bond dimension to keep
    int max_bond_tnqvm;
    /// [TNQVM] Max MPS kraus dimension to keep
    int max_kraus_tnqvm;
    /// [TNQVM] Initial MPS bond dimension to keep
    int initial_bond_tnqvm;
    /// [TNQVM] Initial MPS kraus dimension to keep
    int initial_kraus_tnqvm;
    /// [TNQVM] Absolute SVD cut-off limit for singular value truncation
    double svd_cutoff_tnqvm;
    /// [TNQVM] Relative SVD cut-off limit for singular value truncation
    double rel_svd_cutoff_tnqvm;
    /// [AER backend] Simulator type (e.g., state vec, density matrix, etc.)
    std::string aer_sim_type;
    /// [Emulator tensor network accelerators] Measurement sampling method
    bool measure_sample_sequential;
  };

}

// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

// Qristal
#include "qristal/core/typedefs.hpp"
#include "qristal/core/noise_model/noise_model.hpp"

// STL
#include <optional>
#include <string>

namespace qristal
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
    /// Enable post-execution transpilation and resource estimation
    bool oqm_enabled;
    /// Whether or not out_counts should be calculated
    bool calc_out_counts;
    /// Whether or not jacobian should be calculated
    bool calc_jacobian;
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
    /// Perform circuit execution (simulation or hardware execution) if true
    /// Set false if e.g. just wanting to run transpilation for resource estimation.
    bool execute_circuit;
    /// Enable noisy simulation/emulation
    bool noise;
    /// Runtime parameters for parametrized circuit
    std::vector<double> param_values;
    /// Noise model
    NoiseModel* noise_model;
    /// Noise model owned by this config
    std::shared_ptr<NoiseModel> noise_model_owned;
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
    std::string measure_sample_method;
  };

}

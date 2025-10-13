// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/session.hpp>

namespace qristal {

  /// Emulator backends (cudaq-wrapped emulator backends do not have to be in this list)
  const std::unordered_set<std::string_view> session::EMULATOR_BACKENDS = {
    "qb-statevector-cpu",
    "qb-statevector-gpu",
    "qb-mps",
    "qb-purification",
    "qb-mpdo"
  };

  /// Non-emulator backends
  const std::unordered_set<std::string_view> session::NON_EMULATOR_BACKENDS = {
    "aer",
    "tnqvm",
    "qpp",
    "qsim",
    "sparse-sim"
  };

  /// Backends that support GPU execution AND selection of the executing GPU device(s)
  const std::unordered_set<std::string_view> session::GPU_BACKENDS = {
    "qb-statevector-gpu",
    "qb-mps",
    "qb-purification",
    "qb-mpdo",
    "cudaq:qb_mps",
    "cudaq:qb_purification",
    "cudaq:qb_mpdo"
  };

  /// Backends that support noise
  const std::unordered_set<std::string_view> session::NOISY_BACKENDS = {
    "aer",
    "qb-statevector-cpu",
    "qb-statevector-gpu",
    "qb-mps",
    "qb-purification",
    "qb-mpdo",
    "cudaq:qb_mps",
    "cudaq:qb_purification",
    "cudaq:qb_mpdo",
    "aws-braket"
  };

  /// Backends that *only* support noise, i.e. will not run with noise = false
  const std::unordered_set<std::string_view> session::EXCLUSIVELY_NOISY_BACKENDS = {
      "qb-statevector-cpu",
      "qb-statevector-gpu"
  };

  /// Valid AER simulator types
  const std::unordered_set<std::string_view> session::VALID_AER_SIM_TYPES = {
    "statevector",
    "density_matrix",
    "matrix_product_state"
  };

  /// Recommended maximum qubit number for selected accelerator type
  const std::unordered_map<std::string_view, size_t> session::MAX_QUBITS_ACCS = {
    std::make_pair("aer-density_matrix", 14),
    std::make_pair("aer-statevector", 28),
    std::make_pair("qb-statevector-cpu", 20),
    std::make_pair("qb-statevector-gpu", 28),
    std::make_pair("cudaq:custatevec_fp32", 28),
    std::make_pair("cudaq:custatevec_fp64", 28),
    std::make_pair("cudaq:dm", 14),
    std::make_pair("cudaq:qpp", 28),
    std::make_pair("qpp", 28),
    std::make_pair("qsim", 28)
  };

  /// Recognised noise mitigation strategies
  const std::unordered_set<std::string_view> session::VALID_NOISE_MITIGATIONS = {
    // None
    "",
    // Simple readout mitigation
    "ro-error",
    // Richardson extrapolation (to the zero noise level)
    "rich-extrap",
    // Readout mitigation by multiplying error assignment matrix
    // (inverse of the SPAM matrix)
    "assignment-error-kernel"
  };

  /// Valid placement strategies
  const std::unordered_set<std::string_view> session::VALID_HARDWARE_PLACEMENTS = {
    "swap-shortest-path",
    "noise-aware"
  };

  /// Valid measurement sampling options
  const std::unordered_set<std::string_view> session::VALID_MEASURE_SAMPLING_OPTIONS = {
    "auto",
    "sequential",
    "cutensornet",
    "cutensornet_multishot"
  };

  /// Valid singular value decomposition type
  const std::unordered_set<std::string_view> session::VALID_SVD_TYPE_OPTIONS = {
    "QR",
    "Jacobian"
  };
}


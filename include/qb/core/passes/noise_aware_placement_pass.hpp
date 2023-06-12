// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include "base_pass.hpp"
#include "noise_aware_placement_config.hpp"
#include <memory>
#include <optional>
#include <unordered_map>

// Goal here is to keep XACC out of user space.
namespace xacc {
class Accelerator;
class IRTransformation;
} // namespace xacc

namespace qb {
/// @brief Noise-aware placement pass
/// @details Maps circuit qubits to device qubits satisfying connectivity
/// constraints and using gate error rates and readout errors to find the best
/// placement map.
class noise_aware_placement_pass : public CircuitPass {
public:
  /// Constructor from a user-provided config
  noise_aware_placement_pass(const noise_aware_placement_config &device_info);
  /// Constructor from an Accelerator instance, which provides its
  /// characterization information
  noise_aware_placement_pass(std::shared_ptr<xacc::Accelerator> qpu);
  /// Returns the pass name.
  virtual std::string get_name() const override;

  /// Returns the pass description
  virtual std::string get_description() const override;

  /// Runs the pass over the circuit IR node
  virtual void apply(CircuitBuilder &circuit) override;

private:
  /// The underlying XACC plugin implementation of the IR transformation
  std::shared_ptr<xacc::IRTransformation> m_tket_impl;
  /// Input device information if provided
  std::optional<noise_aware_placement_config> m_device_info;
  /// Input accelerator instance if provided
  std::shared_ptr<xacc::Accelerator> m_qpu;
};

/// Factory function to create a `noise_aware_placement_pass` as a generic
/// `CircuitPass` from a user-provided config
inline std::shared_ptr<CircuitPass> create_noise_aware_placement_pass(
    const noise_aware_placement_config &device_info) {
  return std::make_shared<noise_aware_placement_pass>(device_info);
}

/// Factory function to create a `noise_aware_placement_pass` as a generic
/// `CircuitPass` from an Accelerator instance, which provides its
/// characterization information
inline std::shared_ptr<CircuitPass>
create_noise_aware_placement_pass(std::shared_ptr<xacc::Accelerator> qpu) {
  return std::make_shared<noise_aware_placement_pass>(qpu);
}
} // namespace qb

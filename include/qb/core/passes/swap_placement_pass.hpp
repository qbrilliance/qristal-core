// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include "base_pass.hpp"
#include <memory>
#include <vector>

// Goal here is to keep XACC out of user space.
namespace xacc {
class Accelerator;
class IRTransformation;
} // namespace xacc

namespace qb {
/// @brief Circuit placement pass based on SWAP gate injection to satisfy device
/// topology constraints
class swap_placement_pass : public CircuitPass {
public:
  /// Constructor from a user-provided qubit connectivity topology
  swap_placement_pass(const std::vector<std::pair<int, int>> &connectivity);
  /// Constructor from an Accelerator instance, which provides its
  /// connectivity information
  swap_placement_pass(std::shared_ptr<xacc::Accelerator> qpu);
  /// Returns the pass name.
  virtual std::string get_name() const override;

  /// Returns the pass description
  virtual std::string get_description() const override;

  /// Runs the pass over the circuit IR node
  virtual void apply(CircuitBuilder &circuit) override;

private:
  /// Connectivity information for placement
  std::vector<std::pair<int, int>> m_connectivity;
};

/// Factory function to create a `swap_placement_pass` as a generic
/// `CircuitPass` from a user-provided qubit connectivity topology
inline std::shared_ptr<CircuitPass> create_swap_placement_pass(
    const std::vector<std::pair<int, int>> &connectivity) {
  return std::make_shared<swap_placement_pass>(connectivity);
}
/// Factory function to create a `swap_placement_pass` as a generic
/// `CircuitPass` from an Accelerator instance, which provides its
/// connectivity information
inline std::shared_ptr<CircuitPass>
create_swap_placement_pass(std::shared_ptr<xacc::Accelerator> qpu) {
  return std::make_shared<swap_placement_pass>(qpu);
}
} // namespace qb

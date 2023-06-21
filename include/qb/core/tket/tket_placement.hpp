// Copyright Quantum Brilliance Pty Ltd
#pragma once
#include "IRTransformation.hpp"

namespace qb {
/**
 * @brief Noise-aware circuit placement based on the TKET library
 *
 * This is implemented as a xacc::IRTransformation plugin.
 */
class TketPlacement : public xacc::IRTransformation,
                      public xacc::Cloneable<xacc::IRTransformation> {
public:
  /**
   * @brief Construct a new TketPlacement object
   *
   */
  TketPlacement() {}

  /**
   * @brief Return the type of this IRTransformation plugin
   *
   * @return Type (placement) of this IRTransformation plugin
   */
  const xacc::IRTransformationType type() const override;

  /**
   * @brief Return the plugin name (for retrieval from the plugin registry)
   *
   * @return Name of this plugin
   */
  const std::string name() const override;

  /**
   * @brief Return the plugin text description
   *
   * @return Description string
   */
  const std::string description() const override;

  /**
   * @brief Create a new instance of this service.
   *
   * Note: by default, the service registry will return a ref (as a shared
   * pointer) to the same service instance unless clonable (xacc::Cloneable).
   * Clonable service can be used in a thread-safe manner.
   *
   * @return New instance of this service.
   */
  std::shared_ptr<xacc::IRTransformation> clone() override;

  /**
   * @brief Apply the IR transformation procedure
   *
   * @param program Input circuit IR to be transformed by this plugin
   * @param acc Ref. to the backend accelerator
   * @param options Configuration parameters
   */
  void apply(std::shared_ptr<xacc::CompositeInstruction> program,
             const std::shared_ptr<xacc::Accelerator> acc,
             const xacc::HeterogeneousMap &options = {}) override;

private:
  /// Helper method to parse qubit connectivity from AWS device JSON
  std::vector<std::pair<int, int>>
  parseAwsDeviceConnectivity(const std::string &props_json_str) const;

  /// Helper to parse TKET noise characteristics (single/double qubit gate
  /// errors, readout errors) from the AWS device property JSON.
  std::tuple<tket::avg_node_errors_t, tket::avg_link_errors_t,
             tket::avg_readout_errors_t>
  parseAwsDeviceCharacteristics(
      const std::string &props_json_str,
      const std::vector<std::pair<int, int>> &connectivity);
};
} // namespace qb

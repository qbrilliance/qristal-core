// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once
#include "Accelerator.hpp"

namespace qristal {
/// A xacc::Accelerator wrapper for offloading XACC IR execution to CUDA Quantum
/// simulator backends by converting XACC IR to Quake then QIR.
class cudaq_acc : public xacc::Accelerator {
  /// Name of the CUDAQ backend to use
  std::string m_backend;
  /// Number of measurement shots
  int m_shots;

public:
  /// Constructor
  cudaq_acc(const std::string &backend_name);
  /// Name of this xacc::Accelerator implementation
  virtual const std::string name() const override;
  /// Description of this xacc::Accelerator implementation
  virtual const std::string description() const override;
  /// Initialize this xacc::Accelerator with runtime configurations.
  virtual void initialize(const xacc::HeterogeneousMap &params) override;
  /// Update runtime configurations of this xacc::Accelerator.
  virtual void
  updateConfiguration(const xacc::HeterogeneousMap &config) override;
  /// List of configuration keys that this xacc::Accelerator will look for.
  virtual const std::vector<std::string> configurationKeys() override;
  /// Execute a single circuit and persist the measurement results to the
  /// buffer.
  virtual void execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
                       const std::shared_ptr<xacc::CompositeInstruction>
                           CompositeInstruction) override;
  /// Execute a list of circuits and persist the measurement results to the
  /// buffer.
  virtual void
  execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
          const std::vector<std::shared_ptr<xacc::CompositeInstruction>>
              CompositeInstructions) override;
};
}

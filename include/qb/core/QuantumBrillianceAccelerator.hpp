/***
 *** Copyright (c) 2020 Quantum Brilliance Pty Ltd
 ***/
#ifndef QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEACCELERATOR_HPP_
#define QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEACCELERATOR_HPP_

#include "Accelerator.hpp"

namespace qb {
class QuantumBrillianceAccelerator : public xacc::Accelerator {
public:
  QuantumBrillianceAccelerator() : xacc::Accelerator() {}
  virtual ~QuantumBrillianceAccelerator() {}
  const std::string name() const override { return "qbacc"; }
  const std::string description() const override {
    return "Quantum Brilliance XACC Accelerator";
  }

  const std::vector<std::string> configurationKeys() override {
    return {
        "output_oqm",         "shots", "n_qubits", "m_connectivity",
        "output_oqm_enabled",
    };
  }

  void updateConfiguration(const xacc::HeterogeneousMap &config) override;

  void initialize(const xacc::HeterogeneousMap &params = {}) override;

  xacc::HeterogeneousMap getProperties() override;

  //
  // Quantum Brilliance hardware connectivity constraints
  //
  std::vector<std::pair<int, int>> getConnectivity() override {
    return m_connectivity;
  }

  void execute(std::shared_ptr<xacc::AcceleratorBuffer> buf,
               std::shared_ptr<xacc::CompositeInstruction> f) override;

  void execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
               const std::vector<std::shared_ptr<xacc::CompositeInstruction>>
                   functions) override;

  const std::string &getTranspiledResult() const { return qpuQasmStr_; }

protected:
  // Number of shots (repeats) over which to collect statistics
  int shots = 1024;
  int n_qubits = 2;
  std::string qpuQasmStr_;
  bool output_oqm_enabled_ = true;
  std::string output_oqm = "QBCIRCUIT.inc";
  std::vector<std::pair<int, int>> m_connectivity;
};

} // namespace qb
#endif

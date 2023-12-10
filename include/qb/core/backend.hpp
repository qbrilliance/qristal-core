// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once

#include "Accelerator.hpp"

namespace qb
{

  class backend : public xacc::Accelerator
  {

    public:

      backend() : xacc::Accelerator() {}

      virtual ~backend() {}

      const std::string name() const override; 

      const std::string description() const override;

      const std::vector<std::string> configurationKeys() override;
    
      void updateConfiguration(const xacc::HeterogeneousMap& config) override;
    
      void initialize(const xacc::HeterogeneousMap& params = {}) override;
    
      xacc::HeterogeneousMap getProperties() override;
    
      /// Retrieve hardware connectivity
      std::vector<std::pair<int, int>> getConnectivity() override;
    
      void execute(std::shared_ptr<xacc::AcceleratorBuffer> buf,
                   std::shared_ptr<xacc::CompositeInstruction> f) override;
    
      void execute(std::shared_ptr<xacc::AcceleratorBuffer> buffer,
                   const std::vector<std::shared_ptr<xacc::CompositeInstruction>> functions) override;
    
      const std::string& getTranspiledResult() const;
    
    protected:

      /// Number of shots (repeats) over which to collect statistics
      int shots = 1024;

      int n_qubits = 2;

      std::string qpuQasmStr_;

      bool output_oqm_enabled_ = true;

      std::string output_oqm = "QBCIRCUIT.inc";

      std::vector<std::pair<int, int>> m_connectivity;
  };

}

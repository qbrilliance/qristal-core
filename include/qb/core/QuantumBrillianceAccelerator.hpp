/***
 *** Copyright (c) 2020 Quantum Brilliance Pty Ltd
 ***/
#ifndef QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEACCELERATOR_HPP_
#define QUANTUM_GATE_ACCELERATORS_QUANTUMBRILLIANCEACCELERATOR_HPP_

#include "Accelerator.hpp"
#include "InstructionIterator.hpp"
#include "QuantumBrillianceVisitor.hpp"
#include <cstdint>
#include <fstream>
#include <string>

namespace xacc {
namespace quantum {

class QuantumBrillianceAccelerator : public Accelerator {
public:
  QuantumBrillianceAccelerator() : Accelerator() {}
  virtual ~QuantumBrillianceAccelerator() {}
  const std::string name() const override { return "qbacc"; }
  const std::string description() const override {
    return "Quantum Brilliance XACC Accelerator";
  }

  const std::vector<std::string> configurationKeys() override {
    return {"output_oqm",
            "shots",
            "n_qubits",
            "m_connectivity",
            "output_oqm_enabled",
            };
  }

  void updateConfiguration(const HeterogeneousMap &config) override {
    if (config.keyExists<int>("shots")) {
      shots = config.get<int>("shots");
    }
    if (config.keyExists<std::string>("output_oqm")) {
      output_oqm = config.get<std::string>("output_oqm");
    }
    if (config.keyExists<int>("n_qubits")) {
      n_qubits = config.get<int>("n_qubits");
    }
    if (config.keyExists<std::vector<std::pair<int,int>>>("m_connectivity")) {
      m_connectivity = config.get<std::vector<std::pair<int,int>>>("m_connectivity");
    }
    if (config.keyExists<bool>("output_oqm_enabled")) {
      output_oqm_enabled_ = config.get<bool>("output_oqm_enabled");
    }    
  }

  void initialize(const HeterogeneousMap &params = {}) override {
    updateConfiguration(params);
  }

  HeterogeneousMap getProperties() override {
    HeterogeneousMap m;
    m.insert("shots", shots);
    m.insert("output_oqm", output_oqm);
    m.insert("n_qubits", n_qubits);
    m.insert("m_connectivity", m_connectivity);
    m.insert("output_oqm_enabled", output_oqm_enabled_);
    return m;
  }

  //
  // Quantum Brilliance hardware connectivity constraints
  //
  std::vector<std::pair<int, int>> getConnectivity() override {
    return m_connectivity;
  }

  void execute(std::shared_ptr<xacc::AcceleratorBuffer> buf,
               std::shared_ptr<xacc::CompositeInstruction> f) override {
    execute(buf, std::vector<std::shared_ptr<xacc::CompositeInstruction>>{f});
    // buf->removeChild(0);
  }

  void execute(std::shared_ptr<AcceleratorBuffer> buffer,
               const std::vector<std::shared_ptr<CompositeInstruction>>
                   functions) override {
    // int kernelCounter = 0;
    std::vector<std::string> names;
    for (auto kernel : functions) {
      auto visitor = std::make_shared<xacc::quantum::QuantumBrillianceVisitor>(
          buffer->size());
      names.push_back(kernel->name());
      InstructionIterator it(kernel);
      while (it.hasNext()) {
        auto nextInst = it.next();
        if (nextInst->isEnabled()) {
          nextInst->accept(visitor);
        }
      }
      qpuQasmStr_ = visitor->getFinishedOpenQasmQpu();
      if (output_oqm_enabled_) {
        std::ofstream of(output_oqm);
        if (of.is_open()) {
          of << qpuQasmStr_ << std::endl;
          of.flush();
          of.close();
          // std::cout
          //    << std::endl
          //    << "## 4.0 Transpiled output in OpenQASM format stored in the file: "
          //    << output_oqm << std::endl;
        } else {
          std::cout << std::endl
                    << "## 4.0 Transpiled output in OpenQASM format:"
                    << std::endl;
          std::cout << qpuQasmStr_ << std::endl;
        }
      }
    }
    // std::cout << "Debug: reached end of execute()" << std::endl;
  }

  const std::string & getTranspiledResult() const { return qpuQasmStr_; }

protected:
  // Number of shots (repeats) over which to collect statistics
  int shots = 1024;
  int n_qubits = 2;
  std::string qpuQasmStr_;
  bool output_oqm_enabled_ = true;
  std::string output_oqm = "QBCIRCUIT.inc";
  std::vector<std::pair<int, int>> m_connectivity;
};

} // namespace quantum
} // namespace xacc
#endif

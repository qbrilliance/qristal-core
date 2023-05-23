/***
 *** Copyright (c) 2020 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/QuantumBrillianceAccelerator.hpp"
#include "InstructionIterator.hpp"
#include "xacc.hpp"
#include <fstream>

namespace qb {
void QuantumBrillianceAccelerator::updateConfiguration(
    const xacc::HeterogeneousMap &config) {
  if (config.keyExists<int>("shots")) {
    shots = config.get<int>("shots");
  }
  if (config.keyExists<std::string>("output_oqm")) {
    output_oqm = config.get<std::string>("output_oqm");
  }
  if (config.keyExists<int>("n_qubits")) {
    n_qubits = config.get<int>("n_qubits");
  }
  if (config.keyExists<std::vector<std::pair<int, int>>>("m_connectivity")) {
    m_connectivity =
        config.get<std::vector<std::pair<int, int>>>("m_connectivity");
  }
  if (config.keyExists<bool>("output_oqm_enabled")) {
    output_oqm_enabled_ = config.get<bool>("output_oqm_enabled");
  }
}

void QuantumBrillianceAccelerator::initialize(
    const xacc::HeterogeneousMap &params) {
  updateConfiguration(params);
}

xacc::HeterogeneousMap QuantumBrillianceAccelerator::getProperties() {
  xacc::HeterogeneousMap m;
  m.insert("shots", shots);
  m.insert("output_oqm", output_oqm);
  m.insert("n_qubits", n_qubits);
  m.insert("m_connectivity", m_connectivity);
  m.insert("output_oqm_enabled", output_oqm_enabled_);
  return m;
}

void QuantumBrillianceAccelerator::execute(
    std::shared_ptr<xacc::AcceleratorBuffer> buf,
    std::shared_ptr<xacc::CompositeInstruction> f) {
  execute(buf, std::vector<std::shared_ptr<xacc::CompositeInstruction>>{f});
}

void QuantumBrillianceAccelerator::execute(
    std::shared_ptr<xacc::AcceleratorBuffer> buffer,
    const std::vector<std::shared_ptr<xacc::CompositeInstruction>> functions) {
  auto staq = xacc::getCompiler("staq");
  auto qb_transpiler = xacc::getIRTransformation("qb-gateset-transpiler");
  for (auto &kernel : functions) {
    auto transpiled_ir = xacc::ir::asComposite(kernel->clone());
    qb_transpiler->apply(transpiled_ir, xacc::as_shared_ptr(this));
    qpuQasmStr_ = staq->translate(transpiled_ir);

    if (output_oqm_enabled_) {
      std::ofstream of(output_oqm);
      if (of.is_open()) {
        of << qpuQasmStr_ << std::endl;
        of.flush();
        of.close();
      } else {
        std::cout << std::endl
                  << "## 4.0 Transpiled output in OpenQASM format:"
                  << std::endl;
        std::cout << qpuQasmStr_ << std::endl;
      }
    }
  }
}
} // namespace qb

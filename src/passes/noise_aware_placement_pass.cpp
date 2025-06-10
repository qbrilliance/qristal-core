// Copyright (c) Quantum Brilliance Pty Ltd

#include <qristal/core/passes/noise_aware_placement_pass.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <xacc.hpp>

namespace qristal {
// Constructor
noise_aware_placement_pass::noise_aware_placement_pass(
    const noise_aware_placement_config &device_info)
    : m_device_info(device_info) {
  m_tket_impl = xacc::getIRTransformation("noise-aware");
};
noise_aware_placement_pass::noise_aware_placement_pass(
    std::shared_ptr<xacc::Accelerator> qpu)
    : m_qpu(qpu) {
  m_tket_impl = xacc::getIRTransformation("noise-aware");
};
/// Returns the pass name.
std::string noise_aware_placement_pass::get_name() const { return "noise-aware"; }

/// Returns the pass description
std::string noise_aware_placement_pass::get_description() const {
  return "Noise-aware quantum circuit placement pass based on TKET library.";
}

/// Runs the pass over the circuit IR node
void noise_aware_placement_pass::apply(CircuitBuilder &circuit) {
  if (m_qpu) {
    m_tket_impl->apply(circuit.get(), m_qpu);
  } else {
    assert(m_device_info.has_value());
    m_tket_impl->apply(circuit.get(), nullptr,
                       {{"noise_aware_placement_config", *m_device_info}});
  }
}
}

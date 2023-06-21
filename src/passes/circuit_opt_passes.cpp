// Copyright (c) Quantum Brilliance Pty Ltd
#include "qb/core/passes/circuit_opt_passes.hpp"
#include "qb/core/circuit_builder.hpp"
#include "xacc.hpp"

namespace qb {

/// Constructor
optimization_pass::optimization_pass(const std::string &name)
    : m_plugin_name(name) {
  if (!xacc::hasService<xacc::IRTransformation>(name)) {
    throw std::invalid_argument(
        "IRTransformation plugin named '" + name +
        "' is not available. Please check your input or the installation.");
  }
}
/// Returns name of the pass
std::string optimization_pass::get_name() const { return m_plugin_name; }

/// Returns the pass description
std::string optimization_pass::get_description() const {
  return xacc::getIRTransformation(m_plugin_name)->description();
}

/// Runs the pass over the circuit IR node
void optimization_pass::apply(CircuitBuilder &circuit) {
  xacc::getIRTransformation(m_plugin_name)->apply(circuit.get(), nullptr);
}
} // namespace qb

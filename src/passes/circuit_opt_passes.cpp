// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/passes/circuit_opt_passes.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <xacc.hpp>

namespace qristal {

/// Optimization_pass class
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

/// sequence_pass class
/// Constructor
sequence_pass::sequence_pass(const std::vector<std::string> &pass_list): m_pass_list(pass_list) {}
/// Returns name of the pass
std::string sequence_pass::get_name() const { return "sequence_pass"; }

/// Returns the pass description
std::string sequence_pass::get_description() const {
  return "A transformation that runs a sequence of optimization passes.";
}

/// Runs the pass over the circuit IR node
void sequence_pass::apply(CircuitBuilder &circuit) {
  for (const auto& s : m_pass_list) {
    xacc::getIRTransformation(s)->apply(circuit.get(), nullptr);
  }
}
}

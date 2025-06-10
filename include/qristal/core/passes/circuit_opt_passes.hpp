// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <qristal/core/passes/base_pass.hpp>
#include <memory>
#include <string>

namespace qristal {

  /// @brief Circuit optimization pass
  class optimization_pass : public CircuitPass {
    private:
      /// Name of the underlying IR Transformation plugin
      std::string m_plugin_name;
    public:
      /// Constructor
      optimization_pass(const std::string &name);
      /// Returns name of the pass
      virtual std::string get_name() const override;

      /// Returns the pass description
      virtual std::string get_description() const override;

      /// Runs the pass over the circuit IR node
      virtual void apply(CircuitBuilder &circuit) override;
  };


  /// Pattern-based circuit optimization pass
  inline std::shared_ptr<CircuitPass> create_circuit_optimizer_pass() {
    return std::make_shared<optimization_pass>("circuit-optimizer");
  }

  /// Remove gate-inverse pairs, merges rotations, removes identity rotations,
  /// and removes redundant gates before measure.
  inline std::shared_ptr<CircuitPass> create_remove_redundancies_pass() {
    return std::make_shared<optimization_pass>("redundancy-removal");
  }

  /// Squash together sequences of single- and two-qubit gates into minimal form;
  // decompose CX gates.
  inline std::shared_ptr<CircuitPass> create_two_qubit_squash_pass() {
    return std::make_shared<optimization_pass>("two-qubit-squash");
  }

  /// Peephole optimisation including resynthesis of three-qubit gate sequences.
  inline std::shared_ptr<CircuitPass> create_peephole_pass() {
    return std::make_shared<optimization_pass>("peephole-optimisation");
  }

  /// Whenever a gate transforms a known basis state to another known basis
  /// state, remove it, inserting X gates where necessary to achieve the same
  /// state.
  ///
  /// Note: this is contextual pass (i.e., the circuit may not represent the same
  /// unitary but have the same effect given the initial state)
  inline std::shared_ptr<CircuitPass> create_initial_state_simplify_pass() {
    return std::make_shared<optimization_pass>("simplify-initial");
  }

}

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

  /// @brief Sequence pass
  class sequence_pass : public CircuitPass{
    private:
      /// List of IR Transformation plugin
      std::vector<std::string> m_pass_list;
    public:
      /// Constructor
      sequence_pass(const std::vector<std::string> &pass_list);
    
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

  /// Decomposes each SWAP gates into three CNOT gates
  inline std::shared_ptr<CircuitPass> create_decompose_swap_pass() {
    return std::make_shared<optimization_pass>("decompose-swap");
  }

  /// Applies a collection of commutation rules to move single qubit operations 
  /// past multiqubit operations they commute with, towards the front of the circuit
  inline std::shared_ptr<CircuitPass> create_commute_through_multis_pass() {
    return std::make_shared<optimization_pass>("commute-through-multis");
  }

  /// Removes redundant gates and simplifies circuits after qubit routing 
  inline std::shared_ptr<CircuitPass> create_optimise_post_routing_pass() {
    return std::make_shared<optimization_pass>("optimise-post-routing");
  }

  /// Rebase of a circuit to use only Rz and Rx rotations
  inline std::shared_ptr<CircuitPass> create_decompose_ZX_pass() {
    return std::make_shared<optimization_pass>("decompose-zx");
  }

  /// Rebase of a quantum circuit to Clifford gates, decomposing gates into 
  /// sequences of Clifford operations
  inline std::shared_ptr<CircuitPass> create_rebase_to_clifford_pass() {
    return std::make_shared<optimization_pass>("rebase-to-clifford");
  } 

  /// Applies a number of rewrite rules for simplifying Clifford gate sequences, 
  /// similar to Duncan & Fagan (https://arxiv.org/abs/1901.10114)
  inline std::shared_ptr<CircuitPass> create_optimise_cliffords_pass() {
    return std::make_shared<optimization_pass>("optimise-cliffords");
  }  
}

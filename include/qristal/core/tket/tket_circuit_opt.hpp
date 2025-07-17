// Copyright Quantum Brilliance Pty Ltd
#include <array>

// XACC
#include <IRTransformation.hpp>
#include <xacc.hpp>

// TKET
#include <Circuit/CircPool.hpp>
#include <Transformations/BasicOptimisation.hpp>
#include <Transformations/CliffordOptimisation.hpp>
#include <Transformations/ContextualReduction.hpp>
#include <Transformations/OptimisationPass.hpp>
#include <Transformations/Rebase.hpp>
#include <Transformations/Replacement.hpp>
#include <Transformations/Decomposition.hpp>
#include <Transformations/Combinator.hpp>
#include <qristal/core/tket/tket_ir_converter.hpp>

namespace qristal {
  /// Tket transform pass factory function type
  /// (to be used as a parameter in xacc::IRTransformation template)
  typedef tket::Transform (*TransformFactory)();

  /// A util function to convert string literals (const char*) to std::array<N>
  /// to be used as template parameters.
  template <auto N> consteval auto constexpr_str(char const (&cstr)[N]) {
    std::array<char, N> arr;
    for (std::size_t i = 0; i < N; ++i)
      arr[i] = cstr[i];
    return arr;
  }

  /// @brief Generic template for an optimizing xacc::IRTransformation based on
  /// TKET
  /// @tparam TketTransformFactoryFn Factory function to construct the underlying
  /// tket::Transform pass
  /// @tparam PluginName Name of the xacc::IRTransformation (to retrieve from the
  /// XACC plugin registry)
  /// @tparam PluginDescription
  template <TransformFactory TketTransformFactoryFn, auto PluginName,
            auto PluginDescription>
  class TketCircuitTransformPlugin
      : public xacc::IRTransformation,
        public xacc::Cloneable<xacc::IRTransformation> {

    private:

      /// The underlying TKET pass
      tket::Transform m_transformer = TketTransformFactoryFn();
      /// The rebase pass to make sure only XACC gates remained (since we need to
      /// convert TKET->XACC after optimization)
      // Note: XACC/Qristal don't handle TK1/TK2 gates
      tket::Transform m_rebase = tket::Transforms::rebase_factory(
          {tket::OpType::CX, tket::OpType::CY, tket::OpType::CZ, tket::OpType::CH,
           tket::OpType::CU1, tket::OpType::Rx, tket::OpType::Ry, tket::OpType::Rz,
           tket::OpType::X, tket::OpType::Y, tket::OpType::Z, tket::OpType::S,
           tket::OpType::Sdg, tket::OpType::T, tket::OpType::Tdg,
           tket::OpType::Reset, tket::OpType::ISWAP, tket::OpType::FSim},
          tket::CircPool::CX(), tket::CircPool::tk1_to_rzrx);

    public:

      /// Constructor
      TketCircuitTransformPlugin() {}

      /// Returns the type of this transformation (i.e., circuit optimization)
      const xacc::IRTransformationType type() const override {
        return xacc::IRTransformationType::Optimization;
      }

      /// Name of the plugin (to register with the service registry)
      const std::string name() const override {
        return std::string(PluginName.data());
      }

      /// Plugin description (base plugin API)
      const std::string description() const override {
        return std::string(PluginDescription.data());
      }

      /// Clone this plugin when retrieving from the service registry.
      // In the absence of this method, the service registry will return a reference
      // to a single plugin instance. Having clones of plugins from the registry
      // guarantees thread safety.
      std::shared_ptr<xacc::IRTransformation> clone() override {
        return std::make_shared<TketCircuitTransformPlugin>();
      }

      /// Apply the circuit optimization transformation on the input IR.
      void apply(std::shared_ptr<xacc::CompositeInstruction> program,
                 const std::shared_ptr<xacc::Accelerator> acc,
                 const xacc::HeterogeneousMap &options = {}) override {
        auto tket_circ = *qristal::tket_ir_converter::to_tket(program);
        // whether any changes were made
        const bool ir_changed = m_transformer.apply(tket_circ);
        if (ir_changed) {
          // Remove TKET-specific gates.
          m_rebase.apply(tket_circ);
          auto tket_circ_ptr = xacc::as_shared_ptr(&tket_circ);
          program->clear();
          program->addInstructions(qristal::tket_ir_converter::to_xacc(tket_circ_ptr)->getInstructions());
        }
      }
  };

class SequencePass : public xacc::IRTransformation {
  public:
      SequencePass() {}
  
      void apply(std::shared_ptr<xacc::CompositeInstruction> program,
                 const std::shared_ptr<xacc::Accelerator> acc,
                 const xacc::HeterogeneousMap& options) override {
          
          auto passList = options.get<std::vector<std::string>>("passes");
          for (const auto& passName : passList) {
              auto pass = xacc::getIRTransformation(passName);
              if (pass) {
                  pass->apply(program, acc, options);
              } 
          }
      }
  
      const xacc::IRTransformationType type() const override {
          return xacc::IRTransformationType::Optimization;
      }
  
      const std::string name() const override {
          return "sequence-pass";
      }
  
      const std::string description() const override {
          return "Optimization pass that applies multiple passes in sequence.";
      }
  
      std::shared_ptr<xacc::IRTransformation> clone() {
          return std::make_shared<SequencePass>();
      }
  };
                    
  /// Remove gate-inverse pairs, merges rotations, removes identity rotations,
  /// and removes redundant gates before measure.
  using tket_redundancy_removal_plugin = TketCircuitTransformPlugin<
      tket::Transforms::remove_redundancies, constexpr_str("redundancy-removal"),
      constexpr_str("Remove gate-inverse pairs, merge rotations, and remove "
                    "identity rotations")>;
  /// Squash together sequences of single- and two-qubit gates into minimal form;
  // decompose CX gates.
  using tket_two_qubit_squash_plugin = TketCircuitTransformPlugin<
      []() { return tket::Transforms::two_qubit_squash(); },
      constexpr_str("two-qubit-squash"),
      constexpr_str(
          "Squash sequences of two-qubit operations into minimal form")>;
  /// Peephole optimisation including resynthesis of three-qubit gate sequences.
  using tket_full_peephole_plugin =
      TketCircuitTransformPlugin<[]() {
        return tket::Transforms::full_peephole_optimise();
      },
                                 constexpr_str("peephole-optimisation"),
                                 constexpr_str("Peephole optimisation pass")>;
  /// Whenever a gate transforms a known basis state to another known basis
  /// state, remove it, inserting X gates where necessary to achieve the same
  /// state.
  ///
  /// Note: this is contextual pass (i.e., the circuit may not represent the same
  /// unitary but have the same effect given the initial state)
  using tket_simplify_initial_plugin = TketCircuitTransformPlugin<
      []() { return tket::Transforms::simplify_initial(); },
      constexpr_str("simplify-initial"),
      constexpr_str("Simplify the circuit where it acts on known basis states")>;
  /// Decomposes all SWAP gates into triples of CX gates
  using tket_decompose_swap_plugin = TketCircuitTransformPlugin<
      []() { return tket::Transforms::decompose_SWAP_to_CX(); }, constexpr_str("decompose-swap"),
      constexpr_str("Decomposes all SWAP gates into triples of CX gates")>;
  /// Moves single qubit operations past multiqubit operations they commute with, 
  /// towards the front of the circuit
  using tket_commute_through_multis = TketCircuitTransformPlugin<
      tket::Transforms::commute_through_multis, constexpr_str("commute-through-multis"),
      constexpr_str("Commutes single-qubit gates through multi-qubit gates to simplify circuits")>;
  /// Performs optimizations on the circuit after qubit routing
  /// Cleans up and simplifies the circuit while respecting hardware connectivity             
  using tket_optimise_post_routing = TketCircuitTransformPlugin<
      tket::Transforms::synthesise_tket, constexpr_str("optimise-post-routing"),
      constexpr_str("Optimises the circuit after qubit routing by removing redundant gates"
                    "and simplifying sequences"
                    "Preserves hardware connectivity constraints")>;
  /// Rebases multiple single-qubit gates into equivalent sequences of Rz and Rx gates
  using tket_decompose_ZX = TketCircuitTransformPlugin<
      tket::Transforms::decompose_ZX, constexpr_str("decompose-zx"),
      constexpr_str("Rebase single qubit gates into Rz, Rx")>;
  /// Rewrites single-qubit Clifford gates using only {Z, X, S, V}
  /// Non-Clifford gates (e.g., T) are left unchanged
  using tket_rebase_to_clifford = TketCircuitTransformPlugin<
      tket::Transforms::decompose_cliffords_std, constexpr_str("rebase-to-clifford"),
      constexpr_str("Replaces single-qubit gates that are Clifford but not in the basic"
                    "set {Z, X, S, V} with equivalent gate sequences only using those four")>;
  /// Simplifies Clifford gate sequences using standard rewrite rules,
  /// similar to Duncan & Fagan (arXiv:1901.10114)
  /// May reorder CX gates or introduce wire swaps.
  using tket_optimise_cliffords = TketCircuitTransformPlugin<
      []() { return tket::Transforms::clifford_simp(); }, constexpr_str("optimise-cliffords"),
      constexpr_str("Optimizes Clifford gate sequences using rewrite rules to reduce circuit "
                  "depth and size")>;
}

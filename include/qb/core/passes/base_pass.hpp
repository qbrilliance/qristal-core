// Copyright (c) Quantum Brilliance Pty Ltd

#pragma once
#include <string>

namespace qb {
// Forward declare qb::CircuitBuilder
class CircuitBuilder;
/// A base class for a `Pass` (implementing some form of transformation on the
/// input IR tree)
template <typename IrType> class Pass {
public:
  /// Destructor
  virtual ~Pass() = default;

  /// Returns the derived pass name.
  virtual std::string get_name() const = 0;

  /// Returns the pass description for this pass.
  /// (derived class to add if wanted to)
  virtual std::string get_description() const { return ""; }

  /// Polymorphic API that runs the pass over the circuit IR node.
  virtual void apply(IrType &circuit) = 0;
};

/// Alias the pass that work on CircuitBuilder (i.e., XACC IR)
using CircuitPass = Pass<CircuitBuilder>;
} // namespace qb
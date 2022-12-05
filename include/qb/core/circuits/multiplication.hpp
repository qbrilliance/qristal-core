/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qb {

/// Multiply circuit:
/// Given two n-bit numbers |a> and |b>, this circuit
/// performs the operation |a>|b>|0> -> |a>|b>|a*b>.

class Multiplication : public xacc::quantum::Circuit {
public:
  Multiplication() : xacc::quantum::Circuit("Multiplication") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(Multiplication);
};
} // namespace qb
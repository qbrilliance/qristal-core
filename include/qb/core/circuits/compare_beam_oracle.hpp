/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qbOS {
class CompareBeamOracle : public xacc::quantum::Circuit {
public:
  CompareBeamOracle() : xacc::quantum::Circuit("CompareBeamOracle") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(CompareBeamOracle);
};
} // namespace qbOS
/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once
#include "Circuit.hpp"
namespace qb {
class RippleCarryAdder : public xacc::quantum::Circuit {
public:
  RippleCarryAdder() : xacc::quantum::Circuit("RippleCarryAdder") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(RippleCarryAdder);
};
} // namespace qb
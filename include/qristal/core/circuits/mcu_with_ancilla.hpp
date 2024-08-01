/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once
#include "Circuit.hpp"
namespace qristal {
class MultiControlledUWithAncilla : public xacc::quantum::Circuit {
public:
  MultiControlledUWithAncilla() : xacc::quantum::Circuit("MultiControlledUWithAncilla") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(MultiControlledUWithAncilla);
};
}
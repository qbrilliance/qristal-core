/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qristal {

// This module performs the comparison
// |a>|b>|flag> ---> |a>|b> X^{a>b}|flag>

class CompareGT : public xacc::quantum::Circuit {
public:
  CompareGT() : xacc::quantum::Circuit("CompareGT") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(CompareGT);
};
}
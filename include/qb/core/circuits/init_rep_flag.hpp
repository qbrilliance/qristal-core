/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qbOS {

/// InitRepeatFlag circuit:
/// This circuit initialises the initial_repeat flags to mark which positions in the string
/// were intially a repeat of the symbol before.
class InitRepeatFlag : public xacc::quantum::Circuit {
public:
  InitRepeatFlag() : xacc::quantum::Circuit("InitRepeatFlag") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(InitRepeatFlag);
};
} // namespace qbOS
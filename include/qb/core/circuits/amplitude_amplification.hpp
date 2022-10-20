/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once
#include "Circuit.hpp"
namespace qbOS {

/// Generic Grover-based amplitude amplification circuit
class AmplitudeAmplification : public xacc::quantum::Circuit {
public:
  AmplitudeAmplification() : xacc::quantum::Circuit("AmplitudeAmplification") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(AmplitudeAmplification);
};
} // namespace qbOS
/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once
#include "Circuit.hpp"
namespace qristal {

class AmplitudeAmplification : public xacc::quantum::Circuit {
public:
  AmplitudeAmplification() : xacc::quantum::Circuit("AmplitudeAmplification") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(AmplitudeAmplification);
};
}
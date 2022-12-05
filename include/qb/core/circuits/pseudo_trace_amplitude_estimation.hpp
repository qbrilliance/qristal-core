/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qb {

class PseudoTraceAmplitudeEstimation : public xacc::quantum::Circuit {
public:
  PseudoTraceAmplitudeEstimation() : xacc::quantum::Circuit("PseudoTraceAmplitudeEstimation") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(PseudoTraceAmplitudeEstimation);
};
} // namespace qb
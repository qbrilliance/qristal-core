/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once
#include "Circuit.hpp"
namespace qbOS {

/// Amplitude Estimation circuit (based on phase estimation):
/// Implements the original Quantum Amplitude Estimation (QAE) algorithm,
/// introduced by
// Brassard, G., Hoyer, P., Mosca, M., & Tapp, A. (2000). Quantum Amplitude
// Amplification and Estimation.
class CanonicalAmplitudeEstimation : public xacc::quantum::Circuit {
public:
  CanonicalAmplitudeEstimation() : xacc::quantum::Circuit("CanonicalAmplitudeEstimation") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(CanonicalAmplitudeEstimation);
};
} // namespace qbOS
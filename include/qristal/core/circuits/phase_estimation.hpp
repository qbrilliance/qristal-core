/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qristal {

/// Phase Estimation circuit:
/// The Phase Estimation circuit is used to estimate the phase phi of an
/// eigenvalue e^{2*pi*i*phi} of a unitary operator U. This estimation is a
/// central routine to several well-known algorithms, such as Shor's algorithm
/// or Quantum Amplitude Estimation.
class PhaseEstimation : public xacc::quantum::Circuit {
public:
  PhaseEstimation() : xacc::quantum::Circuit("PhaseEstimation") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(PhaseEstimation);
};
}
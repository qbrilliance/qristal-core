/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include <Circuit.hpp>
namespace qristal {
class AEtoMetric : public xacc::quantum::Circuit {
public:
  AEtoMetric() : xacc::quantum::Circuit("AEtoMetric") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(AEtoMetric);
};
}

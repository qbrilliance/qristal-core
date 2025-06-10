/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include <Circuit.hpp>
namespace qristal {

/// Comparator circuit:
/// The Comparator circuit flags which of the two
/// input bitstrings |a> or |b> encodes the largest value.
/// It is a core component of the decoder module.

class Comparator : public xacc::quantum::Circuit {
public:
  Comparator() : xacc::quantum::Circuit("Comparator") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(Comparator);
};
}
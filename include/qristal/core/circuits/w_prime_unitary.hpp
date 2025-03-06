/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once 

#include "Circuit.hpp"

namespace qristal {

/// W_prime circuit:
/// The W prime circuit encodes a provided probability_table into the 
// input qubits of the decoder
class WPrime : public xacc::quantum::Circuit {
public:
  WPrime() : xacc::quantum::Circuit("WPrime") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(WPrime);
};
}


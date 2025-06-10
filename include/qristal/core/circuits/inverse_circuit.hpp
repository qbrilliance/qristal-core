/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include <Circuit.hpp>
namespace qristal {

// This module takes a composite object input and
// returns the inverse circuit as a composite object
// The circuit A1A2...AN becomes AN*...A2*A1*

class InverseCircuit : public xacc::quantum::Circuit {
public:
  InverseCircuit() : xacc::quantum::Circuit("InverseCircuit") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(InverseCircuit);
};
}
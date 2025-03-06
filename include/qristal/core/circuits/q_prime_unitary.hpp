/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"

namespace qristal{
/// Q_prime circuit:
/// The Q prime circuit is a unitary operator to reset the qubits of the next 
/// symbol and its metric (probability). It is one of three unitary operators
/// generating each step in the advice state, which will eventually encode the
/// whole string and its metric
//class QPrime : public xacc::CompositeInstruction {
class QPrime : public xacc::quantum::Circuit {
public:
  QPrime() : xacc::quantum::Circuit("QPrime") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(QPrime);
  //QPrime() : xacc::CompositeInstruction() {} 
};
}
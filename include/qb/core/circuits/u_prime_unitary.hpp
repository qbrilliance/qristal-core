/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/
#pragma once 
#include "Circuit.hpp"
namespace qb {

/// U_prime circuit:
/// The U prime circuit is a unitary operator to copy the qubits of the next 
/// symbol to the end of the string and its metric (probability) to a quantum 
/// register where it can be processed either now or later. It is one of three unitary operators
/// generating each step in the advice state, which will eventually encode the
/// whole string and its metric
//class UPrime : public xacc::CompositeInstruction {
class UPrime : public xacc::quantum::Circuit {
public:
  UPrime() : xacc::quantum::Circuit("UPrime") {}
  bool expand(const xacc::HeterogeneousMap &runtimeOptions) override;
  const std::vector<std::string> requiredKeys() override;
  DEFINE_CLONE(UPrime);
  //QPrime() : xacc::CompositeInstruction() {} 
};
} // namespace qb
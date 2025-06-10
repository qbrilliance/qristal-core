/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include <qristal/core/circuits/compare_gt.hpp>
#include <CommonGates.hpp>
#include <IRProvider.hpp>
#include <xacc_service.hpp>
#include <CompositeInstruction.hpp>
#include <assert.h>
#include <memory>

namespace qristal {
bool CompareGT::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  std::vector<int> qubits_a;
  std::vector<int> qubits_b;
  int flag;
  int c_in;

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_a")) {
    return false;
  }
  qubits_a = runtimeOptions.get<std::vector<int>>("qubits_a");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_b")) {
    return false;
  }
  qubits_b = runtimeOptions.get<std::vector<int>>("qubits_b");

  if (qubits_a.size() != qubits_b.size()) {
    std::cout << "The two registers qubits_a and qubits_b must "
                 "contain the same number of qubits.\n";
    return false;
  }

  // The two set of indices must be unique and disjoint
  std::set<int> s1(qubits_a.begin(), qubits_a.end());
  std::set<int> s2(qubits_b.begin(), qubits_b.end());
  if (s1.size() != qubits_a.size() || s2.size() != qubits_b.size()) {
    std::cout << "qubits provided in qubits_a and qubits_b must be "
                 "unique and disjoint.\n";
    return false;
  }
  for (const auto &bit : qubits_a) {
    if (std::find(qubits_b.begin(), qubits_b.end(), bit) != qubits_b.end()) {
      std::cout << "qubits provided in qubits_a and qubits_b must "
                   "be unique and disjoint.\n";
      return false;
    }
  }

 if (!runtimeOptions.keyExists<int>("qubit_flag")) {
    return false;
  }
  flag = runtimeOptions.get<int>("qubit_flag");

  if (!runtimeOptions.keyExists<int>("qubit_ancilla")) {
    return false;
  }
  c_in = runtimeOptions.get<int>("qubit_ancilla");

  if (std::find(qubits_b.begin(), qubits_b.end(), flag) != qubits_b.end()) {
    std::cout << "qubit provided in qubit_flag cannot be in qubits_b. \n";
    return false;
  }

  if (std::find(qubits_a.begin(), qubits_a.end(), flag) != qubits_a.end()) {
    std::cout << "qubit provided in qubit_flag cannot be in qubits_a. \n";
    return false;
  }

  if (std::find(qubits_b.begin(), qubits_b.end(), c_in) != qubits_b.end()) {
    std::cout << "qubit provided in qubit_ancilla cannot be in qubits_b. \n";
    return false;
  }

  if (std::find(qubits_a.begin(), qubits_a.end(), c_in) != qubits_a.end()) {
    std::cout << "qubit provided in qubit_ancilla cannot be in qubits_a. \n";
    return false;
  }

  if (flag == c_in) {
      std::cout << "qubit_flag cannot be the same as qubit_ancilla. \n";
      return false;
  }

  bool is_LSB = true;
  if (runtimeOptions.keyExists<bool>("is_LSB")) {
    is_LSB = runtimeOptions.get<bool>("is_LSB");
  }

  // Everything is okay, now do the multiplication

  if (!is_LSB) {
    reverse(qubits_b.begin(), qubits_b.end());
    reverse(qubits_a.begin(), qubits_a.end());
  }

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  for (auto bit : qubits_b) {
      addInstruction(gateRegistry->createInstruction("X", bit));
  }

  auto adder1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("RippleCarryAdder"));
  std::vector<int> adder_bits = qubits_b;
  std::vector<int> sum_bits = qubits_a;
  sum_bits.push_back(flag);
  const bool expand_ok_adder1 = adder1->expand({{"adder_bits", adder_bits},
                {"sum_bits", sum_bits},
                {"c_in", c_in}});
  assert(expand_ok_adder1);
  addInstruction(adder1);

  for (auto bit : qubits_a) {
      addInstruction(gateRegistry->createInstruction("X", bit));
  }

  auto adder2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("RippleCarryAdder"));
  std::vector<int> adder_bits2 = qubits_b;
  std::vector<int> sum_bits2 = qubits_a;
  sum_bits2.push_back(flag);
  const bool expand_ok_adder2 = adder2->expand({{"adder_bits", adder_bits2},
                {"sum_bits", sum_bits2},
                {"c_in", c_in},
                {"no_overflow", true}});
  assert(expand_ok_adder2);
  addInstruction(adder2);

  for (auto bit : qubits_a) {
      addInstruction(gateRegistry->createInstruction("X", bit));
  }

  for (auto bit : qubits_b) {
      addInstruction(gateRegistry->createInstruction("X", bit));
  }

  if (!is_LSB) {
    std::reverse(qubits_b.begin(), qubits_b.end());
    std::reverse(qubits_a.begin(), qubits_a.end());
  }

  return true;
}

const std::vector<std::string> CompareGT::requiredKeys() {
  return {"qubits_a", "qubits_b", "qubit_flag", "qubit_ancilla"};
}
}

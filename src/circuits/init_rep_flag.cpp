/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/init_rep_flag.hpp"
#include "CommonGates.hpp"
#include "xacc_service.hpp"
//#include <array>
#include <assert.h>
#include <bits/c++config.h>
#include <iostream>
#include <ostream>
//#include <cstddef>
//#include <iterator>
//#include <optional>
//#include <ostream>
#include "IRProvider.hpp"

namespace qbOS{
bool InitRepeatFlag::expand(const xacc::HeterogeneousMap &runtimeOptions) {
  // Inputs:
  // iteration: which letter are we up to
  // qubits_init_repeat: Qubit flagging which symbols are initially a repeat
  // qubits_string: qubits assigned to next letter probabilities
  // qubits_next_letter: qubits assigned to next letter
  if (!runtimeOptions.keyExists<int>("iteration")) {
      return false;
  }
  int Iteration = runtimeOptions.get<int>("iteration");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_string")) {
    return false;
  }
  const auto qubits_string =
      runtimeOptions.get<std::vector<int>>("qubits_string");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_next_letter")) {
    return false;
  }
  const auto qubits_next_letter =
      runtimeOptions.get<std::vector<int>>("qubits_next_letter");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_init_repeat")) {
    return false;
  }
  const auto qubits_init_repeat =
      runtimeOptions.get<std::vector<int>>("qubits_init_repeat");

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  // Assert statements here?
  assert(qubits_string.size() > 0);
  assert(qubits_next_letter.size() > 0);
  assert(Iteration>0);

  int num_qubits_next_letter = qubits_next_letter.size();
  std::vector<int> qubits_last_letter;
  //Initialize repetition flags
  for (int index=0; index < num_qubits_next_letter; index++) {
    qubits_last_letter.push_back(qubits_string[(Iteration-1)*num_qubits_next_letter + index]);
  }
  auto repeat_ = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("EqualityChecker"));
  xacc::HeterogeneousMap rep_map = {{"qubits_a", qubits_next_letter}, {"qubits_b", qubits_last_letter}, {"flag",qubits_init_repeat[Iteration]}};
  repeat_->expand(rep_map);
  //Add marking of repeat symbols to init_repeat_flag circuit
  addInstructions(repeat_->getInstructions());

  return true;


}

const std::vector<std::string> InitRepeatFlag::requiredKeys() {
    return {"iteration","qubits_string", "qubits_next_letter", "qubits_init_repeat"};
}
}// namespace qbOS

/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/controlled_swap.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <optional>
#include <vector>

namespace qb {
bool ControlledSwap::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  ////////////////////////////////////////////////////////
  // Get expected inputs or define any required variables
  ////////////////////////////////////////////////////////

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_a")) {
    return false;
  }
  std::vector<int> qubits_a = runtimeOptions.get<std::vector<int>>("qubits_a");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_b")) {
    return false;
  }
  std::vector<int> qubits_b = runtimeOptions.get<std::vector<int>>("qubits_b");

  assert(qubits_a.size() == qubits_b.size());

  bool flags_on_exist;
  bool flags_off_exist;
  std::vector<int> flags_on;
  std::vector<int> flags_off;

  if (!runtimeOptions.keyExists<std::vector<int>>("flags_on")) {
    flags_on_exist = false;
  } else {
    flags_on = runtimeOptions.get<std::vector<int>>("flags_on");
    if (flags_on.size() > 0) {
      flags_on_exist = true;
    }
  }

  if (!runtimeOptions.keyExists<std::vector<int>>("flags_off")) {
    flags_off_exist = false;
  } else {
    flags_off = runtimeOptions.get<std::vector<int>>("flags_off");
    if (flags_off.size() > 0) {
      flags_off_exist = true;
    }
  }

  ////////////////////////////////////////////////////////
  // Add instructions
  ////////////////////////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  if (flags_off_exist) {
    for (int q = 0; q < flags_off.size(); q++) {
      addInstruction(gateRegistry->createInstruction("X", flags_off[q]));
    }
  }

  std::vector<int> control_bits;
  if (flags_on_exist) {
    for (int q = 0; q < flags_on.size(); q++) {
      control_bits.push_back(flags_on[q]);
    }
  }
  if (flags_off_exist) {
    for (int q = 0; q < flags_off.size(); q++) {
      control_bits.push_back(flags_off[q]);
    }
  }

  auto swap = gateRegistry->createComposite("swap");
  for (int q = 0; q < qubits_a.size(); q++) {
    swap->addInstruction(gateRegistry->createInstruction(
        "CX",
        std::vector<std::size_t>{static_cast<unsigned long>(qubits_a[q]),
                                 static_cast<unsigned long>(qubits_b[q])}));
  }
  for (int q = 0; q < qubits_a.size(); q++) {
    swap->addInstruction(gateRegistry->createInstruction(
        "CX",
        std::vector<std::size_t>{static_cast<unsigned long>(qubits_b[q]),
                                 static_cast<unsigned long>(qubits_a[q])}));
  }
  for (int q = 0; q < qubits_a.size(); q++) {
    swap->addInstruction(gateRegistry->createInstruction(
        "CX",
        std::vector<std::size_t>{static_cast<unsigned long>(qubits_a[q]),
                                 static_cast<unsigned long>(qubits_b[q])}));
  }

  if (flags_on_exist || flags_off_exist) {
    auto controlled_swap =
        std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("C-U"));
    controlled_swap->expand({{"U", swap}, {"control-idx", control_bits}});
    addInstruction(controlled_swap);
  } else {
    addInstructions(swap->getInstructions());
  }

  if (flags_off_exist) {
    for (int q = 0; q < flags_off.size(); q++) {
      addInstruction(gateRegistry->createInstruction("X", flags_off[q]));
    }
  }

  return true;
}

const std::vector<std::string> ControlledSwap::requiredKeys() {
  return {"qubits_a", "qubits_b"};
}

} // namespace qb

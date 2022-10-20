/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/controlled_subtraction.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <memory>
#include <optional>
#include <vector>

namespace qbOS {
bool ControlledSubtraction::expand(
    const xacc::HeterogeneousMap &runtimeOptions) {

  std::vector<int> larger;
  std::vector<int> smaller;

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_larger")) {
    return false;
  }
  larger = runtimeOptions.get<std::vector<int>>("qubits_larger");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_smaller")) {
    return false;
  }
  smaller = runtimeOptions.get<std::vector<int>>("qubits_smaller");

  if (larger.size() != smaller.size()) {
    std::cout << "The two registers qubits_larger and qubits_smaller must "
                 "contain the same number of qubits.\n";
    return false;
  }

  // The two set of indices must be unique and disjoint
  std::set<int> s1(larger.begin(), larger.end());
  std::set<int> s2(smaller.begin(), smaller.end());
  if (s1.size() != larger.size() || s2.size() != smaller.size()) {
    std::cout << "qubits provided in qubits_larger and qubits_smaller must be "
                 "unique and disjoint.\n";
    return false;
  }
  for (const auto &bit : larger) {
    if (std::find(smaller.begin(), smaller.end(), bit) != smaller.end()) {
      std::cout << "qubits provided in qubits_larger and qubits_smaller must "
                   "be unique and disjoint.\n";
      return false;
    }
  }

  int qubit_ancilla = -1;
  if (runtimeOptions.keyExists<int>("qubit_ancilla")) {
    qubit_ancilla = runtimeOptions.get<int>("qubit_ancilla");
  }

  std::vector<int> controls_on;
  std::vector<int> controls_off;

  if (runtimeOptions.keyExists<std::vector<int>>("controls_on")) {
    controls_on = runtimeOptions.get<std::vector<int>>("controls_on");
  }
  if (runtimeOptions.keyExists<std::vector<int>>("controls_off")) {
    controls_off = runtimeOptions.get<std::vector<int>>("controls_off");
  }

  bool is_LSB = true;
  if (runtimeOptions.keyExists<bool>("is_LSB")) {
    is_LSB = runtimeOptions.get<bool>("is_LSB");
  }

  if (!is_LSB) {
    std::reverse(larger.begin(), larger.end());
    std::reverse(smaller.begin(), smaller.end());
  }

  // Everything is okay, now do the subtraction
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  for (auto bit : controls_off) {
    addInstruction(gateRegistry->createInstruction("X", bit));
  }

  auto circuit = gateRegistry->createComposite("circuit");

  auto sub = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("Subtraction"));
  sub->expand({{"qubits_larger", larger},
               {"qubits_smaller", smaller},
               {"qubit_ancilla", qubit_ancilla}});
  circuit->addInstruction(sub);

  if (controls_on.size() > 0 || controls_off.size() > 0) {
    auto csub = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    std::vector<int> controls = controls_on;
    for (auto bit : controls_off) {
      controls.push_back(bit);
    }
    csub->expand({{"U", circuit}, {"control-idx", controls}});
    addInstruction(csub);
  } else {
    addInstruction(circuit);
  }

  for (auto bit : controls_off) {
    addInstruction(gateRegistry->createInstruction("X", bit));
  }

  if (!is_LSB) {
    std::reverse(larger.begin(), larger.end());
    std::reverse(smaller.begin(), smaller.end());
  }

  return true;
}

const std::vector<std::string> ControlledSubtraction::requiredKeys() {
  return {"qubits_larger", "qubits_smaller"};
}

} // namespace qbOS

/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/equality_checker.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <assert.h>
#include <bitset>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <algorithm>

namespace qbOS {
bool EqualityChecker::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  // Inputs:

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_a")) {
    return false;
  }
  std::vector<int> qubits_a = runtimeOptions.get<std::vector<int>>("qubits_a");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_b")) {
    return false;
  }
  std::vector<int> qubits_b = runtimeOptions.get<std::vector<int>>("qubits_b");

  if (!runtimeOptions.keyExists<int>("flag")) {
    return false;
  }
  int flag = runtimeOptions.get<int>("flag");

  std::vector<int> controls_on;
  if (runtimeOptions.keyExists<std::vector<int>>("controls_on")) {
    controls_on = runtimeOptions.get<std::vector<int>>("controls_on");
  }

  std::vector<int> controls_off;
  if (runtimeOptions.keyExists<std::vector<int>>("controls_off")) {
    controls_off = runtimeOptions.get<std::vector<int>>("controls_off");
  }

  bool use_ancilla = false;
  if (runtimeOptions.keyExists<bool>("use_ancilla")) {
      use_ancilla = runtimeOptions.get<bool>("use_ancilla");
  }

  std::vector<int> qubits_ancilla;
  if (use_ancilla) {
      if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla")) {
          return false;
      }
      qubits_ancilla = runtimeOptions.get<std::vector<int>>("qubits_ancilla");
  }

  // Construct circuit
  int n = qubits_a.size();
  assert(qubits_b.size() == n);
  if (use_ancilla) { assert(qubits_ancilla.size() == n-1); }

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  if (controls_off.size() > 0) {
    for (int q = 0; q < controls_off.size(); q++) {
      addInstruction(gateRegistry->createInstruction("X", controls_off[q]));
    }
  }
  auto tot = gateRegistry->createComposite("tot");

  for (int i = 0; i < n; i++) {
    tot->addInstruction(gateRegistry->createInstruction("CX",std::vector<std::size_t>
        {static_cast<unsigned long>(qubits_a[i]),static_cast<unsigned long>(qubits_b[i])}));
  }

  for (int i = 0; i < n; i++) {
      tot->addInstruction(gateRegistry->createInstruction("X", qubits_b[i]));
  }

  if (use_ancilla) {
    auto U = gateRegistry->createComposite("U");
    U->addInstruction(gateRegistry->createInstruction("X", flag));
    auto mcu = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("MultiControlledUWithAncilla"));
    const bool expand_ok = mcu->expand({{"qubits_control", qubits_b},
                                        {"qubits_ancilla", qubits_ancilla},
                                        {"U", U}});
    tot->addInstructions(mcu->getInstructions());
  } else {
    auto x_gate = gateRegistry->createComposite("x_gate");
    auto temp_gate = gateRegistry->createInstruction("X", flag);
    temp_gate->setBufferNames({"q"});
    x_gate->addInstruction(temp_gate);
    auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    mcx->expand({{"U", x_gate}, {"control-idx", qubits_b}});
    tot->addInstruction(mcx);
  }

    for (int i = 0; i < n; i++) {
      tot->addInstruction(gateRegistry->createInstruction("X", qubits_b[i]));
  }

    for (int i = 0; i < n; i++) {
    tot->addInstruction(gateRegistry->createInstruction("CX",std::vector<std::size_t>
        {static_cast<unsigned long>(qubits_a[i]),static_cast<unsigned long>(qubits_b[i])}));
  }

  if (controls_off.size()>0 || controls_on.size()>0) {
      std::vector<int> controls;
      for (int q = 0; q < controls_on.size(); q++) {
          controls.push_back(controls_on[q]);
      }
      for (int q = 0; q < controls_off.size(); q++) {
          controls.push_back(controls_off[q]);
      }
    auto mcu = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    mcu->expand({{"U", tot}, {"control-idx", controls}});
    addInstructions(mcu->getInstructions());
  } else {
      addInstructions(tot->getInstructions());
  }

  if (controls_off.size() > 0) {
    for (int q = 0; q < controls_off.size(); q++) {
      addInstruction(gateRegistry->createInstruction("X", controls_off[q]));
    }
  }

  return true;
}

const std::vector<std::string> EqualityChecker::requiredKeys() {
  return {"qubits_a", "qubits_b", "flag"};
}

} // namespace qbOS

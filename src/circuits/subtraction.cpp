/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include "qristal/core/circuits/subtraction.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <CompositeInstruction.hpp>
#include <assert.h>
#include <memory>

namespace qristal {
bool Subtraction::expand(const xacc::HeterogeneousMap &runtimeOptions) {

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

  int qubit_ancilla = -1;
  if (runtimeOptions.keyExists<int>("qubit_ancilla")) {
    qubit_ancilla = runtimeOptions.get<int>("qubit_ancilla");
  }

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

  bool is_LSB = true;
  if (runtimeOptions.keyExists<bool>("is_LSB")) {
    is_LSB = runtimeOptions.get<bool>("is_LSB");
  }

  if (!is_LSB) {
    reverse(larger.begin(), larger.end());
    reverse(smaller.begin(), smaller.end());
  }

  // Everything is okay, now do the subtraction
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  if (qubit_ancilla < 0) {
    for (int i = 0; i < larger.size(); i++) {
      addInstruction(gateRegistry->createInstruction(
          "CX", {static_cast<unsigned long>(smaller[i]),
                 static_cast<unsigned long>(larger[i])}));
      if (i != (int)larger.size() - 1) {
        auto ccx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("C-U"));
        std::vector<int> controls = {smaller[i], larger[i]};
        auto add1 = gateRegistry->createComposite("add1");
        add1->addInstruction(
            gateRegistry->createInstruction("X", smaller[i + 1]));
        for (int j = i + 1; j < smaller.size() - 1; j++) {
          std::vector<int> add1_controls;
          for (int k = i + 1; k <= j; k++) {
            add1_controls.push_back(smaller[k]);
          }
          for (auto bit : add1_controls) {
            add1->addInstruction(gateRegistry->createInstruction("X", bit));
          }
          auto add1_mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
          auto add1_x = gateRegistry->createComposite("add1_x");
          add1_x->addInstruction(
              gateRegistry->createInstruction("X", smaller[j + 1]));
          add1_mcx->expand({{"U", add1_x}, {"control-idx", add1_controls}});
          add1->addInstruction(add1_mcx);
          for (auto bit : add1_controls) {
            add1->addInstruction(gateRegistry->createInstruction("X", bit));
          }
        }
        ccx->expand({{"U", add1}, {"control-idx", controls}});
        addInstruction(ccx);
      }
    }

    for (int i = (int)larger.size() - 2; i >= 0; i--) {
      auto ccx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      std::vector<int> controls = {smaller[i], larger[i]};
      auto take1 = gateRegistry->createComposite("take1");
      take1->addInstruction(
          gateRegistry->createInstruction("X", smaller[i + 1]));
      for (int j = i + 1; j < smaller.size() - 1; j++) {
        std::vector<int> take1_controls;
        for (int k = i + 1; k <= j; k++) {
          take1_controls.push_back(smaller[k]);
        }
        auto take1_mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("C-U"));
        auto take1_x = gateRegistry->createComposite("take1_x");
        take1_x->addInstruction(
            gateRegistry->createInstruction("X", smaller[j + 1]));
        take1_mcx->expand({{"U", take1_x}, {"control-idx", take1_controls}});
        take1->addInstruction(take1_mcx);
      }
      ccx->expand({{"U", take1}, {"control-idx", controls}});
      addInstruction(ccx);
    }
  } else {
    for (auto bit : larger) {
      addInstruction(gateRegistry->createInstruction("X", bit));
    }
    auto add = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("RippleCarryAdder"));
    const bool expand_ok_add = add->expand({{"adder_bits", smaller},
                                            {"sum_bits", larger},
                                            {"c_in", qubit_ancilla},
                                            {"no_overflow", true}});
    assert(expand_ok_add);
    addInstruction(add);
    for (auto bit : larger) {
      addInstruction(gateRegistry->createInstruction("X", bit));
    }
  }

  if (!is_LSB) {
    std::reverse(larger.begin(), larger.end());
    std::reverse(smaller.begin(), smaller.end());
  }

  return true;
}

const std::vector<std::string> Subtraction::requiredKeys() {
  return {"qubits_larger", "qubits_smaller"};
}
}

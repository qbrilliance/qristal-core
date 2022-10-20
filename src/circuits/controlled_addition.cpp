/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/controlled_addition.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <optional>
#include <vector>

namespace qbOS {
bool ControlledAddition::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  ////////////////////////////////////////////////////////
  // Get expected inputs or define any required variables
  ////////////////////////////////////////////////////////

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_adder")) {
    return false;
  }
  std::vector<int> qubits_adder =
      runtimeOptions.get<std::vector<int>>("qubits_adder");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_sum")) {
    return false;
  }
  std::vector<int> qubits_sum =
      runtimeOptions.get<std::vector<int>>("qubits_sum");

  if (!runtimeOptions.keyExists<int>("c_in")) {
    return false;
  }
  int c_in = runtimeOptions.get<int>("c_in");

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

  bool no_overflow = false;
  if (runtimeOptions.keyExists<bool>("no_overflow")) {
    no_overflow = runtimeOptions.get<bool>("no_overflow");
  }

  if (!no_overflow) {
      assert(qubits_adder.size() < qubits_sum.size());
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

  if (flags_on_exist || flags_off_exist) {
    const auto majority =
        [&gateRegistry](size_t a, size_t b, size_t c,
                        std::vector<int> extra_controls,
                        std::shared_ptr<xacc::CompositeInstruction> &comp) {
          auto mcx1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
          auto x1 = gateRegistry->createComposite("x1");
          x1->addInstruction(gateRegistry->createInstruction("X", b));
          std::vector<int> controls1 = extra_controls;
          controls1.push_back(c);
          mcx1->expand({{"U", x1}, {"control-idx", controls1}});
          comp->addInstruction(mcx1);

          auto mcx2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
          auto x2 = gateRegistry->createComposite("x2");
          x2->addInstruction(gateRegistry->createInstruction("X", a));
          std::vector<int> controls2 = extra_controls;
          controls2.push_back(c);
          mcx2->expand({{"U", x2}, {"control-idx", controls2}});
          comp->addInstruction(mcx2);

          auto mcx3 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
          auto x3 = gateRegistry->createComposite("x3");
          x3->addInstruction(gateRegistry->createInstruction("X", c));
          std::vector<int> controls3 = extra_controls;
          controls3.push_back(a);
          controls3.push_back(b);
          mcx3->expand({{"U", x3}, {"control-idx", controls3}});
          comp->addInstruction(mcx3);
        };

    const auto unmajority =
        [&gateRegistry](size_t a, size_t b, size_t c,
                        std::vector<int> extra_controls,
                        std::shared_ptr<xacc::CompositeInstruction> &comp) {
          auto mcx1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
          auto x1 = gateRegistry->createComposite("x1");
          x1->addInstruction(gateRegistry->createInstruction("X", c));
          std::vector<int> controls1 = extra_controls;
          controls1.push_back(a);
          controls1.push_back(b);
          mcx1->expand({{"U", x1}, {"control-idx", controls1}});
          comp->addInstruction(mcx1);

          auto mcx2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
          auto x2 = gateRegistry->createComposite("x2");
          x2->addInstruction(gateRegistry->createInstruction("X", a));
          std::vector<int> controls2 = extra_controls;
          controls2.push_back(c);
          mcx2->expand({{"U", x2}, {"control-idx", controls2}});
          comp->addInstruction(mcx2);

          auto mcx3 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
          auto x3 = gateRegistry->createComposite("x3");
          x3->addInstruction(gateRegistry->createInstruction("X", b));
          std::vector<int> controls3 = extra_controls;
          controls3.push_back(a);
          mcx3->expand({{"U", x3}, {"control-idx", controls3}});
          comp->addInstruction(mcx3);
        };

    auto controlled_addition = gateRegistry->createComposite("QB_ADDER_COMP");
    auto &b = qubits_sum;
    auto &a = qubits_adder;

    majority(c_in, b[0], a[0], control_bits, controlled_addition);

    for (size_t j = 0; j < a.size() - 1; ++j) {
      majority(a[j], b[j + 1], a[j + 1], control_bits, controlled_addition);
    }

    if (!no_overflow) {
      std::vector<int> controls = control_bits;
      controls.push_back(a[a.size() - 1]);
      auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      auto x = gateRegistry->createComposite("x");
      x->addInstruction(gateRegistry->createInstruction("X", b[b.size() - 1]));
      mcx->expand({{"U", x}, {"control-idx", controls}});
      controlled_addition->addInstruction(mcx);
    }

    for (int j = a.size() - 2; j >= 0; --j) {
      unmajority(a[j], b[j + 1], a[j + 1], control_bits, controlled_addition);
    }

    unmajority(c_in, b[0], a[0], control_bits, controlled_addition);

    addInstructions(controlled_addition->getInstructions());
  } else {
    auto addition = gateRegistry->createComposite("add");
    auto ripple_adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("RippleCarryAdder"));
    xacc::HeterogeneousMap options{
        {"adder_bits", qubits_adder}, {"sum_bits", qubits_sum}, {"c_in", c_in}, {"no_overflow", no_overflow}};
    const bool expand_ok = ripple_adder->expand(options);
    assert(expand_ok);
    addition->addInstructions(ripple_adder->getInstructions());
    addInstructions(addition->getInstructions());
  }

  if (flags_off_exist) {
    for (int q = 0; q < flags_off.size(); q++) {
      addInstruction(gateRegistry->createInstruction("X", flags_off[q]));
    }
  }

  return true;
}

const std::vector<std::string> ControlledAddition::requiredKeys() {
  return {"qubits_adder", "qubits_sum", "c_in"};
}

} // namespace qbOS

/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/generalised_mcx.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <optional>
#include <vector>

namespace qb {
bool GeneralisedMCX::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  ////////////////////////////////////////////////////////
  // Get expected inputs or define any required variables
  ////////////////////////////////////////////////////////

  if (!runtimeOptions.keyExists<int>("target")) {
      return false;
  }
  int target = runtimeOptions.get<int>("target");

  bool controls_on_exist;
  bool controls_off_exist;
  std::vector<int> controls_on;
  std::vector<int> controls_off;

  if (!runtimeOptions.keyExists<std::vector<int>>("controls_on")) {
    controls_on_exist = false;
  } else {
    controls_on = runtimeOptions.get<std::vector<int>>("controls_on");
    if (controls_on.size() > 0) {
      controls_on_exist = true;
    }
  }

  if (!runtimeOptions.keyExists<std::vector<int>>("controls_off")) {
    controls_off_exist = false;
  } else {
    controls_off = runtimeOptions.get<std::vector<int>>("controls_off");
    if (controls_off.size() > 0) {
      controls_off_exist = true;
    }
  }

  ////////////////////////////////////////////////////////
  // Add instructions
  ////////////////////////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  if (controls_off_exist) {
    for (int q = 0; q < controls_off.size(); q++) {
      addInstruction(gateRegistry->createInstruction("X", controls_off[q]));
    }
  }

  std::vector<int> control_bits;
  if (controls_on_exist) {
    for (int q = 0; q < controls_on.size(); q++) {
      control_bits.push_back(controls_on[q]);
    }
  }
  if (controls_off_exist) {
    for (int q = 0; q < controls_off.size(); q++) {
      control_bits.push_back(controls_off[q]);
    }
  }

  auto x = gateRegistry->createComposite("x");
  x->addInstruction(gateRegistry->createInstruction("X", target));

  if (controls_on_exist || controls_off_exist) {
    auto mcx =
        std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("C-U"));
    mcx->expand({{"U", x}, {"control-idx", control_bits}});
    addInstruction(mcx);
  } else {
    addInstructions(x->getInstructions());
  }

  if (controls_off_exist) {
    for (int q = 0; q < controls_off.size(); q++) {
      addInstruction(gateRegistry->createInstruction("X", controls_off[q]));
    }
  }

  return true;
}

const std::vector<std::string> GeneralisedMCX::requiredKeys() {
  return {"target"};
}

} // namespace qb

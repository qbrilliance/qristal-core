/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include <qristal/core/circuits/mcu_with_ancilla.hpp>
#include <CommonGates.hpp>
#include <IRProvider.hpp>
#include <xacc_service.hpp>
#include <xacc.hpp>
#include <CompositeInstruction.hpp>
#include <Instruction.hpp>
#include <assert.h>
#include <memory>

namespace qristal {
bool MultiControlledUWithAncilla::expand(
    const xacc::HeterogeneousMap &runtimeOptions) {
  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_control")) {
    return false;
  }
  std::vector<int> ctrl_bits =
      runtimeOptions.get<std::vector<int>>("qubits_control");
  assert(ctrl_bits.size() > 0);

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla")) {
    return false;
  }
  std::vector<int> ancilla_bits =
      runtimeOptions.get<std::vector<int>>("qubits_ancilla");
  assert(ancilla_bits.size() > 0);

  if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("U")) {
    return false;
  }
  auto U_gate = runtimeOptions.getPointerLike<xacc::CompositeInstruction>("U");

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  auto x_gate = gateRegistry->createComposite("x_gate");
  auto temp_gate = gateRegistry->createInstruction("X", ancilla_bits[0]);
  temp_gate->setBufferNames({"q"});
  x_gate->addInstruction(temp_gate);
  auto toffoli1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("C-U"));
  std::vector<int> ctrl_bits_toffoli1 = {ctrl_bits[0], ctrl_bits[1]};
  toffoli1->expand({{"U", x_gate}, {"control-idx", ctrl_bits_toffoli1}});
  addInstruction(toffoli1);

  int i = 2;
  while (i < ctrl_bits.size()) {
    auto x = gateRegistry->createComposite("x");
    auto temp = gateRegistry->createInstruction("X", ancilla_bits[i - 1]);
    temp->setBufferNames({"q"});
    x->addInstruction(temp);
    std::vector<int> ctrl_bits_toffoli = {ctrl_bits[i], ancilla_bits[i - 2]};
    auto toffoli = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    toffoli->expand({{"U", x}, {"control-idx", ctrl_bits_toffoli}});
    addInstruction(toffoli);
    i = i + 1;
  }

  auto CU = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("C-U"));
  CU->expand(
      {{"U", U_gate}, {"control-idx", ancilla_bits[ancilla_bits.size() - 1]}});
  addInstruction(CU);

  i = ctrl_bits.size() - 1;
  while (i > 1) {
    auto x = gateRegistry->createComposite("x");
    auto temp = gateRegistry->createInstruction("X", ancilla_bits[i - 1]);
    temp->setBufferNames({"q"});
    x->addInstruction(temp);
    std::vector<int> ctrl_bits_toffoli = {ctrl_bits[i], ancilla_bits[i - 2]};
    auto toffoli = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    toffoli->expand({{"U", x}, {"control-idx", ctrl_bits_toffoli}});
    addInstruction(toffoli);
    i = i - 1;
  }

  xacc::InstructionIterator it(toffoli1);
  while (it.hasNext()) {
    auto nextInst = it.next();
    if (nextInst->isEnabled() && nextInst->isComposite() &&
        nextInst->name() == "C-U") {
      if (xacc::ir::asComposite(nextInst)->nInstructions() == 0) {
        addInstruction(nextInst->clone());
      }
    }
    if (nextInst->isEnabled() && !nextInst->isComposite()) {
      addInstruction(nextInst->clone());
    }
  }

  return true;
}

const std::vector<std::string> MultiControlledUWithAncilla::requiredKeys() {
  return {};
}
}

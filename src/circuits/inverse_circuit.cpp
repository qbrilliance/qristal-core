/***

 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd

 ***/

#include "qb/core/circuits/inverse_circuit.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <optional>
#include <vector>

namespace qbOS {

bool InverseCircuit::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  ////////////////////////////////////////////////////////
  // Get expected inputs or define any required variables
  ////////////////////////////////////////////////////////

  if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("circ")) {
    return false;
  }
  auto circ = runtimeOptions.getPointerLike<xacc::CompositeInstruction>("circ");

  ////////////////////////////////////////////////////////
  // Add instructions
  ////////////////////////////////////////////////////////

  std::vector<xacc::InstPtr> circ_gates;
  xacc::InstructionIterator it(xacc::as_shared_ptr(circ));

  while (it.hasNext()) {
    auto nextInst = it.next();
    if (nextInst->isEnabled() && nextInst->isComposite() &&
        nextInst->name() == "C-U") {
      if (xacc::ir::asComposite(nextInst)->nInstructions() == 0) {
        circ_gates.emplace_back(nextInst->clone());
      }
    }
    if (nextInst->isEnabled() && !nextInst->isComposite()) {
      circ_gates.emplace_back(nextInst->clone());
    }
  }

  std::vector<xacc::InstPtr> temp_inverse_gates;
  for (const auto &inst : circ_gates) {
    temp_inverse_gates.emplace_back(inst->clone());
  }
  std::reverse(temp_inverse_gates.begin(), temp_inverse_gates.end());
  std::vector<xacc::InstPtr> inverse_gates;

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  for (auto &inst : temp_inverse_gates) {

    // Cannot handle some gates
    if (inst->name() == "Reset") {
        std::cout << "Cannot invert a circuit containing a qubit reset.\n";
        return false;
    } else if (inst->name() == "Measure") {
        std::cout << "Cannot invert a circuit containing a qubit measurement.\n";
        return false;
    } else if (inst->name() == "AnnealingInstruction") {
        std::cout << "Cannot invert a circuit containing an annealing instruction.\n";
        return false;
    } else if (inst->name() == "ifstmt") {
        std::cout << "Cannot invert a circuit containing an ifstmt.\n";
        return false;
    }

    // iSwap gate:
    else if (inst->name() == "iSwap") {
      auto q0 = inst->bits()[0];
      auto q1 = inst->bits()[1];
      inverse_gates.emplace_back(gateRegistry->createInstruction("H", q1));
      inverse_gates.emplace_back(gateRegistry->createInstruction("CNOT", {q1,q0}));
      inverse_gates.emplace_back(gateRegistry->createInstruction("CNOT", {q0,q1}));
      inverse_gates.emplace_back(gateRegistry->createInstruction("H", q0));
      inverse_gates.emplace_back(gateRegistry->createInstruction("Sdg", q0));
      inverse_gates.emplace_back(gateRegistry->createInstruction("Sdg", q1));
    }

    // U gate:
    else if (inst->name() == "U") {
        auto p0 = inst->getParameter(0).template as<double>();
        auto p1 = inst->getParameter(1).template as<double>();
        auto p2 = inst->getParameter(2).template as<double>();
        inst->setParameter(0, -p0);
        inst->setParameter(1, -p2);
        inst->setParameter(2, -p1);
        inverse_gates.emplace_back(inst);
    }

    // fSim gate:
    else if (inst->name() == "fSim") {
        inst->setParameter(0, -inst->getParameter(0).template as<double>());
        inst->setParameter(1, -inst->getParameter(1).template as<double>());
        inverse_gates.emplace_back(inst);
    }

    // Parametric gates:
    else if (inst->name() == "Rx" || inst->name() == "Ry" || inst->name() == "Rz" ||
        inst->name() == "CPhase" || inst->name() == "U1" || inst->name() == "RZZ" ||
        inst->name() == "CRZ" || inst->name() == "Rphi" || inst->name() == "XX" || inst->name() == "XY") {
      inst->setParameter(0, -inst->getParameter(0).template as<double>());
      inverse_gates.emplace_back(inst);
    }

    // Handles T and S gates, etc... => T -> Tdg
    else if (inst->name() == "T") {
      auto tdg = gateRegistry->createInstruction("Tdg", inst->bits()[0]);
      inverse_gates.emplace_back(tdg);
    } else if (inst->name() == "S") {
      auto sdg = gateRegistry->createInstruction("Sdg", inst->bits()[0]);
      inverse_gates.emplace_back(sdg);
    } else if (inst->name() == "Tdg") {
      auto t = gateRegistry->createInstruction("T", inst->bits()[0]);
      inverse_gates.emplace_back(t);
    } else if (inst->name() == "Sdg") {
      auto s = gateRegistry->createInstruction("S", inst->bits()[0]);
      inverse_gates.emplace_back(s);
    }

    // Anything else is its own inverse
    else {
      inverse_gates.emplace_back(inst);
    }
  }

  // assert(inverse_gates.size() == circ_gates.size());
  addInstructions(std::move(inverse_gates), false);
  // instructions.insert(instructions.end(), inverse_gates.begin(), inverse_gates.end());


  return true;
}

const std::vector<std::string> InverseCircuit::requiredKeys() { return {}; }

} // namespace qbOS

/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include "qristal/core/circuit_builder.hpp"
#include "qristal/core/circuits/amplitude_amplification.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <assert.h>
#include "xacc.hpp"

namespace qristal {
bool AmplitudeAmplification::expand(
    const xacc::HeterogeneousMap &runtimeOptions) {
  // power: The number of times the Grover operator is repeated.
  // default = 1
  int power = 1;
  if (runtimeOptions.keyExists<int>("power")) {
    power = runtimeOptions.get<int>("power");
    assert(power > 0);
  }
  // The oracle (reflecting about the bad states)
  if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("oracle")) {
    return false;
  }
  auto oracle_circuit = xacc::as_shared_ptr(
      runtimeOptions.getPointerLike<xacc::CompositeInstruction>("oracle"));
  assert(oracle_circuit->nInstructions() > 0);

  // State-prep:
  // If none provided, use all Hadamard (equal superposition)
  auto state_prep_circuit = [&]() {
    if (runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
            "state_preparation")) {
      return xacc::as_shared_ptr(
          runtimeOptions.getPointerLike<xacc::CompositeInstruction>(
              "state_preparation"));
    }
    const auto Qubits = qristal::uniqueBitsQD(oracle_circuit);
    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    auto state_prep = gateRegistry->createComposite("state_prep");
    for (auto i : Qubits) {
      state_prep->addInstruction(gateRegistry->createInstruction("H", i));
    }
    return state_prep;
  }();

  // Grover operator circuit: customized (provided) or constructed from oracle
  // and state prep
  auto grover_operator_circuit = [&]() {
    if (runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
            "grover_operator")) {
      return xacc::as_shared_ptr(
          runtimeOptions.getPointerLike<xacc::CompositeInstruction>(
              "grover_operator"));
    }
    // Construct Grover operator circuit as:
    // Q = A S0 Adagger Sf
    // S0 => zero reflection circuit
    // Sf => phase oracle (multiplies the good states by -1)
    // A => state-prep
    const auto Qubits = qristal::uniqueBitsQD(oracle_circuit);
    std::vector<int> Qubits_vec;
    for (auto q : Qubits) {
      Qubits_vec.push_back(q);
    }
    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    auto zero_reflection = gateRegistry->createComposite("zero_reflection");
    for (auto q : Qubits) {
      zero_reflection->addInstruction(gateRegistry->createInstruction("X", q));
    }
    if (Qubits.size() == 1) {
      for (auto q : Qubits) {
        zero_reflection->addInstruction(
            gateRegistry->createInstruction("Z", q));
      }
    } else {
      zero_reflection->addInstruction(gateRegistry->createInstruction(
          "H", Qubits_vec[Qubits_vec.size() - 1]));

      // Multi-controlled X:
      std::vector<int> controlled_bits;
      for (int i = 0; i < Qubits_vec.size() - 1; ++i) {
        controlled_bits.emplace_back(Qubits_vec[i]);
      }
      auto x_gate = gateRegistry->createComposite("x_gate");
      x_gate->addInstruction(gateRegistry->createInstruction(
          "X", Qubits_vec[Qubits_vec.size() - 1]));
      auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      mcx->expand({{"U", x_gate}, {"control-idx", controlled_bits}});
      zero_reflection->addInstruction(mcx);
      //=================================

      zero_reflection->addInstruction(gateRegistry->createInstruction(
          "H", Qubits_vec[Qubits_vec.size() - 1]));
    }

    for (auto q : Qubits) {
      zero_reflection->addInstruction(gateRegistry->createInstruction("X", q));
    }

    // Construct the Grover operator circuit:
    auto grover_op_circuit = gateRegistry->createComposite("grover_op");
    // Q = A S0 Adagger Sf
    // Note: appending circuit from right to left
    for (auto &inst : oracle_circuit->getInstructions()) {
      grover_op_circuit->addInstruction(inst->clone());
    }

    std::vector<xacc::InstPtr> state_prep_gates;
    xacc::InstructionIterator it(state_prep_circuit);
    while (it.hasNext()) {
      auto nextInst = it.next();
     if (nextInst->isEnabled() && nextInst->isComposite() &&
          nextInst->name() == "C-U") {
        if (xacc::ir::asComposite(nextInst)->nInstructions() == 0) {
          state_prep_gates.emplace_back(nextInst->clone());
        }
      }
      if (nextInst->isEnabled() && !nextInst->isComposite()) {
        state_prep_gates.emplace_back(nextInst->clone());
      }
    }

    std::vector<xacc::InstPtr> temp_inverse_state_prep_gates;
    for (const auto &inst : state_prep_gates) {
      temp_inverse_state_prep_gates.emplace_back(inst->clone());
    }
    std::reverse(temp_inverse_state_prep_gates.begin(),
                 temp_inverse_state_prep_gates.end());

    std::vector<xacc::InstPtr> inverse_state_prep_gates;
    for (auto &inst : temp_inverse_state_prep_gates) {
      // Parametric gates:
      if (inst->name() == "Rx" || inst->name() == "Ry" ||
          inst->name() == "Rz" || inst->name() == "CPhase" ||
          inst->name() == "U1" || inst->name() == "CRZ") {
        inst->setParameter(0, -inst->getParameter(0).template as<double>());
        inverse_state_prep_gates.emplace_back(inst);
      }
      // Handles T and S gates, etc... => T -> Tdg
      else if (inst->name() == "T") {
        auto tdg = gateRegistry->createInstruction("Tdg", inst->bits()[0]);
        inverse_state_prep_gates.emplace_back(tdg);
      } else if (inst->name() == "S") {
        auto sdg = gateRegistry->createInstruction("Sdg", inst->bits()[0]);
        inverse_state_prep_gates.emplace_back(sdg);
      } else if (inst->name() == "Tdg") {
        auto t = gateRegistry->createInstruction("T", inst->bits()[0]);
        inverse_state_prep_gates.emplace_back(t);
      } else if (inst->name() == "Sdg") {
        auto s = gateRegistry->createInstruction("S", inst->bits()[0]);
        inverse_state_prep_gates.emplace_back(s);
      } else {
        inverse_state_prep_gates.emplace_back(inst);
      }
    }
    assert(inverse_state_prep_gates.size() == state_prep_gates.size());

    grover_op_circuit->addInstructions(inverse_state_prep_gates);
    grover_op_circuit->addInstructions(zero_reflection->getInstructions());
    grover_op_circuit->addInstructions(state_prep_gates);

    return grover_op_circuit;
  }();

  // Construct Grover-based amplitude amplification circuit:
  // State prep:
  addInstructions(state_prep_circuit->getInstructions());

  // Repeat (iterate) grover operator circuit:
  for (int i = 0; i < power; ++i) {
    xacc::InstructionIterator it(grover_operator_circuit);
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
  }

  return true;
}
const std::vector<std::string> AmplitudeAmplification::requiredKeys() {
  // Only the 'oracle' circuit is required
  // (others are optional)
  return {"oracle"};
}
}

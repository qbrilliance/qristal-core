/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/ripple_adder.hpp"
#include "CommonGates.hpp"
#include "xacc_service.hpp"
#include <CompositeInstruction.hpp>
#include <assert.h>
#include "IRProvider.hpp"

namespace qbOS {
bool RippleCarryAdder::expand(const xacc::HeterogeneousMap &runtimeOptions) {
  // Add reg1 to reg2 and save to reg2
  // c_in and c_out are carry bits (in and out)
  // We allow for fully customizable register partition.
  std::vector<int> reg1_indices;
  std::vector<int> reg2_indices;
  // Optional: we may ignore carry bit.
  int c_in_idx = -1;
  int c_out_idx = -1;

  if (!runtimeOptions.keyExists<std::vector<int>>("adder_bits")) {
    return false;
  }
  reg1_indices = runtimeOptions.get<std::vector<int>>("adder_bits");

  if (!runtimeOptions.keyExists<std::vector<int>>("sum_bits")) {
    return false;
  }
  reg2_indices = runtimeOptions.get<std::vector<int>>("sum_bits");

  if (reg2_indices.size() < reg1_indices.size()) {
    return false;
  }

  // The two set of indices must be unique and disjoint
  std::set<int> s1(reg1_indices.begin(), reg1_indices.end());
  std::set<int> s2(reg2_indices.begin(), reg2_indices.end());
  if (s1.size() != reg1_indices.size() || s2.size() != reg2_indices.size()) {
    return false;
  }
  for (const auto &bit : reg1_indices) {
    if (std::find(reg2_indices.begin(), reg2_indices.end(), bit) !=
        reg2_indices.end()) {
      return false;
    }
  }

  c_out_idx = reg2_indices[reg1_indices.size()];

  if (!runtimeOptions.keyExists<int>("c_in")) {
    return false;
  }
  c_in_idx = runtimeOptions.get<int>("c_in");
  if (std::find(reg1_indices.begin(), reg1_indices.end(), c_in_idx) !=
          reg1_indices.end() ||
      std::find(reg2_indices.begin(), reg2_indices.end(), c_in_idx) !=
          reg2_indices.end()) {
    return false;
  }

  bool no_overflow = false;
  if (runtimeOptions.keyExists<bool>("no_overflow")) {
      no_overflow = runtimeOptions.get<bool>("no_overflow");
  }

  if (!no_overflow) {
    if (reg2_indices.size() <= reg1_indices.size()) {
      std::cout << "The result register must have more qubits than the operand "
                   "register for carry-over bit.\n";
      return false;
    }
  }

  // Everything is okay, now do the addition
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  // Helper functions:
  const auto majority = [&gateRegistry](
                            size_t a, size_t b, size_t c,
                            std::shared_ptr<xacc::CompositeInstruction> &comp) {
    comp->addInstruction(gateRegistry->createInstruction("CX", {c, b}));
    comp->addInstruction(gateRegistry->createInstruction("CX", {c, a}));
    auto ccx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    auto x_gate = gateRegistry->createComposite("x_gate");
    x_gate->addInstruction(gateRegistry->createInstruction("X", c));
    std::vector<int> controlled_bits{static_cast<int>(b), static_cast<int>(a)};
    ccx->expand({{"U", x_gate}, {"control-idx", controlled_bits}});
    comp->addInstruction(ccx);
  };

  const auto unmajority = [&gateRegistry](
                              size_t a, size_t b, size_t c,
                              std::shared_ptr<xacc::CompositeInstruction>
                                  &comp) {
    auto ccx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    auto x_gate = gateRegistry->createComposite("x_gate");
    x_gate->addInstruction(gateRegistry->createInstruction("X", c));
    std::vector<int> controlled_bits{static_cast<int>(b), static_cast<int>(a)};
    ccx->expand({{"U", x_gate}, {"control-idx", controlled_bits}});
    comp->addInstruction(ccx);
    comp->addInstruction(gateRegistry->createInstruction("CX", {c, a}));
    comp->addInstruction(gateRegistry->createInstruction("CX", {a, b}));
  };


  auto adder_circuit = gateRegistry->createComposite("QB_ADDER_COMP");
  auto& b = reg2_indices;
  auto& a = reg1_indices;
  majority(c_in_idx, b[0], a[0], adder_circuit);

  for (size_t j = 0; j < a.size() - 1; ++j) {
    majority(a[j], b[j + 1], a[j + 1], adder_circuit);
  }

  if (!no_overflow) {
  adder_circuit->addInstruction(gateRegistry->createInstruction(
      "CX",
      {static_cast<size_t>(a[a.size() - 1]), static_cast<size_t>(c_out_idx)}));
  }

  for (int j = a.size() - 2; j >= 0; --j) {
    unmajority(a[j], b[j + 1], a[j + 1], adder_circuit);
  }

  unmajority(c_in_idx, b[0], a[0], adder_circuit);

  addInstructions(adder_circuit->getInstructions());
  return true;
}

const std::vector<std::string> RippleCarryAdder::requiredKeys() { return {}; }
} // namespace qbOS

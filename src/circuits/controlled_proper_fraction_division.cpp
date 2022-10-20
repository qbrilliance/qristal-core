/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/controlled_proper_fraction_division.hpp"
#include "qb/core/circuits/subtraction.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <CompositeInstruction.hpp>
#include <assert.h>
#include <memory>
#include <string>

namespace qbOS {
bool ControlledProperFractionDivision::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  std::vector<int> denominator;
  std::vector<int> numerator;

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_denominator")) {
    return false;
  }
  denominator = runtimeOptions.get<std::vector<int>>("qubits_denominator");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_numerator")) {
    return false;
  }
  numerator = runtimeOptions.get<std::vector<int>>("qubits_numerator");

  if (numerator.size() != denominator.size()) {
    std::cout << "The two registers qubits_numerator and qubits_denominator must "
                 "contain the same number of qubits.\n";
    return false;
  }

  // The two set of indices must be unique and disjoint
  std::set<int> s1(denominator.begin(), denominator.end());
  std::set<int> s2(numerator.begin(), numerator.end());
  if (s1.size() != denominator.size() || s2.size() != numerator.size()) {
    std::cout << "qubits provided in qubits_numerator and qubits_denominator must be "
                 "unique and disjoint.\n";
    return false;
  }
  for (const auto &bit : denominator) {
    if (std::find(numerator.begin(), numerator.end(), bit) != numerator.end()) {
      std::cout << "qubits provided in qubits_numerator and qubits_denominator must "
                   "be unique and disjoint.\n";
      return false;
    }
  }

  std::vector<int> result;
  std::vector<int> ancilla;

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_fraction")) {
    return false;
  }
  result = runtimeOptions.get<std::vector<int>>("qubits_fraction");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla")) {
    return false;
  }
  ancilla = runtimeOptions.get<std::vector<int>>("qubits_ancilla");

  bool is_LSB = true;
  if (runtimeOptions.keyExists<bool>("is_LSB")) {
    is_LSB = runtimeOptions.get<bool>("is_LSB");
  }

  if (!is_LSB) {
    std::reverse(numerator.begin(), numerator.end());
    std::reverse(denominator.begin(), denominator.end());
  }

  std::vector<int> controls_on;
  std::vector<int> controls_off;
  if (runtimeOptions.keyExists<std::vector<int>>("controls_on")) {
  controls_on = runtimeOptions.get<std::vector<int>>("controls_on");
  }
  if (runtimeOptions.keyExists<std::vector<int>>("controls_off")) {
  controls_off = runtimeOptions.get<std::vector<int>>("controls_off");
  }

  int precision = result.size();
  int bitlength = numerator.size();

  std::vector<int> numreg = numerator;
  for (int i = 0; i < precision; i++) {
      numreg.push_back(ancilla[i]);
  }
  std::vector<int> denomreg = denominator;
  for (int i = 0; i < precision; i++) {
      denomreg.push_back(ancilla[precision+i]);
  }
  int comp_ancilla = ancilla[2*precision];

  assert(numreg.size() == denomreg.size());

  // Ready to go. Now perform the division

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circ = gateRegistry->createComposite("circ");

  for (auto bit : controls_off) {
      addInstruction(gateRegistry->createInstruction("X", bit));
  }

  for (int i = 1; i <= precision; i++) {

      // Multiply the numerator by 2^i
      std::vector<int> temp_numreg;
      std::vector<int> temp1;
      std::vector<int> temp2;
      for (int k = numreg.size()-i; k < numreg.size(); k++) {
          temp1.push_back(numreg[k]);
      }
      for (int k = 0; k < numreg.size()-i; k++) {
          temp2.push_back(numreg[k]);
      }
      for (auto bit : temp1) {
          temp_numreg.push_back(bit);
      }
      for (auto bit: temp2) {
          temp_numreg.push_back(bit);
      }

      // Turn on the ith result bit iff numreg_copy > denomreg
      auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("CompareGT"));
      int tgt;
      if (is_LSB) {
          tgt = result[result.size()-i];
      } else {
          tgt = result[i-1];
      }
      circ->addInstruction(gateRegistry->createInstruction("X", tgt));
      const bool expand_ok_comp = comp->expand({{"qubits_a", denomreg},
                    {"qubits_b", temp_numreg},
                    {"qubit_flag", tgt},
                    {"qubit_ancilla", comp_ancilla},
                    {"is_LSB", true}});
    assert(expand_ok_comp);
    circ->addInstruction(comp);

    // Conditionally update registers
    int ctrl = tgt;
    for (int k = 0; k < i; k++) {
    for (int j = numreg.size()-2; j >= 0; j--) {
      auto swap = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("ControlledSwap"));
      const bool expand_ok_swap = swap->expand({{"qubits_a", std::vector<int> {numreg[j]}},
                                                {"qubits_b", std::vector<int> {numreg[j+1]}},
                                                {"flags_on", std::vector<int> {ctrl}}});
      assert(expand_ok_swap);
      circ->addInstruction(swap);
    }
    }
    auto subtraction = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("ControlledSubtraction"));
    const bool expand_ok_subtraction = subtraction->expand({{"qubits_larger", numreg},
                                                            {"qubits_smaller", denomreg},
                                                            {"qubit_ancilla", comp_ancilla},
                                                            {"controls_on", std::vector<int> {ctrl}}});
    assert(expand_ok_subtraction);
    circ->addInstruction(subtraction);

    for (int k = 0; k < i; k++) {
    for (int j = denomreg.size()-2; j >= 0; j--) {
      auto swap = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("ControlledSwap"));
      const bool expand_ok_swap = swap->expand({{"qubits_a", std::vector<int> {denomreg[j]}},
                                                {"qubits_b", std::vector<int> {denomreg[j+1]}},
                                                {"flags_on", std::vector<int> {ctrl}}});
      assert(expand_ok_swap);
      circ->addInstruction(swap);
    }
    }
  }

  // Uncompute numerator and denominator registers
  for (int i = precision; i >= 1; i--) {
    int ctrl;
    if (is_LSB) {
      ctrl = result[result.size() - i];
    } else {
      ctrl = result[i - 1];
    }

    for (int k = 0; k < i; k++) {
    for (int j = 1; j < denomreg.size(); j++) {
      auto swap = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("ControlledSwap"));
      const bool expand_ok_swap =
          swap->expand({{"qubits_a", std::vector<int> {denomreg[j-1]}},
                        {"qubits_b", std::vector<int> {denomreg[j]}},
                        {"flags_on", std::vector<int>{ctrl}}});
      assert(expand_ok_swap);
      circ->addInstruction(swap);
    }
    }


    auto subtraction = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("ControlledSubtraction"));
    const bool expand_ok_sub = subtraction->expand({{"qubits_larger", numreg},
                                                    {"qubits_smaller", denomreg},
                                                    {"qubit_ancilla", comp_ancilla},
                                                    {"controls_on", std::vector<int> {ctrl}}});
    assert(expand_ok_sub);
    auto inv_sub = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("InverseCircuit"));
    const bool expand_ok_inv_sub = inv_sub->expand({{"circ", subtraction}});
    assert(expand_ok_inv_sub);
    circ->addInstruction(inv_sub);

    for (int k = 0; k < i; k++) {
    for (int j = 1; j < numreg.size(); j++) {
      auto swap = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("ControlledSwap"));
      const bool expand_ok_swap =
          swap->expand({{"qubits_a", std::vector<int> {numreg[j-1]}},
                        {"qubits_b", std::vector<int> {numreg[j]}},
                        {"flags_on", std::vector<int>{ctrl}}});
      assert(expand_ok_swap);
      circ->addInstruction(swap);
    }
    }
  }

  if (controls_on.size() > 0 || controls_off.size() > 0) {
      std::vector<int> controls = controls_on;
      for (auto bit : controls_off) {
          controls.push_back(bit);
      }
      auto c_circ = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      const bool expand_ok = c_circ->expand({{"U", circ}, {"control-idx", controls}});
      assert(expand_ok);
      addInstruction(c_circ);
  } else {
      addInstruction(circ);
  }

  for (auto bit : controls_off) {
      addInstruction(gateRegistry->createInstruction("X", bit));
  }

  if (!is_LSB) {
    std::reverse(numerator.begin(), numerator.end());
    std::reverse(denominator.begin(), denominator.end());
  }

  return true;
}

const std::vector<std::string> ControlledProperFractionDivision::requiredKeys() {
  return {"qubits_numerator", "qubits_denominator", "qubits_fraction", "qubits_ancilla"};
}
} // namespace qbOS

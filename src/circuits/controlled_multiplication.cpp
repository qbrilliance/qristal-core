/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include <qristal/core/circuits/controlled_multiplication.hpp>
#include <CommonGates.hpp>
#include <IRProvider.hpp>
#include <xacc_service.hpp>
#include <CompositeInstruction.hpp>
#include <algorithm>
#include <assert.h>
#include <memory>

namespace qristal {
bool ControlledMultiplication::expand(
    const xacc::HeterogeneousMap &runtimeOptions) {
  ////////////////////////////////////////////////////////
  // Get expected inputs or define any required variables
  ////////////////////////////////////////////////////////
  std::vector<int> qubits_a;
  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_a")) {
    return false;
  }
  qubits_a = runtimeOptions.get<std::vector<int>>("qubits_a");

  std::vector<int> qubits_b;
  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_b")) {
    return false;
  }
  qubits_b = runtimeOptions.get<std::vector<int>>("qubits_b");

  std::vector<int> qubits_result;
  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_result")) {
    return false;
  }
  qubits_result = runtimeOptions.get<std::vector<int>>("qubits_result");

  int qubit_ancilla;
  if (!runtimeOptions.keyExists<int>("qubit_ancilla")) {
    return false;
  }
  qubit_ancilla = runtimeOptions.get<int>("qubit_ancilla");

  bool is_LSB = true;
  if (runtimeOptions.keyExists<bool>("is_LSB")) {
    is_LSB = runtimeOptions.get<bool>("is_LSB");
  }

  std::vector<int> controls_on;
  if (!runtimeOptions.keyExists<std::vector<int>>("controls_on")) {
    return false;
  }
  controls_on = runtimeOptions.get<std::vector<int>>("controls_on");

  std::vector<int> controls_off;
  if (!runtimeOptions.keyExists<std::vector<int>>("controls_off")) {
    return false;
  }
  controls_off = runtimeOptions.get<std::vector<int>>("controls_off");

  assert(qubits_a.size() == qubits_b.size());
  assert(qubits_result.size() == 2 * qubits_a.size());

  if (!is_LSB) {
    reverse(qubits_a.begin(), qubits_a.end());
    reverse(qubits_b.begin(), qubits_b.end());
    reverse(qubits_result.begin(), qubits_result.end());
  }

  ////////////////////////////////////////////////////////
  // Add instructions
  ////////////////////////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  for (auto bit : controls_off) {
    addInstruction(gateRegistry->createInstruction("X", bit));
  }

  auto circ = gateRegistry->createComposite("circ");

  for (int i = 0; i < qubits_b.size(); i++) {
    // Indices targeted by the addition
    std::vector<int> qubits_result_target;
    for (int j = i; j <= i + qubits_a.size(); j++) {
      qubits_result_target.push_back(qubits_result[j]);
    }
    assert(qubits_result_target.size() == qubits_a.size() + 1);

    // Controlled addition
    auto controlled_addition_i =
        std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("ControlledAddition"));
    xacc::HeterogeneousMap options{{"qubits_adder", qubits_a},
                                   {"qubits_sum", qubits_result_target},
                                   {"flags_on", std::vector<int>{qubits_b[i]}},
                                   {"c_in", qubit_ancilla}};
    const bool expand_ok = controlled_addition_i->expand(options);
    assert(expand_ok);
    circ->addInstructions(controlled_addition_i->getInstructions());
  }

  if (controls_on.size() > 0 || controls_off.size() > 0) {
    auto c_circ = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    std::vector<int> control_bits = controls_on;
    for (auto bit : controls_off) {
      control_bits.push_back(bit);
    }
    const bool expand_ok_c_circ =
        c_circ->expand({{"U", circ}, {"control-idx", control_bits}});
    assert(expand_ok_c_circ);
    addInstruction(c_circ);
  } else {
    addInstruction(circ);
  }

  for (auto bit : controls_off) {
    addInstruction(gateRegistry->createInstruction("X", bit));
  }

  if (!is_LSB) {
    reverse(qubits_a.begin(), qubits_a.end());
    reverse(qubits_b.begin(), qubits_b.end());
    reverse(qubits_result.begin(), qubits_result.end());
  }

  return true;
}

const std::vector<std::string> ControlledMultiplication::requiredKeys() {
  return {"qubits_a", "qubits_b", "qubits_result", "qubit_ancilla"};
}
}

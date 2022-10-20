/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuit_builder.hpp"
#include "qb/core/circuits/canonical_amplitude_estimation.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include "xacc.hpp"
#include <assert.h>
#include "xacc.hpp"
#include "Circuit.hpp"
#include "GateModifier.hpp"

namespace qbOS {
bool CanonicalAmplitudeEstimation::expand(const xacc::HeterogeneousMap &runtimeOptions) {
  // Inputs:
  // - num_evaluation_qubits: number of evaluation qubits (control the
  // precisison) More evaluation qubits produce a finer sampling grid, therefore
  // better accuracy.
  // - num_trial_qubits: number of qubits acted on by Q
  // - num_state_qubits: number of qubits acted on by A

  // - state_preparation_circuit: the "A" circuit to prepare the state:
  // i.e. \mathcal{A}|0\rangle = \sqrt{1 - a}|\Psi_0\rangle + \sqrt{a}|\Psi_1\rangle
  // - oracle: the "Sf" circuit to flip the sign of the good states in amplitude amplification
  // If not provided, must provide grover_op_circuit
  // - grover_op_circuit: The unitary circuit which will be repeated and
  // controlled. i.e. Q operator in Brassard et al. (2000) (see eq. 1).
  // If not provided, must provide oracle

  // state_qubits: the indices of the qubits that are acted on by state_prep (optional)
  // evaluation_qubits: the indices of the evaluation_qubits (optional)
  // If qubit registers are not provided the assumed ordering is: evaluation qubits, trial_qubits
  if (!runtimeOptions.keyExists<int>("num_evaluation_qubits")) {
    return false;
  }
  const auto num_evaluation_qubits =
      runtimeOptions.get<int>("num_evaluation_qubits");

   if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
          "state_preparation_circuit")) {
    return false;
  }
  auto A_circ = xacc::as_shared_ptr(
      runtimeOptions.getPointerLike<xacc::CompositeInstruction>(
          "state_preparation_circuit"));


  if (!runtimeOptions.keyExists<int>("num_trial_qubits")) {
      return false;
  }
  int num_trial_qubits = runtimeOptions.get<int>("num_trial_qubits");

  std::vector<int> reflection_qubits;
  if (!runtimeOptions.keyExists<std::vector<int>>("reflection_qubits")) {
      auto sp_qubits_set = qbOS::uniqueBitsQD(A_circ);
      std::vector<int> sp_qubits;
      for (int bit : sp_qubits_set) {
        sp_qubits.push_back(bit);
    }
    reflection_qubits = sp_qubits;
  } else {
    reflection_qubits = runtimeOptions.get<std::vector<int>>("reflection_qubits");
  }

  bool no_state_prep = false;
  if (runtimeOptions.keyExists<bool>("no_state_prep")) { no_state_prep = runtimeOptions.get<bool>("no_state_prep"); }

  std::vector<int> evaluation_qubits = {};
  if (runtimeOptions.keyExists<std::vector<int>>("evaluation_qubits")) {
      evaluation_qubits = runtimeOptions.get<std::vector<int>>("evaluation_qubits");
  }
  if (evaluation_qubits.size() == 0) {
      for (int i = 0; i < num_evaluation_qubits; i++) {
          evaluation_qubits.push_back(i);
      }
  }
  std::vector<int> trial_qubits = {};
  if (runtimeOptions.keyExists<std::vector<int>>("trial_qubits")) {
      trial_qubits = runtimeOptions.get<std::vector<int>>("trial_qubits");
  }
  if (trial_qubits.size() == 0) {
      for (int i = 0; i < num_trial_qubits; i++) {
          trial_qubits.push_back(num_evaluation_qubits + i);
      }
  }

  if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
          "grover_op_circuit") &&
      !runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("oracle")) {
    std::cout << "Either 'oracle' circuit or the 'grover_op_circuit' must be "
                 "provided.\n";
    return false;
  }
  if (runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
          "evaluation_qubits") &&
      !runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("trial_qubits") || !runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
          "evaluation_qubits") &&
      runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("trial_qubits"))  {
  std::cout << "Both 'evaluation_qubits' and 'trial_qubits' must be provided. \n";
  return false;
 }

  // Grover operator circuit: customized (provided) or constructed from oracle
  // and state prep
  auto [Q_circ, global_phase] =
      [&]() -> std::pair<std::shared_ptr<xacc::CompositeInstruction>, double> {
    if (runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
            "grover_op_circuit")) {
      return std::make_pair(
          xacc::as_shared_ptr(
              runtimeOptions.getPointerLike<xacc::CompositeInstruction>(
                  "grover_op_circuit")),
          0.0);
    }
    assert(
        runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("oracle"));
    auto oracle_circuit =
        runtimeOptions.getPointerLike<xacc::CompositeInstruction>("oracle");
    auto &state_prep_circuit = A_circ;

    // Construct Grover operator circuit as:
    // Q = A S0 Adagger Sf
    // S0 => zero reflection circuit
    // Sf => phase oracle (multiplies the good states by -1)
    // A => state-prep
    auto sp_qubits_set = qbOS::uniqueBitsQD(state_prep_circuit);
    std::vector<int> sp_qubits;
    for (int bit : sp_qubits_set) {
        sp_qubits.push_back(bit);
    }
    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    auto zero_reflection = gateRegistry->createComposite("zero_reflection");
    for (int i = 0; i < reflection_qubits.size(); ++i) {
      zero_reflection->addInstruction(gateRegistry->createInstruction("X", reflection_qubits[i]));
    }

    if (reflection_qubits.size() == 1) {
      zero_reflection->addInstruction(gateRegistry->createInstruction("Z", reflection_qubits[0]));
    } else {
      zero_reflection->addInstruction(
          gateRegistry->createInstruction("H", reflection_qubits[0]));

      // Multi-controlled X:
      std::vector<int> controlled_bits;
      for (int i = 1; i < reflection_qubits.size(); ++i) {
        controlled_bits.emplace_back(reflection_qubits[i]);
      }
      auto x_gate = gateRegistry->createComposite("x_gate");
      auto temp_gate = gateRegistry->createInstruction("X", reflection_qubits[0]);
      temp_gate->setBufferNames({"q"});
      x_gate->addInstruction(temp_gate);
      auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      mcx->expand({{"U", x_gate}, {"control-idx", controlled_bits}});
      zero_reflection->addInstruction(mcx);
      //=================================

      zero_reflection->addInstruction(
          gateRegistry->createInstruction("H", reflection_qubits[0]));
    }
    for (int i = 0; i < reflection_qubits.size(); ++i) {
      zero_reflection->addInstruction(gateRegistry->createInstruction("X", reflection_qubits[i]));
    }
    // Construct the Grover operator circuit:
    auto grover_op_circuit = gateRegistry->createComposite("grover_op");
    // Q = A S0 Adagger Sf
    // Note: appending circuit from right to left
    assert(oracle_circuit);
    assert(state_prep_circuit);
    auto state_prep_circuit_inverse = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("InverseCircuit"));
    state_prep_circuit_inverse->expand({{"circ", state_prep_circuit}});
    assert(state_prep_circuit_inverse);

    grover_op_circuit->addInstruction(xacc::as_shared_ptr(oracle_circuit));
    grover_op_circuit->addInstruction(state_prep_circuit_inverse);
    grover_op_circuit->addInstruction(zero_reflection);
    grover_op_circuit->addInstruction(state_prep_circuit);
    return std::make_pair(grover_op_circuit, M_PI);
  }();

  assert(Q_circ->nInstructions() > 0);
  assert(A_circ->nInstructions() > 0);
  assert(num_evaluation_qubits > 0);
  // Add state preparation:
  if (!no_state_prep) {
    addInstructions(A_circ->getInstructions());
  }
  // Add phase estimation:
  auto qpe = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("PhaseEstimation"));
  xacc::HeterogeneousMap options{
      {"unitary", Q_circ}, {"num_evaluation_qubits", num_evaluation_qubits}, {"trial_qubits", trial_qubits}, {"evaluation_qubits", evaluation_qubits}};
  if (std::abs(global_phase) > 1e-12) {
    options.insert("global-phase", global_phase);
  }
  const bool expand_ok = qpe->expand(options);
  assert(expand_ok);
  assert(!qpe->getInstructions().empty());
  addInstruction(qpe);
  return true;
}

const std::vector<std::string> CanonicalAmplitudeEstimation::requiredKeys() {
  return {"num_evaluation_qubits", "num_state_qubits", "num_trial_qubits", "state_preparation_circuit",
          "grover_op_circuit"};
}
} // namespace qbOS

/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include <qristal/core/circuits/compare_beam_oracle.hpp>
#include <CommonGates.hpp>
#include <IRProvider.hpp>
#include <xacc_service.hpp>
#include <CompositeInstruction.hpp>
#include <assert.h>
#include <vector>

namespace qristal {
bool CompareBeamOracle::expand(const xacc::HeterogeneousMap &runtimeOptions) {
  // Inputs:
  if (!runtimeOptions.keyExists<int>("q0")) {
    return false;
  }
  int q0 = runtimeOptions.get<int>("q0");

  if (!runtimeOptions.keyExists<int>("q1")) {
    return false;
  }
  int q1 = runtimeOptions.get<int>("q1");

  if (!runtimeOptions.keyExists<int>("q2")) {
    return false;
  }
  int q2 = runtimeOptions.get<int>("q2");

  if (!runtimeOptions.keyExists<std::vector<int>>("FA")) {
    return false;
  }
  std::vector<int> FA = runtimeOptions.get<std::vector<int>>("FA");

  if (!runtimeOptions.keyExists<std::vector<int>>("FB")) {
    return false;
  }
  std::vector<int> FB = runtimeOptions.get<std::vector<int>>("FB");

  if (!runtimeOptions.keyExists<std::vector<int>>("SA")) {
    return false;
  }
  std::vector<int> SA = runtimeOptions.get<std::vector<int>>("SA");

  bool simplified = true;
  if (runtimeOptions.keyExists<bool>("simplified")) {
    simplified = runtimeOptions.get<bool>("simplified");
  }

  std::vector<int> SB;
  if (!runtimeOptions.keyExists<std::vector<int>>("SB") && !simplified) {
    return false;
  }
  if (!simplified) {
    SB = runtimeOptions.get<std::vector<int>>("SB");
  }

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  // Add FA to FB using CNOT gates. If a_i == b_i, then a_i + b_i = 0.
  for (int i = 0; i < FB.size(); i++) {
    addInstruction(gateRegistry->createInstruction(
        "CX", std::vector<std::size_t>{static_cast<unsigned long>(FA[i]),
                                       static_cast<unsigned long>(FB[i])}));
  }

  // We want to check that all FB qubits are zero.
  auto mcx1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx1->expand({{"controls_off", FB}, {"target", q1}});
  addInstructions(mcx1->getInstructions());

  // Undo all the CX operations with FA.
  for (int i = 0; i < FB.size(); i++) {
    addInstruction(gateRegistry->createInstruction(
        "CX", std::vector<std::size_t>{static_cast<unsigned long>(FA[i]),
                                       static_cast<unsigned long>(FB[i])}));
  }

  int qubits_per_letter = (int)SA.size() / (int)FA.size();

  if (simplified) {
    // If the qubit in FA for a symbol in SA is |0>, perform an MCX operation
    // with the symbol’s qubits in SA as the controls and the corresponding
    // qubit in FB as the target. Therefore if the symbol in SA is
    //|111...1>, an X gate is applied to the qubit in FB.
    for (int i = 0; i < FB.size(); i++) {
      auto mcx2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("GeneralisedMCX"));
      std::vector<int> string_vector;
      int start = i * qubits_per_letter;
      int end = (i + 1) * qubits_per_letter;
      for (int j = start; j < end; j++) {
        string_vector.push_back(SA[j]);
      }
      std::vector<int> off;
      off.push_back(FA[i]);
      mcx2->expand({{"target", FB[i]},
                    {"controls_on", string_vector},
                    {"controls_off", off}});
      addInstructions(mcx2->getInstructions());
    }
  } else {
    // If the qubit in FA for a symbol in SA is |0>, compare
    // the symbol in SA to the corresponding symbol in SB,
    // storing the result on the appropriate flag in FB
    for (int i = 0; i < FA.size(); i++) {
      std::vector<int> string_vectorA;
      std::vector<int> string_vectorB;
      int start = i * qubits_per_letter;
      int end = (i + 1) * qubits_per_letter;
      for (int j = start; j < end; j++) {
        string_vectorA.push_back(SA[j]);
        string_vectorB.push_back(SB[j]);
      }

      // Equality checker
      auto eq = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap options_eq{
          {"qubits_a", string_vectorA},
          {"qubits_b", string_vectorB},
          {"controls_off", std::vector<int>{FA[i]}},
          {"flag", FB[i]}};
      const bool expand_ok_eq = eq->expand(options_eq);
      assert(expand_ok_eq);
      addInstructions(eq->getInstructions());
    }
  }

  // Switch on q2 if all qubits in FB are on
  auto mcx3 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("GeneralisedMCX"));
  mcx3->expand({{"controls_on", FB}, {"target", q2}});
  addInstructions(mcx3->getInstructions());

  // Undo previous operations of FA and FB needed to switch on q2
  if (simplified) {
    for (int i = 0; i < FB.size(); i++) {
      auto mcx2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("GeneralisedMCX"));
      std::vector<int> string_vector;
      int start = i * qubits_per_letter;
      int end = (i + 1) * qubits_per_letter;
      for (int j = start; j < end; j++) {
        string_vector.push_back(SA[j]);
      }
      mcx2->expand({{"target", FB[i]},
                    {"controls_on", string_vector},
                    {"controls_off", std::vector<int> {FA[i]}}});
      addInstructions(mcx2->getInstructions());
    }
  } else {
    for (int i = 0; i < FA.size(); i++) {
      std::vector<int> string_vectorA;
      std::vector<int> string_vectorB;
      int start = i * qubits_per_letter;
      int end = (i + 1) * qubits_per_letter;
      for (int j = start; j < end; j++) {
        string_vectorA.push_back(SA[j]);
        string_vectorB.push_back(SB[j]);
      }

      // Equality checker
      auto eq = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap options_eq{
          {"qubits_a", string_vectorA},
          {"qubits_b", string_vectorB},
          {"controls_off", std::vector<int>{FA[i]}},
          {"flag", FB[i]}};
      const bool expand_ok_eq = eq->expand(options_eq);
      assert(expand_ok_eq);
      addInstructions(eq->getInstructions());
    }
  }

  // If q1 and q2 are on, turn on q0 using a Toffoli gate
  auto toffoli = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("C-U"));
  auto x3 = gateRegistry->createComposite("u3");
  x3->addInstruction(gateRegistry->createInstruction("X", q0));
  std::vector<int> control_q1_q2;
  control_q1_q2.emplace_back(q1);
  control_q1_q2.emplace_back(q2);
  toffoli->expand({{"U", x3}, {"control-idx", control_q1_q2}});
  addInstruction(toffoli);

  return true;
}

const std::vector<std::string> CompareBeamOracle::requiredKeys() {
  return {"q0", "q1", "q2", "FA", "FB", "SA"};
}
}

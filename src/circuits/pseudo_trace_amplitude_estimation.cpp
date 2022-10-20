#include "qb/core/circuits/pseudo_trace_amplitude_estimation.hpp"
#include "xacc.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <assert.h>

namespace qbOS {
bool PseudoTraceAmplitudeEstimation::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  ////////////////////////////////////////////////////////
  // Get expected inputs or define any required variables
  ////////////////////////////////////////////////////////

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_metric")) {
      return false;
  }
  std::vector<int> qubits_metric = runtimeOptions.get<std::vector<int>>("qubits_metric");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_to_be_traced")) {
      return false;
  }
  std::vector<int> qubits_to_be_traced = runtimeOptions.get<std::vector<int>>("qubits_to_be_traced");

  if (!runtimeOptions.keyExists<std::vector<int>>("evaluation_bits")) {
      return false;
  }
  std::vector<int> evaluation_bits = runtimeOptions.get<std::vector<int>>("evaluation_bits");

  if (!runtimeOptions.keyExists<std::vector<int>>("num_precision_bits")) {
      return false;
  }
  std::vector<int> num_precision_bits = runtimeOptions.get<std::vector<int>>("num_precision_bits");

  if (!runtimeOptions.keyExists<std::vector<int>>("ae_state_qubits")) {
      return false;
  }
  std::vector<int> ae_state_qubits = runtimeOptions.get<std::vector<int>>("ae_state_qubits");

  if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("ae_state_prep_circ")) {
    return false;
  }
  auto ae_state_prep_circ =
      runtimeOptions.getPointerLike<xacc::CompositeInstruction>("ae_state_prep_circ");


  ////////////////////////////////////////////////////////
  // Add instructions
  ////////////////////////////////////////////////////////
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  // Apply a Hadamard to all qubits to be pseudo-traced over
  for (auto bit : qubits_to_be_traced) {
      addInstruction(gateRegistry->createInstruction("H", bit));
      if (std::find(ae_state_qubits.begin(), ae_state_qubits.end(), bit) !=
          ae_state_qubits.end()) {
        ae_state_prep_circ->addInstruction(
            gateRegistry->createInstruction("H", bit));
      }
  }

  // Loop through each metric qubit
  for (int q = 0; q < qubits_metric.size(); q++) {

      auto current_bit = qubits_metric[q];

      // Apply a Hadamard to all metric qubits except for the current one
      std::vector<size_t> remove_later;
      for (auto bit : qubits_metric) {
          if (bit != current_bit) {
            addInstruction(gateRegistry->createInstruction("H", bit));
            if (std::find(ae_state_qubits.begin(), ae_state_qubits.end(),
                          bit) != ae_state_qubits.end()) {
              remove_later.push_back(ae_state_prep_circ->nInstructions());
              ae_state_prep_circ->addInstruction(
                  gateRegistry->createInstruction("H", bit));
            }
          }
      }

      // Apply amplitude estimation to the current qubit and store the result on the evaluation qubits
      int precision_bits_so_far = 0;
      for (int i = 0; i < q; i++) {
          precision_bits_so_far += num_precision_bits[i];
      }
      std::vector<int> current_evaluation_bits;
      for (int i = precision_bits_so_far; i < precision_bits_so_far + num_precision_bits[q]; i++) {
          current_evaluation_bits.push_back(evaluation_bits[i]);
      }
      assert(current_evaluation_bits.size() == num_precision_bits[q]);

      std::shared_ptr<xacc::CompositeInstruction> ae_state_prep_circ_clone =
          xacc::ir::asComposite(ae_state_prep_circ->clone());

      auto oracle = gateRegistry->createComposite("oracle");
      //Use the Z gate as the oracle to detect 1's in the metric register, i.e. the marked state is |1>.
      oracle->addInstruction(gateRegistry->createInstruction("Z", current_bit));

      for (auto bit : qubits_metric) {
          assert(std::find(ae_state_qubits.begin(), ae_state_qubits.end(), bit) != ae_state_qubits.end());
      }

      auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
      xacc::HeterogeneousMap options_ae{
          {"state_preparation_circuit", ae_state_prep_circ_clone},
          {"no_state_prep", true},
          {"oracle", oracle},
          {"reflection_qubits", qubits_metric},
          {"evaluation_qubits", current_evaluation_bits},
          {"num_evaluation_qubits", num_precision_bits[q]},
          {"trial_qubits", ae_state_qubits},
          {"num_trial_qubits", (int)ae_state_qubits.size()}};
      const bool expand_ok_ae = ae->expand(options_ae);
      assert(expand_ok_ae);
      addInstructions(ae->getInstructions());

      // Undo the Hadamard to all metric qubits except for the current one
      std::reverse(remove_later.begin(), remove_later.end());
      for (auto idx : remove_later) {
        ae_state_prep_circ->removeInstruction(idx);
      }
      for (auto bit : qubits_metric) {
          if (bit != current_bit) {
            addInstruction(gateRegistry->createInstruction("H", bit));
          }
      }
  }

    // Undo the Hadamard to all qubits to be pseudo-traced over
  for (auto bit : qubits_to_be_traced) {
      addInstruction(gateRegistry->createInstruction("H", bit));
  }


  return true;

}

const std::vector<std::string> PseudoTraceAmplitudeEstimation::requiredKeys() {
  return {};
}

} // namespace

#pragma once
// Copyright (c) Quantum Brilliance Pty Ltd
#include "Algorithm.hpp"
#include "IRProvider.hpp"
#include "InstructionIterator.hpp"
#include "xacc.hpp"
#include "xacc_plugin.hpp"
#include "xacc_service.hpp"
#include "qristal/core/circuit_builder.hpp"
#include <assert.h>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <random>
#include <sstream>
#include <vector>

namespace qristal {
class ExponentialSearch : public xacc::Algorithm {
private:
  xacc::CompositeInstruction *state_prep_circuit_;

  // Note: the oracle circuit requires input parameters: BestScore,
  // num_scoring_qubits optional parameters: trial_score_qubits, flag_qubit,
  // best_score_qubits, ancilla_qubits
  //
  std::function<std::shared_ptr<xacc::CompositeInstruction>(int)>
      oracle_circuit_gen_;

  // Note: the state_prep circuit requires input paramters: num_string_qubits
  // optional parameters: trial_ancilla, trial_score_qubits, trial_qubits
  std::function<std::shared_ptr<xacc::CompositeInstruction>(
      std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>)>
      state_prep_circuit_gen_;

  std::function<int(std::string, int)>
      MLQAE_is_in_good_subspace;   // A function that returns 1 if the measured bit
                             // string is in the good subspace and a 0 otherwise
  int MLQAE_num_runs;              // For MLQAE, default is 4 if none provided
  int MLQAE_num_shots;       // For MLQAE, default is 100 if none provided
  int CQAE_num_evaluation_qubits; // For CQAE, default is 10 if none provided

  std::function<int(int)> f_score_; // Return the score for a bitstring
  xacc::Accelerator *qpu_;          // Accelerator, optional

  int best_score_; // Tracking the best score, default is 0 if none provided
  int total_num_qubits;

  // Qubit registers. These are optional but if any one of them are provided
  // then they should all be provided. Default register structure:
  // |trial_ancilla>|trial_score_qubits>|trial_qubits>|flag_qubit>|best_score_qubits>|ancilla_qubits>
  std::vector<int> qubits_metric;
  std::vector<int> qubits_string;
  std::vector<int> qubits_next_letter;
  std::vector<int> qubits_next_metric;
  std::vector<int> qubits_best_score;
  int qubit_flag;
  std::vector<int> qubits_ancilla_oracle;
  std::vector<int> qubits_ancilla_adder;
  std::vector<int> total_metric;

  // Choose which method to use. Currently supported methods are:
  // "canonical" - canonical exponential search (default)
  // "CQAE" - using canonical QAE
  // "MLQAE" - using MLQAE
  std::string method;

private:
public:
  bool initialize(const xacc::HeterogeneousMap &parameters) override;
  const std::vector<std::string> requiredParameters() const override;

  void
  execute(const std::shared_ptr<xacc::AcceleratorBuffer> buffer) const override;

  const std::string name() const override { return "exponential-search"; }
  const std::string description() const override {
    return "Quantum Exponential Search";
  }

  DEFINE_ALGORITHM_CLONE(ExponentialSearch)
};
}

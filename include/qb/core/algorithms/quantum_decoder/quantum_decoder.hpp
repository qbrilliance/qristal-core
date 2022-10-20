// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#pragma once
#include "Algorithm.hpp"
#include "IRProvider.hpp"
#include "InstructionIterator.hpp"
#include "xacc.hpp"
#include "xacc_plugin.hpp"
#include "xacc_service.hpp"
#include <assert.h>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <random>
#include <sstream>
#include <vector>

namespace qbOS {
class QuantumDecoder : public xacc::Algorithm {
private:
  xacc::CompositeInstruction *state_prep_circuit_;

  //Oracle circuit requires input parameters: BestScore
  //num_scoring_qubits optional parameters: flag_qubit,
  //qubits_best_score, qubits_ancilla_oracle
  std::function<std::shared_ptr<xacc::CompositeInstruction>(
    int, int, std::vector<int>, int, std::vector<int>, std::vector<int>)>
    oracle_circuit_gen_;

  //state_prep circuit requires input parameters: num_string_qubits
  //optional parameters: trial_qubits
  std::function<std::shared_ptr<xacc::CompositeInstruction>(
    int, std::vector<int>, std::vector<int>, std::vector<int>)>
    state_prep_circuit_gen_;

  std::function<int(int)> f_score_; //Return the score for a bitstring
  xacc::Accelerator *qpu_;          //Accelerator, optional

  int BestScore; //Tracking the best score, default is 0 if none provided

  //Qubit registers. These are optional but if any one of them are provided
  //then they should all be provided. Default register structure:
  //|trial_qubits>|flag_qubit>|qubits_best_score>|qubits_ancilla_oracla>
  std::vector<int> qubits_best_score;
  std::vector<int> qubits_ancilla_adder;
  int N_TRIALS;

  //Choose which method to use. Currently supported methods are:
  //"canonical" - canonical exponential search (default)
  //"CQAE" - using canonical QAE
  //"MLQAE" - using MLQAE
  std::string method;

  //Parameters for W prime unitary
  std::vector<std::vector<float>> probability_table;
  int iteration;

  //Qubit register for U prime and Q prime
  std::vector<int> qubits_metric;
  std::vector<int> qubits_string;

  // Qubit registers for decoder kernel
  std::vector<int> qubits_total_metric_copy;
  std::vector<int> qubits_init_null;
  std::vector<int> qubits_init_repeat;
  std::vector<int> qubits_superfluous_flags;
  std::vector<int> qubits_ancilla_pool;
  std::vector<int> qubits_beam_metric;
  std::vector<int> evaluation_bits;
  std::vector<int> precision_bits;

private:
public:
  bool initialize(const xacc::HeterogeneousMap &parameters) override;
  const std::vector<std::string> requiredParameters() const override;

  void
  execute(const std::shared_ptr<xacc::AcceleratorBuffer> buffer) const override;

  const std::string name() const override {
    return "quantum-decoder";
  }
  const std::string description() const override {
    return "Quantum Decoder";
  }

  DEFINE_ALGORITHM_CLONE(QuantumDecoder)
};
} // namespace qbOS
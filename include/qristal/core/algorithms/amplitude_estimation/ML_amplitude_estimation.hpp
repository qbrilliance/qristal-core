// Copyright (c) Quantum Brilliance Pty Ltd
#pragma once
#include "Algorithm.hpp"
#include "xacc.hpp"
#include "xacc_plugin.hpp"
#include "xacc_service.hpp"
#include <assert.h>
#include <iomanip>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace qristal {
class MLAmplitudeEstimation : public xacc::Algorithm {

  // Given a quantum state |psi> = a|good> + b|bad>, Maximum Likelihood Quantum
  // Amplitude Estimation (MLQAE) finds an estimate for the amplitude of the
  // good subspace, a. It works by performing several runs of amplitude
  // amplification with various iterations and recording the number of |good>
  // shots measured. Given this data, it finds the value of a that maximises the
  // likelihood function. See the original reference for details
  // https://arxiv.org/abs/1904.10246?msclkid=1aacb00ca8a111eca32149a417e4af21

private:
  // Required Inputs:
  xacc::CompositeInstruction *A_circuit_;      // The state preparation circuit
  xacc::CompositeInstruction *Oracle_circuit_; // The oracle circuit
  std::function<int(std::string, int)>
      is_in_good_subspace; // A function that returns 1 if the measured bit
                           // string is in the good subspace and a 0 otherwise
  std::vector<int>
      score_qubits; // The indices of the qubits that determine whether the
                    // state is in the good or bad subspace

  // Optional Inputs:
  xacc::Accelerator *qpu_; // The accelerator used to run the circuit
  int num_runs;  // The number of runs of amplitude amplification used to build
                 // the likelihood function
  int shots;     // The number of shots per run
  int bestScore; // The current best score (required for decoder application)

public:
  bool initialize(const xacc::HeterogeneousMap &parameters) override;
  const std::vector<std::string> requiredParameters() const override;
  void
  execute(const std::shared_ptr<xacc::AcceleratorBuffer> buffer) const override;

  const std::string name() const override { return "ML-ae"; }
  const std::string description() const override {
    return "Maximum likelihood amplitude estimation algorithm";
  }
  DEFINE_ALGORITHM_CLONE(MLAmplitudeEstimation)
};
}
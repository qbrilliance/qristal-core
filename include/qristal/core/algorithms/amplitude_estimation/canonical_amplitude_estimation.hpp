// Copyright (c) 2022 Quantum Brilliance Pty Ltd
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
class CanonicalAmplitudeEstimation : public xacc::Algorithm {

  // Given a quantum state |psi> = a|good> + b|bad>, the canonical amplitude
  // estimation algorithm uses QPE to estimate the amplitude of the good
  // subspace, a.

private:
  // Required Inputs:
  xacc::CompositeInstruction *A_circuit_; // State preparation circuit
  int num_evaluation_qubits;              // Number of precision qubits for QPE
  int num_trial_qubits; // Number of qubits acted on by the Grover operator Q
  int num_state_qubits; // Number of qubits acted on by the state prep circuit A

  // Optional Inputs:
  xacc::CompositeInstruction
      *Q_circuit_; // Grover operator Q. If not provided, must provide oracle
  xacc::CompositeInstruction
      *Oracle_circuit_;    // Oracle circuit. If not provided, must provide Q
  xacc::Accelerator *qpu_; // Accelerator for running the AE circuit
  std::vector<int> trial_qubits;      // The indices of qubits acted on by Q
  std::vector<int> evaluation_qubits; // The indices of the precision qubits
  int shots; // The number of shots used by the accelerator

public:
  bool initialize(const xacc::HeterogeneousMap &parameters) override;
  const std::vector<std::string> requiredParameters() const override;
  void
  execute(const std::shared_ptr<xacc::AcceleratorBuffer> buffer) const override;

  const std::string name() const override { return "canonical-ae"; }
  const std::string description() const override {
    return "Canonical amplitude estimation algorithm";
  }
  DEFINE_ALGORITHM_CLONE(CanonicalAmplitudeEstimation)
};
}
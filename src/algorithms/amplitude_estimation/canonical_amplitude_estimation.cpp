// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/algorithms/amplitude_estimation/canonical_amplitude_estimation.hpp"

namespace qristal {

// Define a function to assist with post-processing
std::pair<std::unordered_map<std::string, double>,
          std::unordered_map<int, double>>
process_count_results(const std::map<std::string, int> &in_counts, bool in_msb,
                      int in_nbEvalBits) {
  std::unordered_map<std::string, double> samples;
  std::unordered_map<int, double> measurements;
  const auto shots =
      std::accumulate(in_counts.begin(), in_counts.end(), 0,
                      [](const std::size_t previous, const auto &p) {
                        return previous + p.second;
                      });
  assert(shots > 0);
  for (const auto &[state, count] : in_counts) {
    const auto bitString =
        in_msb ? state : std::string(state.rbegin(), state.rend());
    const auto y = std::stoi(bitString, 0, 2);
    const double probability =
        static_cast<double>(count) / static_cast<double>(shots);
    assert(measurements.find(y) == measurements.end());
    measurements[y] = probability;
    //   std::cout << y << ": " << probability << "\n";
    const double a =
        std::pow(std::sin(y * M_PI / std::pow(2, in_nbEvalBits)), 1);
    constexpr int DECIMAL_PLACES = 6;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(DECIMAL_PLACES) << a;
    std::string aStr = ss.str();
    auto iter = samples.find(aStr);
    if (iter != samples.end()) {
      samples[aStr] = samples[aStr] + probability;
    } else {
      samples[aStr] = probability;
    }
  }
  return std::make_pair(samples, measurements);
}

bool CanonicalAmplitudeEstimation::initialize(
    const xacc::HeterogeneousMap &parameters) {

  // Get required inputs
  if (!parameters.pointerLikeExists<xacc::CompositeInstruction>(
          "state_preparation_circuit")) {
    return false;
  }
  A_circuit_ = parameters.getPointerLike<xacc::CompositeInstruction>(
      "state_preparation_circuit");
  assert(A_circuit_->nInstructions() > 0);

  if (!parameters.keyExists<int>("num_evaluation_qubits")) {
    return false;
  }
  num_evaluation_qubits = parameters.get<int>("num_evaluation_qubits");
  assert(num_evaluation_qubits > 0);

  if (!parameters.keyExists<int>("num_trial_qubits")) {
    return false;
  }
  num_trial_qubits = parameters.get<int>("num_trial_qubits");
  assert(num_trial_qubits > 0);

  if (!parameters.keyExists<int>("num_state_qubits")) {
    return false;
  }
  num_state_qubits = parameters.get<int>("num_state_qubits");
  int num_qubits = num_trial_qubits + num_evaluation_qubits;
  assert(num_state_qubits > 0);
  assert(num_qubits > num_state_qubits);

  // Get optional inputs or define default
  if (!parameters.pointerLikeExists<xacc::CompositeInstruction>(
          "grover_op_circuit") &&
      !parameters.pointerLikeExists<xacc::CompositeInstruction>("oracle")) {
    return false;
  }

  Oracle_circuit_ = nullptr;
  Q_circuit_ = nullptr;
  const bool grover_op_provided =
      parameters.pointerLikeExists<xacc::CompositeInstruction>(
          "grover_op_circuit");
  if (grover_op_provided) {
    Q_circuit_ = parameters.getPointerLike<xacc::CompositeInstruction>(
        "grover_op_circuit");
  } else {
    Oracle_circuit_ =
        parameters.getPointerLike<xacc::CompositeInstruction>("oracle");
  }

  evaluation_qubits = {};
  if (parameters.keyExists<std::vector<int>>("evaluation_qubits")) {
    evaluation_qubits = parameters.get<std::vector<int>>("evaluation_qubits");
  }
  if (evaluation_qubits.size() == 0) {
    for (int i = 0; i < num_evaluation_qubits; i++) {
      evaluation_qubits.push_back(i);
    }
  }
  assert(evaluation_qubits.size() == num_evaluation_qubits);

  trial_qubits = {};
  if (parameters.keyExists<std::vector<int>>("trial_qubits")) {
    trial_qubits = parameters.get<std::vector<int>>("trial_qubits");
  }
  if (trial_qubits.size() == 0) {
    for (int i = 0; i < num_trial_qubits; i++) {
      trial_qubits.push_back(num_evaluation_qubits + i);
    }
  }
  assert(trial_qubits.size() == num_trial_qubits);

  shots = parameters.get_or_default("shots", (int)1024);

  if (!parameters.pointerLikeExists<xacc::Accelerator>("qpu")) {
    static auto qpp = xacc::getAccelerator("qpp", {{"shots", shots}});
    // Default to qpp if none provided
    qpu_ = qpp.get();
  } else {
    qpu_ = parameters.getPointerLike<xacc::Accelerator>("qpu");
    qpu_->updateConfiguration({{"shots", shots}});
  }
  return true;
}
const std::vector<std::string>
CanonicalAmplitudeEstimation::requiredParameters() const {
  return {"num_evaluation_qubits", "num_state_qubits", "num_trial_qubits",
          "state_preparation_circuit"};
}

void CanonicalAmplitudeEstimation::execute(
    const std::shared_ptr<xacc::AcceleratorBuffer> buffer) const {
  assert(A_circuit_);
  assert(Q_circuit_ || Oracle_circuit_);
  const std::string opt_label = Q_circuit_ ? "grover_op_circuit" : "oracle";
  auto *circuitPtr = Q_circuit_ ? Q_circuit_ : Oracle_circuit_;

  int nbEvaluationQubits = num_evaluation_qubits;
  int nbQubits = num_trial_qubits + num_evaluation_qubits;
  int nbStateQubits = num_state_qubits;
  int nbTrialQubits = num_trial_qubits;
  assert(nbStateQubits > 0);

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto ae_circ = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
  const bool expand_ok =
      ae_circ->expand({{"state_preparation_circuit", A_circuit_},
                       {opt_label, circuitPtr},
                       {"num_evaluation_qubits", nbEvaluationQubits},
                       {"num_state_qubits", nbStateQubits},
                       {"num_trial_qubits", nbTrialQubits},
                       {"trial_qubits", trial_qubits},
                       {"evaluation_qubits", evaluation_qubits}});
  assert(expand_ok);

  // Measure evaluation qubits:
  for (int i = 0; i < num_evaluation_qubits; ++i) {
    ae_circ->addInstruction(
        gateRegistry->createInstruction("Measure", evaluation_qubits[i]));
  }
  qpu_->execute(buffer, ae_circ);
  auto [samples, measurements] = process_count_results(
      buffer->getMeasurementCounts(),
      qpu_->getBitOrder() == xacc::Accelerator::BitOrder::MSB,
      num_evaluation_qubits);
  std::vector<double> amplitudes, probabilities;
  for (const auto &[amp, prob] : samples) {
    amplitudes.emplace_back(std::stof(amp));
    probabilities.emplace_back(prob);
  }

  // Sort by amplitudes:
  std::vector<std::size_t> index_vec;
  for (std::size_t i = 0; i != amplitudes.size(); ++i) {
    index_vec.emplace_back(i);
  }
  std::sort(index_vec.begin(), index_vec.end(),
            [&](std::size_t a, std::size_t b) {
              return amplitudes[a] < amplitudes[b];
            });
  std::vector<double> sorted_amplitudes, sorted_probabilities;
  for (std::size_t i = 0; i != index_vec.size(); ++i) {
    sorted_amplitudes.emplace_back(amplitudes[index_vec[i]]);
    sorted_probabilities.emplace_back(probabilities[index_vec[i]]);
  }

  // Determine the most likely estimate
  const double amplitude_estimate =
      std::stof(std::max_element(samples.begin(), samples.end(),
                                 [](const auto &p1, const auto &p2) {
                                   return p1.second < p2.second;
                                 })
                    ->first);

  // Add amplitude estimation data to the buffer:
  buffer->addExtraInfo("amplitude-estimation", amplitude_estimate);
  buffer->addExtraInfo("amplitudes", sorted_amplitudes);
  buffer->addExtraInfo("amplitude-probs", sorted_probabilities);
}
}

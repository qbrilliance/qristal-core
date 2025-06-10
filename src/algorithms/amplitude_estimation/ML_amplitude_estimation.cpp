// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/algorithms/amplitude_estimation/ML_amplitude_estimation.hpp>
#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

namespace qristal {
// Define the likelihood function for post-processing
long double likelihood(std::vector<int> iterations, std::vector<int> shots,
                       std::vector<int> good_counts, long double theta) {
  std::vector<long double> Ls = {};
  for (int i = 0; i < iterations.size(); i++) {
    long double Li =
        std::pow(sin((2 * iterations[i] + 1) * theta), 2 * good_counts[i]) *
        std::pow(cos((2 * iterations[i] + 1) * theta),
                 2 * (shots[i] - good_counts[i]));
    long double log_Li = std::log(Li);
    Ls.push_back(log_Li);
  }
  const auto total = std::accumulate(Ls.begin(), Ls.end(), 0.0);
  return total;
}

bool MLAmplitudeEstimation::initialize(
    // Get required inputs
    const xacc::HeterogeneousMap &parameters) {
  if (!parameters.pointerLikeExists<xacc::CompositeInstruction>(
          "state_preparation_circuit")) {
    return false;
  }
  A_circuit_ = parameters.getPointerLike<xacc::CompositeInstruction>(
      "state_preparation_circuit");
  assert(A_circuit_->nInstructions() > 0);

  if (!parameters.pointerLikeExists<xacc::CompositeInstruction>(
          "oracle_circuit")) {
    return false;
  }
  Oracle_circuit_ =
      parameters.getPointerLike<xacc::CompositeInstruction>("oracle_circuit");
  assert(Oracle_circuit_->nInstructions() > 0);

  if (!parameters.keyExists<std::function<int(std::string, int)>>(
          "is_in_good_subspace")) {
    return false;
  }
  is_in_good_subspace = parameters.get<std::function<int(std::string, int)>>(
      "is_in_good_subspace");

  if (!parameters.keyExists<std::vector<int>>("score_qubits")) {
    return false;
  }
  score_qubits = parameters.get<std::vector<int>>("score_qubits");
  assert(score_qubits.size() >= 0);

  // Get optional inputs or define default
  num_runs = 4;
  if (parameters.keyExists<int>("num_runs")) {
    num_runs = parameters.get<int>("num_runs");
  }
  assert(num_runs >= 0);

  shots = 100;
  if (parameters.keyExists<int>("shots")) {
    shots = parameters.get<int>("shots");
  }
  assert(shots >= 0);

  bestScore = parameters.get_or_default("bestScore", (int)0);

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
MLAmplitudeEstimation::requiredParameters() const {
  return {"state_preparation_circuit", "oracle_circuit", "score_qubits",
          "is_in_good_subspace"};
}

void MLAmplitudeEstimation::execute(
    const std::shared_ptr<xacc::AcceleratorBuffer> buffer) const {
  assert(A_circuit_);
  assert(Oracle_circuit_);

  std::vector<int> h = {}; // Number of |good> measurements in each run
  std::vector<int> m = {}; // Number of iterations in each run
  std::vector<int> N = {}; // Number of shots in each run

  for (int i = 0; i < num_runs; i++) {
    m.push_back(1 << i);
    N.push_back(shots);

    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    auto mlqae_circ = gateRegistry->createComposite("__TEMP__MLQAE__");

    auto amp_circ = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("AmplitudeAmplification"));
    const bool expand_ok =
        amp_circ->expand({{"power", 1 << i},
                          {"oracle", Oracle_circuit_},
                          {"state_preparation", A_circuit_}});
    assert(expand_ok);

    mlqae_circ->addInstructions(amp_circ->getInstructions());

    // Measure the score qubits
    for (int i = 0; i < score_qubits.size(); ++i) {
      mlqae_circ->addInstruction(
          gateRegistry->createInstruction("Measure", score_qubits[i]));
    }

    // Run the circuit
    auto temp_buffer = xacc::qalloc(buffer->size());
    qpu_->execute(temp_buffer, mlqae_circ);
    auto results = temp_buffer->getMeasurementCounts();

    int hi = 0;
    for (const auto &[state, count] : results) {
      if (is_in_good_subspace(state, bestScore) == 1) {
        hi += count;
      }
    }
    h.push_back(hi);
  }

  // Maximise the likelihood function
  std::vector<long double> angles;
  std::vector<long double> values;
  long double accuracy = 0.001;
  for (int i = 0; i < 1 / accuracy; i++) {
    long double angle = (M_PI / 2) * accuracy * i;
    angles.push_back(angle);
    long double value = -likelihood(m, N, h, angle);
    values.push_back(value);
  }

  long double optimal_value = *std::min_element(values.begin(), values.end());
  std::vector<long double>::iterator itr =
      std::find(values.begin(), values.end(), optimal_value);
  int index = std::distance(values.begin(), itr);
  double optimal_theta = angles[index];
  auto amplitude_estimate = sin(optimal_theta);

  // Add the result to the buffer
  buffer->addExtraInfo("amplitude-estimation", amplitude_estimate);
}
}

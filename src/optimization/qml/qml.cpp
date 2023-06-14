// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#include "qb/core/optimization/qml/qml.hpp"

namespace qb::qml {

ParamCirc::ParamCirc(std::size_t numQubits, qml::DefaultAnsatzes ansatzType,
                     std::size_t numAnsatzRepetitions,
                     qb::VectorString varGates)
    : numQubits_(numQubits), numAnsatzRepetitions_(numAnsatzRepetitions),
      varGates_(varGates), CircuitBuilder(),
      encodingAnsatz_(gate_provider_->createComposite("encodingAnsatz")) {
  // Switch between circuit type and implement correct ansatz
  switch (ansatzType) {
  case qml::DefaultAnsatzes::qrlRDBMS: {
    queryOptAnsatz();
    break;
  }
  default:
    throw std::invalid_argument("\nInvalid default ansatz selected.\n");
    break;
  }

  MeasureAll(numQubits_);
}

/* Take gate name, index and type, then add it to the main circuit as well as
 the encoding ansatz (if it is an input encoding gate) in case reuploading is
required */
void ParamCirc::processAndAddGate(std::string gateName,
                                  std::vector<size_t> qubits, char *type) {
  std::string variableName;
  bool isEncodingGate = false;

  if (strcmp(type, "input") == 0) {
    variableName = "theta_i" + std::to_string(numInputs_);
    isEncodingGate = true;
    encParamIndices_.emplace_back(numInputs_ + numParams_);
    numInputs_++;
  } else if (strcmp(type, "variational") == 0) {
    variableName = "theta_v" + std::to_string(numParams_);
    varParamIndices_.emplace_back(numInputs_ + numParams_);
    numParams_++;
  } else {
    throw std::runtime_error("Invalid parameter type!");
  }
  auto gate =
      gate_provider_->createInstruction(gateName, qubits, {variableName});
  circuit_->addVariable(variableName);
  circuit_->addInstruction(gate);
  if (isEncodingGate) {
    encodingAnsatz_->addVariable(variableName);
    encodingAnsatz_->addInstruction(gate);
  }
}

void ParamCirc::queryOptAnsatz() {
  std::size_t numVarGates = varGates_.size();
  std::string gateName;
  // Use Rx encoding gates as default to encode input parameters
  size_t numEncodingGates = numQubits_;
  for (std::size_t i = 0; i < numEncodingGates; i++) {
    RX(i, "input");
  }
  // Add variational gates
  for (std::size_t i = 0; i < numAnsatzRepetitions_; i++) {
    for (std::size_t qubit = 0; qubit < numQubits_; qubit++) {
      for (std::size_t j = 0; j < numVarGates; j++) {
        if (strcmp(varGates_[j][0].c_str(), "Rx") == 0) {
          RX(qubit, "variational");
        } else if (strcmp(varGates_[j][0].c_str(), "Ry") == 0) {
          RY(qubit, "variational");
        } else if (strcmp(varGates_[j][0].c_str(), "Rz") == 0) {
          RZ(qubit, "variational");
        } else {
          std::runtime_error("Invalid variational gate.\n");
        }
      }
    }
    // add circular CNOTs

    CNOT(numQubits_ - 1, 0);
    for (std::size_t qubit = 0; qubit < numQubits_ - 1; qubit++) {
      CNOT(qubit, qubit + 1);
    }
  }
}

QMLExecutor::QMLExecutor(ParamCirc inputCirc, std::vector<double> initInputs,
                         std::vector<double> initWeights)
    : targetCircuit_(inputCirc), inputParamVals_(initInputs),
      weightParamVals_(initWeights),
      outputBuffer_(xacc::qalloc(targetCircuit_.getNumQubits())),
      gradientBuffer_(xacc::qalloc(targetCircuit_.getNumQubits())),
      observables_(xacc::quantum::getObservable("pauli", std::string(" "))),
      gradientMethod_(xacc::getGradient(
          gradientTypeToString(),
          {{"observable", observables_}, {"shift-scalar", 0.5}})) {}

// Get string gradient type (for XACC input) from enum gradient type
std::string QMLExecutor::gradientTypeToString() {
  switch (gradientType_) {
  case GradientTypes::parameter_shift:
    return "parameter-shift";
  default:
    throw std::runtime_error("Invalid gradient type defined.\n");
  }
}

std::vector<double> QMLExecutor::constructFullParamVector() {
  std::vector<double> params(inputParamVals_.size() + weightParamVals_.size(),
                             0);
  for (size_t i = 0; i < inputParamVals_.size(); i++) {
    size_t index = targetCircuit_.encParamIndices_[i];
    params[index] = inputParamVals_[i];
  }
  for (size_t i = 0; i < weightParamVals_.size(); i++) {
    size_t index = targetCircuit_.varParamIndices_[i];
    params[index] = weightParamVals_[i];
  }
  return params;
}

void QMLExecutor::run() {
  if (seeded_) {
    accPointer_ =
        xacc::getAccelerator(acc, {{"shots", (int)numShots_}, {"seed", seed_}});
  } else {
    accPointer_ = xacc::getAccelerator(acc, {{"shots", (int)numShots_}});
  }
  std::shared_ptr<xacc::CompositeInstruction> instructionSet =
      targetCircuit_.get();
  std::vector<double> params = constructFullParamVector();
  std::shared_ptr<xacc::CompositeInstruction> evaledCirc =
      instructionSet->operator()(params);
  accPointer_->execute(outputBuffer_, evaledCirc);
}

std::vector<double>
QMLExecutor::getStatsFromShots(std::map<std::string, int> shotsDict) {
  std::size_t numQubits = targetCircuit_.getNumQubits();
  std::vector<double> probs(
      pow(2, numQubits)); // zero initialize since shotsDict may not contain all
                          // possible bitstrings
  for (auto const &[bitstring, count] : shotsDict) {
    std::size_t idx = std::stoi(bitstring, nullptr, 2);
    probs[idx] = count / ((double)numShots_);
  }

  return probs;
}

void QMLExecutor::runGradients() {
  if (seeded_) {
    accPointer_ =
        xacc::getAccelerator(acc, {{"shots", (int)numShots_}, {"seed", seed_}});
  } else {
    accPointer_ = xacc::getAccelerator(acc, {{"shots", (int)numShots_}});
  }
  std::vector<double> params = constructFullParamVector();
  auto gradientInstructions =
      gradientMethod_->getGradientExecutions(targetCircuit_.get(), params);
  accPointer_->execute(gradientBuffer_, gradientInstructions);
}

std::vector<std::vector<double>> QMLExecutor::getStatGradients() {
  auto gradientResults =
      gradientBuffer_->getChildren(); // Output of gradient executions
  std::size_t inputSize = targetCircuit_.getNumInputs();
  std::size_t numOutputs = pow(2, targetCircuit_.getNumQubits());
  std::vector<std::vector<double>> gradStats(targetCircuit_.getNumParams(),
                                             std::vector<double>(numOutputs));

  // Iterate over all gradient executions to obtain <+> and <-> shifted
  // probabilities, then calculate the probability gradient
  for (std::size_t i = 2 * inputSize; i < gradientResults.size() - 1; i += 2) {
    std::vector<double> statsPlus =
        getStatsFromShots(gradientResults[i]->getMeasurementCounts());
    std::vector<double> statsMinus =
        getStatsFromShots(gradientResults[i + 1]->getMeasurementCounts());
    for (std::size_t j = 0; j < numOutputs; j++) {
      gradStats[(i / 2) - inputSize][j] = 0.5 * (statsPlus[j] - statsMinus[j]);
    }
  }

  return gradStats;
}

} // namespace qb::qml
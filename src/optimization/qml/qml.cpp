// Copyright (c) 2023 Quantum Brilliance Pty Ltd

#include "qb/core/optimization/qml/qml.hpp"

namespace qb::qml {

ParamCirc::ParamCirc(std::size_t numQubits, DefaultAnsatzes ansatzType, std::size_t numAnsatzRepetitions, qb::VectorString varGates)
  : numQubits_(numQubits), numInputs_(numQubits), numAnsatzRepetitions_(numAnsatzRepetitions), varGates_(varGates), 
    numParams_(numAnsatzRepetitions * varGates.size() * numQubits),
    instructionSet_(std::make_shared<xacc::quantum::Circuit>("instructionSet_", genParamNames()))
{
  // Switch between circuit type and implement correct ansatz
  switch (ansatzType) {
  case DefaultAnsatzes::qrlRDBMS: {
    queryOptAnsatz();
    break;
  }
  default:
    throw std::invalid_argument("\nNo default ansatz selected.\n");
    break;
  }

  // Add measurements
  std::vector<std::shared_ptr<xacc::Instruction>> measurements;
  for (std::size_t i = 0; i < numInputs_; i++) {
    measurements.emplace_back(gateRegistry_->createInstruction("Measure", {i}));
  }
  instructionSet_->addInstructions({measurements});
}

std::vector<std::string> ParamCirc::genParamNames() {
  std::vector<std::string> paramNames;
  for (std::size_t i = 0; i < numInputs_; i++) {
    paramNames.emplace_back("theta_i" + std::to_string(i));
  }
  for (std::size_t i = 0; i < numParams_; i++) {
    paramNames.emplace_back("theta_w" + std::to_string(i));
  }
  return paramNames;
}

void ParamCirc::queryOptAnsatz() {
  std::size_t numVarGates = varGates_.size();
  std::string gateName;
  // Use Rx encoding gates as default to encode input parameters
  addEncodingGates({"Rx"});
  // Add variational gates
  for (std::size_t i = 0; i < numAnsatzRepetitions_; i++) {
    for (std::size_t qubit = 0; qubit < numQubits_; qubit++) {
      for (std::size_t j = 0; j < numVarGates; j++) {
        std::size_t layerOffset = numVarGates * numQubits_ * i;
        std::size_t idx = qubit + layerOffset + j * numQubits_;
        gateName = varGates_[j][0];
        instructionSet_->addInstruction( gateRegistry_->createInstruction(
            gateName, {qubit}, {"theta_w" + std::to_string(idx)}) );
      }
    }
    // add circular CNOTs
    instructionSet_->addInstruction( gateRegistry_->createInstruction("CNOT", {numQubits_ - 1, 0}) );
    for (std::size_t qubit = 0; qubit < numQubits_ - 1; qubit++) {
      instructionSet_->addInstruction( gateRegistry_->createInstruction("CNOT", {qubit, qubit + 1}) );
    }
  }
}

void ParamCirc::addEncodingGates(std::vector<std::string> encGates) {
  for (std::size_t gateIdx = 0; gateIdx < encGates.size(); gateIdx++) {
    std::string gate = encGates[gateIdx];
    for (std::size_t i = 0; i < numInputs_; i++) {
      instructionSet_->addInstruction(gateRegistry_->createInstruction(
          gate, {i}, {"theta_i" + std::to_string(i)}));
    }
  }
}

QMLExecutor::QMLExecutor(ParamCirc inputCirc, std::vector<double> initInputs, std::vector<double> initWeights)
  : circuit_(inputCirc), inputParams_(initInputs), weightParams_(initWeights),
    outputBuffer_(xacc::qalloc(circuit_.getNumQubits())),
    gradientBuffer_(xacc::qalloc(circuit_.getNumQubits())),
    observables_(xacc::quantum::getObservable("pauli", std::string(" "))),
    gradientMethod_(xacc::getGradient( gradientTypeToString(), {{"observable", observables_}, {"shift-scalar", 0.5}} )) 
  {}

// Get string gradient type (for XACC input) from enum gradient type
std::string QMLExecutor::gradientTypeToString() {
    switch (gradientType_) {
    case GradientTypes::parameter_shift:
      return "parameter-shift";
    default:
      throw std::runtime_error("Invalid gradient type defined.\n");
    }
  }

void QMLExecutor::run() {
  std::vector<double> params = inputParams_;
  params.insert(params.end(), weightParams_.begin(), weightParams_.end());
  auto instructionSet_ = circuit_.getInstructionSet();
  std::shared_ptr<xacc::CompositeInstruction> evaledCirc = instructionSet_->operator()(params);
  acc_->execute(outputBuffer_, evaledCirc);
}

std::vector<double> QMLExecutor::getStatsFromShots(std::map<std::string, int> shotsDict) {
  std::size_t numQubits = circuit_.getNumQubits();
  std::vector<double> probs( pow(2, numQubits) ); // zero initialize since shotsDict may not contain all possible bitstrings
  for (auto const& [bitstring, count] : shotsDict) {
    std::size_t idx = std::stoi(bitstring, nullptr, 2);
    probs[idx] = count / ((double)numShots_);
  }

  return probs;
}

void QMLExecutor::runGradients() {
  std::vector<double> params = inputParams_;
  params.insert(params.end(), weightParams_.begin(), weightParams_.end());
  auto gradientInstructions = gradientMethod_->getGradientExecutions( circuit_.getInstructionSet(), params );
  acc_->execute(gradientBuffer_, gradientInstructions);
}

std::vector<std::vector<double>> QMLExecutor::getStatGradients() {
  auto gradientResults = gradientBuffer_->getChildren(); // Output of gradient executions
  std::size_t inputSize = circuit_.getNumInputs();
  //std::size_t nShifts = gradientResults.size();
  std::size_t numOutputs = pow(2, circuit_.getNumQubits());
  std::vector<std::vector<double>> gradStats(circuit_.getNumParams(), std::vector<double>(numOutputs));
  
  // Iterate over all gradient executions to obtain <+> and <-> shifted probabilities, then calculate the probability gradient
  for (std::size_t i = 2 * inputSize; i < gradientResults.size() - 1; i += 2) {
    // Add circuit_.numInputs_ to each i-value, to ensure that
    std::vector<double> statsPlus  = getStatsFromShots(gradientResults[i]->getMeasurementCounts());
    std::vector<double> statsMinus = getStatsFromShots(gradientResults[i + 1]->getMeasurementCounts());
    for (std::size_t j = 0; j < numOutputs; j++) {
      gradStats[(i / 2) - inputSize][j] = 0.5 * (statsPlus[j] - statsMinus[j]);
    }
  }

  return gradStats;
}

} // namespace qb::qml
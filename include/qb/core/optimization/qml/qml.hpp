// Copyright (c) 2023 Quantum Brilliance Pty Ltd

#ifndef _QB_QML_
#define _QB_QML_

#include "Circuit.hpp"
#include "Gate.hpp"
#include "qb/core/typedefs.hpp"
#include "xacc.hpp"
#include "xacc_observable.hpp"
#include <cmath>
#include <iostream>
#include <optional>
#include <string>

namespace qb::qml {

enum class DefaultAnsatzes { qrlRDBMS };

enum class GradientTypes { parameter_shift };

// Creates a parametric circuit (xacc::quantum::Circuit) that is used as the basis for obtaining shot counts and gradient calculation
//      Inputs:
//          std::size_t numQubits: number of qubits (also currently sets number of input parameters) 
//          enum defaultAnsatzes: type of parametric circuit ansatz to be created (currently only query opt. implemented) std::size_t
//          numReps: number of layers of the ansatz 
//          qb::VectorString varGates: which gates to be optimised variationally, given as a vector of gate names
class ParamCirc {
private:
  std::size_t numQubits_;            // number of qubits
  std::size_t numInputs_;            // number of input parameters (defaults to be equal to number of qubits)
  std::size_t numAnsatzRepetitions_; // number of repetitions of the base ansatz
  qb::VectorString varGates_;   // The variational gates to use
  std::size_t numParams_;            // number of gate parameters (calculated using number of qubits and layers)

  std::shared_ptr<xacc::quantum::Circuit> instructionSet_; // stores the circuit in parametric form
  std::shared_ptr<xacc::IRProvider> gateRegistry_{xacc::getIRProvider("quantum")}; // IRProvider to obtain gates in XACC IR

  // Add the gates that encode the input values for the variational quantum circuit
  void addEncodingGates(std::vector<std::string> encGates);

  // Adds default ansatz for RDBMS query optimization to instructionSet_,
  // based on number of repetitions (numAnsatzRepetitions_) and variational gates (varGates_).
  void queryOptAnsatz();

  // create names params for instructionSet_ based on numInputs_ and numParams_
  std::vector<std::string> genParamNames();

public:
  // Constructor invalid without user input, therefore delete
  ParamCirc() = delete;

  ParamCirc(std::size_t numQubits, DefaultAnsatzes ansatzType, std::size_t numAnsatzRepetitions, qb::VectorString varGates);

  // Getters for attributes
  std::size_t getNumInputs() { return numInputs_; }
  std::size_t getNumParams() { return numParams_; }
  std::size_t getNumQubits() { return numQubits_; }
  std::size_t getNumAnsatzRepetitions() { return numAnsatzRepetitions_; }
  std::shared_ptr<xacc::quantum::Circuit> getInstructionSet() { return instructionSet_;}
};

// A class which handles executing and obtaining shots from a parametrised cicuit, as well as gradient calculations.
//      Inputs:
//          qb::qml::ParamCirc inputCirc: parametrised circuit to be executed
//          vector<double> initInputs: inputs to initialise the execution with
//          vector<double> initWeights: weights/gate parameters to initialise the execution with
class QMLExecutor {
private:
  ParamCirc circuit_;  // Circuit to be executed
  int seed_{-1};       // Seed for executor
  bool seeded_{false}; // Whether seed has been set
  std::size_t numShots_{1024};
  std::string accName_ = "aer";
  std::shared_ptr<xacc::Accelerator> acc_{xacc::getAccelerator( accName_, {{"shots",(int)numShots_}})}; // The backend accelerator (currently set to "aer")
  std::vector<double> inputParams_;  // Parameters which encode the input values
  std::vector<double> weightParams_; // Parameters which encode the weights/ansatz gate rotation angles

  std::shared_ptr<xacc::AcceleratorBuffer> outputBuffer_;   // Buffer to store shot results
  std::shared_ptr<xacc::AcceleratorBuffer> gradientBuffer_; // Buffer to store gradient shot results
  
  // The observables to calc gradients (currently set to none, but needed for gradient definition). 
  // Don't load observables b/c calculating gradients from stats
  std::shared_ptr<xacc::Observable> observables_;
 
  GradientTypes gradientType_{GradientTypes::parameter_shift}; // Gradient type (currently only parameter-shift supported)
  std::shared_ptr<xacc::AlgorithmGradientStrategy> gradientMethod_; // Reference to XACC gradient strategy
  std::vector<double> getStatsFromShots(std::map<std::string, int> shotsDict);

  // Get string gradient type (for XACC input) from enum gradient type
  std::string gradientTypeToString();

public:
  // Constructor invalid without user input, therefore delete
  QMLExecutor() = delete;

  QMLExecutor(ParamCirc inputCirc, std::vector<double> initInputs, std::vector<double> initWeights);

  // setter
  void setInputParams(std::vector<double> inputs) { inputParams_ = inputs; }
  void setWeights(std::vector<double> weights) { weightParams_ = weights; }
  void setCircuit(ParamCirc newCirc) { circuit_ = newCirc; }
  
  void setAcc(std::string accName) {
    accName_ = accName;
    if (seeded_) {
      acc_ = xacc::getAccelerator(accName, {{"seed", seed_}, {"shots", (int)numShots_}});
    }
    else {
      acc_ = xacc::getAccelerator(accName, {{"shots", (int)numShots_}});
    }
  }
  
  void setNumShots(std::size_t numShots) { numShots_ = numShots; };

  void setSeed(int seed) {
    seed_ = seed;
    seeded_ = true;
    acc_ = xacc::getAccelerator(accName_, {{"seed", seed_}, {"shots", (int)numShots_}});
  };

  // getter
  std::vector<double> getInputParams() { return inputParams_; }
  std::vector<double> getWeights() { return weightParams_; }
  ParamCirc getCircuit() { return circuit_; }
  std::size_t getNumShots() { return numShots_; };
  std::string getAcc() { return accName_; }
  std::shared_ptr<xacc::AcceleratorBuffer> getGradBuffer() { return gradientBuffer_; }
  
  int getSeed() {
    if (seeded_) {
      return seed_;
    } else {
      throw std::runtime_error("No seed has been set.\n");
    }
  };

  // Run the circuit
  void run();
  // Run gradient executions (+/- shifted result)
  void runGradients();

  // Obtain output probabilities from the buffer after running
  std::vector<double> getStats() { return getStatsFromShots(outputBuffer_->getMeasurementCounts());}
  // Obtain probability gradients
  std::vector<std::vector<double>> getStatGradients();
};

} // namespace qb::qml

#endif // _QB_QML_
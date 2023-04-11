// Copyright (c) 2023 Quantum Brilliance Pty Ltd

#ifndef _QB_QML_
#define _QB_QML_

#include "qb/core/circuit_builder.hpp"
#include "qb/core/typedefs.hpp"
#include "xacc_observable.hpp"
#include <iostream>

namespace qb::qml {

/**
 * @brief Types of default ansatzes that can be used with the pre-defined ansatz
 * parametrized circuit constructor.
*/
enum class DefaultAnsatzes { 
  /**
   * @brief QML Ansatz for optimizing join orders in relational database management
   * systems (such as PostgreSQL).
  */
  qrlRDBMS
}; 

/**
 * @brief Types of methods that can be used to calculate the gradients of parameters
 * in a defined parametrized circuit.
*/
enum class GradientTypes { 
  parameter_shift //<! Parameter-shift gradient.
}; 

/**
 * @brief This class is used to build parametrized quantum circuits for 
 * execution as part of quantum machine learning algorithms. It can use any of 
 * the standard gates from qb::CircuitBuilder, as well as create input/
 * variational parameters using the single-parameter gates shown below.
*/


class ParamCirc : public CircuitBuilder {
friend class QMLExecutor;
private:
  std::size_t numQubits_{0}; // number of qubits
  std::size_t numInputs_{0}; // number of input parameters (defaults to be equal
                             // to number of qubits)
  std::size_t numAnsatzRepetitions_{
      0};                     // number of repetitions of the base ansatz
  qb::VectorString varGates_; // The variational gates to use
  std::size_t numParams_{0};  // number of gate parameters (calculated using
                              // number of qubits and layers)

  bool reuploading_{
      false}; // Whether the input encoding ansatz has been reuploaded
  std::shared_ptr<xacc::CompositeInstruction>
      encodingAnsatz_; // The input encoding ansatz
  std::vector<size_t> encParamIndices_; // Indices of the encoding parameters
  std::vector<size_t> varParamIndices_; // Indices of the variational parameters
  
  // Adds default ansatz for RDBMS query optimization to circuit_,
  // based on number of repetitions (numAnsatzRepetitions_) and variational
  // gates (varGates_).
  void queryOptAnsatz();

  // create names params for circuit_ based on numInputs_ and numParams_
  std::vector<std::string> genParamNames();

  // Process a gate name, then add it to the parametrised circuit.
  void processAndAddGate(std::string gateName, std::vector<size_t> qubits,
                         char *type);

public:
  // ParamCirc constructors
  /**
   * @brief Creates a parametrized circuit with a pre-defined ansatz.
   * @param numQubits Number of qubits in the parametrised circuit (also 
   * currently sets number of input parameters) [size_t]
   * @param defaultAnsatzes Type of parametric circuit ansatz to be created 
   * (currently only query opt. implemented) [qb::qml::DefaultAnsatzes]
   * @param numReps Number of layers of the ansatz [size_t]
   * @param varGates Which gates to be optimised variationally, given as a 
   * vector of gate names within {"Rx","Ry","Rz"} [qb::VectorString]
  */
  ParamCirc(std::size_t numQubits, qml::DefaultAnsatzes ansatzType,
            std::size_t numAnsatzRepetitions, qb::VectorString varGates);

  /**
   * @brief Constructs an empty parametrized circuit to be used with
   * the circuit builder.
   * @param numQubits Number of qubits in the parametrised circuit
  */
  ParamCirc(size_t numQubits)
      : numQubits_(numQubits), CircuitBuilder(),
        encodingAnsatz_(gate_provider_->createComposite("encodingAnsatz")) {}

  /**
  * @brief RX gate
  *
  * This method adds an x-axis rotation (RX) gate to the circuit.
  *
  * The RX gate is defined by its action on the basis states
  *
  * \f[
  * RX(\theta)\ket{0} \rightarrow \cos(\theta/2)\ket{0} - i\sin(\theta/2)\ket{1}
  *
  * RX(\theta)\ket{1} \rightarrow -i\sin(\theta/2)\ket{0} + \cos(\theta/2)\ket{1}
  * \f]
  *
  * @param idx the index of the qubit being acted on [size_t]
  * @param type the type of input parameter ("input" or "variational") [std::string]
  */
  void RX(size_t index, char *type) {
    std::string gateName = "Rx";
    processAndAddGate(gateName, {index}, type);
  }

  /**
  * @brief RY gate
  *
  * This method adds a y-axis rotation (RY) gate to the circuit.
  *
  * The RY gate is defined by its action on the basis states
  *
  * \f[
  * RY(\theta)\ket{0} \rightarrow \cos(\theta/2)\ket{0} + \sin(\theta/2)\ket{1}
  *
  * RY(\theta)\ket{1} \rightarrow -\sin(\theta/2)\ket{0} + \cos(\theta/2)\ket{1}
  * \f]
  *
  * @param idx the index of the qubit being acted on [size_t]
  * @param type the type of input parameter ("input" or "variational") [std::string]
  */
  void RY(size_t index, char *type) {
    std::string gateName = "Ry";
    processAndAddGate(gateName, {index}, type);
  }

  /**
  * @brief RZ gate
  *
  * This method adds a z-axis rotation (RZ) gate to the circuit.
  *
  * The RZ gate is defined by its action on the basis states
  *
  * \f[
  * RZ(\theta)\ket{0} \rightarrow e^{-i\theta/2}\ket{0}
  *
  * RZ(\theta)\ket{1} \rightarrow e^{i\theta/2}\ket{1}
  * \f]
  *
  * @param idx the index of the qubit being acted on [size_t]
  * @param type the type of input parameter ("input" or "variational") [std::string]
  */
  void RZ(size_t index, char *type) {
    std::string gateName = "Rz";
    processAndAddGate(gateName, {index}, type);
  }

  /**
  * @brief U1 gate
  *
  * This method adds a phase (U1) gate to the circuit.
  *
  * The U1 gate is defined by its action on the basis states
  *
  * \f[
  * U1(\theta)\ket{0} \rightarrow \ket{0}
  *
  * U1(\theta)\ket{1} \rightarrow e^{i\theta}\ket{1}
  * \f]
  *
  * @param idx the index of the qubit being acted on [size_t]
  * @param type the type of input parameter ("input" or "variational") [std::string]
  */
  void U1(size_t index, char *type) {
    std::string gateName = "U1";
    processAndAddGate(gateName, {index}, type);
  }

  // CPhase == CU1
  /**
  * @brief CPhase gate
  *
  * This method adds a controlled-U1 (CPhase) gate to the circuit.
  *
  * The CPHase gate performs a U1 gate on the target qubit
  * conditional on the control qubit being in the \f$\ket{1}\f$ state. That is:
  *
  * \f[
  * 
  * CPhase(\theta)\ket{ab} \rightarrow \ket{a}U1(\theta)^a \ket{b}
  *
  * \f]
  *
  * @param ctrl_idx the index of the control qubit [size_t]
  * @param target_idx the index of the target qubit [size_t]
  * @param type the type of input parameter ("input" or "variational") [std::string]
  */
  void CPhase(size_t ctrl_index, size_t target_index, char *type) {
    std::string gateName = "CPhase";
    processAndAddGate(gateName, {ctrl_index, target_index}, type);
  }
  /**
   * @brief Function for reuploading within the circuit, allowing for repeating the
   * encoding ansatz.
  */
  void reupload() {
    circuit_->addInstruction(encodingAnsatz_->clone());
    reuploading_ = true;
  }

  /**
   * @brief Return the number of input parameters.
  */
  std::size_t getNumInputs() { return numInputs_; } 
  /**
   * Return the number of variational parameters.
  */
  std::size_t getNumParams() { return numParams_; }  
  /**
   * @brief  Return the number of qubits in the parametrized circuit.
  */
  std::size_t getNumQubits() { return numQubits_; } 
  /**
   * @brief Return the number of repetitions of the base ansatz (currently 
   * only has utility for DefaultAnsatz constructor).
  */
  std::size_t getNumAnsatzRepetitions() { return numAnsatzRepetitions_; } 
};

/**
 * @brief A class which handles executing and obtaining shots from a parametrised
 * cicuit, as well as gradient calculations.
*/

class QMLExecutor {
private:
  ParamCirc targetCircuit_; // Circuit to be executed
  int seed_{-1};            // Seed for executor
  bool seeded_{false};      // Whether seed has been set
  std::size_t numShots_{1024};
  std::shared_ptr<xacc::Accelerator> accPointer_{xacc::getAccelerator(
      "aer", {{"shots",
        (int)numShots_}})}; // The backend accelerator 
  std::vector<double>
      inputParamVals_; // Parameters which encode the input values
  std::vector<double> weightParamVals_; // Parameters which encode the
                                        // weights/ansatz gate rotation angles

  std::shared_ptr<xacc::AcceleratorBuffer>
      outputBuffer_; // Buffer to store shot results
  std::shared_ptr<xacc::AcceleratorBuffer>
      gradientBuffer_; // Buffer to store gradient shot results

  // The observables to calc gradients (currently set to none, but needed for
  // gradient definition). Don't load observables b/c calculating gradients from
  // stats
  std::shared_ptr<xacc::Observable> observables_;

  GradientTypes gradientType_{
      GradientTypes::parameter_shift}; // Gradient type (currently only
                                       // parameter-shift supported)
  std::shared_ptr<xacc::AlgorithmGradientStrategy>
      gradientMethod_; // Reference to XACC gradient strategy
  std::vector<double> getStatsFromShots(
      std::map<std::string, int>
          shotsDict); // Obtain a probability vector from a dict of shots

  // Get string gradient type (for XACC input) from enum gradient type
  std::string gradientTypeToString();

public:
  /**
   * @brief Creates an executor for a defined parametrized circuit with initial
   * specifications for inputs and variational parameters.
   * @param inputCirc Parametrized circuit to be executed [qb::qml::ParamCirc]
   * @param initInputs Inputs to initialise the execution with [std::vector<double>]
   * @param initWeights Weights/variational gate parameters to initialise the 
   * execution with [std::vector<double>]
  */
  QMLExecutor(ParamCirc inputCirc, std::vector<double> initInputs,
              std::vector<double> initWeights);
  /**
   * @brief The currently defined accelerator (defaults to "aer"). 
   * Valid settings: "aer" | "tnqvm" | "qpp"
  */
  std::string acc = "aer";
  /**
   * @brief Return full parameter vector, constructed from 
   * the defined input and weight parameter values.
  */
  std::vector<double> constructFullParamVector();
  /**
   * @brief Execute the circuit.
  */
  void run(); 
  /**
   * @brief Execute the circuit for gradient calculation (e.g. execute multiple 
   * +/- shifted circuits for the parameter-shift rule).
  */
  void runGradients();
  /**
   * @brief Obtain bitstring probabilities from the output buffer after running.
  */
  std::vector<double> getStats() {
    return getStatsFromShots(outputBuffer_->getMeasurementCounts());
  }
  /**
   * @brief Obtain the gradients of output probabilities w.r.t. the variational 
   * parameters.
  */
  std::vector<std::vector<double>> getStatGradients();

  /**
   * @brief Manually set the input parameters for parametrized circuit 
   * execution.
  */
  void setInputParams(std::vector<double> inputs) { inputParamVals_ = inputs; }
  /**
   * @brief Manually set the variational/weight parameters for parametrized 
   * circuit execution.
  */
  void setWeights(std::vector<double> weights) { weightParamVals_ = weights; }
  /**
   * @brief Manually set the parametrized circuit (qb::qml::ParamCirc) to be executed.
  */
  void setCircuit(ParamCirc newCirc) { targetCircuit_ = newCirc; }

  /**
   * @brief Manually set the number of shots to run the circuit for during execution.
  */
  void setNumShots(std::size_t numShots) { numShots_ = numShots; };
  /**
   * @brief Manually set the seed for the circuit executor.
  */
  void setSeed(int seed) {
    seed_ = seed;
    seeded_ = true;
    accPointer_ = xacc::getAccelerator(acc,
                                {{"seed", seed_}, {"shots", (int)numShots_}});
  };

  /**
   * @brief Return the currently defined input parameter values.
  */
  std::vector<double> getInputParams() { return inputParamVals_; }
  /**
   * @brief Return the currently defined variational parameter values.
  */
  std::vector<double> getWeights() { return weightParamVals_; }
  /**
   * @brief Return the currently defined parametrized circuit that is to be
   *  executed.
  */
  ParamCirc getCircuit() { return targetCircuit_; }
  /**
   * @brief Return the currently defined number of shots.
  */
  std::size_t getNumShots() { return numShots_; };
  /**
   * @brief Return the currently defined seed.
  */
  int getSeed() {
    if (seeded_) {
      return seed_;
    } else {
      throw std::runtime_error("No seed has been set.\n");
    }
  }
  /**
   * @brief Return the gradient buffer which stores the gradient executions and 
   * shot results.
  */
  std::shared_ptr<xacc::AcceleratorBuffer> getGradBuffer() {
    return gradientBuffer_;
  }
  /**
   * @brief Return the output buffer which stores the shot results from 
   * execution.
  */
  std::shared_ptr<xacc::AcceleratorBuffer> getBuffer() { return outputBuffer_; }
};

} // namespace qb::qml

#endif // _QB_QML_
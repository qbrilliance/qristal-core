#include "qb/core/optimization/qml/qml.hpp"
#include "xacc.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <numeric>
#include <random>

/***
The first 6 tests are all for the ParamCirc class.
They could be merged size_to one test depending on best practice.
***/

TEST(QMLTester, test_ParamCirc_constructor) {
  /***
  Testing the constructor of the ParamCirc class.
  ***/

  size_t numQubits = 5;
  size_t numAnsatzReps = 4;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;

  EXPECT_NO_THROW(qb::qml::ParamCirc circuit(numQubits, circuitType,
                                             numAnsatzReps, varGates););
}

TEST(QMLTester, test_ParamCirc_getNumInputs) {
  /***
  Tests the getter function for the private member variable NumInputs of the
  ParamCirc class.
  ***/

  size_t numQubits = 4;
  size_t numAnsatzReps = 5;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  EXPECT_EQ(circuit.getNumInputs(), numQubits);

  // The instruction set is a xacc CompositeInstruction object
  auto instructions = circuit.getInstructionSet();
  EXPECT_EQ(circuit.getNumInputs(), instructions->nPhysicalBits());
}

TEST(QMLTester, test_ParamCirc_getNumParams) {
  /***
  Tests the getter function for the private member variable NumParams of the
  ParamCirc class.
  ***/

  size_t numQubits = 4;
  size_t numAnsatzReps = 5;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  EXPECT_EQ(circuit.getNumParams(),
            varGates.size() * numQubits * numAnsatzReps);

  // The instruction set is a xacc CompositeInstruction object
  auto instructions = circuit.getInstructionSet();
  EXPECT_EQ(circuit.getNumParams(),
            instructions->getVariables().size() - numQubits);
}

TEST(QMLTester, test_ParamCirc_getNumQubits) {
  /***
  Tests the getter function for the private member variable NumQubits of the
  ParamCirc class.
  ***/

  size_t numQubits = 4;
  size_t numAnsatzReps = 5;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  EXPECT_EQ(circuit.getNumQubits(), numQubits);

  // The instruction set is a xacc CompositeInstruction object
  auto instructions = circuit.getInstructionSet();
  EXPECT_EQ(circuit.getNumQubits(), instructions->nPhysicalBits());
}

TEST(QMLTester, test_ParamCirc_getNumAnsatzRepetitions) {
  /***
  Tests the getter function for the private member variable
  numAnsatzRepetitions_ of the ParamCirc class.
  ***/

  size_t numQubits = 4;
  size_t numAnsatzReps = 5;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  EXPECT_EQ(circuit.getNumAnsatzRepetitions(), numAnsatzReps);

  // The instruction set is a xacc::quantum::Circuit object
  auto instructions = circuit.getInstructionSet();
  EXPECT_EQ(circuit.getNumAnsatzRepetitions(),
            (instructions->depth() - 1) / (2 + numQubits));
}

TEST(QMLTester, test_ParamCirc_CircuitStructure) {
  /***
  Tests:
    the getter function for the private member variable InstructionSet of the
    ParamCirc class the construction of the queryOptAnsatz circuit structure the
    correct implementation of the variable order expected by getVarOrder
  ***/

  size_t numQubits = 2;
  size_t numAnsatzReps = 2;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  // Testing circuit construction
  auto instructions = circuit.getInstructionSet();
  EXPECT_EQ(instructions->getInstructions().size(),
            numQubits +
                (varGates.size() * numQubits + numQubits) * numAnsatzReps +
                numQubits);
  std::string expected_circ =
      "Rx(theta_i0) q0\nRx(theta_i1) q1\nRy(theta_w0) q0\nRz(theta_w2) "
      "q0\nRy(theta_w1) q1\nRz(theta_w3) q1\nCNOT q1,q0\nCNOT "
      "q0,q1\nRy(theta_w4) q0\nRz(theta_w6) q0\nRy(theta_w5) q1\nRz(theta_w7) "
      "q1\nCNOT q1,q0\nCNOT q0,q1\nMeasure q0\nMeasure q1\n";
  EXPECT_EQ(expected_circ, instructions->toString());
}

/***
The next 4 tests are for the construction of the QMLExecutor object
and the getter and setter helper functions for private member variables
They could be combined size_to one test depending on best practice
***/

TEST(QMLTester, test_QMLExecutor_constructor) {
  /***
  Tests the constructor of the QMLExecutor class
  ***/
  size_t numQubits = 4;
  size_t numAnsatzReps = 5;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  size_t numParamGates = varGates.size() * numQubits * numAnsatzReps;
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  std::vector<double> inputVec(numQubits);
  std::default_random_engine gen{std::random_device{}()};
  std::uniform_real_distribution<double> dist(-3.14159, 3.14159);
  std::generate(std::begin(inputVec), std::end(inputVec),
                [&] { return dist(gen); });

  std::vector<double> paramVec(numParamGates);
  std::generate(std::begin(paramVec), std::end(paramVec),
                [&] { return dist(gen); });

  EXPECT_NO_THROW(qb::qml::QMLExecutor exec(circuit, inputVec, paramVec););
}

TEST(QMLTester, test_QMLExecutor_InputParams) {
  /***
  Tests the getter and setter methods for the private member variable
  InputParams in the QMLExecutor class.
  ***/
  size_t numQubits = 4;
  size_t numAnsatzReps = 5;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  size_t numParamGates = varGates.size() * numQubits * numAnsatzReps;
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  std::vector<double> inputVec(numQubits);
  std::vector<double> inputVecReplacement(numQubits);

  std::default_random_engine gen{std::random_device{}()};
  std::uniform_real_distribution<double> dist(-3.14159, 3.14159);
  std::generate(std::begin(inputVec), std::end(inputVec),
                [&] { return dist(gen); });
  std::generate(std::begin(inputVecReplacement), std::end(inputVecReplacement),
                [&] { return dist(gen); });
  std::vector<double> paramVec(numParamGates);
  std::generate(std::begin(paramVec), std::end(paramVec),
                [&] { return dist(gen); });

  qb::qml::QMLExecutor exec(circuit, inputVec, paramVec);

  EXPECT_EQ(exec.getInputParams(), inputVec);
  exec.setInputParams(inputVecReplacement);
  EXPECT_EQ(exec.getInputParams(), inputVecReplacement);
}

TEST(QMLTester, test_QMLExecutor_Weights) {
  /***
  Tests the getter and setter methods for the private member variable Weights in
  the QMLExecutor class.
  ***/
  size_t numQubits = 4;
  size_t numAnsatzReps = 5;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  size_t numParamGates = varGates.size() * numQubits * numAnsatzReps;
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  std::vector<double> inputVec(numQubits);
  std::default_random_engine gen{std::random_device{}()};
  std::uniform_real_distribution<double> dist(-3.14159, 3.14159);
  std::generate(std::begin(inputVec), std::end(inputVec),
                [&] { return dist(gen); });

  std::vector<double> paramVec(numParamGates);
  std::vector<double> paramVecReplacement(numParamGates);
  std::generate(std::begin(paramVec), std::end(paramVec),
                [&] { return dist(gen); });
  std::generate(std::begin(paramVecReplacement), std::end(paramVecReplacement),
                [&] { return dist(gen); });
  qb::qml::QMLExecutor exec(circuit, inputVec, paramVec);

  EXPECT_EQ(exec.getWeights(), paramVec);
  exec.setWeights(paramVecReplacement);
  EXPECT_EQ(exec.getWeights(), paramVecReplacement);
}

TEST(QMLTester, test_QMLExecutor_run_1) {
  /***
  Tests the run method and the getStats methods in the QMLExecutor class

  Input state: |00>
  Input weights: All weights set to 0
  Circuit: "yz" ansatz with 2 reps

  Expected output distribution: {00 : 100%}
  ***/

  size_t numQubits = 2;
  size_t numAnsatzReps = 2;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  size_t numParamGates = varGates.size() * numQubits * numAnsatzReps;
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  std::vector<double> inputVec(numQubits);
  std::vector<double> paramVec(numParamGates);
  qb::qml::QMLExecutor exec(circuit, inputVec, paramVec);

  exec.setSeed(1000);

  exec.run();
  auto shots = exec.getStats();

  // Verify getStats
  EXPECT_EQ(shots.size(), std::pow(2, numQubits)); // 2^n outcomes
  EXPECT_DOUBLE_EQ(std::accumulate(shots.begin(), shots.end(), 0.0),
                   1.0); // probs sum to 1

  // Verify run
  EXPECT_DOUBLE_EQ(shots[0], 1.0);
  EXPECT_DOUBLE_EQ(shots[1], 0.0);
  EXPECT_DOUBLE_EQ(shots[2], 0.0);
  EXPECT_DOUBLE_EQ(shots[3], 0.0);
}

TEST(QMLTester, test_QMLExecutor_run_2) {
  /***
  Tests the run method and the getStats methods in the QMLExecutor class

  Input state: |++>
  Input weights: All weights set to pi/4
  Circuit: "yz" ansatz with 2 reps

  Expected output distribution: {00 : 18.75% , 01 : 6.25% , 10 : 61.4% , 11
  : 13.6%}
  ***/

  size_t numQubits = 2;
  size_t numAnsatzReps = 2;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  size_t numParamGates = varGates.size() * numQubits * numAnsatzReps;
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  std::vector<double> inputVec(numQubits, M_PI_2);
  std::vector<double> paramVec(numParamGates, M_PI_4);
  qb::qml::QMLExecutor exec(circuit, inputVec, paramVec);

  exec.setSeed(1000);

  exec.run();
  auto shots = exec.getStats();

  // Verify getStats
  EXPECT_EQ(shots.size(), std::pow(2, numQubits));
  EXPECT_DOUBLE_EQ(std::accumulate(shots.begin(), shots.end(), 0.0), 1.0);

  // Verify circuit output
  EXPECT_NEAR(shots[0], 0.1875, 0.02);
  EXPECT_NEAR(shots[1], 0.0625, 0.02);
  EXPECT_NEAR(shots[2], 0.614, 0.02);
  EXPECT_NEAR(shots[3], 0.136, 0.02);
}

TEST(QMLTester, test_QMLExecutor_runGradients) {
  /***
  Tests the runGradients and the getStatGradients methods in the QMLExecutor
  class

  Input state is a variation on |++> (params = {pi/2 + 0.05, pi/2 + 0.05})
  Input weights set to {pi/4 + 0.07*n} where n is parameter number
  Circuit will be run for theta_i - pi/2 and theta_i + pi/2 for all parameters
  theta_i and difference calculated

  Expected output shown below
  ***/
  size_t numQubits = 2;
  size_t numAnsatzReps = 2;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  size_t numParamGates = varGates.size() * numQubits * numAnsatzReps;
  size_t numOutputs = std::pow(2, numQubits);
  size_t numTotalParams = numQubits + numParamGates;
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  std::vector<double> inputVec(numQubits);
  std::vector<double> paramVec(numParamGates);
  // The following is necessary as XACC shifts all numerically equivalent
  // parameters at the same time, which is not expected behaviour for this rule
  for (size_t i = 0; i < numQubits; i++) {
    inputVec[i] = M_PI_2 + 0.05 * i;
  }
  for (size_t i = 0; i < numParamGates; i++) {
    paramVec[i] = M_PI_4 + 0.07 * i;
  }
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);
  qb::qml::QMLExecutor exec(circuit, inputVec, paramVec);

  exec.setSeed(1000);
  exec.runGradients();
  auto gradients = exec.getStatGradients();
  // Verify gradient execution setup

  EXPECT_EQ(exec.getGradBuffer()->getChildren().size(), 2 * numTotalParams);

  // Verify getStatGradients

  EXPECT_EQ(gradients.size(), numParamGates);
  EXPECT_EQ(gradients[0].size(), numOutputs);
  // Verify output
  std::vector<std::vector<double>> expectedGrad = {
      {0, 0, 0, 0},
      {0.00830078, 0.000976562, -0.00634766, -0.00292969},
      {-0.0400391, 0.0400391, 0.172363, -0.172363},
      {-0.284668, 0.0366211, 0.174805, 0.0732422},
      {-0.0791016, -0.118652, 0.124023, 0.0737305},
      {0.0273438, -0.0273438, 0.157715, -0.157715},
      {0, 0, 0, 0},
      {0, 0, 0, 0}};
  for (size_t i = 0; i < numParamGates; i++) {
    for (size_t j = 0; j < numOutputs; j++) {
      EXPECT_NEAR(gradients[i][j], expectedGrad[i][j], 1e-5);
    }
  }
}
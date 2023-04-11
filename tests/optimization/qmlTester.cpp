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

  size_t numQubits = 4;
  size_t numAnsatzReps = 5;
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
  auto instructions = circuit.get();
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
  auto instructions = circuit.get();
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
  auto instructions = circuit.get();
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
  auto instructions = circuit.get();
  EXPECT_EQ(circuit.getNumAnsatzRepetitions(),
            (instructions->depth() - 1) / (2 + numQubits));
}

TEST(QMLTester, test_ParamCirc_CircuitStructure) {
  /***
  Tests the getter function for the private member variable InstructionSet of
  the ParamCirc class, as well as the construction of the default queryOptAnsatz
  circuit structure.
  ***/

  size_t numQubits = 2;
  size_t numAnsatzReps = 2;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits, circuitType, numAnsatzReps, varGates);

  // Testing circuit construction
  auto instructions = circuit.get();
  EXPECT_EQ(instructions->getInstructions().size(),
            numQubits +
                (varGates.size() * numQubits + numQubits) * numAnsatzReps +
                numQubits);
  std::string expected_circ =
      "Rx(theta_i0) q0\nRx(theta_i1) q1\nRy(theta_v0) q0\nRz(theta_v1) "
      "q0\nRy(theta_v2) q1\nRz(theta_v3) q1\nCNOT q1,q0\nCNOT "
      "q0,q1\nRy(theta_v4) q0\nRz(theta_v5) q0\nRy(theta_v6) q1\nRz(theta_v7) "
      "q1\nCNOT q1,q0\nCNOT q0,q1\nMeasure q0\nMeasure q1\n";
  EXPECT_EQ(expected_circ, instructions->toString());
}

TEST(QMLTester, test_ParamCirc_CircuitBuilder) {
  /***
  Tests the circuit builder and reuploading functionality of the ParamCirc
  class.
  ***/
  size_t numQubits = 2;
  size_t numAnsatzRepetitions = 2;
  qb::qml::ParamCirc circuit = qb::qml::ParamCirc(numQubits);
  for (size_t i = 0; i < numQubits; i++) {
    circuit.RX(i, "input");
  }
  for (size_t i = 0; i < numAnsatzRepetitions; i++) {
    if (i >= 1) {
      circuit.reupload();
    }
    for (size_t j = 0; j < numQubits; j++) {
      circuit.RY(j, "variational");
    }
    circuit.CNOT(numQubits - 1, 0);
    for (std::size_t qubit = 0; qubit < numQubits - 1; qubit++) {
      circuit.CNOT(qubit, qubit + 1);
    }
  }

  circuit.MeasureAll(numQubits);
  auto instructions = circuit.get();
  std::string expected_circ =
      "Rx(theta_i0) q0\nRx(theta_i1) q1\nRy(theta_v0) q0\nRy(theta_v1) "
      "q1\nCNOT q1,q0\nCNOT q0,q1\nRx(theta_i0) q0\nRx(theta_i1) "
      "q1\n\nRy(theta_v2) q0\nRy(theta_v3) q1\nCNOT q1,q0\nCNOT q0,q1\nMeasure "
      "q0\nMeasure q1\n";
  EXPECT_EQ(expected_circ, instructions->toString());
}

/***
The next 4 tests are for the construction of the QMLExecutor object
and the getter and setter helper functions for private member variables.
They could be combined into one test depending on best practice
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

TEST(QMLTester, test_QMLExecutor_parameter_placement) {
  /***
  Tests the parameter placement in the executor.

  Input state: |00>
  All weights set to 0, except the last input param
  (which is pi/2 = 1.57).
  Circuit: "yz" ansatz with 2 reps, Rx at end of circuit

  ***/

  size_t numQubits = 2;
  size_t numAnsatzReps = 2;
  qb::VectorString varGates = {{"Ry"}, {"Rz"}};
  size_t numParamGates = varGates.size() * numQubits * numAnsatzReps;
  auto circuitType = qb::qml::DefaultAnsatzes::qrlRDBMS;
  qb::qml::ParamCirc circuit(numQubits);
  for (size_t i = 0; i < numQubits; i++) {
    circuit.RX(i, "input");
  }
  for (size_t i = 0; i < numAnsatzReps; i++) {
    if (i >= 1) {
      circuit.reupload();
    }
    for (size_t j = 0; j < numQubits; j++) {
      circuit.RY(j, "variational");
      circuit.RZ(j, "variational");
    }
    circuit.CNOT(numQubits - 1, 0);
    for (std::size_t qubit = 0; qubit < numQubits - 1; qubit++) {
      circuit.CNOT(qubit, qubit + 1);
    }
  }
  circuit.RX(0, "input");
  std::vector<double> inputVec(numQubits + 1);
  std::vector<double> paramVec(numParamGates);
  // Ensure only the last input, after all variational gates,
  // is non-zero
  inputVec[2] = M_PI_2;
  qb::qml::QMLExecutor exec(circuit, inputVec, paramVec);
  std::vector<double> fullParamVec = exec.constructFullParamVector();
  std::shared_ptr<xacc::CompositeInstruction> evaledCirc =
      exec.getCircuit().get()->operator()(fullParamVec);
  std::string expectedCirc =
      "Rx(0) q0\nRx(0) q1\nRy(0) q0\nRz(0) q0\nRy(0) q1\nRz(0) q1\nCNOT "
      "q1,q0\nCNOT q0,q1\nRx(0) q0\nRx(0) q1\nRy(0) q0\nRz(0) q0\nRy(0) "
      "q1\nRz(0) q1\nCNOT q1,q0\nCNOT q0,q1\nRx(1.5708) q0\n";

  EXPECT_EQ(evaledCirc->toString(), expectedCirc);
}

TEST(QMLTester, test_QMLExecutor_run_1) {
  /***
  Tests the run method and the getStats methods in the QMLExecutor class

  Input state: |00>
  All weights set to 0
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
  /***./CI
  Tests the run method and the getStats methods in the QMLExecutor class

  Input state: |++>
  Input weights: Set to pi/2
  Variational weights: Set to pi/4
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

TEST(QMLTester, test_QMLExecutor_run_reupload) {
  /***
  Tests the functionality of the ParamCirc class to run with reuploaded
  parameters.
  ***/
  size_t numQubits = 2;
  size_t numAnsatzRepetitions = 2;
  qb::qml::ParamCirc circuit = qb::qml::ParamCirc(numQubits);
  for (size_t i = 0; i < numQubits; i++) {
    circuit.RX(i, "input");
  }
  for (size_t i = 0; i < numAnsatzRepetitions; i++) {
    if (i == 1) {
      circuit.reupload();
    }
    for (size_t j = 0; j < numQubits; j++) {
      circuit.RY(j, "variational");
      circuit.RZ(j, "variational");
    }
    circuit.CNOT(numQubits - 1, 0);
    for (std::size_t qubit = 0; qubit < numQubits - 1; qubit++) {
      circuit.CNOT(qubit, qubit + 1);
    }
  }

  circuit.MeasureAll(numQubits);
  std::vector<double> inputVec(circuit.getNumInputs(), M_PI_2);
  std::vector<double> paramVec(circuit.getNumParams(), M_PI_4);
  qb::qml::QMLExecutor exec(circuit, inputVec, paramVec);
  exec.setSeed(1000);
  exec.run();
  auto shots = exec.getStats();
  EXPECT_NEAR(shots[0], 0.124, 0.02);
  EXPECT_NEAR(shots[1], 0.1328, 0.02);
  EXPECT_NEAR(shots[2], 0.7158, 0.02);
  EXPECT_NEAR(shots[3], 0.0273, 0.02);
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
      {0.000000, 0.000000, 0.000000, 0.000000},
      {-0.038086, 0.038086, 0.213867, -0.213867},
      {0.007812, 0.000488, -0.005371, -0.002930},
      {-0.291504, 0.042969, 0.165039, 0.083496},
      {-0.089355, -0.108398, 0.112793, 0.084961},
      {0.000000, 0.000000, 0.000000, 0.000000},
      {0.022949, -0.022949, 0.123047, -0.123047},
      {0.000000, 0.000000, 0.000000, 0.000000}};

  for (size_t i = 0; i < numParamGates; i++) {
    for (size_t j = 0; j < numOutputs; j++) {
      EXPECT_NEAR(gradients[i][j], expectedGrad[i][j], 1e-5);
    }
  }
  // std::cout << "{";
  // for (size_t i = 0; i < numParamGates; i++) {
  //   std::cout << "{";
  //   for (size_t j = 0; j < numOutputs; j++) {
  //     // EXPECT_NEAR(gradients[i][j], expectedGrad[i][j], 1e-5);
  //     std::cout << std::to_string(gradients[i][j]) << ",";
  //   }
  //   std::cout <<"},\n";
  // }
  // std::cout << "}\n";
}

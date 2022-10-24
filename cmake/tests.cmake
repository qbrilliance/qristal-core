include(add_gtest)

# Commandline interface test
add_gtest(coreCLI)
target_link_libraries(coreCLITester qb::core)

# Noise Model test
add_gtest(NoiseModel PATH noise_model)
target_link_libraries(NoiseModelTester qb::core)

# AWS backend
add_gtest(AWSAccelerator PATH aws_braket)
add_gtest(AwsOpenQasm3 PATH aws_braket)

# Algorithms
add_gtest(CanonicalAmplitudeEstimationAlgorithm PATH algorithms/amplitude_estimation)
add_gtest(MLAmplitudeEstimationAlgorithm PATH algorithms/amplitude_estimation)
add_gtest(ExponentialSearchAlgorithm PATH algorithms/exponential_search)

# Circuit builders
add_gtest(ExponentBuilder PATH circuit_builders)

# Circuits
set(circuit_tests
  PhaseEstimationCircuit
  CanonicalAmplitudeEstimationCircuit
  ComparatorCircuit
  AmplitudeAmplification
  EfficientEncodingCircuit
  UQPrime
  QPrime
  UPrime
  WPrime
  MultiControlledUWithAncillaCircuit
  EqualityCheckerCircuit
  ControlledSwapCircuit
  ControlledAdditionCircuit
  GeneralisedMCXCircuit
  CompareBeamOracleCircuit
  CompareBeamOracleGeneralCircuit
  InitRepeatFlag
  InverseCircuitCircuit
  QDBeamStatePrepCircuit
  AEtoMetricCircuit
  SuperpositionAdderCircuit
  PseudoTraceAmplitudeEstimationCircuit
  CU
  ControlledQAE
  SubtractionCircuit
  ControlledSubtractionCircuit
  PFDCircuit
  ControlledPFDCircuit
  CompareGTCircuit
  MultiplicationCircuit
  ControlledMultiplicationCircuit
  MeanValueFinderCircuit
)
foreach(test ${circuit_tests})
  add_gtest(${test} PATH circuits)
endforeach()

# Upgrade conan and build standalone qiskit-aer for mock testing
# TODO do we really want to do this?
execute_process(COMMAND ${Python_EXECUTABLE} -m pip install conan --upgrade --quiet)
include(ExternalProject)
ExternalProject_Add(qiskit-aer
  GIT_REPOSITORY    https://github.com/Qiskit/qiskit-aer.git
  GIT_TAG           0.10.4
  SOURCE_DIR        "${CMAKE_BINARY_DIR}/qiskit-aer-src"
  BINARY_DIR        "${CMAKE_BINARY_DIR}/qiskit-aer-build"
  CMAKE_ARGS    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}
)
set(QB_STANDALONE_AER_EXE ${CMAKE_CURRENT_BINARY_DIR}/bin/qasm_simulator)
configure_file(tests/lambda/fake_aer_server.py.in
  ${CMAKE_BINARY_DIR}/plugins/qb_lambda/tests/fake_aer_server.py
)
add_gtest(QBLambda PATH lambda)
add_dependencies(QBLambdaTester qiskit-aer)

add_gtest(QBSparseSim PATH sparse_simulator)

add_gtest(UCCSD PATH uccsd)
target_link_libraries(UCCSDTester cppitertools::cppitertools)

add_gtest(VqeGen PATH vqe)
target_link_libraries(VqeGenTester Eigen3_interface)

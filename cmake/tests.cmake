# Copyright (c) 2022 Quantum Brilliance Pty Ltd

configure_file(tests/lambda/fake_aer_server.py.in
  ${CMAKE_BINARY_DIR}/plugins/qb_lambda/tests/fake_aer_server.py
)

add_executable(CITests
  ##tests/algorithms/amplitude_estimation/MLAmplitudeEstimationAlgorithmTester.cpp # FAILS
  ##tests/aws_braket/AWSAcceleratorTester.cpp # FAILS
  ##tests/aws_braket/AwsOpenQasm3Tester.cpp # FAILS
  ##tests/lambda/QBLambdaTester.cpp # FAILS
  ##tests/noise_model/EmulatorNoiseModelTester.cpp # FAILS ; no emulator
  ##tests/vqe/VqeGenTester.cpp # FAILS ; [XACC Error] [xacc::getObservable()] Invalid observable type: pyscf
  ##tests/circuit_builders/ExponentBuilderTester.cpp # FAILS/PASSES (?) SLOW
  tests/XaccInitialisedTests.cpp
  tests/algorithms/amplitude_estimation/CanonicalAmplitudeEstimationAlgorithmTester.cpp
  #tests/algorithms/exponential_search/ExponentialSearchAlgorithmTester.cpp #randomly fails
  #tests/circuits/AEtoMetricCircuitTester.cpp #randomly fails
  tests/circuits/AmplitudeAmplificationTester.cpp
  tests/circuits/CUTester.cpp
  tests/circuits/CanonicalAmplitudeEstimationCircuitTester.cpp
  tests/circuits/ComparatorCircuitTester.cpp
  tests/circuits/CompareBeamOracleCircuitTester.cpp
  tests/circuits/CompareBeamOracleGeneralCircuitTester.cpp
  tests/circuits/CompareGTCircuitTester.cpp
  tests/circuits/ControlledAdditionCircuitTester.cpp
  tests/circuits/ControlledMultiplicationCircuitTester.cpp
  tests/circuits/ControlledSwapCircuitTester.cpp
  tests/circuits/EfficientEncodingCircuitTester.cpp
  tests/circuits/EqualityCheckerCircuitTester.cpp
  tests/circuits/GeneralisedMCXCircuitTester.cpp
  tests/circuits/InitRepeatFlagTester.cpp
  tests/circuits/InverseCircuitCircuitTester.cpp
  tests/circuits/MultiControlledUWithAncillaCircuitTester.cpp
  tests/circuits/MultiplicationCircuitTester.cpp
  tests/circuits/PFDCircuitTester.cpp
  tests/circuits/PhaseEstimationCircuitTester.cpp
  tests/circuits/QDBeamStatePrepCircuitTester.cpp
  tests/circuits/QPrimeTester.cpp
  tests/circuits/SubtractionCircuitTester.cpp
  tests/circuits/UPrimeTester.cpp
  tests/circuits/UQPrimeTester.cpp
  tests/circuits/WPrimeTester.cpp
  tests/coreCLITester.cpp
  tests/noise_model/NoiseModelTester.cpp
  tests/sparse_simulator/QBSparseSimTester.cpp
  tests/uccsd/UCCSDTester.cpp
  tests/optimization/vqeeTester.cpp
  tests/optimization/qaoaTester.cpp
  tests/optimization/qmlTester.cpp
  ##tests/circuits/ControlledPFDCircuitTester.cpp #SLOW(?)
  ##tests/circuits/ControlledQAETester.cpp #SLOW (?)
  ##tests/circuits/ControlledSubtractionCircuitTester.cpp # SLOW(?)
  ##tests/circuits/MeanValueFinderCircuitTester.cpp #SLOW(?)
  ##tests/circuits/PseudoTraceAmplitudeEstimationCircuitTester.cpp # SLOW (?)
  ##tests/circuits/SuperpositionAdderCircuitTester.cpp # SLOW (?)
  tests/qcstack/QuantumBrillianceRemoteAcceleratorTester.cpp # unit-test harness for Qristal-Qcstack server interface. 
  tests/tket/tketTester.cpp
  tests/transpilationTester.cpp
  tests/qobj/qobjTester.cpp
  tests/jensen_shannon.cpp
  tests/QuantumBrillianceAcceleratorTester.cpp
  tests/sessionTester.cpp
)

add_test(NAME ci_tester COMMAND CITests)

target_link_libraries(CITests
  PRIVATE
    qb::core
    cppitertools::cppitertools
)

set_target_properties(CITests
  PROPERTIES
    BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR};${XACC_ROOT}/lib"
)
target_compile_definitions(CITests PRIVATE TKET_TEST_RESOURCE_DIR="${PROJECT_SOURCE_DIR}/tests/tket/resources")
add_dependencies(CITests qasm_simulator)

# Install assets needed for defining tests downstream
install(
  FILES tests/XaccInitialisedTests.cpp
  DESTINATION ${CMAKE_INSTALL_PREFIX}/tests
)

# add CITests without GPU
option(BUILD_TESTS_WITHOUT_GPU "build tests that do not use GPU." OFF)

# Adding CUDAQ tests
# Note: the test requires extra deps and C++20; hence making it a standalone test suite rather than combining with the overall CITests.
if (WITH_CUDAQ)
  add_executable(CudaqCITests tests/cudaq/CudaqTester.cpp)
  set_property(TARGET CudaqCITests PROPERTY CXX_STANDARD 20)
  target_link_options(CudaqCITests PRIVATE -Wl,--no-as-needed)
  target_link_libraries(CudaqCITests PUBLIC qb::core)
  include(CheckLanguage)
  check_language(CUDA)
  if(CMAKE_CUDA_COMPILER AND NOT BUILD_TESTS_WITHOUT_GPU)
    message(STATUS "CUDA language found. Enable CUDAQ GPU tests.")
    target_compile_definitions(CudaqCITests PUBLIC ENABLE_CUDA_TESTS)
  endif()
  # Add the test
  add_test(NAME cudaq_ci_test COMMAND CudaqCITests)
endif()

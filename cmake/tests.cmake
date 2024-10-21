# Copyright (c) Quantum Brilliance Pty Ltd

# When you add a new test, please keep the alphabetical ordering!
add_executable(CITests
  #tests/advice/QPrimeTester.cpp
  #tests/advice/UPrimeTester.cpp
  #tests/advice/UQPrimeTester.cpp
  #tests/advice/WPrimeTester.cpp
  tests/algorithms/amplitude_estimation/CanonicalAmplitudeEstimationAlgorithmTester.cpp
  #tests/algorithms/amplitude_estimation/MLAmplitudeEstimationAlgorithmTester.cpp
  tests/algorithms/exponential_search/ExponentialSearchAlgorithmTester.cpp
  tests/benchmark/metrics/CircuitFidelityTester.cpp
  tests/benchmark/metrics/QuantumProcessFidelityTester.cpp
  tests/benchmark/metrics/QuantumStateFidelityTester.cpp
  tests/benchmark/metrics/ConfusionMatrixTester.cpp
  tests/benchmark/workflows/PyGSTiBenchmarkTester.cpp
  tests/benchmark/workflows/QuantumProcessTomographyTester.cpp
  tests/benchmark/workflows/QuantumStateTomographyTester.cpp
  tests/benchmark/workflows/RotationSweepTester.cpp
  tests/benchmark/workflows/SPAMBenchmarkTester.cpp
  tests/circuit_builders/ExponentBuilderTester.cpp
  tests/circuit_builders/ParametrizedCircuitTester.cpp
  #tests/circuit_builders/RyEncodingTester.cpp
  tests/circuits/AEtoMetricCircuitTester.cpp
  tests/circuits/AmplitudeAmplificationTester.cpp
  tests/circuits/CUTester.cpp
  tests/circuits/CanonicalAmplitudeEstimationCircuitTester.cpp
  tests/circuits/ComparatorCircuitTester.cpp
  tests/circuits/CompareBeamOracleCircuitTester.cpp
  tests/circuits/CompareBeamOracleGeneralCircuitTester.cpp
  tests/circuits/CompareGTCircuitTester.cpp
  tests/circuits/ControlledAdditionCircuitTester.cpp
  tests/circuits/ControlledMultiplicationCircuitTester.cpp
  #tests/circuits/ControlledPFDCircuitTester.cpp
  #tests/circuits/ControlledQAETester.cpp
  #tests/circuits/ControlledSubtractionCircuitTester.cpp
  tests/circuits/ControlledSwapCircuitTester.cpp
  tests/circuits/EfficientEncodingCircuitTester.cpp
  tests/circuits/EqualityCheckerCircuitTester.cpp
  tests/circuits/GeneralisedMCXCircuitTester.cpp
  tests/circuits/InitRepeatFlagTester.cpp
  tests/circuits/InverseCircuitCircuitTester.cpp
  #tests/circuits/MeanValueFinderCircuitTester.cpp
  tests/circuits/MultiControlledUWithAncillaCircuitTester.cpp
  tests/circuits/MultiplicationCircuitTester.cpp
  tests/circuits/PFDCircuitTester.cpp
  tests/circuits/PhaseEstimationCircuitTester.cpp
  #tests/circuits/PseudoTraceAmplitudeEstimationCircuitTester.cpp
  tests/circuits/QDBeamStatePrepCircuitTester.cpp
  tests/circuits/QPrimeTester.cpp
  tests/circuits/SubtractionCircuitTester.cpp
  #tests/circuits/SuperpositionAdderCircuitTester.cpp
  tests/circuits/UPrimeTester.cpp
  tests/circuits/UQPrimeTester.cpp
  tests/circuits/WPrimeTester.cpp
  tests/misc_cpp/XaccInitialisedTests.cpp
  tests/misc_cpp/backendTester.cpp
  tests/misc_cpp/coreCLITester.cpp
  tests/misc_cpp/error_mitigation.cpp
  tests/misc_cpp/jensen_shannon.cpp
  tests/misc_cpp/sessionTester.cpp
  tests/misc_cpp/transpilationTester.cpp
  tests/noise_model/NoiseModelTester.cpp
  tests/optimization/qaoaTester.cpp
  tests/optimization/vqeeTester.cpp
  tests/qobj/qobjTester.cpp
  tests/sparse_simulator/QBSparseSimTester.cpp
  #tests/test_async/test_async.cpp
  tests/uccsd/UCCSDTester.cpp
  tests/vqe/VqeGenTester.cpp
)

add_executable(HardwareTests
  tests/misc_cpp/XaccInitialisedTests.cpp
  tests/qcstack/qbqpuTester.cpp
)

# This needs to be a separate test due to problems caused by restarting the python interpreter after it completes.
# See "Interpreter lifetime" at https://pybind11.readthedocs.io/en/stable/advanced/embedding.html for more info.
add_executable(BraketTests
  tests/misc_cpp/XaccInitialisedTests.cpp
  tests/aws_braket/AWSBraketHostedTester.cpp
)

add_test(NAME ci_tester COMMAND CITests)
add_test(NAME ci_hardware_tester COMMAND HardwareTests)
add_test(NAME ci_braket_tester COMMAND BraketTests)

target_link_libraries(CITests
  PRIVATE
    qristal::core
    cppitertools::cppitertools
)
target_link_libraries(HardwareTests
  PRIVATE
    qristal::core
    cppitertools::cppitertools
)
target_link_libraries(BraketTests
  PRIVATE
    qristal::core
    cppitertools::cppitertools
)

set_target_properties(CITests HardwareTests BraketTests
  PROPERTIES
    BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/${qristal_core_LIBDIR};${XACC_ROOT}/lib"
)
add_dependencies(CITests qasm_simulator)

# Install assets needed for defining tests downstream
install(
  FILES tests/misc_cpp/XaccInitialisedTests.cpp
  DESTINATION ${CMAKE_INSTALL_PREFIX}/tests
)

# add CITests using Tket
if (WITH_TKET)
  add_executable(TketTests
    tests/misc_cpp/XaccInitialisedTests.cpp
    tests/tket/tketTester.cpp
  )
  add_test(NAME ci_tket_tester COMMAND TketTests)
  target_link_libraries(TketTests
    PRIVATE
      qristal::core
      cppitertools::cppitertools
  )
  set_target_properties(TketTests
    PROPERTIES
      BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/${qristal_core_LIBDIR};${XACC_ROOT}/lib"
      COMPILE_DEFINITIONS TKET_TEST_RESOURCE_DIR="${PROJECT_SOURCE_DIR}/tests/tket/resources"
  )
endif()

# add CITests without GPU
option(BUILD_TESTS_WITHOUT_GPU "Build only tests that do not use GPUs." OFF)

# Adding CUDAQ tests
# Note: the test requires extra deps and C++20; hence making it a standalone test suite rather than combining with the overall CITests.
if (WITH_CUDAQ)
  add_executable(CudaqCITests tests/cudaq/CudaqTester.cpp)
  set_property(TARGET CudaqCITests PROPERTY CXX_STANDARD 20)
  target_link_options(CudaqCITests PRIVATE -Wl,--no-as-needed)
  target_link_libraries(CudaqCITests PUBLIC qristal::core)
  include(CheckLanguage)
  check_language(CUDA)
  if(CMAKE_CUDA_COMPILER AND NOT BUILD_TESTS_WITHOUT_GPU)
    message(STATUS "CUDA language found. Enable CUDAQ GPU tests.")
    target_compile_definitions(CudaqCITests PUBLIC ENABLE_CUDA_TESTS)
  endif()
  # Add the test
  add_test(NAME cudaq_ci_test COMMAND CudaqCITests)
endif()

if (WITH_PROFILING)
  add_executable(ProfilingCITests tests/benchmark/workflows/RuntimeAnalyzerTester.cpp)
  set_property(TARGET ProfilingCITests PROPERTY CXX_STANDARD 20)
  target_link_options(ProfilingCITests PRIVATE -Wl,--no-as-needed)
  target_link_libraries(ProfilingCITests PUBLIC qristal::core)
  add_test(NAME profiling_ci_test COMMAND ProfilingCITests)
endif()

# Copyright (c) 2022 Quantum Brilliance Pty Ltd

find_program(QB_STANDALONE_AER_EXE qasm_simulator)

if(${QB_STANDALONE_AER_EXE} STREQUAL "QB_STANDALONE_AER_EXE-NOTFOUND")
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
else()
  add_custom_target(qiskit-aer)
endif()

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
  ##tests/circuits/ControlledPFDCircuitTester.cpp #SLOW(?)
  ##tests/circuits/ControlledQAETester.cpp #SLOW (?)
  ##tests/circuits/ControlledSubtractionCircuitTester.cpp # SLOW(?)
  ##tests/circuits/MeanValueFinderCircuitTester.cpp #SLOW(?)
  ##tests/circuits/PseudoTraceAmplitudeEstimationCircuitTester.cpp # SLOW (?)
  ##tests/circuits/SuperpositionAdderCircuitTester.cpp # SLOW (?)
)

add_dependencies(CITests qiskit-aer)

add_test(NAME ci_tester COMMAND CITests)

target_link_libraries(CITests
  PRIVATE
	GTest::gtest
	qb::core_headers
	xacc::xacc
	xacc::quantum_gate
	cpr
	qb::core
	cppitertools::cppitertools
	Eigen3::Eigen
)

set_target_properties(CITests
  PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${XACC_ROOT}/lib"
    BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${XACC_ROOT}/lib"
)

# Copyright (c) 2022 Quantum Brilliance Pty Ltd

add_qbos_test(PhaseEstimationCircuit)
target_link_libraries(PhaseEstimationCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(CanonicalAmplitudeEstimationCircuit)
target_link_libraries(CanonicalAmplitudeEstimationCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(ComparatorCircuit)
target_link_libraries(ComparatorCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(AmplitudeAmplification)
target_link_libraries(AmplitudeAmplificationTester xacc::xacc xacc::quantum_gate)

add_qbos_test(EfficientEncodingCircuit)
target_link_libraries(EfficientEncodingCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(UQPrime)
target_link_libraries(UQPrimeTester xacc::xacc xacc::quantum_gate)

add_qbos_test(QPrime)
target_link_libraries(QPrimeTester xacc::xacc xacc::quantum_gate)

add_qbos_test(UPrime)
target_link_libraries(UPrimeTester xacc::xacc xacc::quantum_gate)

add_qbos_test(WPrime)
target_link_libraries(WPrimeTester xacc::xacc xacc::quantum_gate)

add_qbos_test(MultiControlledUWithAncillaCircuit)
target_link_libraries(MultiControlledUWithAncillaCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(EqualityCheckerCircuit)
target_link_libraries(EqualityCheckerCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(ControlledSwapCircuit)
target_link_libraries(ControlledSwapCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(ControlledAdditionCircuit)
target_link_libraries(ControlledAdditionCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(GeneralisedMCXCircuit)
target_link_libraries(GeneralisedMCXCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(CompareBeamOracleCircuit)
target_link_libraries(CompareBeamOracleCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(CompareBeamOracleGeneralCircuit)
target_link_libraries(CompareBeamOracleGeneralCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(InitRepeatFlag)
target_link_libraries(InitRepeatFlagTester xacc::xacc xacc::quantum_gate)

add_qbos_test(InverseCircuitCircuit)
target_link_libraries(InverseCircuitCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(QDBeamStatePrepCircuit)
target_link_libraries(QDBeamStatePrepCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(DecoderKernelCircuit)
target_link_libraries(DecoderKernelCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(AEtoMetricCircuit)
target_link_libraries(AEtoMetricCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(SuperpositionAdderCircuit)
target_link_libraries(SuperpositionAdderCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(PseudoTraceAmplitudeEstimationCircuit)
target_link_libraries(PseudoTraceAmplitudeEstimationCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(CU)
target_link_libraries(CUTester xacc::xacc xacc::quantum_gate)

add_qbos_test(ControlledQAE)
target_link_libraries(ControlledQAETester xacc::xacc xacc::quantum_gate)

add_qbos_test(SubtractionCircuit)
target_link_libraries(SubtractionCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(ControlledSubtractionCircuit)
target_link_libraries(ControlledSubtractionCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(PFDCircuit)
target_link_libraries(PFDCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(ControlledPFDCircuit)
target_link_libraries(ControlledPFDCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(CompareGTCircuit)
target_link_libraries(CompareGTCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(MultiplicationCircuit)
target_link_libraries(MultiplicationCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(ControlledMultiplicationCircuit)
target_link_libraries(ControlledMultiplicationCircuitTester xacc::xacc xacc::quantum_gate)

add_qbos_test(MeanValueFinderCircuit)
target_link_libraries(MeanValueFinderCircuitTester xacc::xacc xacc::quantum_gate)
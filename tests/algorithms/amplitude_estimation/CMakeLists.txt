# Copyright (c) 2022 Quantum Brilliance Pty Ltd

add_qbos_test(CanonicalAmplitudeEstimationAlgorithm)
target_link_libraries(CanonicalAmplitudeEstimationAlgorithmTester xacc::xacc xacc::quantum_gate)

add_qbos_test(MLAmplitudeEstimationAlgorithm)
target_link_libraries(MLAmplitudeEstimationAlgorithmTester xacc::xacc xacc::quantum_gate)
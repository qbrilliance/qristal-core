# Copyright (c) 2022 Quantum Brilliance Pty Ltd

add_qbos_test(ExponentialSearchAlgorithm)
target_link_libraries(ExponentialSearchAlgorithmTester xacc::xacc xacc::quantum_gate)
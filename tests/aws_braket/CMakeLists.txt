# Copyright (c) 2022 Quantum Brilliance Pty Ltd

add_qbos_test(AWSAccelerator)
target_link_libraries(AWSAcceleratorTester xacc::xacc xacc::quantum_gate)

add_qbos_test(AwsOpenQasm3)
target_link_libraries(AwsOpenQasm3Tester xacc::xacc xacc::quantum_gate)
target_include_directories(AwsOpenQasm3Tester PUBLIC ../accelerator)
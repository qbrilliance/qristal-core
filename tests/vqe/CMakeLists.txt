add_qbos_test(VqeGen)
target_link_libraries(VqeGenTester xacc::xacc xacc::quantum_gate)
target_include_directories(VqeGenTester PUBLIC ${XACC_DIR}/include/eigen)
add_qbos_test(UCCSD)
target_link_libraries(UCCSDTester xacc::xacc xacc::quantum_gate cppitertools::cppitertools)
target_include_directories(UCCSDTester PRIVATE ${PROJECT_SOURCE_DIR}/include)
# CMake Util to add C++ gtest
macro(add_qbos_test _TEST_NAME)
    add_executable(${_TEST_NAME}Tester ${_TEST_NAME}Tester.cpp)
    add_test(NAME qbos_${_TEST_NAME}Tester COMMAND ${_TEST_NAME}Tester)
    target_link_libraries(${_TEST_NAME}Tester gtest gtest_main)
endmacro()


add_executable(qbosExecutorTester
  qbosExecutorTester.cpp
)

# qbOS Executor test
add_test(NAME qbosExecutorTester COMMAND qbosExecutorTester)

target_link_libraries(qbosExecutorTester
  gtest
  gtest_main
  qb::core
)

# Noise Model test
add_executable(qbNoiseModelTester
  noise_model/NoiseModelTester.cpp
)

add_test(NAME qbNoiseModelTester COMMAND qbNoiseModelTester)

target_link_libraries(qbNoiseModelTester
  gtest
  gtest_main
  qb::core
)

# Add tests defined in sub-dirs
# TODO: hoist them up to this CMakeLists.txt 
add_subdirectory(sparse_simulator)
add_subdirectory(uccsd)
add_subdirectory(vqe)
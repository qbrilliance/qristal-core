# Copyright (c) 2022 Quantum Brilliance Pty Ltd

enable_testing()

# CMake Util to add C++ gtest
macro(add_gtest _TEST_NAME)
    cmake_parse_arguments(arg "" "PATH" "" "${ARGN}")
    set(arg_PATH tests/${arg_PATH})
    add_executable(${_TEST_NAME}Tester ${arg_PATH}/${_TEST_NAME}Tester.cpp)
    add_test(NAME qbos_${_TEST_NAME}Tester COMMAND ${_TEST_NAME}Tester)
    target_link_libraries(${_TEST_NAME}Tester
      gtest
      gtest_main
      qb::core_headers
      xacc::xacc
      xacc::quantum_gate
      cpr::cpr)
endmacro()

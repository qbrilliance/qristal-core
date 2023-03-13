# Copyright (c) 2022 Quantum Brilliance Pty Ltd

# Add a C++ example that gets compiled when building and installed unbuilt when installing
macro(add_example NAME)

  set(flags QODA)
  set(multiValueArgs SOURCES EXTRAS)
  cmake_parse_arguments(arg "${flags}" "" "${multiValueArgs}" "${ARGN}")
  if (SOURCES IN_LIST arg_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "[add_example]: SOURCES requires at least one value")
  endif()

  if (NOT arg_QODA)
    add_executable(${NAME} ${arg_SOURCES})
  #elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # Can only do this when building Qristal using clang; needs more testing even in that case.
  #  add_qoda_executable(${NAME} ${arg_SOURCES})
  endif()

  if(TARGET ${NAME})
    target_link_libraries(${NAME} PRIVATE qb::core)
    set_target_properties(${NAME} PROPERTIES BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${XACC_ROOT}/lib")
  endif()
  
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/${NAME}/CMakeLists.txt.in
                  ${CMAKE_BINARY_DIR}/configured_example_files/${NAME}/CMakeLists.txt
                  @ONLY)
  install(
    FILES
      ${arg_SOURCES}
      ${arg_EXTRAS}
      ${CMAKE_BINARY_DIR}/configured_example_files/${NAME}/CMakeLists.txt
    DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/cpp/${NAME}
  )

endmacro()


# Build and install examples by default
set(WITH_EXAMPLES ON CACHE BOOL "Enable examples")
if(WITH_EXAMPLES)

  add_example(demo1 SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/demo1/demo1.cpp)
  add_example(vqee SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/vqee/vqee_example.cpp)
  add_example(qaoa SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qaoa/qaoa_example.cpp)
  add_example(qbsdkcli SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qbsdkcli/qbsdkcli.cpp)
  if (WITH_QODA)
    set(CMAKE_CXX_STANDARD 20 CACHE STRING "Required C++ standard for QODA.")
    add_example(qoda_demo1 QODA SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qoda_demo1/demo1-qoda.cpp)
    add_example(qoda_vqe_cobyla QODA SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qoda_vqe_cobyla/vqe-cobyla-qoda.cpp)
    add_example(qoda_vqe_lbfgs QODA SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qoda_vqe_lbfgs/vqe-lbfgs-qoda.cpp)
    add_example(qoda_vqe_hydrogens QODA SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qoda_vqe_hydrogens/vqe-hydrogens-qoda.cpp EXTRAS ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qoda_vqe_hydrogens/gen_h_chain.py)
  endif()
  
  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/python
    DESTINATION ${CMAKE_INSTALL_PREFIX}/examples
  )

  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/README.md
    DESTINATION ${CMAKE_INSTALL_PREFIX}/examples
  )

  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qbsdkcli/README.md
    DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/cpp/qbsdkcli
  )

endif()

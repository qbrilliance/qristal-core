# qbos_core command-line executable
add_executable(qbos_core
  qbos_core.cpp
  ${version_file}
)

target_include_directories(qbos_core
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
    "${PROJECT_BINARY_DIR}"
    "${XACC_DIR}/include"
    "${XACC_DIR}/include/xacc"
    "${XACC_DIR}/include/quantum/gate"
    "${XACC_DIR}/include/gtest"
    "${XACC_DIR}/include/cppmicroservices4"
)

target_link_directories(qbos_core
  PUBLIC
    "${EXATN_DIR}/lib"
    "${EXATN_DIR}/plugins"
    "${XACC_DIR}/lib"
    "${XACC_DIR}/plugins"
)

target_link_libraries(qbos_core
  PRIVATE
    "${XACC_DIR}/lib/libgtest.so"
    "${XACC_DIR}/lib/libgtest_main.so"
    tnqvm-exatn
    xacc
    xacc-quantum-gate
    CppMicroServices
    qbnoise
    qbos-base
)

install(TARGETS qbos_core DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Share examples
# install(DIRECTORY ../../../010_examples/
#         DESTINATION share/qbos)

add_custom_command(
        TARGET qbos_core POST_BUILD
        COMMAND
      ${CMAKE_COMMAND} -E copy
      ${CMAKE_SOURCE_DIR}/qbos_cfg.json
      ${CMAKE_CURRENT_BINARY_DIR}/qbos_cfg.json)

add_custom_command(
        TARGET qbos_core POST_BUILD
        COMMAND
      ${CMAKE_COMMAND} -E copy
      ${CMAKE_SOURCE_DIR}/../test/qbqe_if_model.py
      ${CMAKE_INSTALL_PREFIX}/bin/qbqe_if_model.py)

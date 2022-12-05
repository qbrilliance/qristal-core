# QB SDK command-line executable
add_executable(qbsdkcli
  CLI_executable/absdkcli.cpp
  ${version_file}
)

target_include_directories(qbsdkcli
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

target_link_directories(qbsdkcli
  PUBLIC
    "${EXATN_DIR}/lib"
    "${EXATN_DIR}/plugins"
    "${XACC_DIR}/lib"
    "${XACC_DIR}/plugins"
)

target_link_libraries(qbsdkcli
  PRIVATE
    "${XACC_DIR}/lib/libgtest.so"
    "${XACC_DIR}/lib/libgtest_main.so"
    tnqvm-exatn
    xacc
    xacc-quantum-gate
    CppMicroServices
    qbnoise
    qb::core
)

install(TARGETS qbsdkcli DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_custom_command(
        TARGET qbsdkcli POST_BUILD
        COMMAND
      ${CMAKE_COMMAND} -E copy
      ${CMAKE_SOURCE_DIR}/sdk_cfg.json
      ${CMAKE_CURRENT_BINARY_DIR}/sdk_cfg.json)

add_custom_command(
        TARGET qbsdkcli POST_BUILD
        COMMAND
      ${CMAKE_COMMAND} -E copy
      ${CMAKE_SOURCE_DIR}/../test/session_if_model.py
      ${CMAKE_INSTALL_PREFIX}/bin/session_if_model.py)

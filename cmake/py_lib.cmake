pybind11_add_module (${PROJECT_NAME}
  python_module/core/pybindings.cpp
  python_module/core/session_getter_setter.cpp
  python_module/core/session_py_help_strings.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    qb::core
    #xacc-aer
    #xacc-ibm
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    INSTALL_RPATH "$ORIGIN;${CMAKE_INSTALL_RPATH};${XACC_ROOT}/lib"
)

install(
  TARGETS ${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qb
)

# Ensure that the resulting python module is picked up by Python without needing to modify the PYTHONPATH env variable.
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
                                                       -DXACC_ROOT=${XACC_ROOT} \
                                                       -DPYTHON_PACKAGES_PATH=${PYTHON_PACKAGES_PATH} \
                                                       -DPROJECT_NAME=${PROJECT_NAME} \
                                                       -P${CMAKE_CURRENT_SOURCE_DIR}/cmake/py_packages_path.cmake)")

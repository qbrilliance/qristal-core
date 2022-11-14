pybind11_add_module (${PROJECT_NAME}
  python_module/core/pybindings.cpp
  python_module/core/methods_getter_setter.cpp
  python_module/core/methods_py_help_strings.cpp
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

# This ensures that the resulting python module is picked up by Python without needing to modify the PYTHONPATH env variable.
install(CODE "file(WRITE ${LOCAL_PYTHON_SITE_PACKAGES}/qb${PROJECT_NAME}.pth \"${CMAKE_INSTALL_PREFIX}/lib\\n${XACC_ROOT}\")")

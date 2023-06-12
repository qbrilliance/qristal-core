pybind11_add_module (py${PROJECT_NAME}
  python_module/core/pybindings.cpp
  python_module/core/session_getter_setter.cpp
  python_module/core/session_py_help_strings.cpp
  python_module/core/py_placement.cpp
)

target_link_libraries(py${PROJECT_NAME}
  PRIVATE
    qb::core
)

target_include_directories(py${PROJECT_NAME} 
  PRIVATE
    python_module/core
)

set_target_properties(py${PROJECT_NAME}
  PROPERTIES
    INSTALL_RPATH "$ORIGIN;${CMAKE_INSTALL_RPATH};${XACC_ROOT}/lib"
    OUTPUT_NAME ${PROJECT_NAME}
)

install(
  TARGETS py${PROJECT_NAME}
  EXPORT py${PROJECT_NAME}Targets
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/qb
)

set(pyTargetsFile "py${PROJECT_NAME}Targets.cmake")
install(
  EXPORT py${PROJECT_NAME}Targets
  FILE ${pyTargetsFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
  NAMESPACE ${NAMESPACE}::
)

pybind11_add_module (py${PROJECT_NAME}
  python_module/src/pybindings.cpp
  python_module/src/session_getter_setter.cpp
  python_module/src/session_py_help_strings.cpp
  python_module/src/py_placement.cpp
  python_module/src/py_circuit_opt.cpp
  python_module/src/py_circuit_builder.cpp
  python_module/src/py_session.cpp
  python_module/src/py_job_handle.cpp
  python_module/src/py_stl_containers.cpp
  python_module/src/py_noise_model.cpp
  python_module/src/py_optimization_vqee.cpp
  python_module/src/py_optimization_qaoa_simple.cpp
  python_module/src/py_optimization_qaoa_recursive.cpp
  python_module/src/py_optimization_qaoa_warm_start.cpp
)

target_link_libraries(py${PROJECT_NAME}
  PRIVATE
    qb::core
)

target_include_directories(py${PROJECT_NAME} 
  PRIVATE
    python_module/include
)

set_target_properties(py${PROJECT_NAME}
  PROPERTIES
    INSTALL_RPATH "$ORIGIN;${CMAKE_INSTALL_RPATH};${XACC_ROOT}/lib"
    OUTPUT_NAME ${PROJECT_NAME}
)

install(
  TARGETS py${PROJECT_NAME}
  EXPORT py${PROJECT_NAME}Targets
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR}/qb
)

set(pyTargetsFile "py${PROJECT_NAME}Targets.cmake")
install(
  EXPORT py${PROJECT_NAME}Targets
  FILE ${pyTargetsFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
  NAMESPACE ${NAMESPACE}::
)

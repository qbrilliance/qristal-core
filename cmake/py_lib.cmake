pybind11_add_module (py${PROJECT_NAME}
  # Source files needed only for the Python module, in alphabetical order
  src/python/pybindings.cpp
  src/python/py_circuit_opt.cpp
  src/python/py_circuit_builder.cpp
  src/python/py_job_handle.cpp
  src/python/py_noise_model.cpp
  src/python/py_optimization_qaoa_recursive.cpp
  src/python/py_optimization_qaoa_simple.cpp
  src/python/py_optimization_qaoa_warm_start.cpp
  src/python/py_optimization_vqee.cpp
  src/python/py_placement.cpp
  src/python/py_session.cpp
  src/python/py_stl_containers.cpp
)

target_link_libraries(py${PROJECT_NAME}
  PRIVATE
    qristal::core
)

target_include_directories(py${PROJECT_NAME}
  PRIVATE
    include
)

set_target_properties(py${PROJECT_NAME}
  PROPERTIES
    INSTALL_RPATH "$ORIGIN;${CMAKE_INSTALL_RPATH};${XACC_ROOT}/lib"
    OUTPUT_NAME ${PROJECT_NAME}
)

install(
  TARGETS py${PROJECT_NAME}
  EXPORT py${PROJECT_NAME}Targets
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${qristal_core_LIBDIR}/qristal
)

set(pyTargetsFile "py${PROJECT_NAME}Targets.cmake")
install(
  EXPORT py${PROJECT_NAME}Targets
  FILE ${pyTargetsFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
  NAMESPACE ${NAMESPACE}::
)

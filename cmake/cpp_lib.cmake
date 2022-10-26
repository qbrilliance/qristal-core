set(source_files
  src/async_executor.cpp
  src/method_utils.cpp
  src/methods.cpp
  src/pretranspiler.cpp
  src/profiler.cpp
  #src/qbTketNoiseAwarePlacement.cpp
  src/QuantumBrillianceRemoteAccelerator.cpp
  src/thread_pool.cpp
  src/noise_model/noise_channel.cpp
  src/noise_model/noise_model.cpp
  src/noise_model/noise_model_factory.cpp
  # We need these in order to complete the linking
  # TODO: refactor methods.hpp to no longer contain getters/setters declarations...
  python_module/core/methods_getter_setter.cpp
  python_module/core/methods_py_help_strings.cpp
)

set(headers
  include/qb/core/qblib.inc
  include/qb/core/qpu_config.json
  include/qb/core/async_executor.hpp
  include/qb/core/cmake_variables.hpp
  include/qb/core/circuit_builder.hpp
  include/qb/core/circuit_builders/exponent.hpp
  include/qb/core/methods.hpp
  include/qb/core/method_utils.hpp
  include/qb/core/pretranspiler.hpp
  include/qb/core/profiler.hpp
  include/qb/core/QuantumBrillianceAccelerator.hpp
  include/qb/core/QuantumBrillianceRemoteAccelerator.hpp
  include/qb/core/QuantumBrillianceRemoteVisitor.hpp
  include/qb/core/QuantumBrillianceVisitor.hpp
  include/qb/core/remote_async_accelerator.hpp
  include/qb/core/thread_pool.hpp
  include/qb/core/typedefs.hpp
)

# Header-only interface target
set(LIBRARY_NAME ${NAMESPACE}${PROJECT_NAME})
add_library(${LIBRARY_NAME}_headers INTERFACE ${headers})
add_library(${NAMESPACE}::${PROJECT_NAME}_headers ALIAS ${LIBRARY_NAME}_headers)
target_include_directories(${LIBRARY_NAME}_headers
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${LIBRARY_NAME}_headers INTERFACE xacc::xacc xacc::quantum_gate)

# Compiled core library
if (NOT QBCORE_HEADER_ONLY)

  add_library(${LIBRARY_NAME} SHARED ${source_files} ${headers})
  add_library(${NAMESPACE}::${PROJECT_NAME} ALIAS ${LIBRARY_NAME})

  set_target_properties(${LIBRARY_NAME}
    PROPERTIES
      VERSION ${PROJECT_VERSION}
      FRAMEWORK TRUE
      INSTALL_RPATH "$ORIGIN;${CMAKE_INSTALL_RPATH};${XACC_ROOT}/lib"
  )

  # Include dependencies.
  target_include_directories(${LIBRARY_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
      # Util headers from Qiskit for Json serialization of complex types
      # TODO install these headers as a dependency rather than shipping them
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/deps/qiskit>
  )

  # Link dependencies
  target_link_libraries(${LIBRARY_NAME}
    PUBLIC
      xacc::xacc
      xacc::quantum_gate
      args
      nlohmann_json::nlohmann_json
      cpr::cpr
      Eigen3::Eigen
    )

  # Install the library
  install(
    TARGETS ${LIBRARY_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  )

  # Install the headers
  install(
    DIRECTORY include
    DESTINATION ${CMAKE_INSTALL_PREFIX}
  )

endif()


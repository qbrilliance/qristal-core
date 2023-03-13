set(source_files
  src/async_executor.cpp
  src/utils.cpp
  src/session.cpp
  src/pretranspiler.cpp
  src/profiler.cpp
  #src/qbTketNoiseAwarePlacement.cpp
  src/QuantumBrillianceRemoteAccelerator.cpp
  src/thread_pool.cpp
  src/optimization/vqee/case_generator.cpp
  src/optimization/vqee/vqee.cpp
  src/optimization/qaoa/qaoa_base.cpp
  src/optimization/qaoa/qaoa_simple.cpp
  src/optimization/qaoa/qaoa_recursive.cpp
  src/optimization/qaoa/qaoa_warmStart.cpp
  src/optimization/qaoa/qaoa_validators.cpp
  src/optimization/qml/qml.cpp
  # We need these in order to complete the linking
  # TODO: refactor session.hpp to no longer contain getters/setters declarations...
  python_module/core/session_getter_setter.cpp
  python_module/core/session_py_help_strings.cpp
)

if (WITH_QODA)
  # Add qoda-related code when building with QODA
  list(APPEND source_files src/qoda/qoda_session.cpp)
  list(APPEND source_files src/qoda/ir_converter.cpp)
endif()

set(headers
  include/qb/core/qblib.inc
  include/qb/core/qpu_config.json
  include/qb/core/async_executor.hpp
  include/qb/core/cmake_variables.hpp
  include/qb/core/circuit_builder.hpp
  include/qb/core/circuit_builders/exponent.hpp
  include/qb/core/circuit_builders/ry_encoding.hpp
  include/qb/core/session.hpp
  include/qb/core/utils.hpp
  include/qb/core/pretranspiler.hpp
  include/qb/core/profiler.hpp
  include/qb/core/QuantumBrillianceAccelerator.hpp
  include/qb/core/QuantumBrillianceRemoteAccelerator.hpp
  include/qb/core/QuantumBrillianceRemoteVisitor.hpp
  include/qb/core/QuantumBrillianceVisitor.hpp
  include/qb/core/remote_async_accelerator.hpp
  include/qb/core/thread_pool.hpp
  include/qb/core/typedefs.hpp
  include/qb/core/optimization/vqee/vqee.hpp
  include/qb/core/optimization/qaoa/qaoa.hpp
  include/qb/core/optimization/qml/qml.hpp
)

# Lightweight header-only interface target
add_library(lightweight_${PROJECT_NAME} INTERFACE ${headers})
add_library(${NAMESPACE}::lightweight_${PROJECT_NAME} ALIAS lightweight_${PROJECT_NAME})

target_include_directories(lightweight_${PROJECT_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lightweight_${PROJECT_NAME}
  INTERFACE
    qb::core::noise
    xacc::xacc
    xacc::quantum_gate
    nlohmann::json
    autodiff::autodiff
    # The following are not strictly needed, but save dependent packages from having to locate them themselves.
    taywee::args
    Eigen3::Eigen
    Python::Python
    pybind11::pybind11
    GTest::gtest
    GTest::gtest_main
)

# Install the library
install(
  TARGETS lightweight_${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  EXPORT lightweight_${PROJECT_NAME}Targets
)

# Install the Targets.cmake file for the library
set(cppTargetsFile "lightweight_${PROJECT_NAME}Targets.cmake")
install(
  EXPORT lightweight_${PROJECT_NAME}Targets
  FILE ${cppTargetsFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
  NAMESPACE ${NAMESPACE}::
)


# Compiled core library
if (NOT SUPPORT_EMULATOR_BUILD_ONLY)

  add_library(${PROJECT_NAME} SHARED ${source_files} ${headers})
  add_library(${NAMESPACE}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

  set_target_properties(${PROJECT_NAME}
    PROPERTIES
      VERSION ${PROJECT_VERSION}
      FRAMEWORK TRUE
      INSTALL_RPATH "$ORIGIN;${CMAKE_INSTALL_RPATH};${XACC_ROOT}/lib"
  )

  # Include dependencies.
  target_include_directories(${PROJECT_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )

  # Link dependencies
  target_link_libraries(${PROJECT_NAME}
    PUBLIC
      qb::core::noise
      xacc::xacc
      xacc::quantum_gate
      xacc::pauli
      taywee::args
      nlohmann::json
      autodiff::autodiff
    PRIVATE
      cpr
      CURL::libcurl
    INTERFACE
      # The following are not strictly needed, but save dependent packages locating them themselves.
      Eigen3::Eigen
      Python::Python
      pybind11::pybind11
      GTest::gtest
      GTest::gtest_main
   )

  # Enable QODA if available
  if (WITH_QODA)
    target_include_directories(${PROJECT_NAME}
      PUBLIC
        ${QODA_INCLUDE_DIR}
    )
    # QODA needs C++20 (std::span, std::stringview, etc.)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_QODA)

    # List of QODA libraries for linking
    list(APPEND 
      QODA_LIBS
        qoda 
        qoda-builder
        qoda-common
        qoda-qpud-client
        qoda-spin
    )
    
    # Find the paths to the corresponding *.so files for linking.
    foreach(QODA_LIB ${QODA_LIBS})
      find_library(${QODA_LIB}_FULL_PATH
        NAMES ${QODA_LIB}
        HINTS ${QODA_LIB_DIR}
      )

      if(NOT ${QODA_LIB}_FULL_PATH)
        message(FATAL_ERROR "\nUnable to find ${QODA_LIB} in ${QODA_LIB_DIR}. Please check your QODA installation.")
      endif()
      target_link_libraries(${PROJECT_NAME} PUBLIC ${${QODA_LIB}_FULL_PATH})
    endforeach()
  endif()

  # Install the library
  install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    EXPORT ${PROJECT_NAME}Targets
  )

  # Install the Targets.cmake file for the library
  set(cppTargetsFile "${PROJECT_NAME}Targets.cmake")
  install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${cppTargetsFile}
    DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
    NAMESPACE ${NAMESPACE}::
  )

endif()

# Install the headers
install(
  DIRECTORY include
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# Import the qoda_utilities so that they are available even in lightweight mode.
include(qoda_utilities)

# Generate the Dependencies.cmake file. 
set(dependenciesFile "coreDependencies.cmake")
set(deps_cmake_file "${CMAKE_CURRENT_BINARY_DIR}/${dependenciesFile}")
file(WRITE ${deps_cmake_file} "# Import all transitive dependencies of qbcore.\n# Auto-generated by cmake. ") 
file(APPEND ${deps_cmake_file} "\nif (NOT COMMAND add_qoda_executable)\
                                \n  include(${CMAKE_INSTALL_PREFIX}/cmake/qoda_utilities.cmake)\
                                \nendif()\
                                \nif (NOT COMMAND add_xacc_plugin)\
                                \n  include(${CMAKE_INSTALL_PREFIX}/cmake/xacc_utilities.cmake)\
                                \nendif()\
                                \nset(XACC_DIR ${XACC_DIR})\
                                \nset(nlohmann_json_DIR ${nlohmann_json_DIR})\
                                \nset(args_DIR ${args_DIR})\
                                \nset(pybind11_DIR ${pybind11_DIR})\
                                \nset(autodiff_DIR ${autodiff_DIR})")
if(EXISTS ${XACC_EIGEN_PATH})
  file(APPEND ${deps_cmake_file} "\nset(XACC_EIGEN_PATH ${XACC_DIR}/include/eigen)") 
else()
  file(APPEND ${deps_cmake_file} "\nset(Eigen3_DIR ${Eigen3_DIR})")
endif()
if(NOT "${GTest_DIR}" STREQUAL "GTest_DIR-NOTFOUND")
  file(APPEND ${deps_cmake_file} "\nset(GTest_DIR ${GTest_DIR})")
endif()
file(APPEND ${deps_cmake_file} "\ncache_install_path()\
                                \n  find_dependency(XACC)\
                                \nreset_install_path()\
                                \nfind_dependency(nlohmann_json ${nlohmann_json_VERSION})\
                                \nfind_dependency(args)\
                                \nfind_dependency(autodiff ${autodiff_VERSION})\
                                \nfind_dependency(pybind11 ${pybind11_VERSION})\
                                \nfind_dependency(Python 3 COMPONENTS Interpreter Development)\
                                \nif(NOT TARGET GTest::gtest)\
                                \n  find_dependency(GTest ${GTest_VERSION})\
                                \nendif()")
if(EXISTS ${XACC_EIGEN_PATH})
  file(APPEND ${deps_cmake_file} "\nadd_eigen_from_xacc()")
else()
  file(APPEND ${deps_cmake_file} "\nfind_dependency(Eigen3 ${Eigen3_VERSION})")
endif()

if (WITH_QODA) 
  file(APPEND ${deps_cmake_file} "\nadd_compile_definitions(WITH_QODA)")
endif()

# Install the Dependencies.cmake file for the library
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${dependenciesFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
)

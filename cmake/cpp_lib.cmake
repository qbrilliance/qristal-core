set(source_files
  src/async_executor.cpp
  src/utils.cpp
  src/session.cpp
  src/session_validators.cpp
  src/session_utils.cpp
  src/pretranspiler.cpp
  src/profiler.cpp
  src/QuantumBrillianceRemoteAccelerator.cpp
  src/QuantumBrillianceAccelerator.cpp
  src/QuantumBrillianceRemoteVisitor.cpp
  src/thread_pool.cpp
  src/optimization/vqee/case_generator.cpp
  src/optimization/vqee/vqee.cpp
  src/optimization/vqee/vqee_nlopt.cpp
  src/optimization/vqee/vqee_mlpack.cpp
  src/optimization/qaoa/qaoa_base.cpp
  src/optimization/qaoa/qaoa_simple.cpp
  src/optimization/qaoa/qaoa_recursive.cpp
  src/optimization/qaoa/qaoa_warmStart.cpp
  src/optimization/qaoa/qaoa_validators.cpp
  src/optimization/qml/qml.cpp
  src/passes/noise_aware_placement_pass.cpp
  src/passes/swap_placement_pass.cpp
  src/passes/circuit_opt_passes.cpp
  # We need these in order to complete the linking
  # TODO: refactor session.hpp to no longer contain getters/setters declarations...
  python_module/src/session_getter_setter.cpp
  python_module/src/session_py_help_strings.cpp
)

if (WITH_CUDAQ)
  # Add cudaq-related code when building with CUDAQ
  list(APPEND source_files src/cudaq/cudaq_session.cpp)
  list(APPEND source_files src/cudaq/ir_converter.cpp)
  list(APPEND source_files src/cudaq/sim_pool.cpp)
  list(APPEND source_files src/cudaq/cudaq_acc.cpp)
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
  include/qb/core/remote_async_accelerator.hpp
  include/qb/core/thread_pool.hpp
  include/qb/core/typedefs.hpp
  include/qb/core/optimization/vqee/vqee.hpp
  include/qb/core/optimization/qaoa/qaoa.hpp
  include/qb/core/optimization/qml/qml.hpp
  include/qb/core/passes/base_pass.hpp
  include/qb/core/passes/noise_aware_placement_config.hpp
  include/qb/core/passes/noise_aware_placement_pass.hpp
  include/qb/core/passes/swap_placement_pass.hpp
  include/qb/core/passes/circuit_opt_passes.hpp
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
    yaml-cpp::yaml-cpp
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
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR}
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
      yaml-cpp::yaml-cpp
      autodiff::autodiff
      CURL::libcurl
    PRIVATE
      cpr
    INTERFACE
      # The following are not strictly needed, but save dependent packages locating them themselves.
      Eigen3::Eigen
      Python::Python
      pybind11::pybind11
      GTest::gtest
      GTest::gtest_main
   )

  # Enable CUDAQ if available
  if (WITH_CUDAQ)
    target_include_directories(${PROJECT_NAME}
      PUBLIC
        ${CUDAQ_INCLUDE_DIR}
        # Remove this include path once https://github.com/NVIDIA/cuda-quantum/pull/94 is merged in.
        # (it was an oversight in CUDA Quantum lib)
        ${CUDAQ_INCLUDE_DIR}/common
    )
    # CUDAQ needs C++20 (std::span, std::stringview, etc.)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
    target_compile_definitions(${PROJECT_NAME} PUBLIC WITH_CUDAQ)

    # List of CUDAQ libraries for linking
    list(APPEND 
      CUDAQ_LIBS
        cudaq 
        cudaq-builder
        cudaq-common
        cudaq-em-qir
        cudaq-ensmallen
        cudaq-nlopt
        cudaq-platform-default
        cudaq-spin
        nvqir
    )
    
    # Find the paths to the corresponding *.so files for linking.
    foreach(CUDAQ_LIB ${CUDAQ_LIBS})
      find_library(${CUDAQ_LIB}_FULL_PATH
        NAMES ${CUDAQ_LIB}
        HINTS ${CUDAQ_LIB_DIR}
      )

      if(NOT ${CUDAQ_LIB}_FULL_PATH)
        message(FATAL_ERROR "\nUnable to find ${CUDAQ_LIB} in ${CUDAQ_LIB_DIR}. Please check your CUDAQ installation.")
      endif()
      target_link_libraries(${PROJECT_NAME} PUBLIC ${${CUDAQ_LIB}_FULL_PATH})
    endforeach()
  endif()

  # Install the library
  install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR}
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

# Import the cudaq_utilities so that they are available even in lightweight mode.
include(cudaq_utilities)

# Generate the afterCPMAddPackage.cmake file.
set(afterCPMAddPackageFile "afterCPMAddPackage.cmake")
set(outfile "${CMAKE_CURRENT_BINARY_DIR}/${afterCPMAddPackageFile}")
file(WRITE ${outfile} "# Set all cmake variables needed when using CPMAddPackage or find_package to import qbcore.\
                     \n# Auto-generated by cmake.\
                     \nset(XACC_DIR ${XACC_DIR})\
                     \ncache_install_path()\
                     \n  find_dependency(XACC)\
                     \nreset_install_path()\
                     \nset(qbcore_LIBDIR ${qbcore_LIBDIR})\
                     ")
                                                             
# Generate the coreDependencies.cmake file.
set(dependenciesFile "coreDependencies.cmake")
set(outfile "${CMAKE_CURRENT_BINARY_DIR}/${dependenciesFile}")
file(WRITE ${outfile} "# Import all transitive dependencies of qbcore needed when using find_package.\
                     \n# Auto-generated by cmake.\
                     \nset(nlohmann_json_DIR ${nlohmann_json_DIR})\
                     \nfind_dependency(nlohmann_json ${nlohmann_json_VERSION})\
                     \nset(yaml-cpp_DIR ${yaml-cpp_DIR})\
                     \nfind_dependency(yaml-cpp)\
                     \nset(args_DIR ${args_DIR})\
                     \nfind_dependency(args)\
                     \nfind_dependency(Python 3 COMPONENTS Interpreter Development)\
                     \nset(pybind11_DIR ${pybind11_DIR})\
                     \nfind_dependency(pybind11 ${pybind11_VERSION})\
                     \nset(autodiff_DIR ${autodiff_DIR})\
                     \nfind_dependency(autodiff ${autodiff_VERSION})\
                     \nset(CURL_NO_CURL_CMAKE ON)\
                     \nfind_dependency(CURL)") 
if(EXISTS ${XACC_EIGEN_PATH})
  file(APPEND ${outfile} "\nset(XACC_EIGEN_PATH ${XACC_DIR}/include/eigen)\
                          \nadd_eigen_from_xacc()")
else()
  file(APPEND ${outfile} "\nset(Eigen3_DIR ${Eigen3_DIR})\
                          \nfind_dependency(Eigen3 ${Eigen3_VERSION})")
endif()
if(NOT "${GTest_DIR}" STREQUAL "GTest_DIR-NOTFOUND")
  file(APPEND ${outfile} "\nset(GTest_DIR ${GTest_DIR})")
endif()
file(APPEND ${outfile} "\nfind_dependency(GTest ${GTest_VERSION})")
if (WITH_CUDAQ) 
  file(APPEND ${outfile} "\nadd_compile_definitions(WITH_CUDAQ)")
endif()

# Install both files
install(
  FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/${afterCPMAddPackageFile}
    ${CMAKE_CURRENT_BINARY_DIR}/${dependenciesFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
)

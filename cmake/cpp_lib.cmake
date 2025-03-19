set(source_files
  # C++ library source files in alphabetical order
  src/async_executor.cpp
  src/backend.cpp
  src/backend_utils.cpp
  src/circuit_builder.cpp
  src/primitives.cpp
  src/pretranspiler.cpp
  src/profiler.cpp
  src/session.cpp
  src/session_getter_setter.cpp
  src/session_validators.cpp
  src/thread_pool.cpp
  src/utils.cpp
  src/backends/qb_hardware/qb_qpu.cpp
  src/backends/qb_hardware/qb_visitor.cpp
  src/backends/qb_hardware/options.cpp
  src/backends/aws_braket/options.cpp
  src/benchmark/DataLoaderGenerator.cpp
  src/benchmark/workflows/PyGSTiBenchmark.cpp
  src/benchmark/workflows/RotationSweep.cpp
  src/benchmark/workflows/SPAMBenchmark.cpp
  src/benchmark/workflows/QuantumProcessTomography.cpp
  src/benchmark/metrics/QuantumStateDensity.cpp
  src/benchmark/metrics/QuantumProcessMatrix.cpp
  src/benchmark/metrics/QuantumStateFidelity.cpp
  src/benchmark/metrics/QuantumProcessFidelity.cpp
  src/optimization/qaoa/qaoa_base.cpp
  src/optimization/qaoa/qaoa_simple.cpp
  src/optimization/qaoa/qaoa_recursive.cpp
  src/optimization/qaoa/qaoa_warmStart.cpp
  src/optimization/vqee/case_generator.cpp
  src/optimization/vqee/vqee.cpp
  src/optimization/vqee/vqee_nlopt.cpp
  src/optimization/vqee/vqee_mlpack.cpp
  src/passes/circuit_opt_passes.cpp
  src/passes/noise_aware_placement_pass.cpp
  src/passes/swap_placement_pass.cpp
)

if (WITH_CUDAQ)
  # Cudaq-related C++ source files in alphabetical order
  list(APPEND source_files src/cudaq/cudaq_acc.cpp)
  list(APPEND source_files src/cudaq/cudaq_session.cpp)
  list(APPEND source_files src/cudaq/ir_converter.cpp)
  list(APPEND source_files src/cudaq/sim_pool.cpp)
endif()

set(headers
  # C++ library header files in alphabetical order
  include/qristal/core/qristal.inc
  include/qristal/core/async_executor.hpp
  include/qristal/core/backend.hpp
  include/qristal/core/backend_utils.hpp
  include/qristal/core/cmake_variables.hpp
  include/qristal/core/circuit_builder.hpp
  include/qristal/core/circuit_builders/exponent.hpp
  include/qristal/core/circuit_builders/ry_encoding.hpp
  include/qristal/core/pretranspiler.hpp
  include/qristal/core/primitives.hpp
  include/qristal/core/profiler.hpp
  include/qristal/core/remote_async_accelerator.hpp
  include/qristal/core/session.hpp
  include/qristal/core/session_utils.hpp
  include/qristal/core/thread_pool.hpp
  include/qristal/core/typedefs.hpp
  include/qristal/core/utils.hpp
  include/qristal/core/backends/qb_hardware/qb_qpu.hpp
  include/qristal/core/backends/qb_hardware/qb_visitor.hpp
  include/qristal/core/benchmark/Concepts.hpp
  include/qristal/core/benchmark/DataLoaderGenerator.hpp
  include/qristal/core/benchmark/Serializer.hpp
  include/qristal/core/benchmark/Task.hpp
  include/qristal/core/benchmark/metrics/CircuitFidelity.hpp
  include/qristal/core/benchmark/metrics/PyGSTiResults.hpp
  include/qristal/core/benchmark/metrics/QuantumProcessFidelity.hpp
  include/qristal/core/benchmark/metrics/QuantumProcessMatrix.hpp
  include/qristal/core/benchmark/metrics/QuantumStateDensity.hpp
  include/qristal/core/benchmark/metrics/QuantumStateFidelity.hpp
  include/qristal/core/benchmark/metrics/ConfusionMatrix.hpp
  include/qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp
  include/qristal/core/benchmark/workflows/QuantumProcessTomography.hpp
  include/qristal/core/benchmark/workflows/QuantumStateTomography.hpp
  include/qristal/core/benchmark/workflows/RotationSweep.hpp
  include/qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp
  include/qristal/core/benchmark/workflows/SPAMBenchmark.hpp
  include/qristal/core/optimization/qaoa/qaoa.hpp
  include/qristal/core/optimization/vqee/vqee.hpp
  include/qristal/core/passes/base_pass.hpp
  include/qristal/core/passes/circuit_opt_passes.hpp
  include/qristal/core/passes/noise_aware_placement_config.hpp
  include/qristal/core/passes/noise_aware_placement_pass.hpp
  include/qristal/core/passes/swap_placement_pass.hpp
  include/qristal/core/tools/zip_tool.hpp
)

if (WITH_PROFILING)
  # Add code that requires profiling library dependencies 
  list(APPEND headers include/qristal/core/benchmark/workflows/RuntimeAnalyzer.hpp)
endif()

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
    qristal::core::noise
    xacc::xacc
    xacc::quantum_gate
    xacc::pauli
    taywee::args
    nlohmann::json
    yaml-cpp::yaml-cpp
    autodiff::autodiff
    cereal::cereal
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

if (WITH_PROFILING)
 target_link_libraries(${PROJECT_NAME}
   PUBLIC
     cppuprofile::cppuprofile
 )
endif()

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
      cudaq-em-default
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

if(ADD_MPI)
  target_link_libraries(${PROJECT_NAME} PUBLIC MPI::MPI_CXX)
endif()

# Install the library
install(
  TARGETS ${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${qristal_core_LIBDIR}
  EXPORT ${PROJECT_NAME}Targets
)

# Install the default remote backend database
install(
  FILES "remote_backends.yaml"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# Install the Targets.cmake file for the library
set(cppTargetsFile "${PROJECT_NAME}Targets.cmake")
install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${cppTargetsFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
  NAMESPACE ${NAMESPACE}::
)

# Install the headers
install(
  DIRECTORY include
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# Generate the afterCPMAddPackage.cmake file.
set(afterCPMAddPackageFile "afterCPMAddPackage.cmake")
set(outfile "${CMAKE_CURRENT_BINARY_DIR}/${afterCPMAddPackageFile}")
file(WRITE ${outfile} "# Set all cmake variables needed when using CPMAddPackage or find_package to import qristal::core.\
                     \n# Auto-generated by cmake.\
                     \nset(XACC_DIR ${XACC_DIR})\
                     \ncache_install_path()\
                     \n  find_dependency(XACC)\
                     \nreset_install_path()\
                     \nset(qristal_core_LIBDIR ${qristal_core_LIBDIR})\
                     ")

# Generate the coreDependencies.cmake file.
set(dependenciesFile "coreDependencies.cmake")
set(outfile "${CMAKE_CURRENT_BINARY_DIR}/${dependenciesFile}")
file(WRITE ${outfile} "# Import all transitive dependencies of qristal::core needed when using find_package.\
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
                     \nset(cereal_DIR ${cereal_DIR})\
                     \nset(autodiff_DIR ${autodiff_DIR})\
                     \nfind_dependency(autodiff ${autodiff_VERSION})\
                     \nset(CURL_NO_CURL_CMAKE ON)\
                     \nfind_dependency(cereal ${cereal_VERSION})\
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
if (WITH_PROFILING)
  file(APPEND ${outfile} "\nset(cppuprofile_DIR ${cppuprofile_DIR})\
                          \nfind_dependency(cppuprofile)")
endif()

# Install both files
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${afterCPMAddPackageFile}
    ${CMAKE_CURRENT_BINARY_DIR}/${dependenciesFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
)

set(source_files
  # C++ library source files in alphabetical order
  src/backend_utils.cpp
  src/backend.cpp
  src/backends/hardware/qb/options.cpp
  src/backends/hardware/qb/qdk.cpp
  src/backends/hardware/qb/visitor.cpp
  src/backends/hardware/qb/visitor_ACZ.cpp
  src/backends/hardware/qb/visitor_CZ.cpp
  src/backends/sims/aws/braket/options.cpp
  src/benchmark/DataLoaderGenerator.cpp
  src/benchmark/metrics/QuantumProcessFidelity.cpp
  src/benchmark/metrics/QuantumProcessMatrix.cpp
  src/benchmark/metrics/QuantumStateDensity.cpp
  src/benchmark/metrics/QuantumStateFidelity.cpp
  src/benchmark/workflows/PyGSTiBenchmark.cpp
  src/benchmark/workflows/QuantumProcessTomography.cpp
  src/benchmark/workflows/RotationSweep.cpp
  src/benchmark/workflows/SPAMBenchmark.cpp
  src/circuit_builder.cpp
  src/jensen_shannon.cpp
  src/optimization/vqee/case_generator.cpp
  src/optimization/vqee/vqee_mlpack.cpp
  src/optimization/vqee/vqee_nlopt.cpp
  src/optimization/vqee/vqee.cpp
  src/passes/circuit_opt_passes.cpp
  src/passes/gate_deferral_pass.cpp
  src/passes/noise_aware_placement_pass.cpp
  src/passes/swap_placement_pass.cpp
  src/extension_loader.cpp
  src/pretranspiler.cpp
  src/primitives.cpp
  src/profiler.cpp
  src/session_getter_setter.cpp
  src/session_parameter_string_constants.cpp
  src/session.cpp
  src/thread_pool.cpp
  src/utils.cpp
)

if (WITH_CUDAQ)
  # Cudaq-related C++ source files in alphabetical order
  list(APPEND source_files src/cudaq/cudaq_acc.cpp)
  list(APPEND source_files src/cudaq/cudaq_session.cpp)
  list(APPEND source_files src/cudaq/ir_converter.cpp)
  list(APPEND source_files src/cudaq/sim_pool.cpp)
endif()

set(headers
  # C++ and OpenQASM header files in alphabetical order
  include/qristal/core/backend_utils.hpp
  include/qristal/core/backend.hpp
  include/qristal/core/backends/hardware/qb/qdk.hpp
  include/qristal/core/backends/hardware/qb/visitor.hpp
  include/qristal/core/backends/hardware/qb/visitor_ACZ.hpp
  include/qristal/core/backends/hardware/qb/visitor_CZ.hpp
  include/qristal/core/benchmark/Concepts.hpp
  include/qristal/core/benchmark/DataLoaderGenerator.hpp
  include/qristal/core/benchmark/metrics/CircuitFidelity.hpp
  include/qristal/core/benchmark/metrics/ConfusionMatrix.hpp
  include/qristal/core/benchmark/metrics/PyGSTiResults.hpp
  include/qristal/core/benchmark/metrics/QuantumProcessFidelity.hpp
  include/qristal/core/benchmark/metrics/QuantumProcessMatrix.hpp
  include/qristal/core/benchmark/metrics/QuantumStateDensity.hpp
  include/qristal/core/benchmark/metrics/QuantumStateFidelity.hpp
  include/qristal/core/benchmark/Serializer.hpp
  include/qristal/core/benchmark/Task.hpp
  include/qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp
  include/qristal/core/benchmark/workflows/QuantumProcessTomography.hpp
  include/qristal/core/benchmark/workflows/QuantumStateTomography.hpp
  include/qristal/core/benchmark/workflows/RotationSweep.hpp
  include/qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp
  include/qristal/core/benchmark/workflows/SPAMBenchmark.hpp
  include/qristal/core/benchmark/workflows/WorkflowAddins.hpp
  include/qristal/core/circuit_builder.hpp
  include/qristal/core/circuit_builders/exponent.hpp
  include/qristal/core/circuit_builders/ry_encoding.hpp
  include/qristal/core/circuit_language.hpp
  include/qristal/core/cmake_variables.hpp
  include/qristal/core/jensen_shannon.hpp
  include/qristal/core/optimization/vqee/vqee.hpp
  include/qristal/core/passes/base_pass.hpp
  include/qristal/core/passes/circuit_opt_passes.hpp
  include/qristal/core/passes/gate_deferral_pass.hpp
  include/qristal/core/passes/noise_aware_placement_config.hpp
  include/qristal/core/passes/noise_aware_placement_pass.hpp
  include/qristal/core/passes/swap_placement_pass.hpp
  include/qristal/core/extension_loader.hpp
  include/qristal/core/pretranspiler.hpp
  include/qristal/core/primitives.hpp
  include/qristal/core/profiler.hpp
  include/qristal/core/qristal.inc
  include/qristal/core/remote_async_accelerator.hpp
  include/qristal/core/session.hpp
  include/qristal/core/thread_pool.hpp
  include/qristal/core/utils.hpp
  include/qristal/core/wait_until.hpp
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
    range-v3::range-v3
  PRIVATE
    cpr::cpr
  INTERFACE
    # The following are not strictly needed, but save dependent packages locating them themselves.
    Eigen3::Eigen
    Python::Python
    pybind11::pybind11
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

set(targets ${PROJECT_NAME})
if(WITH_MPI)
  # Add in alphabetical order
  set(mpi_header_files
    include/qristal/core/mpi/message_types.hpp
    include/qristal/core/mpi/mpi_manager.hpp
    include/qristal/core/mpi/results_serialisation.hpp
    include/qristal/core/mpi/results_types.hpp
    include/qristal/core/mpi/workload_partitioning.hpp
  )

  # Add in alphabetical order
  set(mpi_source_files
    src/mpi/mpi_manager.cpp
    src/mpi/results_serialisation.cpp
    src/mpi/workload_partitioning.cpp
  )

  # Create core's MPI library
  set(MPI_SUPPORT_LIBRARY_NAME mpi_acceleration)
  add_library(${MPI_SUPPORT_LIBRARY_NAME} SHARED ${mpi_source_files} ${mpi_header_files})
  add_library(${NAMESPACE}::${PROJECT_NAME}::${MPI_SUPPORT_LIBRARY_NAME} ALIAS ${MPI_SUPPORT_LIBRARY_NAME})
  target_include_directories(${MPI_SUPPORT_LIBRARY_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
  )
  # Note: modern cmake only requires the MPI::MPI_CXX target to be linked.
  # All other MPI_* variables populated by find_package(MPI) are for older
  # cmake versions that do not create the MPI::MPI_CXX target.
  # See https://cliutils.gitlab.io/modern-cmake/chapters/packages/MPI.html
  target_link_libraries(${MPI_SUPPORT_LIBRARY_NAME}
    PUBLIC
      MPI::MPI_CXX
      range-v3::range-v3
  )
  list(APPEND targets ${MPI_SUPPORT_LIBRARY_NAME})

  # Add to core
  target_link_libraries(${PROJECT_NAME} PUBLIC ${MPI_SUPPORT_LIBRARY_NAME})
  target_compile_definitions(${PROJECT_NAME} PUBLIC USE_MPI)
endif()

# Install the library
install(
  TARGETS ${targets}
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
                     \nfind_dependency(cereal ${cereal_VERSION})\
                     \nset(range-v3_DIR ${range-v3_DIR})\
                     \nfind_dependency(range-v3)")
if(EXISTS ${XACC_EIGEN_PATH})
  file(APPEND ${outfile} "\nset(XACC_EIGEN_PATH ${XACC_DIR}/include/eigen)\
                          \nadd_eigen_from_xacc()")
else()
  file(APPEND ${outfile} "\nset(Eigen3_DIR ${Eigen3_DIR})\
                          \nfind_dependency(Eigen3 ${EIGEN3_VERSION_STRING})")
endif()
if (WITH_CUDAQ)
  file(APPEND ${outfile} "\nadd_compile_definitions(WITH_CUDAQ)")
endif()
if (WITH_PROFILING)
  file(APPEND ${outfile} "\nset(cppuprofile_DIR ${cppuprofile_DIR})\
                          \nfind_dependency(cppuprofile)")
endif()
if(WITH_MPI)
  file(APPEND ${outfile} "\nset(MPI_HOME ${MPI_HOME})
                          \nfind_dependency(MPI COMPONENTS CXX)")
endif()

# Install both files
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${afterCPMAddPackageFile}
    ${CMAKE_CURRENT_BINARY_DIR}/${dependenciesFile}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
)

# Set minimum cmake version
set(MIN_CMAKE_VERSION 3.20)
cmake_minimum_required(VERSION ${MIN_CMAKE_VERSION} FATAL_ERROR)

# Check whether to build with MPI support
option(WITH_MPI "Build Qristal with MPI support" OFF)
if(WITH_MPI OR DEFINED MPI_HOME)
  set(WITH_MPI ON CACHE BOOL "Cache variable for building Qristal with MPI support" FORCE)
endif()
# Controls whether to use MPI acceleration in third party dependencies.
set(ENABLE_MPI_IN_DEPS OFF CACHE BOOL "MPI acceleration within xacc")

# Get version number from git tag. Must be done before the project command is called.
find_package(Git)
if(GIT_FOUND)
  execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND git describe --tags --abbrev=0
    OUTPUT_VARIABLE PROJECT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE GIT_DESCRIBE_RESULT
  )

  if (NOT GIT_DESCRIBE_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to get git tag: git describe returned ${GIT_DESCRIBE_RESULT}")
  endif()

  string(REGEX REPLACE "^v(.*)" "\\1" PROJECT_VERSION "${PROJECT_VERSION}")
endif()

# Project
project(core
  VERSION ${PROJECT_VERSION}
  DESCRIPTION "Quantum Brilliance Qristal Core"
  LANGUAGES CXX
)

# Set path to cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Basic project config
include(base_config)

# Locate / set up retrieval of dependencies
include(dependencies)

# Noise modeling library (used by qristal::core)
include(noise_model_lib)

# core headers and libqristal_core.so shared library
include(cpp_lib)

# XACC plugin shared libraries
# installed to XACC framework for both core and emulator
include(xacc_plugins)

# Print out the installation directory.
message(STATUS "Installation directory for ${PROJECT_NAME} set to ${CMAKE_INSTALL_PREFIX}.")

# Nothing more to do if the caller only needs the headers
if (NOT SUPPORT_EMULATOR_BUILD_ONLY)

  # Tket middleware module (placement/circuit optimizer)
  include(add_tket)

  # Python core module (Python bindings for libqristal_core.so)
  include(py_lib)

  # Examples
  include(examples)

  # Tests
  include(tests)

  # Adds documentation targets (i.e. Doxygen and Sphinx targets) as dependencies of the default build targets.
  # This results in documentation targets being part of a normal build.
  # If enabled, it will **require** Doxygen and Sphinx (including the required extensions, e.g., Breathe and Exhale)
  option (BUILD_DOCS "Build the documentation." OFF)
  if (BUILD_DOCS)
    include(docs)
  endif()

endif()

# Install the xacc_utilities.cmake, py_packages_path.cmake and manifest.json template files
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/xacc_utilities.cmake
          ${CMAKE_CURRENT_SOURCE_DIR}/cmake/py_packages_path.cmake
          ${CMAKE_CURRENT_SOURCE_DIR}/cmake/manifest.json.in
    DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
)

# Ensure that all installed python modules are picked up by Python without needing to modify the PYTHONPATH env variable.
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
                                                       -Dqristal_core_LIBDIR=${qristal_core_LIBDIR} \
                                                       -DXACC_ROOT=${XACC_ROOT} \
                                                       -DPYTHON_EXECUTABLE=${Python_EXECUTABLE} \
                                                       -DPYTHON_PACKAGES_PATH=${PYTHON_PACKAGES_PATH} \
                                                       -DPROJECT_NAME=${PROJECT_NAME} \
                                                       -P${CMAKE_INSTALL_PREFIX}/cmake/py_packages_path.cmake)")

# Generate and install the qristal_core-config.cmake and qristal_core-config-version.cmake files
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/${NAMESPACE}_${PROJECT_NAME}-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${NAMESPACE}_${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${NAMESPACE}_${PROJECT_NAME}-config-version.cmake"
  COMPATIBILITY SameMinorVersion
)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${NAMESPACE}_${PROJECT_NAME}-config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/${NAMESPACE}_${PROJECT_NAME}-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
)

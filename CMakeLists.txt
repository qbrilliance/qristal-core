# Set minimum cmake version
set(MIN_CMAKE_VERSION 3.20)
cmake_minimum_required(VERSION ${MIN_CMAKE_VERSION} FATAL_ERROR)

# Get version number from git tag.  Must be done before the project command is called.
find_package(Git)
if(GIT_FOUND)
  execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND git describe --tags --abbrev=0
    OUTPUT_VARIABLE PROJECT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  string(REGEX REPLACE "^v(.*)" "\\1" PROJECT_VERSION "${PROJECT_VERSION}")
endif()

# Project
project(core
  VERSION ${PROJECT_VERSION}
  DESCRIPTION "Quantum Brilliance SDK Core"
  LANGUAGES CXX
)

# Set path to cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Basic project config
include(base_config)

# Locate / set up retrieval of dependencies
include(dependencies)

# Noise modeling library (used by qbcore)
include(noise_model_lib)

# core headers and libqbcore.so shared library
include(cpp_lib)

# Print out the installation directory.
message(STATUS "Installation directory for ${PROJECT_NAME} set to ${CMAKE_INSTALL_PREFIX}.")

# Nothing more to do if the caller only needs the headers
if (NOT SUPPORT_EMULATOR_BUILD_ONLY)

  # XACC plugin shared libraries
  include(xacc_plugins)

  # Python core module (Python bindings for libqbcore.so)
  include(py_lib)

  # Examples
  include(examples)

  # Tests
  include(tests)

  # Adds documentation targets (i.e. Doxygen and Sphinx targets) as dependencies of the default build targets.
  # This results in documentation targets being part of a normal build.
  # If enabled, it will **require** Doxygen and Sphinx (including the required extensions, e.g., Breathe and Exhale)
  option (QB_BUILD_DOCS "Build the documentation." OFF)
  if (QB_BUILD_DOCS)
    include(docs)
  endif()

endif()

# Ensure that all installed python modules are picked up by Python without needing to modify the PYTHONPATH env variable.
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
                                                       -DXACC_ROOT=${XACC_ROOT} \
                                                       -DPYTHON_PACKAGES_PATH=${PYTHON_PACKAGES_PATH} \
                                                       -DPROJECT_NAME=${PROJECT_NAME} \
                                                       -P${CMAKE_CURRENT_SOURCE_DIR}/cmake/py_packages_path.cmake)")

# Install the xacc_utilities.cmake file and manifest.json template
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/xacc_utilities.cmake
          ${CMAKE_CURRENT_SOURCE_DIR}/cmake/manifest.json.in
    DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
)

# Generate and install the qbcore-config.cmake and qbcore-config-version.cmake files
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/${NAMESPACE}${PROJECT_NAME}-config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${NAMESPACE}${PROJECT_NAME}-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${NAMESPACE}${PROJECT_NAME}-config-version.cmake"
  COMPATIBILITY SameMinorVersion
)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${NAMESPACE}${PROJECT_NAME}-config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/${NAMESPACE}${PROJECT_NAME}-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_PREFIX}"
)

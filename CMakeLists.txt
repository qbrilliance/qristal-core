# Set minimum cmake version
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Set path to cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Pull in CPM and other things needed for adding dependencies
include(add_dependency)

# Project
project(core
  VERSION ${PROJECT_VERSION}
  DESCRIPTION "Quantum Brilliance SDK Core"
  LANGUAGES CXX
)

# Basic project config
include(base_config)

# Save the version numbers for use in the code.
configure_file(cmake/cmake_variables.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/qb/core/cmake_variables.hpp)

# Locate / set up retrieval of dependencies
include(dependencies)

# core headers and libqbcore.so shared library
include(cpp_lib)

# Print out the installation directory.
message(STATUS "Installation directory for ${CMAKE_PROJECT_NAME} set to ${CMAKE_INSTALL_PREFIX}.")

# Nothing more to do if the caller only needs the headers
if (NOT QBCORE_HEADER_ONLY)

  # XACC plugin shared libraries
  include(xacc_plugins)

  # Python core module (Python bindings for libqbcore.so)
  include(py_lib)

  # Examples
  #add_subdirectory(examples)

  # Tests
  include(tests)

endif()

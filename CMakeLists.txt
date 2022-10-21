# Set minimum cmake version
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# Set path to cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# Time of running cmake
string(TIMESTAMP TODAY)

# Get version number from git tag
# TODO will not work when code tarball is downloaded instead of cloning git repo
find_package(Git)

if(GIT_FOUND)
  execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND git describe --tags --abbrev=0
    OUTPUT_VARIABLE FULL_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${FULL_VERSION}")
  string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${FULL_VERSION}")
  string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_REVISION "${FULL_VERSION}")
  string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-*(.*)" "\\1" VERSION_PATCH "${FULL_VERSION}")
endif()

set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}")
if(VERSION_PATCH)
  set(PROJECT_VERSION "${PROJECT_VERSION}.${VERSION_PATCH}")
endif()
set(SHORT_VERSION "Version: ${VERSION_PATCH} Build time ${TODAY}")
configure_file(cmake/cmake_variables.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/qb/core/cmake_variables.hpp)

# Project
project(core
  VERSION ${PROJECT_VERSION}
  DESCRIPTION "Quantum Brilliance SDK Core"
  LANGUAGES CXX
)

# Basic project config
include(base_config)

# Locate / set up retrieval of dependencies
include(dependencies)

# core headers and libqbcore.so shared library
include(cpp_lib)

# Nothing more to do if the caller only needs the headers
if (NOT QBCORE_HEADER_ONLY)

  # XACC plugin shared libraries
  include(xacc_plugins)

  # Python core module (Python bindings for libqbcore.so)
  include(py_lib)

  # Examples
  #add_subdirectory(examples)

  # Tests
  # Enable testing for the project
  enable_testing()
  add_subdirectory(tests)

endif()

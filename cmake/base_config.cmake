# Time of running cmake
string(TIMESTAMP TODAY)

# Get version number from git tag
# TODO will not work when code tarball is downloaded instead of cloning git repo!
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
  set(SHORT_VERSION "Version: ${VERSION_PATCH} Build time ${TODAY}")
else()
  set(SHORT_VERSION "Version: ${PROJECT_VERSION} Build time ${TODAY}")
endif()

# Set default installation dir to the build dir
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Installation path." FORCE)
endif()

# Set default RPATH to the lib dir of the installation dir
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib CACHE PATH "Search path for shared libraries to encode into binaries." FORCE)

# Project output target namespace
set(NAMESPACE qb)

# Set Compiler Support
set(CMAKE_CXX_STANDARD 17 CACHE STRING "Adopted C++ standard.")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunreachable-code -Wunused") # -Werror

# Generate a CompilationDatabase (compile_commands.json file) for our build,
# for use by code auto-complete, IDE IntelliSense, etc.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  # Print info on available build types if and only if this is top-level project.
  if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(STATUS "No build type selected. Defaulting to 'None'.
    Available options are:
      * -DCMAKE_BUILD_TYPE=None - For an unoptimized build with no assertions or debug info.
      * -DCMAKE_BUILD_TYPE=Release - For an optimized build with no assertions or debug info.
      * -DCMAKE_BUILD_TYPE=Debug - For an unoptimized build with assertions and debug info.
      * -DCMAKE_BUILD_TYPE=RelWithDebInfo - For an optimized build with no assertions but with debug info.
      * -DCMAKE_BUILD_TYPE=MinSizeRel - For a build optimized for size instead of speed.")
  endif()
  set(CMAKE_BUILD_TYPE "None" CACHE STRING "Type of build: None, Release, Debug, RelWithDebInfo or MinSizeRel." FORCE)
  set_property(
    CACHE
      CMAKE_BUILD_TYPE
    PROPERTY
      STRINGS "None" "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )
endif()

# Do we want to generate test coverage report (with gcov)?
set(WITH_COVERAGE OFF CACHE BOOL "Enable profiling for test coverage")

if(WITH_COVERAGE)
  message(STATUS "Enable code coverage profiling.")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

# Determine make command to use for external builds
include(ProcessorCount)
ProcessorCount(N)
if(CMAKE_MAKE_PROGRAM MATCHES "make$")
  set(MAKE_PARALLEL ${CMAKE_MAKE_PROGRAM} -j${N})
else()
  set(MAKE_PARALLEL ${CMAKE_MAKE_PROGRAM})
endif()

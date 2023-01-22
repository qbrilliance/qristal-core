# Set default installation dir to the build dir.  Must be done before calling add_dependency.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Installation path." FORCE)
endif()

# Set default RPATH to the lib dir of the installation dir.  Must be done after default installation dir is set.
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib CACHE PATH "Search path for shared libraries to encode into binaries." FORCE)

# Work out build type.  Must be done before calling add_dependency.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected. Defaulting to 'None'.
  Available options are:
    * -DCMAKE_BUILD_TYPE=None - For an unoptimized build with no assertions or debug info.
    * -DCMAKE_BUILD_TYPE=Release - For an optimized build with no assertions or debug info.
    * -DCMAKE_BUILD_TYPE=Debug - For an unoptimized build with assertions and debug info.
    * -DCMAKE_BUILD_TYPE=RelWithDebInfo - For an optimized build with no assertions but with debug info.
    * -DCMAKE_BUILD_TYPE=MinSizeRel - For a build optimized for size instead of speed.")
  set(CMAKE_BUILD_TYPE "None" CACHE STRING "Type of build: None, Release, Debug, RelWithDebInfo or MinSizeRel." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "None" "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Project output target namespace
set(NAMESPACE qb)

# Set Compiler Support
set(CMAKE_CXX_STANDARD 17 CACHE STRING "Adopted C++ standard.")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wunreachable-code -Wunused") # -Werror

# Add linker paths to installed binary rpaths
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Generate a CompilationDatabase (compile_commands.json file) for our build,
# for use by code auto-complete, IDE IntelliSense, etc.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Do we want to generate test coverage report (with gcov)?
set(WITH_COVERAGE OFF CACHE BOOL "Enable profiling for test coverage")

# Enable code coverage reporting
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

# Save the version numbers for use in the code.
configure_file(cmake/cmake_variables.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/qb/core/cmake_variables.hpp)

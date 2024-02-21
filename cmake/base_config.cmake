# Include the GNU standard installation directories
include(GNUInstallDirs)

# Set default installation dir to the build dir.  Must be done before calling add_dependency.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Installation path." FORCE)
endif()

# Set the library dir to the value of CMAKE_INSTALL_LIBDIR
set(qbcore_LIBDIR ${CMAKE_INSTALL_LIBDIR})

# Set default RPATH to the lib dir of the installation dir.  Must be done after default installation dir is set.
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR} CACHE PATH "Search path for shared libraries to encode into binaries." FORCE)

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

# Determine whether to permit optimisations based on the local architecture
option(COMPILE_FOR_LOCAL_ARCH OFF)
if(COMPILE_FOR_LOCAL_ARCH)
  message(STATUS "Enabling optimisations based on local architecture.")
else()
  message(STATUS "Using cross-compilation mode (no optimisations based on local architecture).")
endif()

# Parse INSTALL_MISSING
if (INSTALL_MISSING STREQUAL "CXX" OR INSTALL_MISSING STREQUAL "CPP")
  set(INSTALL_MISSING_CXX ON)
  set(INSTALL_MISSING_PYTHON OFF)
elseif (INSTALL_MISSING STREQUAL "Python" OR INSTALL_MISSING STREQUAL "PYTHON")
  set(INSTALL_MISSING_CXX OFF)
  set(INSTALL_MISSING_PYTHON ON)
else()
  set(INSTALL_MISSING_CXX ${INSTALL_MISSING})
  set(INSTALL_MISSING_PYTHON ${INSTALL_MISSING})
endif()

# Project output target namespace
set(NAMESPACE qb)

# Set Compiler Support
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set minimum compiler versions
set(MIN_CLANG_VERSION 16.0.6)
set(MIN_GNU_VERSION 11.4.0)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MIN_GNU_VERSION)
    message(FATAL_ERROR "Please use a later version of gcc. Qristal supports v${MIN_GNU_VERSION} and later, but you have ${CMAKE_CXX_COMPILER_VERSION}.")
  endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS MIN_CLANG_VERSION)
    message(FATAL_ERROR "Please use a later version of clang. Qristal supports v${MIN_CLANG_VERSION} and later, but you have ${CMAKE_CXX_COMPILER_VERSION}.")
  endif()
endif()

# Enable/disable warnings
if(WARNINGS)
  message(STATUS "Warnings enabled")
  add_compile_options(-Wall -Wextra -Wunreachable-code -Wunused) # -Werror
else()
  add_compile_options(-w)
  set(XACC_CMAKE_CXX_FLAGS "\"-DBOOST_DISABLE_PRAGMA_MESSAGE=ON -w\"")
  set(dashw_IF_NOT_WARNINGS "-w")
endif()

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

# Always prefer config mode of find_package to module mode
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

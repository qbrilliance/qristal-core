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

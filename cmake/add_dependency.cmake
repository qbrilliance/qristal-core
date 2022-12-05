# Copyright (c) 2022 Quantum Brilliance Pty Ltd

# Time of running cmake
string(TIMESTAMP TODAY)

# Get version number from git tag.  Must be done before the project command is called.
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

# Set default installation dir to the build dir.  Must be done before calling add_dependency.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Installation path." FORCE)
endif()

# Set default RPATH to the lib dir of the installation dir.  Must be done after default installation dir is set.
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib CACHE PATH "Search path for shared libraries to encode into binaries." FORCE)

# Work out build type.  Must be done before calling add_dependency.
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

# Include CPM for managing dependencies, and set it up to cache them in the deps folder. Must be done before calling add_dependency.
set(CPM_DOWNLOAD_VERSION 0.36.0)
set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_LIST_DIR}/../deps" CACHE PATH "Dependencies path.")
set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
  )
endif()
include(${CPM_DOWNLOAD_LOCATION})

# Add a dependent package using CPM, first looking to see if it has been installed already.
macro(add_dependency NAME VERSION)

  set(oneValueArgs
    FIND_PACKAGE_NAME
    FIND_PACKAGE_VERSION
    FIND_PACKAGE_ARGUMENTS
    FORCE
    GIT_TAG
    DOWNLOAD_ONLY
    GITHUB_REPOSITORY
    GITLAB_REPOSITORY
    BITBUCKET_REPOSITORY
    GIT_REPOSITORY
    SOURCE_DIR
    DOWNLOAD_COMMAND
    NO_CACHE
    GIT_SHALLOW
    EXCLUDE_FROM_ALL
    SOURCE_SUBDIR
  )

  set(multiValueArgs URL OPTIONS)

  cmake_parse_arguments(arg "" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")

  if (OPTIONS IN_LIST arg_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "[add_dependency]: OPTIONS requires at least one value")
  endif()
  if (URL IN_LIST arg_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "[add_dependency]: URL requires at least one value")
  endif()

  if (NOT arg_GIT_TAG)
    set(arg_GIT_TAG v${VERSION})
  endif()
  if (NOT arg_FIND_PACKAGE_NAME)
    set(arg_FIND_PACKAGE_NAME ${NAME})
  endif()

  if (arg_FIND_PACKAGE_VERSION)
    if(${arg_FIND_PACKAGE_VERSION} STREQUAL " ")
      find_package(${arg_FIND_PACKAGE_NAME} QUIET)
    else()
      find_package(${arg_FIND_PACKAGE_NAME} ${arg_FIND_PACKAGE_VERSION} QUIET)
    endif()
  else()
    find_package(${arg_FIND_PACKAGE_NAME} ${VERSION} QUIET)
  endif()

  if(${arg_FIND_PACKAGE_NAME}_FOUND)

    message(STATUS "System installation of ${NAME} found: version ${${arg_FIND_PACKAGE_NAME}_VERSION}")

  else()

    if(INSTALL_MISSING)

      CPMAddPackage(
        NAME ${NAME}
        VERSION ${VERSION}
        FORCE ${arg_FORCE}
        GIT_TAG ${arg_GIT_TAG}
        DOWNLOAD_ONLY ${arg_DOWNLOAD_ONLY}
        GITHUB_REPOSITORY ${arg_GITHUB_REPOSITORY}
        GITLAB_REPOSITORY ${arg_GITLAB_REPOSITORY}
        BITBUCKET_REPOSITORY ${arg_BITBUCKET_REPOSITORY}
        GIT_REPOSITORY ${arg_GIT_REPOSITORY}
        SOURCE_DIR ${arg_SOURCE_DIR}
        DOWNLOAD_COMMAND ${arg_DOWNLOAD_COMMAND}
        FIND_PACKAGE_ARGUMENTS ${arg_FIND_PACKAGE_ARGUMENTS}
        NO_CACHE ${arg_NO_CACHE}
        GIT_SHALLOW ${arg_GIT_SHALLOW}
        EXCLUDE_FROM_ALL ${arg_EXCLUDE_FROM_ALL}
        SOURCE_SUBDIR ${arg_SOURCE_SUBDIR}
        OPTIONS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}" ${arg_OPTIONS}
        URL ${arg_URL}
      )

    else()

      message(STATUS "System installation of ${NAME} version ${VERSION} not found.")
      # User says not to install; just keep track of what was missing.
      set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${NAME}")

    endif()

  endif()

endmacro()

# If any packages are missing, fail.
macro(check_missing)
  if(MISSING_DEPENDENCIES AND NOT INSTALL_MISSING)

    message("\nThe following dependencies were not found by cmake:")
    foreach(package ${MISSING_DEPENDENCIES})
      message("  ${package}")
    endforeach()
    message("To have cmake automatically install the missing packages, rerun with -DINSTALL_MISSING=ON.\n")
    message(FATAL_ERROR "Either rerun with -DINSTALL_MISSING=ON, or fix the search paths passed to cmake for finding installed dependencies.")

  endif()
endmacro()

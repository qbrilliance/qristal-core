# Copyright (c) Quantum Brilliance Pty Ltd

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

    if(INSTALL_MISSING_CXX)

      set(OPTION_LIST CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_Fortran_COMPILER CMAKE_BUILD_TYPE)
      foreach(option ${OPTION_LIST})
        if(${option})
          list(PREPEND arg_OPTIONS "${option}=${${option}}")
        endif()
      endforeach()
      list(PREPEND arg_OPTIONS "CMAKE_CXX_FLAGS=${dashw_IF_NOT_WARNINGS}")

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
        OPTIONS ${arg_OPTIONS}
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
  if(MISSING_DEPENDENCIES AND NOT (INSTALL_MISSING_CXX AND INSTALL_MISSING_PYTHON))

    message("\nThe following dependencies were not found by cmake:")
    foreach(package ${MISSING_DEPENDENCIES})
      message("  ${package}")
    endforeach()
    message("To have cmake automatically install the missing packages, rerun with -DINSTALL_MISSING=ON.\n")
    message(FATAL_ERROR "Either rerun with -DINSTALL_MISSING=ON, or fix the search paths passed to cmake for finding installed dependencies.")

  endif()
endmacro()

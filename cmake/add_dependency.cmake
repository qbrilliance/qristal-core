# Copyright (c) 2022 Quantum Brilliance Pty Ltd

# Add a dependent package, first looking to see if it has been installed already.
macro(add_dependency NAME VERSION)

  set(oneValueArgs
      CMAKE_PACKAGE_NAME
      FORCE
      GIT_TAG
      DOWNLOAD_ONLY
      GITHUB_REPOSITORY
      GITLAB_REPOSITORY
      BITBUCKET_REPOSITORY
      GIT_REPOSITORY
      SOURCE_DIR
      DOWNLOAD_COMMAND
      FIND_PACKAGE_ARGUMENTS
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
  if (NOT arg_CMAKE_PACKAGE_NAME)
    set(arg_CMAKE_PACKAGE_NAME ${NAME})
  endif()

  find_package(${arg_CMAKE_PACKAGE_NAME} ${VERSION} QUIET)

  if(${arg_CMAKE_PACKAGE_NAME}_FOUND)

    message("System installation of ${NAME} found: version ${${arg_CMAKE_PACKAGE_NAME}_VERSION}")

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

      # User says not to install; just keep track of what was missing.
      set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${NAME}")

    endif()

  endif()

endmacro()

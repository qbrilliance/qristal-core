# Add a dependent package using manual commands, first looking to see if it has been installed already.
macro(add_poorly_behaved_dependency NAME VERSION)

  set(flags
      UPDATE_SUBMODULES
  )

  set(oneValueArgs
      CMAKE_PACKAGE_NAME
      GIT_TAG
      GIT_REPOSITORY
  )

  set(multiValueArgs PREAMBLE OPTIONS)

  cmake_parse_arguments(arg "${flags}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")

  if (NOT arg_GIT_REPOSITORY)
    message(FATAL_ERROR "[add_poorly_behaved_dependency]: GIT_REPOSITORY is required.")
  endif()

  if (NOT arg_UPDATE_SUBMODULES)
    set(arg_UPDATE_SUBMODULES False)
  endif()

  if (OPTIONS IN_LIST arg_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "[add_poorly_behaved_dependency]: OPTIONS requires at least one value")
  endif()
  if (PREAMBLE IN_LIST arg_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "[add_poorly_behaved_dependency]: PREAMBLE requires at least one value")
  endif()

  if (arg_GIT_TAG)
    set(dir "${PROJECT_SOURCE_DIR}/deps/${NAME}/${VERSION}-${arg_GIT_TAG}")
  else()
    set(dir "${PROJECT_SOURCE_DIR}/deps/${NAME}/${VERSION}")
    set(arg_GIT_TAG v${VERSION})
  endif()
  if (NOT arg_CMAKE_PACKAGE_NAME)
    set(arg_CMAKE_PACKAGE_NAME ${NAME})
  endif()

  # Check if a previous installation was interrupted and left CMAKE_INSTALL_PREFIX in a corrupted state.
  if (DEFINED CACHE{DEP_CMAKE_INSTALL_PREFIX} AND CMAKE_INSTALL_PREFIX STREQUAL DEP_CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for QB SDK core." FORCE)
    unset(DEP_CMAKE_INSTALL_PREFIX CACHE)
  else()
    # Otherwise, make a backup of the current CMAKE_INSTALL_PREFIX, as the following detection/installation process will mess with it.
    set(CMAKE_INSTALL_PREFIX_BACKUP ${CMAKE_INSTALL_PREFIX} CACHE PATH "Backup copy of install path for QB SDK core (allows restoration after XACC overwrites CMAKE_INSTALL_PREFIX)." FORCE)
  endif()

  # Locate a system-installed version of CMAKE_PACKAGE_NAME.  Will work if the path to an installed CMAKE_PACKAGE_NAME is given as one of:
  #  - environment variable ${CMAKE_PACKAGE_NAME}_ROOT
  #  - cmake variable ${CMAKE_PACKAGE_NAME}_ROOT (set with -D at cmake invocation)
  #  - cmake variable ${CMAKE_PACKAGE_NAME} (set with -D at cmake invocation)
  find_package(${arg_CMAKE_PACKAGE_NAME} QUIET)

  if(${arg_CMAKE_PACKAGE_NAME}_FOUND AND "${${arg_CMAKE_PACKAGE_NAME}_VERSION}" STREQUAL "${VERSION}-${arg_GIT_TAG}")

    message(STATUS "System installation of ${NAME} found: version ${${arg_CMAKE_PACKAGE_NAME}_VERSION}")

  else()

    message(STATUS "System installation of ${NAME} v${VERSION}-${arg_GIT_TAG} not found.")

    if(INSTALL_MISSING)

      # Run any preamble functions supplied (for finding/installing dependencies)
      foreach(command ${arg_PREAMBLE})
        cmake_language(CALL ${command})
      endforeach()

      # Parse any options given into the actual invocation of cmake.
      set(cmake_invocation ${CMAKE_COMMAND})
      list(APPEND cmake_invocation "-B_deps/${NAME}/build" "_deps/${NAME}" "-DCMAKE_INSTALL_PREFIX=${dir}" "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
      foreach(option ${arg_OPTIONS})
        string(REGEX REPLACE " " "=" option "${option}")
        string(CONFIGURE ${option} option)
        list(APPEND cmake_invocation "-D${option}")
      endforeach()

      # Checkout, cmake, build and install the pacakge.
      message("   CMake will now download and install ${NAME} v${VERSION}.")
      execute_process(RESULT_VARIABLE result COMMAND git clone ${arg_GIT_REPOSITORY} _deps/${NAME})
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to clone git repository for ${NAME} v${VERSION} failed.  This is expected if e.g. you are disconnected from the internet.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${NAME} git checkout -q ${arg_GIT_TAG})
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to checkout git tag ${VERSION} of ${NAME} failed.")
      endif()
      if (arg_UPDATE_SUBMODULES)
        execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${NAME} git submodule init)
        if(NOT ${result} STREQUAL "0")
          message(FATAL_ERROR "Attempt to run git submodule init in ${NAME} failed.")
        endif()
        execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${NAME} git submodule update --init --recursive)
        if(NOT ${result} STREQUAL "0")
          message(FATAL_ERROR "Attempt to run git submodule update in ${NAME} failed.")
        endif()
      endif()
      set(DEP_CMAKE_INSTALL_PREFIX ${dir} CACHE PATH "Installation path of badly-behaved dependency => Installation is not yet complete." FORCE)
      execute_process(RESULT_VARIABLE result COMMAND ${cmake_invocation})
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Running cmake for ${NAME} failed.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${NAME}/build ${MAKE_PARALLEL} install)
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Building ${NAME} failed.")
      endif()
      message("   ...done.")

      # Check that it installed correctly, and import any relevant cmake variables.
      set(${arg_CMAKE_PACKAGE_NAME}_ROOT "${dir}")
      find_package(${arg_CMAKE_PACKAGE_NAME} REQUIRED)
      unset(DEP_CMAKE_INSTALL_PREFIX CACHE)

    else()

      # User says not to install; just keep track of what was missing.
      set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${NAME}")

    endif()

  endif()

  # Reset the CMAKE_INSTALL_PREFIX to its value before the detection/installation fiddled with it.
  set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for QB SDK core." FORCE)

endmacro()

# Cache the install path as an insurance against the poorly-behaved dependency messing with it.
macro(cache_install_path)
  # Check if a previous installation was interrupted and left CMAKE_INSTALL_PREFIX in a corrupted state.
  if (DEFINED CACHE{DEP_CMAKE_INSTALL_PREFIX} AND CMAKE_INSTALL_PREFIX STREQUAL DEP_CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for ${CMAKE_PROJECT_NAME}." FORCE)
    unset(DEP_CMAKE_INSTALL_PREFIX CACHE)
  else()
    # Otherwise, make a backup of the current CMAKE_INSTALL_PREFIX, before a poorly-behaved dependency's detection/installation process messes with it.
    set(CMAKE_INSTALL_PREFIX_BACKUP ${CMAKE_INSTALL_PREFIX} CACHE PATH "Backup copy of install path for ${CMAKE_PROJECT_NAME} (allows restoration after e.g. XACC overwrites CMAKE_INSTALL_PREFIX)." FORCE)
  endif()
endmacro()


# Reset CMAKE_INSTALL_PREFIX to its value before the detection/installation of the poorly-behaved dependency fiddled with it.
macro(reset_install_path)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for ${CMAKE_PROJECT_NAME}." FORCE)
endmacro()

# Attempt to locate a system-installed version of CMAKE_PACKAGE_NAME using a dry run of find_package.  Does this in an independent cmake session,
# so that successful return of find_package does not pollute the current cmake session (if e.g. the found version turns out to be the wrong one).
# Find_package will return successfully if the path to an installed CMAKE_PACKAGE_NAME is given as one of:
#  - environment variable ${CMAKE_PACKAGE_NAME}_ROOT
#  - cmake variable ${CMAKE_PACKAGE_NAME}_ROOT (set with -D at cmake invocation)
#  - cmake variable ${CMAKE_PACKAGE_NAME}_DIR (set with -D at cmake invocation)
function(find_package_dry_run NAME VERSION arg_CMAKE_PACKAGE_NAME arg_GIT_TAG SUCCESS)

  # Before running find_package, make sure that the _ROOT and _DIR paths are consistent.
  if(${arg_CMAKE_PACKAGE_NAME}_ROOT AND ${arg_CMAKE_PACKAGE_NAME}_DIR AND
   NOT ${arg_CMAKE_PACKAGE_NAME}_ROOT STREQUAL ${arg_CMAKE_PACKAGE_NAME}_DIR)
    message(FATAL_ERROR "${arg_CMAKE_PACKAGE_NAME}_ROOT and ${arg_CMAKE_PACKAGE_NAME}_DIR are both non-empty, but different. To change the ${NAME} installation used, when invoking cmake please unset one or set them consistently.")
  endif()

  execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/_dry_runs)
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/_dry_runs)
  configure_file(cmake/dry_run_CMakeLists.txt.in ${CMAKE_BINARY_DIR}/_dry_runs/CMakeLists.txt @ONLY)
  execute_process(COMMAND ${CMAKE_COMMAND} -B${CMAKE_BINARY_DIR}/_dry_runs ${CMAKE_BINARY_DIR}/_dry_runs -D${arg_CMAKE_PACKAGE_NAME}_ROOT=${${arg_CMAKE_PACKAGE_NAME}_ROOT} -D${arg_CMAKE_PACKAGE_NAME}_DIR=${${arg_CMAKE_PACKAGE_NAME}_DIR} ERROR_VARIABLE DRY_RUN_RESULTS ERROR_STRIP_TRAILING_WHITESPACE OUTPUT_QUIET)
  execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/_dry_runs)

  string(REGEX REPLACE ".*poorly_behaved_dependency_dry_run: (.*)(\n|$)" "\\1" DRY_RUN_RESULTS "${DRY_RUN_RESULTS}")
  if(DRY_RUN_RESULTS STREQUAL "Found")
    message(STATUS "System installation of ${NAME} found: version ${VERSION}-${arg_GIT_TAG}")
    set(${SUCCESS} True PARENT_SCOPE)
  else()
    set(${SUCCESS} False PARENT_SCOPE)
    if(DRY_RUN_RESULTS STREQUAL "Not found")
      message(STATUS "System installation of ${NAME} v${VERSION}-${arg_GIT_TAG} not found.")
    else()
      message(STATUS "System installation of ${NAME} is the wrong version - not using it.")
      message("   Wanted v${VERSION}-${arg_GIT_TAG}, but got ${DRY_RUN_RESULTS}.")
    endif()
  endif()

endfunction()


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

  # Take out insurance against changes to CMAKE_INSTALL_PREFIX
  cache_install_path()

  # Do a dry run of find_package
  find_package_dry_run(${NAME} ${VERSION} ${arg_CMAKE_PACKAGE_NAME} ${arg_GIT_TAG} DRY_RUN_SUCCEEDED)

  # If the dry run worked, we should be safe to find the dependency with find_package
  if(DRY_RUN_SUCCEEDED)

    find_package(${arg_CMAKE_PACKAGE_NAME} QUIET)
    if(NOT ${arg_CMAKE_PACKAGE_NAME}_FOUND OR NOT "${${arg_CMAKE_PACKAGE_NAME}_VERSION}" STREQUAL "${VERSION}-${arg_GIT_TAG}")
      message(FATAL_ERROR "Inconsistent cmake results: dry run of find_package for ${arg_CMAKE_PACKAGE_NAME} succeeded, but actual run failed!")
    endif()

  else()

    # Check if there was a successful local installation already by cmake.
    set(LOCAL_INSTALL_FOUND False)
    if(${arg_CMAKE_PACKAGE_NAME}_ROOT_SUCCESSFULLY_INSTALLED_BY_CMAKE STREQUAL dir)
      set(${arg_CMAKE_PACKAGE_NAME}_ROOT ${${arg_CMAKE_PACKAGE_NAME}_ROOT_SUCCESSFULLY_INSTALLED_BY_CMAKE})
      set(${arg_CMAKE_PACKAGE_NAME}_DIR ${${arg_CMAKE_PACKAGE_NAME}_ROOT})
      find_package(${arg_CMAKE_PACKAGE_NAME} QUIET)
      if(${arg_CMAKE_PACKAGE_NAME}_FOUND AND "${${arg_CMAKE_PACKAGE_NAME}_VERSION}" STREQUAL "${VERSION}-${arg_GIT_TAG}")
        message("   However, a previous installation by cmake was found at ${${arg_CMAKE_PACKAGE_NAME}_ROOT}.")
        set(LOCAL_INSTALL_FOUND True)
      endif()
    endif()

    if(NOT LOCAL_INSTALL_FOUND)

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
        execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E rm -rf _deps/${NAME})
        if(NOT ${result} STREQUAL "0")
          message(FATAL_ERROR "Attempt to remove existing git repository for ${NAME} v${VERSION} failed.")
        endif()
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
        set(${arg_CMAKE_PACKAGE_NAME}_ROOT "${dir}" CACHE PATH "Path to installed ${NAME}." FORCE)
        find_package(${arg_CMAKE_PACKAGE_NAME} REQUIRED)
        unset(DEP_CMAKE_INSTALL_PREFIX CACHE)

        # Save the path to the version installed by cmake, to avoid re-installing it.
        set(${arg_CMAKE_PACKAGE_NAME}_ROOT_SUCCESSFULLY_INSTALLED_BY_CMAKE "${dir}" CACHE PATH "Successful path into which ${NAME} was installed." FORCE)

      else()

        # User says not to install; just keep track of what was missing.
        set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${NAME}")

      endif()

    endif()

  endif()

  # Call in insurance against changes to CMAKE_INSTALL_PREFIX
  reset_install_path()

endmacro()

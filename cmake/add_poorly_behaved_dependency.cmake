# Bring in the macros allowing caching of the install path.
include(xacc_utilities)

# Attempt to locate a system-installed version of FIND_PACKAGE_NAME using a dry run of find_package.  Does this in an independent cmake session,
# so that successful return of find_package does not pollute the current cmake session (if e.g. the found version turns out to be the wrong one).
# Find_package will return successfully if the path to an installed FIND_PACKAGE_NAME is given as one of:
#  - environment variable ${FIND_PACKAGE_NAME}_ROOT
#  - cmake variable ${FIND_PACKAGE_NAME}_ROOT (set with -D at cmake invocation)
#  - cmake variable ${FIND_PACKAGE_NAME}_DIR (set with -D at cmake invocation)
function(find_package_dry_run NAME VERSION arg_FIND_PACKAGE_NAME arg_GIT_TAG SUCCESS)

  # Before running find_package, make sure that the _ROOT and _DIR paths are consistent.
  if(${arg_FIND_PACKAGE_NAME}_ROOT AND ${arg_FIND_PACKAGE_NAME}_DIR AND
   NOT ${arg_FIND_PACKAGE_NAME}_ROOT STREQUAL ${arg_FIND_PACKAGE_NAME}_DIR)
    message(FATAL_ERROR "${arg_FIND_PACKAGE_NAME}_ROOT and ${arg_FIND_PACKAGE_NAME}_DIR are both non-empty, but different. To change the ${NAME} installation used, when invoking cmake please unset one or set them consistently.")
  endif()

  execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/_dry_runs)
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/_dry_runs)
  configure_file(${CMAKE_CURRENT_LIST_DIR}/dry_run_CMakeLists.txt.in ${CMAKE_BINARY_DIR}/_dry_runs/CMakeLists.txt @ONLY)
  execute_process(COMMAND ${CMAKE_COMMAND} -B${CMAKE_BINARY_DIR}/_dry_runs ${CMAKE_BINARY_DIR}/_dry_runs -D${arg_FIND_PACKAGE_NAME}_ROOT=${${arg_FIND_PACKAGE_NAME}_ROOT} -D${arg_FIND_PACKAGE_NAME}_DIR=${${arg_FIND_PACKAGE_NAME}_DIR} ERROR_VARIABLE DRY_RUN_RESULTS ERROR_STRIP_TRAILING_WHITESPACE OUTPUT_QUIET)
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
      FIND_PACKAGE_NAME
      GIT_TAG
      GIT_REPOSITORY
      PATCH_FILE
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
    set(dir "${CPM_SOURCE_CACHE}/${NAME}/${VERSION}-${arg_GIT_TAG}")
  else()
    set(dir "${CPM_SOURCE_CACHE}/${NAME}/${VERSION}")
    set(arg_GIT_TAG v${VERSION})
  endif()
  if (NOT arg_FIND_PACKAGE_NAME)
    set(arg_FIND_PACKAGE_NAME ${NAME})
  endif()

  # Take out insurance against changes to CMAKE_INSTALL_PREFIX
  cache_install_path()

  # Do a dry run of find_package
  find_package_dry_run(${NAME} ${VERSION} ${arg_FIND_PACKAGE_NAME} ${arg_GIT_TAG} DRY_RUN_SUCCEEDED)

  # If the dry run worked, we should be safe to find the dependency with find_package
  if(DRY_RUN_SUCCEEDED)

    find_package(${arg_FIND_PACKAGE_NAME} QUIET)
    if(NOT ${arg_FIND_PACKAGE_NAME}_FOUND OR NOT "${${arg_FIND_PACKAGE_NAME}_VERSION}" STREQUAL "${VERSION}-${arg_GIT_TAG}")
      message(FATAL_ERROR "Inconsistent cmake results: dry run of find_package for ${arg_FIND_PACKAGE_NAME} succeeded, but actual run failed!")
    endif()

  else()

    # Check if there was a successful local installation already by cmake.
    set(LOCAL_INSTALL_FOUND False)
    if(${arg_FIND_PACKAGE_NAME}_ROOT_SUCCESSFULLY_INSTALLED_BY_CMAKE STREQUAL dir)
      set(${arg_FIND_PACKAGE_NAME}_ROOT "${dir}")
      set(${arg_FIND_PACKAGE_NAME}_DIR "${dir}")
      find_package(${arg_FIND_PACKAGE_NAME} QUIET)
      if(${arg_FIND_PACKAGE_NAME}_FOUND AND "${${arg_FIND_PACKAGE_NAME}_VERSION}" STREQUAL "${VERSION}-${arg_GIT_TAG}")
        message("   However, a previous installation by cmake was found at ${${arg_FIND_PACKAGE_NAME}_ROOT}.")
        set(LOCAL_INSTALL_FOUND True)
        set(${arg_FIND_PACKAGE_NAME}_ROOT "${dir}" CACHE PATH "Path to installed ${NAME}." FORCE)
        set(${arg_FIND_PACKAGE_NAME}_DIR "${dir}" CACHE PATH "Path to installed ${NAME}." FORCE)
      endif()
    endif()

    if(NOT LOCAL_INSTALL_FOUND)

      if(INSTALL_MISSING_CXX)

        # Run any preamble functions supplied (for finding/installing dependencies)
        foreach(command ${arg_PREAMBLE})
          cmake_language(CALL ${command})
        endforeach()

        # Parse any options given into the actual invocation of cmake.
        set(cmake_invocation ${CMAKE_COMMAND})
        list(APPEND cmake_invocation "-B_deps/${NAME}/build" "_deps/${NAME}" "-DCMAKE_INSTALL_PREFIX=${dir}" "-Wno-dev" "-DCMAKE_CXX_FLAGS=${dashw_IF_NOT_WARNINGS}")
        # Add options that are always passed if they are non-empty.
        # If these have been set in the call to add_poorly_behaved_dependency,
        # the versions set in the macro call will take precedence.
        set(OPTION_LIST CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_Fortran_COMPILER CMAKE_BUILD_TYPE)
        foreach(option ${OPTION_LIST})
          if(${option})
            list(APPEND cmake_invocation "-D${option}=${${option}}")
          endif()
        endforeach()
        # Add options passed in the macro invocation
        foreach(option ${arg_OPTIONS})
          string(FIND "${options}" "\"" HAS_QUOTE)
          if (HAS_QUOTE STREQUAL "-1")
            string(REGEX REPLACE " " "=" option "${option}")
          else()
            string(REGEX REPLACE "^(.*) \"" "\\1=\"" option "${option}")
          endif()
          string(REGEX REPLACE "^(.*) \"" "\\1=\"" option "${option}")
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
        execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${NAME} git apply ${arg_PATCH_FILE})
        if(NOT ${result} STREQUAL "0")
          message(FATAL_ERROR "Attemp to git apply ${PATCH_FILE} of ${NAME} failed.")
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
        set(${arg_FIND_PACKAGE_NAME}_ROOT "${dir}" CACHE PATH "Path to installed ${NAME}." FORCE)
        set(${arg_FIND_PACKAGE_NAME}_DIR "${dir}" CACHE PATH "Path to installed ${NAME}." FORCE)
        find_package(${arg_FIND_PACKAGE_NAME} REQUIRED)
        set(${arg_FIND_PACKAGE_NAME}_ROOT "${${arg_FIND_PACKAGE_NAME}_DIR}")
        unset(DEP_CMAKE_INSTALL_PREFIX CACHE)

        # Save the path to the version installed by cmake, to avoid re-installing it.
        set(${arg_FIND_PACKAGE_NAME}_ROOT_SUCCESSFULLY_INSTALLED_BY_CMAKE "${dir}" CACHE PATH "Successful path into which ${NAME} was installed." FORCE)

      else()

        # User says not to install; just keep track of what was missing.
        set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${NAME}")

      endif()

    endif()

  endif()

  # Call in insurance against changes to CMAKE_INSTALL_PREFIX
  reset_install_path()

endmacro()

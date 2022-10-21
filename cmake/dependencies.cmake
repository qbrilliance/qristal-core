#-----------------------------------------------------------------------
# XACC

set(name "xacc")
set(ver "1.0.0")
set(tag "f64e9da8")
set(dl "https://github.com/eclipse/xacc")
set(dir "${PROJECT_SOURCE_DIR}/deps/${name}/${ver}-${tag}")

# Check if a previous installation was interrupted and left CMAKE_INSTALL_PREFIX in a corrupted state.
if (DEFINED CACHE{DEP_CMAKE_INSTALL_PREFIX} AND CMAKE_INSTALL_PREFIX STREQUAL DEP_CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for QB SDK core." FORCE)
  unset(DEP_CMAKE_INSTALL_PREFIX CACHE)
else()
  # Otherwise, make a backup of the current CMAKE_INSTALL_PREFIX, as the following detection/installation process will mess with it.
  set(CMAKE_INSTALL_PREFIX_BACKUP ${CMAKE_INSTALL_PREFIX} CACHE PATH "Backup copy of install path for QB SDK core (allows restoration after XACC overwrites CMAKE_INSTALL_PREFIX)." FORCE)
endif()

# Locate a system-installed version of XACC.  Will work if the path to an installed XACC is given as one of:
#  - environment variable XACC_ROOT
#  - cmake variable XACC_ROOT (set with -D at cmake invocation)
#  - cmake variable XACC_DIR (set with -D at cmake invocation)
find_package(XACC QUIET)

if(XACC_FOUND AND "${XACC_VERSION}" STREQUAL "${ver}-${tag}")

  message("System installation of ${name} found: v${XACC_VERSION}")

else()

  message("System installation of ${name} v${ver}-${tag} not found.")

  if(INSTALL_MISSING)

    message("CMake will now download and install ${name} v${ver}.")
    execute_process(RESULT_VARIABLE result COMMAND git clone ${dl} _deps/${name})
    if(NOT ${result} STREQUAL "0")
      message(FATAL_ERROR "Attempt to clone git repository for ${name} v${ver} failed.  This is expected if e.g. you are disconnected from the internet.")
    endif()
    execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name} git checkout -q ${tag})
    if(NOT ${result} STREQUAL "0")
      message(FATAL_ERROR "Attempt to checkout git tag ${ver} of ${name} failed.")
    endif()
    set(DEP_CMAKE_INSTALL_PREFIX ${dir} CACHE PATH "Installation path of badly-behaved dependency => Installation is not yet complete." FORCE)
    execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -B_deps/${name}/build _deps/${name} -DCMAKE_INSTALL_PREFIX=${dir} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
    if(NOT ${result} STREQUAL "0")
      message(FATAL_ERROR "Running cmake for ${name} failed.")
    endif()
    execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name}/build ${MAKE_PARALLEL} install)
    if(NOT ${result} STREQUAL "0")
      message(FATAL_ERROR "Building ${name} failed.")
    endif()

    message("...done.")

    set(XACC_ROOT "${dir}")
    find_package(XACC REQUIRED)
    unset(DEP_CMAKE_INSTALL_PREFIX CACHE)

  else()

    # User says not to install; just keep track of what was missing.
    set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${name}")

  endif()

endif()

# Reset the CMAKE_INSTALL_PREFIX to its value before the detection/installation fiddled with it.
set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for QB SDK core." FORCE)

# Export the XACC root to any calling project
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(XACC_ROOT "${XACC_ROOT}" PARENT_SCOPE)
endif()


# Add dependencies needed for compiled libraries.
if (NOT QBCORE_HEADER_ONLY)

  #-----------------------------------------------------------------------
  # EXATN
  # TODO still needs work around version checking, BLAS detection and uniformity of implied add_badly_formed_dependency function to be created for installing XACC and EXATN (and TNQVM?)

  set(name "exatn")
  set(ver "1.0.0")
  set(tag "7509dce")
  set(dl "https://github.com/ornl-qci/exatn")
  set(dir "${PROJECT_SOURCE_DIR}/deps/${name}/${ver}-${tag}")

  # Check if a previous installation was interrupted and left CMAKE_INSTALL_PREFIX in a corrupted state.
  if (DEFINED CACHE{DEP_CMAKE_INSTALL_PREFIX} AND CMAKE_INSTALL_PREFIX STREQUAL DEP_CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for QB SDK core." FORCE)
    unset(DEP_CMAKE_INSTALL_PREFIX CACHE)
  else()
    # Otherwise, make a backup of the current CMAKE_INSTALL_PREFIX, as the following detection/installation process will mess with it.
    set(CMAKE_INSTALL_PREFIX_BACKUP ${CMAKE_INSTALL_PREFIX} CACHE PATH "Backup copy of install path for QB SDK core (allows restoration after XACC overwrites CMAKE_INSTALL_PREFIX)." FORCE)
  endif()

  # Locate a system-installed version of EXATN.  Will work if the path to an installed EXATN is given as one of:
  #  - environment variable EXATN_ROOT
  #  - cmake variable EXATN_ROOT (set with -D at cmake invocation)
  #  - cmake variable EXATN_DIR (set with -D at cmake invocation)
  find_package(EXATN QUIET)

  if(EXATN_FOUND) #AND "${EXATN_VERSION}" STREQUAL "${ver}-${tag}")

    message("System installation of ${name} found: v${EXATN_VERSION}")

  else()

    message("System installation of ${name} v${ver}-${tag} not found.")

    if(INSTALL_MISSING)

      set(BLA_VENDOR OpenBLAS)
      find_package(BLAS)
      if(BLAS_FOUND)
        cmake_path(GET BLAS_LIBRARIES PARENT_PATH BLAS_PATH)
      else()
        message(FATAL_ERROR "System installation of OpenBLAS not found. This is required for installing EXATN.")
      endif()

      message("CMake will now download and install ${name} v${ver}.")
      execute_process(RESULT_VARIABLE result COMMAND git clone ${dl} _deps/${name})
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to clone git repository for ${name} v${ver} failed.  This is expected if e.g. you are disconnected from the internet.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name} git checkout -q ${tag})
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to checkout git tag ${ver} of ${name} failed.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name} git submodule init)
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to run git submodule init in ${name} failed.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name} git submodule update --init --recursive)
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to run git submodule update in ${name} failed.")
      endif()
      set(DEP_CMAKE_INSTALL_PREFIX ${dir} CACHE PATH "Installation path of badly-behaved dependency => Installation is not yet complete." FORCE)
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -B_deps/${name}/build _deps/${name} -DCMAKE_INSTALL_PREFIX=${dir} -DBLAS_LIB=OPENBLAS -DBLAS_PATH=${BLAS_PATH} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Running cmake for ${name} failed.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name}/build ${MAKE_PARALLEL} install)
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Building ${name} failed.")
      endif()

      message("...done.")

      set(EXATN_ROOT "${dir}")
      find_package(EXATN REQUIRED)
      unset(DEP_CMAKE_INSTALL_PREFIX CACHE)

    else()

      # User says not to install; just keep track of what was missing.
      set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${name}")

    endif()

  endif()

  # Reset the CMAKE_INSTALL_PREFIX to its value before the detection/installation fiddled with it.
  set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for QB SDK core." FORCE)


  #-----------------------------------------------------------------------
  # TNQVM

  set(name "tnqvm")
  set(ver "1.0.0")
  set(tag "68a03dd")
  set(dl "https://github.com/ornl-qci/tnqvm")
  set(dir "${PROJECT_SOURCE_DIR}/deps/${name}/${ver}-${tag}")

  # Check if a previous installation was interrupted and left CMAKE_INSTALL_PREFIX in a corrupted state.
  if (DEFINED CACHE{DEP_CMAKE_INSTALL_PREFIX} AND CMAKE_INSTALL_PREFIX STREQUAL DEP_CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for QB SDK core." FORCE)
    unset(DEP_CMAKE_INSTALL_PREFIX CACHE)
  else()
    # Otherwise, make a backup of the current CMAKE_INSTALL_PREFIX, as the following detection/installation process will mess with it.
    set(CMAKE_INSTALL_PREFIX_BACKUP ${CMAKE_INSTALL_PREFIX} CACHE PATH "Backup copy of install path for QB SDK core (allows restoration after XACC overwrites CMAKE_INSTALL_PREFIX)." FORCE)
  endif()

  # Locate a system-installed version of TNQVM.  Will work if the path to an installed TNQVM is given as one of:
  #  - environment variable TNQVM_ROOT
  #  - cmake variable TNQVM_ROOT (set with -D at cmake invocation)
  #  - cmake variable TNQVM_DIR (set with -D at cmake invocation)
  find_package(TNQVM QUIET)

  if(TNQVM_FOUND AND "${TNQVM_VERSION}" STREQUAL "${ver}-${tag}")

    message("System installation of ${name} found: v${TNQVM_VERSION}")

  else()

    message("System installation of ${name} v${ver}-${tag} not found.")

    if(INSTALL_MISSING)

      message("CMake will now download and install ${name} v${ver}.")
      execute_process(RESULT_VARIABLE result COMMAND git clone ${dl} _deps/${name})
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to clone git repository for ${name} v${ver} failed.  This is expected if e.g. you are disconnected from the internet.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name} git checkout -q ${tag})
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to checkout git tag ${ver} of ${name} failed.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name} git submodule init)
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to run git submodule init in ${name} failed.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name} git submodule update --init --recursive)
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Attempt to run git submodule update in ${name} failed.")
      endif()
      set(DEP_CMAKE_INSTALL_PREFIX ${dir} CACHE PATH "Installation path of badly-behaved dependency => Installation is not yet complete." FORCE)
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -B_deps/${name}/build _deps/${name} -DCMAKE_INSTALL_PREFIX=${dir} -DXACC_DIR=${XACC_ROOT} -DEXATN_DIR=${EXATN_ROOT} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Running cmake for ${name} failed.")
      endif()
      execute_process(RESULT_VARIABLE result COMMAND ${CMAKE_COMMAND} -E chdir _deps/${name}/build ${MAKE_PARALLEL} install)
      if(NOT ${result} STREQUAL "0")
        message(FATAL_ERROR "Building ${name} failed.")
      endif()

      message("...done.")

      set(TNQVM_ROOT "${dir}")
      find_package(TNQVM REQUIRED)
      unset(DEP_CMAKE_INSTALL_PREFIX CACHE)

    else()

      # User says not to install; just keep track of what was missing.
      set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${name}")

    endif()

  endif()

  # Reset the CMAKE_INSTALL_PREFIX to its value before the detection/installation fiddled with it.
  set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX_BACKUP} CACHE PATH "Install path for QB SDK core." FORCE)


  #Python 3 interpreter and libraries
  find_package(Python 3 COMPONENTS Interpreter Development REQUIRED)

  # Include CPM for managing dependencies, and set it up to cache them in the deps folder
  set(CPM_DOWNLOAD_VERSION 0.36.0)
  set(CPM_SOURCE_CACHE "${PROJECT_SOURCE_DIR}/deps")
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
  if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
         https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
         ${CPM_DOWNLOAD_LOCATION}
    )
  endif()
  include(${CPM_DOWNLOAD_LOCATION})

  # Helper wrapper for CPMAddPackage
  include(add_dependency)

  add_dependency(args 6.4.1
    GITHUB_REPOSITORY Taywee/args
    GIT_TAG 6.4.1
    OPTIONS
      "ARGS_BUILD_EXAMPLE OFF"
      "ARGS_BUILD_UNITTESTS OFF"
  )

  add_dependency(nlohmann_json 3.9.1
    GITHUB_REPOSITORY /nlohmann/json
    OPTIONS
      "JSON_BuildTests OFF"
  )

  if(CMAKE_BUILD_TYPE STREQUAL "None")
    set(cpr_CMAKE_BUILD_TYPE "Release")
  else()
    set(cpr_CMAKE_BUILD_TYPE CMAKE_BUILD_TYPE)
  endif()
  add_dependency(cpr 1.9.2
    GIT_TAG 1.9.2
    GITHUB_REPOSITORY libcpr/cpr
    OPTIONS
      "CMAKE_BUILD_TYPE ${cpr_CMAKE_BUILD_TYPE}"
  )

  add_dependency(cppitertools 2.1
    GITHUB_REPOSITORY ryanhaining/cppitertools
    OPTIONS
      "cppitertools_INSTALL_CMAKE_DIR share"
  )

  add_dependency(Eigen3 3.4.0
    GITLAB_REPOSITORY libeigen/eigen
  )
  # Add missing interface library for Eigen3
  if (EIGEN3_FOUND)
    if(NOT EXISTS ${EIGEN3_INCLUDE_DIR}/Eigen/Eigen)
      message(FATAL_ERROR "Your Eigen installation appears to be broken: could not find file ${EIGEN3_INCLUDE_DIR}/Eigen/Eigen.")
    endif()
  add_library(Eigen3_interface INTERFACE)
  target_include_directories(Eigen3_interface INTERFACE "${EIGEN3_INCLUDE_DIR}")
  endif()

  add_dependency(pybind11 2.10.0
    GITHUB_REPOSITORY pybind/pybind11
  )

endif()


# If any packages are still missing, fail.
if(NOT INSTALL_MISSING AND MISSING_DEPENDENCIES)

  message("\nThe following dependencies were not found by cmake:")
  foreach(package ${MISSING_DEPENDENCIES})
    message("  ${package}")
  endforeach()
  message("To have cmake automatically install the missing packages, rerun with -DINSTALL_MISSING=ON.\n")
  message(FATAL_ERROR "Either rerun with -DINSTALL_MISSING=ON, or fix the search paths passed to cmake for finding installed dependencies.")

endif()

# Add dependencies needed for header-only usage
include(add_poorly_behaved_dependency)

# XACC
add_poorly_behaved_dependency(xacc 1.0.0
  CMAKE_PACKAGE_NAME XACC
  GIT_TAG f64e9da8
  GIT_REPOSITORY https://github.com/eclipse/xacc
)

# Add dependencies needed for compiled libraries.
if (NOT QBCORE_HEADER_ONLY)

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
  include(add_dependency)

  #Python 3 interpreter and libraries
  find_package(Python 3 COMPONENTS Interpreter Development REQUIRED)

  # BLAS; used as a dependency for EXATN.
  macro(find_blas)
    set(BLA_VENDOR OpenBLAS)
    find_package(BLAS)
    if(BLAS_FOUND)
      cmake_path(GET BLAS_LIBRARIES PARENT_PATH BLAS_PATH)
    else()
      message(FATAL_ERROR "System installation of OpenBLAS not found. This is required for installing EXATN.")
    endif()
  endmacro()

  # EXATN
  add_poorly_behaved_dependency(exatn 1.0.0
    CMAKE_PACKAGE_NAME EXATN
    GIT_TAG 2549394
    GIT_REPOSITORY https://github.com/ornl-qci/exatn
    UPDATE_SUBMODULES True
    PREAMBLE
      find_blas
    OPTIONS
      "BLAS_LIB OPENBLAS"
      "BLAS_PATH @BLAS_PATH@"
      "EXATN_BUILD_TESTS False"
  )

  # TNQVM
  add_poorly_behaved_dependency(tnqvm 1.0.0
    CMAKE_PACKAGE_NAME TNQVM
    GIT_TAG 68a03dd
    GIT_REPOSITORY https://github.com/ornl-qci/tnqvm
    OPTIONS
     "XACC_DIR ${XACC_ROOT}"
     "EXATN_DIR ${EXATN_ROOT}"
  )

  # args library
  add_dependency(args 6.4.1
    GITHUB_REPOSITORY Taywee/args
    GIT_TAG 6.4.1
    OPTIONS
      "ARGS_BUILD_EXAMPLE OFF"
      "ARGS_BUILD_UNITTESTS OFF"
  )

  # json library
  add_dependency(nlohmann_json 3.9.1
    GITHUB_REPOSITORY /nlohmann/json
    OPTIONS
      "JSON_BuildTests OFF"
  )

  # CPR curl wrapper
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

  # C++ itertools
  add_dependency(cppitertools 2.1
    GITHUB_REPOSITORY ryanhaining/cppitertools
    OPTIONS
      "cppitertools_INSTALL_CMAKE_DIR share"
  )

  # Eigen
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

  # Pybind11
  add_dependency(pybind11 2.10.0
    GITHUB_REPOSITORY pybind/pybind11
  )

  # Gtest
  add_dependency(googletest 1.12.1
    GITHUB_REPOSITORY google/googletest
    GIT_TAG release-1.12.1
    OPTIONS
      "INSTALL_GTEST OFF"
      "gtest_force_shared_crt"
  )

endif()


# If any packages are still missing, fail.
if(MISSING_DEPENDENCIES AND NOT INSTALL_MISSING)

  message("\nThe following dependencies were not found by cmake:")
  foreach(package ${MISSING_DEPENDENCIES})
    message("  ${package}")
  endforeach()
  message("To have cmake automatically install the missing packages, rerun with -DINSTALL_MISSING=ON.\n")
  message(FATAL_ERROR "Either rerun with -DINSTALL_MISSING=ON, or fix the search paths passed to cmake for finding installed dependencies.")

endif()

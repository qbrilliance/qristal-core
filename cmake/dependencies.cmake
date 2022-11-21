include(add_poorly_behaved_dependency)

# curl (needed by XACC and CPR)
set(CURL_NO_CURL_CMAKE ON)
find_package(CURL REQUIRED)

# XACC
add_poorly_behaved_dependency(xacc 1.0.0
  FIND_PACKAGE_NAME XACC
  GIT_TAG f64e9da8
  GIT_REPOSITORY https://github.com/eclipse/xacc
)

# Pybind11
add_dependency(pybind11 2.10.0
  GITHUB_REPOSITORY pybind/pybind11
)

# Gtest
add_dependency(googletest 1.12.1
  GITHUB_REPOSITORY google/googletest
  FIND_PACKAGE_NAME GTest
  GIT_TAG release-1.12.1
  OPTIONS
    "INSTALL_GTEST OFF"
    "gtest_force_shared_crt"
)

# json library
add_dependency(nlohmann_json 3.7.3
  GITHUB_REPOSITORY /nlohmann/json
  OPTIONS
    "JSON_BuildTests OFF"
)

# BLAS; used as a dependency for EXATN and Eigen.
set(BLA_VENDOR OpenBLAS)
find_package(BLAS)
if(BLAS_FOUND)
  cmake_path(GET BLAS_LIBRARIES PARENT_PATH BLAS_PATH)
else()
  message(FATAL_ERROR "System installation of OpenBLAS not found. This is required for installing Eigen and EXATN.")
endif()

# Eigen
add_dependency(Eigen3 3.3.7
  GITLAB_REPOSITORY libeigen/eigen
  GIT_TAG 3.3.7
  DOWNLOAD_ONLY YES
)
if(Eigen3_ADDED)
  add_library(Eigen INTERFACE IMPORTED)
  target_include_directories(Eigen INTERFACE ${Eigen3_SOURCE_DIR})
  add_library(Eigen3::Eigen ALIAS Eigen)
endif()


# Add dependencies needed only for compiled libraries.
if (NOT QBCORE_HEADER_ONLY)

  # Python 3 interpreter and libraries
  find_package(Python 3 COMPONENTS Interpreter Development REQUIRED)
  if(NOT LOCAL_PYTHON_SITE_PACKAGES)
    execute_process(RESULT_VARIABLE FAILURE
                    OUTPUT_VARIABLE LOCAL_PYTHON_SITE_PACKAGES
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    COMMAND python3 -c "import sysconfig; print(sysconfig.get_paths()['purelib'])")
    if(FAILURE)
      message(FATAL_ERROR "Failed to determine your local Python site-packages dir. Please set it manually with -DLOCAL_PYTHON_SITE_PACKAGES=/path/to/dir.")
    endif()
  endif()

  # Boost headers
  find_package(Boost 1.71 REQUIRED)

  # EXATN
  add_poorly_behaved_dependency(exatn 1.0.0
    FIND_PACKAGE_NAME EXATN
    GIT_TAG d8a15b1
    GIT_REPOSITORY https://github.com/ornl-qci/exatn
    UPDATE_SUBMODULES True
    OPTIONS
      "BLAS_LIB OPENBLAS"
      "BLAS_PATH @BLAS_PATH@"
      "EXATN_BUILD_TESTS OFF"
  )

  # TNQVM
  add_poorly_behaved_dependency(tnqvm 1.0.0
    FIND_PACKAGE_NAME TNQVM
    GIT_TAG 68a03dd
    GIT_REPOSITORY https://github.com/ornl-qci/tnqvm
    OPTIONS
     "XACC_DIR ${XACC_ROOT}"
     "EXATN_DIR ${EXATN_ROOT}"
     "TNQVM_BUILD_TESTS OFF"
  )
  # Add symlinks to {XACC_ROOT}/plugins where XACC finds its plugins by default.
  # TODO: XACC has an API (xacc::addPluginSearchPath) to add a search path to find plugin's so files.
  # To remove the below symlink step, we need to:
  # (1) Modify XACC's initialization in the Python binding module to add proper path.
  # (2) Modify plugin gtest files which currently assume vanilla xacc::Initialize()
  file(GLOB TNQVM_LIBS ${TNQVM_ROOT}/plugins/*)
  foreach(lib ${TNQVM_LIBS})
    cmake_path(GET lib FILENAME filename)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${lib} ${XACC_ROOT}/plugins/${filename})")
  endforeach()

  # args library
  add_dependency(args 6.4.1
    GITHUB_REPOSITORY Taywee/args
    FIND_PACKAGE_VERSION " " #for manual cmake build
    GIT_TAG 6.4.1
    OPTIONS
      "ARGS_BUILD_EXAMPLE OFF"
      "ARGS_BUILD_UNITTESTS OFF"
  )
  if(TARGET taywee::args)
    add_library(args ALIAS taywee::args)
  endif()

  # CPR curl wrapper
  list(APPEND CMAKE_PREFIX_PATH "/usr/local/")
  if(CMAKE_BUILD_TYPE STREQUAL "None")
    set(cpr_CMAKE_BUILD_TYPE "Release")
  else()
    set(cpr_CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
  endif()
  add_dependency(cpr 1.3.0
    GIT_TAG 2305262
    GITHUB_REPOSITORY ornl-qci/cpr
    OPTIONS
      "CMAKE_BUILD_TYPE ${cpr_CMAKE_BUILD_TYPE}"
      "USE_SYSTEM_CURL ON"
      "BUILD_CPR_TESTS OFF"
  )

  # C++ itertools
  add_dependency(cppitertools 2.1
    FIND_PACKAGE_VERSION "2.0" #for manual cmake build
    GITHUB_REPOSITORY ryanhaining/cppitertools
    OPTIONS
      "cppitertools_INSTALL_CMAKE_DIR share"
  )

endif()


# If any packages are still missing, fail.
check_missing()

include(add_dependency)
include(add_poorly_behaved_dependency)
include(add_python_module)

# add compatibility for user provided boost
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.27.0")
  cmake_policy(SET CMP0144 NEW)
endif()
find_package(Boost 1.7.1 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# curl (needed by XACC and CPR)
set(CURL_NO_CURL_CMAKE ON)
find_package(CURL REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
  get_filename_component(OPENSSL_INSTALL_DIR ${OPENSSL_SSL_LIBRARY} DIRECTORY)
  message(STATUS "OPENSSL_INSTALL_DIR set to: ${OPENSSL_INSTALL_DIR}")
else()
  message(FATAL_ERROR "System installation of OpenSSL not found. Please add the OpenSSL development package using the appropriate management tool for your system libraries.")
endif()

# OpenMP
find_package(OpenMP)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
  message(STATUS "OpenMP was not detected...continuing without it...")
endif()

# For XACC, but impacts Qristal too.
set(ENABLE_MPI OFF CACHE BOOL "MPI Capability")

# XACC
set(XACC_TAG "05164c13")
if(CMAKE_BUILD_TYPE STREQUAL "None")
  set(XACC_CMAKE_BUILD_TYPE "Release")
else()
  set(XACC_CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()
add_poorly_behaved_dependency(xacc 1.0.0
  FIND_PACKAGE_NAME XACC
  GIT_TAG ${XACC_TAG}
  GIT_REPOSITORY https://github.com/eclipse/xacc
  PATCH_FILE ${CMAKE_CURRENT_LIST_DIR}/patches/xacc.patch
  OPTIONS
    "XACC_ENABLE_MPI @ENABLE_MPI@"
    "COMPILE_FOR_LOCAL_ARCH ${COMPILE_FOR_LOCAL_ARCH}"
    "CMAKE_BUILD_TYPE ${XACC_CMAKE_BUILD_TYPE}"
    "OPENSSL_ROOT_DIR ${OPENSSL_INSTALL_DIR}"
    "CMAKE_INSTALL_LIBDIR lib"
)

# Python 3 interpreter and libraries
find_package(Python 3 COMPONENTS Interpreter Development REQUIRED)

# Pybind11.
set(pybind11_VERSION "2.10.0")
add_dependency(pybind11 ${pybind11_VERSION}
  GITHUB_REPOSITORY pybind/pybind11
  OPTIONS
    "PYBIND11_INSTALL ON"
)
if(pybind11_ADDED)
  set(pybind11_DIR ${CMAKE_INSTALL_PREFIX}/cmake/pybind11/pybind11)
  install(
    DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/cmake/pybind11
    DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/pybind11
  )
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_INSTALL_PREFIX}/share/cmake/pybind11)")
endif()

# Workaround for https://gitlab.kitware.com/cmake/cmake/-/issues/23075
# Redirect YAML_CPP_DIR environment variable to cmake variable yaml-cpp_DIR
# only when it was not provided on the command line via
# -Dyaml-cpp_DIR (which will take precedent)
if (NOT DEFINED yaml-cpp_DIR OR "${yaml-cpp_DIR}" STREQUAL "")
  if (DEFINED ENV{YAML_CPP_DIR} AND EXISTS "$ENV{YAML_CPP_DIR}")
    message(STATUS "Setting CMake var yaml-cpp_DIR to: $ENV{YAML_CPP_DIR}/${qbcore_LIBDIR}/cmake/yaml-cpp")
    set(yaml-cpp_DIR $ENV{YAML_CPP_DIR}/${qbcore_LIBDIR}/cmake/yaml-cpp)
  endif()
endif()
set(yamlcpp_VERSION "0.7.0")
add_dependency(yamlcpp ${yamlcpp_VERSION}
  GITHUB_REPOSITORY jbeder/yaml-cpp
  FIND_PACKAGE_NAME yaml-cpp
  FIND_PACKAGE_VERSION " " #for manual cmake build
  GIT_TAG b888265
  PATCH_FILE ${CMAKE_CURRENT_LIST_DIR}/patches/yaml-cpp.patch
  OPTIONS
    "YAML_BUILD_SHARED_LIBS ON"
    "YAML_CPP_INSTALL ON"
    "CMAKE_INSTALL_CONFIG_NAME None"
    "CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include"
    "CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR}"
  )
if (NOT DEFINED yaml-cpp_DIR OR "${yaml-cpp_DIR}" STREQUAL "yaml-cpp_DIR-NOTFOUND")
  set(yaml-cpp_DIR "${CMAKE_INSTALL_PREFIX}/cmake/yaml-cpp/yaml-cpp")
endif()
if(NOT yamlcpp_ADDED AND TARGET yaml-cpp)
  add_library(yaml-cpp::yaml-cpp ALIAS yaml-cpp)
endif()

# Gtest
set(GTest_VERSION "1.12.1")
add_dependency(googletest ${GTest_VERSION}
  GITHUB_REPOSITORY google/googletest
  FIND_PACKAGE_NAME GTest
  FIND_PACKAGE_ADDITIONAL_RESULT_PATH_VAR GTEST_INCLUDE_DIR
  GIT_TAG release-1.12.1
  OPTIONS
    "INSTALL_GTEST ON"
    "gtest_force_shared_crt"
)
if(googletest_ADDED)
  set(GTest_DIR ${CMAKE_INSTALL_PREFIX}/cmake/googletest/GTest CACHE PATH "GTest Installation path." FORCE)
  install(
    DIRECTORY ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR}/cmake/GTest
    DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/googletest
  )
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR}/cmake/GTest)")
endif()

# json library
set(nlohmann_json_VERSION "3.1.1")
add_dependency(nlohmann_json ${nlohmann_json_VERSION}
  GITHUB_REPOSITORY /nlohmann/json
  FIND_PACKAGE_ARGUMENTS "EXACT"
  OPTIONS
    "JSON_BuildTests OFF"
)
if(TARGET nlohmann_json)
  add_library(nlohmann::json ALIAS nlohmann_json)
  add_library(AER_DEPENDENCY_PKG::nlohmann_json ALIAS nlohmann_json)
elseif(TARGET nlohmann_json::nlohmann_json)
  add_library(nlohmann::json ALIAS nlohmann_json::nlohmann_json)
  add_library(AER_DEPENDENCY_PKG::nlohmann_json ALIAS nlohmann_json::nlohmann_json)
endif()
if(nlohmann_json_ADDED)
  set(nlohmann_json_DIR ${CMAKE_INSTALL_PREFIX}/cmake/nlohmann/nlohmann_json)
  install(
    DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/cmake/nlohmann_json
    DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/nlohmann
  )
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_INSTALL_PREFIX}/lib/cmake/nlohmann_json)")
endif()

# BLAS; used as a dependency for EXATN and Eigen.
set(BLA_VENDOR OpenBLAS)
find_package(BLAS)
if(BLAS_FOUND)
  cmake_path(GET BLAS_LIBRARIES PARENT_PATH BLAS_PATH)
else()
  message(FATAL_ERROR "System installation of OpenBLAS not found. This is required for installing Eigen and EXATN.")
endif()

# Eigen
# Check if XACC is installed and provides Eigen3. XACC_DIR is the XACC install path,
# which is always in place if XACC is added using add_poorly_behaved_dependency.
set(XACC_EIGEN_PATH "${XACC_DIR}/include/eigen")
if (EXISTS ${XACC_EIGEN_PATH})
  add_eigen_from_xacc()
else()
  #Eigen from system or CPM
  message(STATUS "Eigen3 not found from XACC.")
  add_dependency(Eigen3 3.3.7
    GITLAB_REPOSITORY libeigen/eigen
    GIT_TAG 3.3.7
    DOWNLOAD_ONLY YES
  )
  if(Eigen3_ADDED)
    add_library(Eigen INTERFACE IMPORTED)
    target_include_directories(Eigen INTERFACE ${Eigen3_SOURCE_DIR})
  endif()
  if(TARGET Eigen)
    add_library(Eigen3::Eigen ALIAS Eigen)
  endif()
endif()

# tket
set(WITH_TKET OFF CACHE BOOL "Enable TKET for noise-aware circuit placement.")
set(TKET_VERSION 1.11.1)
set(TKET_TAG "1cd9fe36c")
if(WITH_TKET)
  add_poorly_behaved_dependency(TKET ${TKET_VERSION}
    FIND_PACKAGE_NAME TKET
    GIT_TAG ${TKET_TAG}
    GIT_REPOSITORY https://github.com/CQCL/tket
    UPDATE_SUBMODULES True
    OPTIONS
      "TKET_BUILD_TESTS OFF"
      "CMAKE_BUILD_TYPE Release"
      "CMAKE_EXPORT_COMPILE_COMMANDS ON"
      "INSTALL_MISSING_CXX ON"
      "JSON_VERSION ${nlohmann_json_VERSION}"
      "XACC_DIR ${XACC_DIR}"
    PATCH_FILE ${CMAKE_CURRENT_LIST_DIR}/patches/tket.patch
  )
endif()


# autodiff
set(autodiff_VERSION "0.6.12")
add_dependency(autodiff ${autodiff_VERSION}
  GITHUB_REPOSITORY autodiff/autodiff
  OPTIONS
    "AUTODIFF_BUILD_TESTS OFF"
    "AUTODIFF_BUILD_PYTHON OFF"
    "AUTODIFF_BUILD_EXAMPLES OFF"
    "AUTODIFF_BUILD_DOCS OFF"
    "CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}"
)
if(autodiff_ADDED)
  set(autodiff_DIR ${CMAKE_INSTALL_PREFIX}/cmake/autodiff)
endif()

# cereal
set(cereal_VERSION "1.3.2")
add_dependency(cereal ${cereal_VERSION}
  GITHUB_REPOSITORY USCiLab/cereal
  OPTIONS
    "SKIP_PORTABILITY_TEST ON"
    "BUILD_DOC OFF"
    "BUILD_SANDBOX OFF"
    "SKIP_PERFORMANCE_COMPARISON ON"
    "THREAD_SAFE ON"
    "CEREAL_INSTALL ON"
    "CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}"
)
if(cereal_ADDED)
  set(cereal_DIR ${CMAKE_INSTALL_PREFIX}/cmake/cereal)
endif()

# args library
set(args_VERSION "6.4.1")
add_dependency(args ${args_VERSION}
  GITHUB_REPOSITORY Taywee/args
  FIND_PACKAGE_VERSION " " #for manual cmake build
  GIT_TAG ${args_VERSION}
  OPTIONS
    "ARGS_BUILD_EXAMPLE OFF"
    "ARGS_BUILD_UNITTESTS OFF"
)
# If args was not found by findPackage, fill in the install steps that are missing when it is added as a subdirectory.
if(args_ADDED)
  set(args_DIR ${CMAKE_INSTALL_PREFIX}/cmake/args)
  add_library(taywee::args ALIAS args)
  install(TARGETS args EXPORT args-targets)
  install(EXPORT args-targets
      FILE args-config.cmake
      NAMESPACE taywee::
      DESTINATION ${args_DIR})
  install(FILES ${args_SOURCE_DIR}/args.hxx
          DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif()


# Add dependencies needed only for compiled libraries.
if (NOT SUPPORT_EMULATOR_BUILD_ONLY)

  # Python modules.  Add to this alphabetically please!
  add_python_module(ase
                    ast
                    boto3
                    botocore
                    braket:amazon-braket-sdk
                    flask
                    http
                    json
                    matplotlib
                    numpy
                    os
                    pathlib
                    pip
                    pytest
                    qiskit
                    queue
                    random
                    re
                    subprocess
                    threading
                    time
                    timeit
                    uuid
                   )

  # EXATN
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(EXATN_CXX_COMPILER ${CMAKE_CXX_COMPILER})
    set(EXATN_C_COMPILER ${CMAKE_C_COMPILER})
  else()
    set(EXATN_CXX_COMPILER "g++")
    set(EXATN_C_COMPILER "gcc")
  endif()
  add_poorly_behaved_dependency(exatn 1.0.0
    FIND_PACKAGE_NAME EXATN
    GIT_TAG d8a15b1
    GIT_REPOSITORY https://github.com/ornl-qci/exatn
    UPDATE_SUBMODULES True
    OPTIONS
      "BLAS_LIB OPENBLAS"
      "BLAS_PATH @BLAS_PATH@"
      "EXATN_BUILD_TESTS OFF"
      "CMAKE_CXX_COMPILER ${EXATN_CXX_COMPILER}"
      "CMAKE_C_COMPILER ${EXATN_C_COMPILER}"
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
     "CMAKE_CXX_COMPILER ${EXATN_CXX_COMPILER}"
  )
  # Add symlinks to {XACC_ROOT}/plugins where XACC finds its plugins by default.
  # Delete symlinks to all other versions of the plugin to avoid issues when upgrading to a new version.
  # TODO: XACC has an API (xacc::addPluginSearchPath) to add a search path to find plugin's so files.
  # To remove the below symlink step, we need to:
  # (1) Modify XACC's initialization in the Python binding module to add proper path.
  # (2) Modify plugin gtest files which currently assume vanilla xacc::Initialize()
  file(GLOB TNQVM_LIBS ${TNQVM_ROOT}/plugins/*)
  foreach(lib ${TNQVM_LIBS})
    cmake_path(GET lib FILENAME filename)
    cmake_path(GET lib STEM stem)
    file(GLOB OLD_SYMLINKS ${XACC_ROOT}/plugins/${stem}.*)
    foreach(link ${OLD_SYMLINKS})
      install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -f ${link})")
    endforeach()
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${lib} ${XACC_ROOT}/plugins/${filename})")
  endforeach()

  # CPR curl wrapper
  IF( DEFINED ENV{CPR_DIR} )
    SET( CPR_DIR "$ENV{CPR_DIR}" )
  ENDIF()
  find_path(CPR_INCLUDE_DIR
                NAMES cpr/cpr.h
                HINTS ${CPR_DIR}/include)
  find_library(CPR_LIBRARY
                NAMES cpr
                HINTS ${CPR_DIR}/lib)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(CPR REQUIRED_VARS CPR_LIBRARY CPR_INCLUDE_DIR)
  if(CPR_FOUND)
    set(CPR_LIBRARIES ${CPR_LIBRARY})
    set(CPR_INCLUDE_DIRS ${CPR_INCLUDE_DIR})
    add_library(cpr INTERFACE)
    target_link_libraries(cpr INTERFACE ${CPR_LIBRARY})
    message(STATUS "cpr system install found: ${CPR_LIBRARY}.")
    message(STATUS "cpr include dir: ${CPR_INCLUDE_DIR}.")
  else()
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/")
    if(CMAKE_BUILD_TYPE STREQUAL "None")
      set(cpr_CMAKE_BUILD_TYPE "Release")
    else()
      set(cpr_CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
    endif()
    add_dependency(cpr 1.3.0
      GIT_TAG 2305262
      GITHUB_REPOSITORY ornl-qci/cpr
      FIND_PACKAGE_VERSION " " #for manual cmake build
      OPTIONS
        "CMAKE_BUILD_TYPE ${cpr_CMAKE_BUILD_TYPE}"
        "USE_SYSTEM_CURL ON"
        "BUILD_CPR_TESTS OFF"
        "CMAKE_POSITION_INDEPENDENT_CODE ON"
    )
    # If cpr was not found by findPackage, fill in the install steps that are missing when it is added as a subdirectory.
    if(cpr_ADDED)
      # Remove the INTERFACE_INCLUDE_DIRECTORIES property erroneously added by cpr/CMakeLists.txt's use of target_include_directories with PUBLIC keyword.
      set_target_properties(cpr PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "")
      # Now do it properly.
    endif()
  endif()
  if (TARGET cpr)
    target_include_directories(cpr INTERFACE
      $<BUILD_INTERFACE:${CPR_INCLUDE_DIRS}>
      $<BUILD_INTERFACE:${CURL_INCLUDE_DIRS}>
    )
    install(TARGETS cpr EXPORT cpr-targets)
    install(EXPORT cpr-targets
        FILE cpr-config.cmake
        NAMESPACE cpr::
        DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/cpr)
    if (NOT qbcore_LIBDIR STREQUAL "lib")
      install(
          FILES ${CPR_LIBRARIES}
          DESTINATION ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR}
      )
    endif()
  endif()

  # C++ itertools
  add_dependency(cppitertools 2.1
    FIND_PACKAGE_VERSION "2.0" #for manual cmake build 2.0
    GITHUB_REPOSITORY ryanhaining/cppitertools
    OPTIONS
      "cppitertools_INSTALL_CMAKE_DIR share"
  )

  # QASM simulator.
  find_program(QB_STANDALONE_AER_EXE qasm_simulator)
  is_in_install_path(${QB_STANDALONE_AER_EXE} AER_FOUND_IN_INSTALLED_QRISTAL)
  if(AER_FOUND_IN_INSTALLED_QRISTAL OR ${QB_STANDALONE_AER_EXE} STREQUAL "QB_STANDALONE_AER_EXE-NOTFOUND")
    # Install required dependency spdlog
    set(name "spdlog")
    set(repo "https://github.com/gabime/spdlog.git")
    set(tag "1.5.0")
    add_dependency(${name} ${tag}
      GIT_REPOSITORY ${repo}
      GIT_TAG v${tag}
    )
    if(TARGET spdlog)
      add_library(AER_DEPENDENCY_PKG::spdlog ALIAS spdlog)
    elseif(TARGET spdlog::spdlog)
      add_library(AER_DEPENDENCY_PKG::spdlog ALIAS spdlog::spdlog)
    endif()
    # Now install the QASM simulator
    set(name "qasm_simulator")
    set(repo "https://github.com/Qiskit/qiskit-aer.git")
    set(tag "0.10.4")
    set(patch ${CMAKE_CURRENT_LIST_DIR}/patches/qasm_simulator.patch)
    list(APPEND PREHASH GIT_REPOSITORY ${repo})
    list(APPEND PREHASH GIT_TAG ${tag})
    list(APPEND PREHASH PATCH_COMMAND git apply ${patch})
    list(APPEND PREHASH URL)
    list(SORT PREHASH)
    string(SHA1 hash "${PREHASH}")
    add_dependency(${name} ${tag}
      GIT_REPOSITORY ${repo}
      GIT_TAG ${tag}
      PATCH_FILE ${patch}
      OPTIONS
        "CMAKE_MODULE_PATH ${CPM_SOURCE_CACHE}/${name}/${hash}/cmake"
        "CMAKE_BUILD_TYPE Release"
        "DISABLE_CONAN ON"
    )
    if(qasm_simulator_ADDED)
      set(QB_STANDALONE_AER_EXE ${CMAKE_INSTALL_PREFIX}/bin/qasm_simulator)
    else()
      find_program(QB_STANDALONE_AER_EXE qasm_simulator)
      add_custom_target(qasm_simulator)
    endif()
  else()
    add_custom_target(qasm_simulator)
    message(STATUS "qasm_simulator found: ${QB_STANDALONE_AER_EXE}")
  endif() # (qasm-simulator NOTFOUND)

endif() # (NOT SUPPORT_EMULATOR_BUILD_ONLY)

# Get rid of other cmake paths
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR}/cmake)")
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_INSTALL_PREFIX}/share)")
if (NOT qbcore_LIBDIR STREQUAL "lib")
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_INSTALL_PREFIX}/lib)")
endif()


# If any packages are still missing, fail.
check_missing()

# Detect if we can find CUDAQ
find_program(NVQPP_EXECUTABLE NAMES nvq++)
if (NVQPP_EXECUTABLE)
  message(STATUS "Found nvq++ compiler at ${NVQPP_EXECUTABLE}. Enable CUDAQ support.")
  # Flag that we have CUDAQ
  set(WITH_CUDAQ TRUE)
  get_filename_component(CUDAQ_BIN_DIR ${NVQPP_EXECUTABLE} DIRECTORY)
  get_filename_component(CUDAQ_ROOT_DIR ${CUDAQ_BIN_DIR} DIRECTORY)
  set(CUDAQ_INCLUDE_DIR ${CUDAQ_ROOT_DIR}/include)
  set(CUDAQ_LIB_DIR ${CUDAQ_ROOT_DIR}/lib)
  message(STATUS "* CUDAQ bin directory: ${CUDAQ_BIN_DIR}.")
  message(STATUS "* CUDAQ include directory: ${CUDAQ_INCLUDE_DIR}.")
  message(STATUS "* CUDAQ lib directory: ${CUDAQ_LIB_DIR}.")
endif ()

include(add_dependency)
include(add_poorly_behaved_dependency)
include(add_python_module)

# curl (needed by XACC and CPR)
set(CURL_NO_CURL_CMAKE ON)
find_package(CURL REQUIRED)

# For XACC, but impacts Qristal too.
set(ENABLE_MPI OFF)
set(XACC_TAG "8e3b96fe")
# XACC
add_poorly_behaved_dependency(xacc 1.0.0
  FIND_PACKAGE_NAME XACC
  GIT_TAG ${XACC_TAG}
  GIT_REPOSITORY https://github.com/eclipse/xacc
  OPTIONS
    "XACC_ENABLE_MPI @ENABLE_MPI@"
    "CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}"
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

# Gtest
set(GTest_VERSION "1.12.1")
add_dependency(googletest ${GTest_VERSION}
  GITHUB_REPOSITORY google/googletest
  FIND_PACKAGE_NAME GTest
  GIT_TAG release-1.12.1
  OPTIONS
    "INSTALL_GTEST ON"
    "gtest_force_shared_crt"
)
if(googletest_ADDED)
  set(GTest_DIR ${CMAKE_INSTALL_PREFIX}/cmake/googletest/GTest CACHE PATH "GTest Installation path." FORCE)
  install(
    DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/cmake/GTest
    DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/googletest
  )
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_INSTALL_PREFIX}/lib/cmake/GTest)")
endif()

# json library
set(nlohmann_json_VERSION "3.1.1")
add_dependency(nlohmann_json ${nlohmann_json_VERSION}
  GITHUB_REPOSITORY /nlohmann/json
  OPTIONS
    "JSON_BuildTests OFF"
)
if(TARGET nlohmann_json)
  add_library(nlohmann::json ALIAS nlohmann_json)
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

# autodiff
set(autodiff_VERSION "0.6.12")
add_dependency(autodiff ${autodiff_VERSION}
  GITHUB_REPOSITORY autodiff/autodiff
  OPTIONS
    "AUTODIFF_BUILD_TESTS OFF"
    "AUTODIFF_BUILD_PYTHON OFF"
    "AUTODIFF_BUILD_EXAMPLES OFF"
    "AUTODIFF_BUILD_DOCS OFF"
    "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}"
    "CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}"
)
if(autodiff_ADDED)
  set(autodiff_DIR ${CMAKE_INSTALL_PREFIX}/cmake/autodiff)
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
                    conan
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
                    torch 
                    torchviz 
                    uuid
                   )

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
    target_include_directories(cpr INTERFACE
      $<BUILD_INTERFACE:${CPR_INCLUDE_DIRS}>
      $<BUILD_INTERFACE:${CURL_INCLUDE_DIRS}>
    )
    install(TARGETS cpr EXPORT cpr-targets)
    install(EXPORT cpr-targets
        FILE cpr-config.cmake
        NAMESPACE cpr::
        DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/cpr)
  endif()

  # C++ itertools
  add_dependency(cppitertools 2.1
    FIND_PACKAGE_VERSION "2.0" #for manual cmake build 2.0
    GITHUB_REPOSITORY ryanhaining/cppitertools
    OPTIONS
      "cppitertools_INSTALL_CMAKE_DIR share"
  )

  # QASM simulator.
  if (false) # This needs to be re-enabled once the CI image has the qasm-simulator installed.

    set(name "qasm_simulator")
    set(repo "https://github.com/Qiskit/qiskit-aer.git")
    set(tag "0.10.4")
    list(APPEND PREHASH GIT_REPOSITORY ${repo})
    list(APPEND PREHASH GIT_TAG ${tag})
    list(SORT PREHASH)
    string(SHA1 hash "${PREHASH}")
    add_dependency(${name} ${tag}
      GIT_REPOSITORY ${repo}
      GIT_TAG ${tag}
      OPTIONS
        "CMAKE_MODULE_PATH ${CPM_SOURCE_CACHE}/${name}/${hash}/cmake"
        "CMAKE_BUILD_TYPE Release"
    )
    if(qasm_simulator_ADDED)
      set(QB_STANDALONE_AER_EXE ${CMAKE_INSTALL_PREFIX}/bin/qasm_simulator)
    else()
      find_program(QB_STANDALONE_AER_EXE qasm_simulator)
      add_custom_target(qasm_simulator)
    endif()

  else()

    find_program(QB_STANDALONE_AER_EXE qasm_simulator) 
    if(${QB_STANDALONE_AER_EXE} STREQUAL "QB_STANDALONE_AER_EXE-NOTFOUND")
      include(ExternalProject)
      ExternalProject_Add(qasm_simulator
        GIT_REPOSITORY    https://github.com/Qiskit/qiskit-aer.git
        GIT_TAG           0.10.4
        SOURCE_DIR        "${CMAKE_BINARY_DIR}/qiskit-aer-src"
        BINARY_DIR        "${CMAKE_BINARY_DIR}/qiskit-aer-build"
        CMAKE_ARGS    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}
      )
      set(QB_STANDALONE_AER_EXE ${CMAKE_CURRENT_BINARY_DIR}/bin/qasm_simulator)
    else()
      add_custom_target(qasm_simulator)
    endif()

  endif()



endif()

# Get rid of other cmake paths
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_INSTALL_PREFIX}/lib/cmake)")
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_INSTALL_PREFIX}/share)")

# If any packages are still missing, fail.
check_missing()

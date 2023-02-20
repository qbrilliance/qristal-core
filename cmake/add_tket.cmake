# Copyright (c) Quantum Brilliance Pty Ltd

# TKET-related components (implemented as XACC plugins)
# Note: currently, TKET is an optional dependency and is only used for the noise-aware circuit placement plugin.

# Flag to enable TKET noise-aware placement.
set(WITH_TKET OFF CACHE BOOL "Enable TKET for noise-aware circuit placement.")

# Default TKET install directory
set(TKET_DIR /opt/tket CACHE PATH "Search path for TKET installation.")

# Only when WITH_TKET is set
if (WITH_TKET)
  set(TKET_PLACEMENT_LIB tket_placement)
  ##############################
  # Build tket placement plugin
  ##############################
  add_xacc_plugin(${TKET_PLACEMENT_LIB}
    SOURCES
      src/placement/tket_placement.cpp  
  )
  # Use C++20 (TKET requirement)
  set_property(TARGET ${TKET_PLACEMENT_LIB} PROPERTY CXX_STANDARD 20)
  target_link_libraries(${TKET_PLACEMENT_LIB} PRIVATE xacc::xacc xacc::quantum_gate Eigen3::Eigen)
  # Check if there is a valid TKET installation
  if(EXISTS "${TKET_DIR}" AND EXISTS "${TKET_DIR}/lib" AND EXISTS "${TKET_DIR}/include")
    # Include Tket headers and its deps
    target_include_directories(${TKET_PLACEMENT_LIB} PRIVATE 
      ${TKET_DIR}/include 
      ${TKET_DIR}/tkassert/include
      ${TKET_DIR}/tklog/include
      ${TKET_DIR}/tkrng/include
      ${TKET_DIR}/tktokenswap/include
      ${TKET_DIR}/tkwsm/include
      ${TKET_DIR}/nlohmann_json/include 
      ${TKET_DIR}/gmp/include
      ${TKET_DIR}/symengine/include
    )

    # List of TKET libraries for linking
    list(APPEND 
      TKET_LIBS
        tket-ArchAwareSynth
        tket-Architecture
        tket-Characterisation
        tket-Circuit
        tket-Clifford
        tket-Converters
        tket-Diagonalisation
        tket-Gate
        tket-Graphs
        tket-Mapping
        tket-MeasurementSetup
        tket-Ops
        tket-OpType
        tket-PauliGraph
        tket-Placement
        tket-Predicates
        tket-Simulation
        tket-Transformations
        tket-Utils
        tket-ZX
    )

    foreach(TKET_LIB ${TKET_LIBS})
      find_library(${TKET_LIB}_FULL_PATH
        NAMES ${TKET_LIB}
        HINTS ${TKET_DIR}/lib
      )

      if(NOT ${TKET_LIB}_FULL_PATH)
        message(FATAL_ERROR "\nUnable to find ${TKET_LIB} in '${TKET_DIR}/lib' directory. Please check your TKET installation.")
      endif()
      target_link_libraries(${TKET_PLACEMENT_LIB} PRIVATE ${${TKET_LIB}_FULL_PATH})
    endforeach()
  # TKET not found, check whether install missing
  else()
    if (INSTALL_MISSING)
      # Adding Tket via CPM to use as a circuit placement/optimizer
      # TKET doesn't have a top-level CMakeLists.txt (its build system is based on conan) 
      # hence we need to add top-level CMake hooks here to build TKET C++ dirs  
      ##########################################
      # Pull in TKet's dependencies
      ##########################################
      # Note: TKet uses conan to pull its deps, 
      # we just use CPM instead
      
      # TKET needs symengine
      cmake_policy(SET CMP0048 NEW)
      add_dependency(symengine 0.9.0
        GITHUB_REPOSITORY symengine/symengine
        OPTIONS
          "BUILD_TESTS OFF"
          "BUILD_BENCHMARKS OFF"
          "INTEGER_CLASS boostmp"
          "WITH_SYMENGINE_RCP ON"
          "CMAKE_BUILD_TYPE Release"
      )

      #####################
      # Pull in tket by CPM
      #####################
      CPMAddPackage(
        NAME tket
        GITHUB_REPOSITORY CQCL/tket
        VERSION 1.11.1
        GIT_TAG "v1.11.1"
        DOWNLOAD_ONLY YES
        PATCH_COMMAND  git apply ${CMAKE_CURRENT_LIST_DIR}/tket.patch
      )

      # Disable warnings since older gcc versions (e.g., 9.4) may give false-positive warnings
      # and Tket is compiled with -Werror.
      # Note: Tket recommended gcc-11+. 
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
      # We don't use conan, just create a stub for TKet's conan cmake file.
      set(stub_tket_libs_conan_cmake_file "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
      file(RELATIVE_PATH tket_relative ${CMAKE_SOURCE_DIR} ${tket_SOURCE_DIR})
      set(stub_tket_conan_cmake_file "${CMAKE_BINARY_DIR}/${tket_relative}/tket/src/conanbuildinfo.cmake")
      set(dummy_conanbuildinfo_content "macro(conan_basic_setup)\nendmacro()")
      file(WRITE ${stub_tket_libs_conan_cmake_file} "${dummy_conanbuildinfo_content}") 
      file(WRITE ${stub_tket_conan_cmake_file} "${dummy_conanbuildinfo_content}")

      ############################################################
      # Build TKet's utils libs, e.g., its log, assert, etc. utils
      ############################################################
      # Note: the linking deps was specified in conan files, 
      # we just need to convert them to target_link_libraries
      add_subdirectory(${tket_SOURCE_DIR}/libs/tklog/src)
      set_property(TARGET tklog PROPERTY POSITION_INDEPENDENT_CODE ON)
      add_subdirectory(${tket_SOURCE_DIR}/libs/tkrng/src)
      set_property(TARGET tkrng PROPERTY POSITION_INDEPENDENT_CODE ON)
      add_subdirectory(${tket_SOURCE_DIR}/libs/tkassert/src)
      set_property(TARGET tkassert PROPERTY POSITION_INDEPENDENT_CODE ON)
      target_link_libraries(tkassert PRIVATE tklog)
      add_subdirectory(${tket_SOURCE_DIR}/libs/tktokenswap/src)
      set_property(TARGET tktokenswap PROPERTY POSITION_INDEPENDENT_CODE ON)
      target_link_libraries(tktokenswap PRIVATE tklog tkassert tkrng)
      add_subdirectory(${tket_SOURCE_DIR}/libs/tkwsm/src)
      target_link_libraries(tkwsm PRIVATE tkassert tkrng tklog)
      set_property(TARGET tkwsm PROPERTY POSITION_INDEPENDENT_CODE ON)
      target_include_directories(${TKET_PLACEMENT_LIB} PRIVATE ${symengine_SOURCE_DIR} ${symengine_BINARY_DIR})
      target_link_libraries(${TKET_PLACEMENT_LIB} PRIVATE tklog tkrng tkassert tktokenswap tkwsm nlohmann::json)
      ###################################
      # Build main TKet libraries/targets
      ####################################
      add_subdirectory(${tket_SOURCE_DIR}/tket/src)
      get_property(TKET_SUBDIRS DIRECTORY ${tket_SOURCE_DIR}/tket/src PROPERTY SUBDIRECTORIES)
      foreach(SUBDIR IN LISTS TKET_SUBDIRS)
        get_property(TKET_TARGETS DIRECTORY ${SUBDIR} PROPERTY BUILDSYSTEM_TARGETS)
        target_include_directories(${TKET_PLACEMENT_LIB} PRIVATE ${SUBDIR}/include)
        foreach(TKET_TARGET IN LISTS TKET_TARGETS)
          set_property(TARGET ${TKET_TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)
          # Add deps via cmake (rather than conan)
          target_include_directories(${TKET_TARGET} PRIVATE ${symengine_SOURCE_DIR} ${symengine_BINARY_DIR})
          target_link_libraries(${TKET_TARGET} PRIVATE symengine tklog tkrng tkassert tktokenswap tkwsm Eigen3::Eigen nlohmann::json)  
          target_link_libraries(${TKET_PLACEMENT_LIB} PRIVATE ${TKET_TARGET})    
        endforeach()
      endforeach()
    else()
      set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "TKET")
      check_missing()
    endif()
  endif()
endif()
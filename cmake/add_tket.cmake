# Copyright (c) Quantum Brilliance Pty Ltd

# TKET-related components (implemented as XACC plugins)
# Note: currently, TKET is an optional dependency and is only used for the noise-aware circuit placement plugin.

# Flag to enable TKET noise-aware placement.
set(WITH_TKET OFF CACHE BOOL "Enable TKET for noise-aware circuit placement.")

# Only when WITH_TKET is set
if (WITH_TKET)
  set(TKET_PLACEMENT_LIB tket_placement)
  ##############################
  # Build tket placement plugin
  ##############################
  add_xacc_plugin(${TKET_PLACEMENT_LIB}
    SOURCES
      src/tket/tket_placement.cpp
      src/tket/tket_ir_converter.cpp
      src/tket/tket_plugin.cpp
  )
  # Use C++20 (TKET requirement)
  set_property(TARGET ${TKET_PLACEMENT_LIB} PROPERTY CXX_STANDARD 20)
  target_link_libraries(${TKET_PLACEMENT_LIB} PRIVATE Eigen3::Eigen xacc::xacc xacc::quantum_gate nlohmann::json Boost::boost)

  # tket was added as dependency. Check if the installation is valid
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
    # Add Tket lib directory to RPATH
    set_target_properties(${TKET_PLACEMENT_LIB} PROPERTIES
      INSTALL_RPATH "${INSTALL_RPATH}:${TKET_DIR}/lib"
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
    message(STATUS "Loaded tket libs from ${TKET_DIR}/lib")
  else()
    message(ERROR "tket TKET_DIR or TKET_DIR/lib or TKET_DIR/include not found -- check tket installation!")
  endif()
endif()
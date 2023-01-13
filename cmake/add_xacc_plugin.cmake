# Copyright (c) 2022 Quantum Brilliance Pty Ltd

# Make a shared library that is able to function as a XACC plugin
macro(add_xacc_plugin LIBRARY_NAME)

  set(flags)
  set(args NAME DESCRIPTION SOURCE_PATH)
  set(listArgs SOURCES HEADERS DEPENDENCIES)

  cmake_parse_arguments(arg "${flags}" "${args}" "${listArgs}" ${ARGN})

  if (NOT arg_SOURCES)
      message(FATAL_ERROR "[add_xacc_plugin]: SOURCES is a required argument.")
  endif()
  if (NOT XACC_ROOT)
      message(FATAL_ERROR "[add_xacc_plugin]: XACC_ROOT must be set before using this macro.")
  endif()
  if (SOURCES IN_LIST arg_KEYWORDS_MISSING_VALUES)
      message(FATAL_ERROR "[add_xacc_plugin]: SOURCES requires at least one value")
  endif()
  if (HEADERS IN_LIST arg_KEYWORDS_MISSING_VALUES)
      message(FATAL_ERROR "[add_xacc_plugin]: HEADERS requires at least one value")
  endif()

  # Process the path to the core library in the calling project
  if(NOT core_SOURCE_DIR)
    set(core_SOURCE_DIR PROJECT_SOURCE_DIR)
  endif()

  # Use "_plugin_bundle" suffix for bundle symbolic name
  set(QB_XACC_PLUGIN_SYMBOLIC_NAME ${LIBRARY_NAME}_plugin_bundle)

  # Human readable name and description string are optional
  if (arg_NAME)
      set(QB_XACC_PLUGIN_NAME ${arg_NAME})
  else()
      # Use LIBRARY_NAME as default if not specified
      set(QB_XACC_PLUGIN_NAME ${LIBRARY_NAME})
  endif()

  if (arg_DESCRIPTION)
      set(QB_XACC_PLUGIN_DESCRIPTION ${arg_DESCRIPTION})
  else()
      # Use a generic description
      set(QB_XACC_PLUGIN_DESCRIPTION "Quantum Brilliance XACC plugin.")
  endif()

  # Get the directory (in build folder) to put the generated manifest.json in
  list(GET arg_SOURCES 0 FIRST_SOURCE_FILE)
  get_filename_component(REL_SRC_DIR  ${FIRST_SOURCE_FILE} DIRECTORY)
  set(MANIFEST_JSON_DIR ${CMAKE_CURRENT_BINARY_DIR}/${REL_SRC_DIR})
  # Generate manifest.json
  configure_file(${core_SOURCE_DIR}/cmake/manifest.json.in
               ${MANIFEST_JSON_DIR}/manifest.json)

  usfunctiongetresourcesource(TARGET ${LIBRARY_NAME} OUT arg_SOURCES)
  usfunctiongeneratebundleinit(TARGET ${LIBRARY_NAME} OUT arg_SOURCES)

  add_library(${LIBRARY_NAME} SHARED EXCLUDE_FROM_ALL ${arg_SOURCES} ${arg_HEADERS})
  add_dependencies(xacc-plugins ${LIBRARY_NAME})

  target_include_directories(${LIBRARY_NAME}
    PUBLIC
      "${PROJECT_SOURCE_DIR}/include"
      "${core_SOURCE_DIR}/include"
  )

  target_link_libraries(${LIBRARY_NAME}
    PUBLIC
      xacc::xacc
      xacc::quantum_gate
      ${arg_DEPENDENCIES}
  )

  set_target_properties(${LIBRARY_NAME}
    PROPERTIES
      VERSION ${PROJECT_VERSION}
      FRAMEWORK TRUE
      INSTALL_RPATH "$ORIGIN;${CMAKE_INSTALL_RPATH};${XACC_ROOT}/lib"
      COMPILE_DEFINITIONS
        US_BUNDLE_NAME=${QB_XACC_PLUGIN_SYMBOLIC_NAME}
        US_BUNDLE_NAME ${QB_XACC_PLUGIN_SYMBOLIC_NAME}
  )

  # Important: manifest.json must be at the top-level of the WORKING_DIRECTORY
  # when using usfunctionembedresources since this will create a zip file (respecting the full directory structure)
  # to be appended to the binary and CppMicroservices expect to see manifest.json at the top level folder unzipping the embedded resources.
  usfunctionembedresources(
    TARGET
      ${LIBRARY_NAME}
    WORKING_DIRECTORY
      ${MANIFEST_JSON_DIR}
    FILES
      manifest.json
  )

  # Install the library. Headers are automatically copied when installing libqbcore.so.
  install(
    TARGETS ${LIBRARY_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  )

  # Add a symlink to {XACC_ROOT}/plugins where XACC finds its plugins by default.
  # Delete symlinks to all other versions of the plugin to avoid issues when upgrading to a new version.
  # TODO: XACC has an API (xacc::addPluginSearchPath) to add a search path to find plugin's so files.
  # To remove the below symlink step, we need to:
  # (1) Modify the XACC's initialization in the Python binding module to add proper path.
  # (2) Modify plugin gtest files which currently assume vanilla xacc::Initialize()
  file(GLOB OLD_SYMLINKS ${XACC_ROOT}/plugins/*${LIBRARY_NAME}.*)
  foreach(link ${OLD_SYMLINKS})
    # Now instead of just deleting ${link}, which may accidentally match other plugins, we just get each of 
    # the extensions and append it to the actual library name that we know our plugin has.
    # We can't use a wildcard for this, as there is no shell expansion performed inside execute_process. 
    cmake_path(GET link EXTENSION extension)    
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E rm -f ${XACC_ROOT}/plugins/$<TARGET_FILE_PREFIX:${LIBRARY_NAME}>$<TARGET_FILE_BASE_NAME:${LIBRARY_NAME}>${extension})")
  endforeach()
  install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \
     ${CMAKE_INSTALL_PREFIX}/lib/$<TARGET_FILE_NAME:${LIBRARY_NAME}> \
     ${XACC_ROOT}/plugins/$<TARGET_FILE_NAME:${LIBRARY_NAME}>)"
  )

endmacro()


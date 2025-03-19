set(DOXYGEN_MIN_VERSION 1.9.1)
find_package(Doxygen ${DOXYGEN_MIN_VERSION} REQUIRED dot)
if (NOT DOXYGEN_FOUND)
  message(FATAL_ERROR "Cannot locate doxygen. Please install Doxygen or disable documentation build (e.g., do not set -DBUILD_DOCS=ON/TRUE when running cmake).")
endif ()

set(QB_GOOGLE_ANALYTICS_ID G-EML76VL6ZZ)
set(QB_CLARITY_ID i8ah5yqrew)

add_python_package(
  sphinx==5.0.2
  sphinx_rtd_theme
  exhale
  myst-parser
)

# Set up variables to configure the Doxygen config file
# Public headers to generate docs for.
# TODO: refactor to use all cpplib headers once all the headers are doxygen-ready.
# We use strict Doxygen build mode (see below) hence gradually adding more files to this list once they're ready.
set(PUBLIC_HEADERS
  include/qristal/core/circuit_builder.hpp
  include/qristal/core/profiler.hpp
  include/qristal/core/session.hpp
  include/qristal/core/thread_pool.hpp
  include/qristal/core/backends/qb_hardware/qb_qpu.hpp
  include/qristal/core/backends/qb_hardware/qb_visitor.hpp
  include/qristal/core/cudaq/ir_converter.hpp
  include/qristal/core/cudaq/sim_pool.hpp
  include/qristal/core/noise_model/noise_model.hpp
  include/qristal/core/noise_model/noise_properties.hpp
  include/qristal/core/noise_model/readout_error.hpp
  include/qristal/core/optimization/vqee/case_generator.hpp
  include/qristal/core/optimization/vqee/vqee.hpp
  include/qristal/core/passes/base_pass.hpp
  include/qristal/core/passes/noise_aware_placement_config.hpp
  include/qristal/core/passes/noise_aware_placement_pass.hpp
  include/qristal/core/passes/swap_placement_pass.hpp
  include/qristal/core/passes/circuit_opt_passes.hpp
)
# Convert to absolute paths and use space as delimiters (to configure Shpinx's conf.py file)
list(TRANSFORM PUBLIC_HEADERS PREPEND ${PROJECT_SOURCE_DIR}/)
string(REPLACE ";" " " DOXYGEN_PUBLIC_HEADERS "${PUBLIC_HEADERS}")

# If emulator directory is provided, include its readme/getting started guid and API docs
if (EMULATOR_DIR)
  set(EMULATOR_DOXYGEN_CONFIG_FILE ${EMULATOR_DIR}/docs/emulator_docs.cmake)
  # Read the config file, which defines the list of public headers (EMULATOR_PUBLIC_HEADERS)
  include(${EMULATOR_DOXYGEN_CONFIG_FILE})
  string(REPLACE ";" " " EMULATOR_HEADERS_LIST "${EMULATOR_PUBLIC_HEADERS}")
  # Add them to the list of headers to be processed.
  string(APPEND  DOXYGEN_PUBLIC_HEADERS " ${EMULATOR_HEADERS_LIST}")
  # Write an emulator.rst file and symlink README.md from the emulator
  file(COPY_FILE ${PROJECT_SOURCE_DIR}/docs/rst/emulator.rst.enabled ${PROJECT_SOURCE_DIR}/docs/rst/emulator.rst)
  file(CREATE_LINK ${EMULATOR_DIR}/README.md ${PROJECT_SOURCE_DIR}/docs/md/emulator.md SYMBOLIC)
  message(STATUS "Found emulator documentation.")
else()
  # Write a dummy emulator.rst
  file(COPY_FILE ${PROJECT_SOURCE_DIR}/docs/rst/emulator.rst.disabled ${PROJECT_SOURCE_DIR}/docs/rst/emulator.rst)
endif()

set(SPHINX_SOURCE_DIR ${PROJECT_SOURCE_DIR}/docs)
set(SPHINX_OUTPUT_DIR ${PROJECT_BINARY_DIR}/docs)
set(SPHINX_CONFIG_IN  ${CMAKE_CURRENT_LIST_DIR}/conf.py.in)
set(SPHINX_CONFIG_OUT ${SPHINX_OUTPUT_DIR}/conf.py)
set(SPHINX_BUILD_DIR ${SPHINX_OUTPUT_DIR}/_build)
set(RTD_INDEX_FILE ${SPHINX_BUILD_DIR}/html/index.html)

set(GOOGLE_ANALYTICS_CONFIG_IN ${CMAKE_CURRENT_LIST_DIR}/qb_ga.js.in)
set(GOOGLE_ANALTYICS_CONFIG_OUT ${SPHINX_SOURCE_DIR}/static/js/qb_ga.js)

# Set up dependencies for cmake incremental build
# All the extra markdown and reStructuredText files.
# i.e., cmake to invoke the custom target below when these files are changed.
file(GLOB EXTRA_RST_FILES ${SPHINX_SOURCE_DIR}/rst/*.rst)
file(GLOB EXTRA_MARKDOWN_FILES ${SPHINX_SOURCE_DIR}/md/*.md)
file(GLOB EXTRA_STATIC_IMAGE_FILES ${SPHINX_SOURCE_DIR}/static/img/*)
file(GLOB EXTRA_STATIC_STYLE_FILES ${SPHINX_SOURCE_DIR}/static/styles/*)
list(APPEND EXTRA_MARKDOWN_FILES ${PROJECT_SOURCE_DIR}/examples/README.md)
list(APPEND EXTRA_MARKDOWN_FILES ${PROJECT_SOURCE_DIR}/examples/cpp/qristal_cli/README.md)
list(APPEND EXTRA_MARKDOWN_FILES ${PROJECT_SOURCE_DIR}/examples/cpp/vqeeCalculator/README.md)

#Replace variables inside @@ with the current values
configure_file(${SPHINX_CONFIG_IN} ${SPHINX_CONFIG_OUT} @ONLY)
configure_file(${GOOGLE_ANALYTICS_CONFIG_IN} ${GOOGLE_ANALTYICS_CONFIG_OUT} @ONLY)


# Sphinx build flags:
#   (1) "-E -a": force rebuild
#   (2) "-W": treat warnings as errors (e.g., missing Doxygen comments in the files that we want to document)
#   Note: as we add more file to ${PUBLIC_HEADERS}, each file should have proper Doxygen comments
#   (3) "-q": quiet build (less verbose)
set(SPHINX_BUILD_OPTIONS -E -a -W -q)
set(SPHINX_PY_COMMAND ${Python_EXECUTABLE} -m sphinx)
# Add documentation build target
# Copy static docs (rst/md) files to build directory before generating the html documentation.
# The conf.py is auto generated in the build directory and Sphinx needs all files in the same directory
# in order to generate the html.
# This will keep the docs build completely out-of-source.
add_custom_target(CopyRstMdDocs COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SPHINX_SOURCE_DIR}" "${SPHINX_OUTPUT_DIR}")
add_custom_command(OUTPUT ${RTD_INDEX_FILE}
                  # This depends on the Python binding module (target name ${PROJECT_NAME})
                  # since we parse docstrings by dynamically loading the Python module.
                  # TODO: statically generates Python module documentation.
                  DEPENDS
                    ${PUBLIC_HEADERS} ${EMULATOR_PUBLIC_HEADERS} py${PROJECT_NAME}
                    ${EXTRA_MARKDOWN_FILES} ${EXTRA_RST_FILES} ${EXTRA_STATIC_IMAGE_FILES}
                    ${EXTRA_STATIC_STYLE_FILES}
                  COMMAND ${SPHINX_PY_COMMAND} -M html  ${SPHINX_OUTPUT_DIR} ${SPHINX_BUILD_DIR} ${SPHINX_BUILD_OPTIONS}
                  WORKING_DIRECTORY ${SPHINX_OUTPUT_DIR}
                  COMMENT "Generating docs")
add_custom_target(ReadTheDocsHtmlBuild ALL DEPENDS ${RTD_INDEX_FILE} CopyRstMdDocs)

# Install the documentation HTML site to the CMAKE_INSTALL_PREFIX
install(DIRECTORY ${SPHINX_BUILD_DIR}/html DESTINATION ${CMAKE_INSTALL_PREFIX}/docs)

set(DOXYGEN_MIN_VERSION 1.9.1)
find_package(Doxygen ${DOXYGEN_MIN_VERSION} REQUIRED dot)
if (NOT DOXYGEN_FOUND)
  message(FATAL_ERROR "Cannot locate doxygen. Please install Doxygen or disable documentation build (e.g., do not set -DBUILD_DOCS=ON/TRUE when running cmake).")
endif ()

set(QB_GOOGLE_ANALYTICS_ID G-EML76VL6ZZ)
set(QB_CLARITY_ID i8ah5yqrew)

execute_process(COMMAND ${Python_EXECUTABLE} -c "import sphinx" RESULT_VARIABLE SPHINX_EXISTS)
if (SPHINX_EXISTS EQUAL "1")
  message(FATAL_ERROR "Cannot locate sphinx. Please install sphinx 4.5.0 (e.g., 'python3 -m pip install sphinx==4.5.0') or disable documentation build (e.g., do not set -DBUILD_DOCS=ON/TRUE when running cmake).")
endif ()

# Determine sphinx version
# Check sphinx version if required (compatibility issues with exhale for versions >= 5, with sphinx_rtd_theme and myst_parser for versions < 4)
set(SPHINX_PY_COMMAND ${Python_EXECUTABLE} -m sphinx)
execute_process(COMMAND ${SPHINX_PY_COMMAND} --version
                OUTPUT_VARIABLE SPHINX_VERSION
                ERROR_VARIABLE  SPHINX_VERSION)
if(SPHINX_VERSION MATCHES "__main__.py ([0-9]+\\.[0-9]+\\.[0-9]+)")
  set(SPHINX_VERSION_STRING "${CMAKE_MATCH_1}")
  string (REPLACE "." ";" _SPHINX_VERSION "${SPHINX_VERSION_STRING}")
  list(GET _SPHINX_VERSION 0 SPHINX_VERSION_MAJOR)
  list(GET _SPHINX_VERSION 1 SPHINX_VERSION_MINOR)
  list(GET _SPHINX_VERSION 2 SPHINX_VERSION_PATCH)
  if (NOT SPHINX_VERSION_MAJOR EQUAL 4)
    message(FATAL_ERROR "This version of sphinx is likely to cause conflicts with other dependencies. Please install version 4.5.0 by running python3 -m pip install sphinx==4.5.0 --force-reinstall.")
  endif()
else()
  message(FATAL_ERROR "Cannot determine sphinx-build version.")
endif()

# Check required additional Sphinx extensions (Python modules)
find_package(Python COMPONENTS Interpreter REQUIRED)
execute_process(COMMAND ${Python_EXECUTABLE} -c "import sphinx_rtd_theme" RESULT_VARIABLE SPHINX_RTD_THEME_EXISTS)
# Note: exhale also includes breathe as one of its dependencies
execute_process(COMMAND ${Python_EXECUTABLE} -c "import exhale" RESULT_VARIABLE SPHINX_EXHALE_EXISTS)
execute_process(COMMAND ${Python_EXECUTABLE} -c "import myst_parser" RESULT_VARIABLE SPHINX_MY_ST_PARSER_EXISTS)
if(SPHINX_RTD_THEME_EXISTS EQUAL "1" OR SPHINX_EXHALE_EXISTS EQUAL "1" OR SPHINX_MY_ST_PARSER_EXISTS EQUAL "1")
  # Construct the error message with info about all the missing packages.
  set(ERROR_MSG " Missing some required Sphinx extension(s): ")
  if (SPHINX_RTD_THEME_EXISTS EQUAL "1")
    string(CONCAT ERROR_MSG "${ERROR_MSG}" " \n  - 'sphinx_rtd_theme': please install by: python3 -m pip install sphinx_rtd_theme==1.2.0")
  endif()
  if (SPHINX_EXHALE_EXISTS EQUAL "1")
    string(CONCAT ERROR_MSG "${ERROR_MSG}" " \n  - 'exhale': please install by: python3 -m pip install exhale==0.3.6")
  endif()
  if (SPHINX_MY_ST_PARSER_EXISTS EQUAL "1")
    string(CONCAT ERROR_MSG "${ERROR_MSG}" " \n  - 'myst-parser': please install by: python3 -m pip install myst-parser==0.18.1")
  endif()
  message(FATAL_ERROR "${ERROR_MSG}")
endif()

execute_process(COMMAND ${Python_EXECUTABLE} -c "import sphinx_rtd_theme; print(sphinx_rtd_theme.__version__)" OUTPUT_VARIABLE SPHINX_RTD_THEME_VERSION)
execute_process(COMMAND ${Python_EXECUTABLE} -c "import exhale; print(exhale.__version__)" OUTPUT_VARIABLE SPHINX_EXHALE_VERSION)
execute_process(COMMAND ${Python_EXECUTABLE} -c "import myst_parser; print(myst_parser.__version__)" OUTPUT_VARIABLE SPHINX_MY_ST_PARSER_VERSION)

if(NOT SPHINX_RTD_THEME_VERSION STREQUAL "1.2.0\n" OR NOT SPHINX_EXHALE_VERSION STREQUAL "0.3.6\n" OR NOT SPHINX_MY_ST_PARSER_VERSION STREQUAL "0.18.1\n")
  # Construct the error message with info about all the incorrect versions of the packages, and reinstall instructions.
  set(ERROR_MSG "Possible version conflicts with some required Sphinx extension(s): ")
  if (NOT SPHINX_RTD_THEME_VERSION STREQUAL "1.2.0\n")
    string(CONCAT ERROR_MSG "${ERROR_MSG}" " \n  - 'sphinx_rtd_theme': wanted 1.2.0, got ${SPHINX_RTD_THEME_VERSION}")
    string(CONCAT PKGS_TO_UNINSTALL "${PKGS_TO_UNINSTALL}" " sphinx_rtd_theme")
    string(CONCAT PKGS_TO_INSTALL "${PKGS_TO_INSTALL}" " sphinx_rtd_theme==1.2.0")
  endif()
  if (NOT SPHINX_EXHALE_VERSION STREQUAL "0.3.6\n")
    string(CONCAT ERROR_MSG "${ERROR_MSG}" " \n  - 'exhale': wanted 0.3.6, got ${SPHINX_EXHALE_VERSION}")
    string(CONCAT PKGS_TO_UNINSTALL "${PKGS_TO_UNINSTALL}" " exhale")
    string(CONCAT PKGS_TO_INSTALL "${PKGS_TO_INSTALL}" " exhale==0.3.6")
  endif()
  if (NOT SPHINX_MY_ST_PARSER_VERSION STREQUAL "0.18.1\n")
    string(CONCAT ERROR_MSG "${ERROR_MSG}" " \n  - 'myst-parser': wanted 0.18.1, got ${SPHINX_MY_ST_PARSER_VERSION}")
    string(CONCAT PKGS_TO_UNINSTALL "${PKGS_TO_UNINSTALL}" " myst-parser")
    string(CONCAT PKGS_TO_INSTALL "${PKGS_TO_INSTALL}" " myst-parser==0.18.1")
  endif()
  string(CONCAT ERROR_MSG "${ERROR_MSG}" "Please reinstall the correct version of the modules by running the following pip commands in succession:\npython3 -m pip uninstall${PKGS_TO_UNINSTALL}\npython3 -m pip install${PKGS_TO_INSTALL}\n")
  message(FATAL_ERROR "${ERROR_MSG}")
endif()

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
                  DEPENDS ${PUBLIC_HEADERS} ${EMULATOR_PUBLIC_HEADERS} py${PROJECT_NAME} ${EXTRA_MARKDOWN_FILES} ${EXTRA_RST_FILES}
                  COMMAND ${SPHINX_PY_COMMAND} -M html  ${SPHINX_OUTPUT_DIR} ${SPHINX_BUILD_DIR} ${SPHINX_BUILD_OPTIONS}
                  WORKING_DIRECTORY ${SPHINX_OUTPUT_DIR}
                  COMMENT "Generating docs")
add_custom_target(ReadTheDocsHtmlBuild ALL DEPENDS ${RTD_INDEX_FILE} CopyRstMdDocs)

# Install the documentation HTML site to the CMAKE_INSTALL_PREFIX
install(DIRECTORY ${SPHINX_BUILD_DIR}/html DESTINATION ${CMAKE_INSTALL_PREFIX}/docs)

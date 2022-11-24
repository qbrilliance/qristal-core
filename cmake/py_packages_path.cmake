# Copyright Quantum Brilliance
# Simple helper script for telling a user's Python installation where to find the SDK python module.


# If the path is given directly when invoking cmake, just use the value given there.  Otherwise, work it out.
if(NOT PYTHON_PACKAGES_PATH)

  # Work out if the user is running with root privileges or not.
  execute_process(RESULT_VARIABLE FAILURE OUTPUT_VARIABLE IAM COMMAND whoami OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(FAILURE)
    message(FATAL_ERROR "Could not determine if user is running with root privileges or not!")
  endif()

  # Switch according to whether install is run as root or not.
  if (IAM STREQUAL "root")

    # Running as root/sudo, so we should write to the system site-packages dir.
    execute_process(RESULT_VARIABLE FAILURE
                    OUTPUT_VARIABLE PYTHON_PACKAGES_PATH
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    COMMAND python3 -c "import sysconfig; print(sysconfig.get_paths()['purelib'])")
    if(FAILURE)
      message(FATAL_ERROR "Failed to determine the path to your Python site-packages dir. Please set it manually with -DPYTHON_PACKAGES_PATH=/path/to/dir.")
    endif()
    message(STATUS "Installing with root privileges -- writing SDK package info to ${PYTHON_PACKAGES_PATH}.")

  else()
    # Not running with root privileges, so we should try to write to the local site-packages dir.

    # Determine if using venv or not
    execute_process(RESULT_VARIABLE PYTHON_VENV_USED
                    ERROR_VARIABLE DISCARD
                    COMMAND python3 -c "import sys; assert sys.prefix == sys.base_prefix")

    if(PYTHON_VENV_USED)
      execute_process(RESULT_VARIABLE FAILURE
                      OUTPUT_VARIABLE PYTHON_PACKAGES_PATH
                      OUTPUT_STRIP_TRAILING_WHITESPACE
                      COMMAND python3 -c "import sysconfig; print(sysconfig.get_paths()['purelib'])")
    else()
      execute_process(RESULT_VARIABLE FAILURE
                      OUTPUT_VARIABLE PYTHON_PACKAGES_PATH
                      OUTPUT_STRIP_TRAILING_WHITESPACE
                      COMMAND python3 -m site --user-site)
    endif()

    if(FAILURE)
      if(FAILURE STREQUAL "1" OR FAILURE STREQUAL "2")
        message(FATAL_ERROR "Attempting a local install, but failed because your local Python installation has its user site directory (site-packages) disabled. Try enabling it, or installing as root.")
      else()
        message(FATAL_ERROR "Attempting a local install, but failed to determine the path to your local Python site-packages directory. Please set it manually with -DPYTHON_PACKAGES_PATH=/path/to/dir.")
      endif()
    endif()
    message(STATUS "Performing a local install -- writing SDK package info to ${PYTHON_PACKAGES_PATH}.")

  endif()

else()

  message(STATUS "Writing SDK package info to location specified using -DPYTHON_PACKAGES_PATH: ${PYTHON_PACKAGES_PATH}.")

endif()

file(WRITE ${PYTHON_PACKAGES_PATH}/qb${PROJECT_NAME}.pth "${CMAKE_INSTALL_PREFIX}/lib\n${XACC_ROOT}")

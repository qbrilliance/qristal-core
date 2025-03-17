# Copyright (c) Quantum Brilliance Pty Ltd

set(PIP_INSTALL_ARGUMENTS "")

# Stupidly, cmake does not cache the path to the python executable between runs, but nor
# does it rediscover it after the first run either.
if (NOT Python_EXECUTABLE) 
  set(Python_EXECUTABLE PYBIND11_PYTHON_EXECUTABLE_LAST)
endif()

# Add a python package dependency.
#
# Expects a list of pip packages in the same package and version specification format pip uses:
#    <name>==<version>
#
# This function parses the input arguments in the above format to check the system for
# whether those packages are already available. The reason this isn't left up to `pip` to 
# figure out and manage is because the system may have pip packages installed via apt.
# For this reason, we cannot just `pip install` a list of packages otherwise the system
# will end up with the same packages (and potentially different versions) installed
# alongside each other.
function(add_python_package)

  # Iterate over all requested packages to see if they are installed
  foreach(item ${ARGN})

    # Split the list item into a list where item 1 is the package name and item 2 is 
    # the pip package version
    string(REPLACE "==" ";" item ${item})

    unset(package_name)
    unset(package_version)

    list(LENGTH item item_length)
    if(${item_length} EQUAL 1)
      # There was no version supplied so pip will install the latest compatible version
      set(package_name ${item})
    elseif(${item_length} EQUAL 2)
      list(GET item 0 package_name)
      list(GET item 1 package_version)
      message(STATUS "package_name: ${package_name}; package_version: ${package_version}")
    else()
      message(FATAL_ERROR
        "Invalid syntax for add_python_package. Expects pip package in the format <name>:<version>."
      )
    endif()

    # Check if the package is installed in Python as different package managers don't talk to one another
    execute_process(
      COMMAND ${Python_EXECUTABLE} -c 
        "import importlib.metadata; print(importlib.metadata.version(\"${package_name}\"))" 
      RESULT_VARIABLE return_val
      OUTPUT_VARIABLE installed_version
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )

    if(return_val)
      # If importing failed, depending on INSTALL_MISSING_PYTHON, throw an error or install the 
      # missing package using pip.
      if(NOT package_version)
        message(STATUS "Python package ${package_name} not found") 
      else()
        message(STATUS "Python package ${package_name}:${package_version} not found") 
      endif()
      if(INSTALL_MISSING_PYTHON)
        if(package_version)
          list(APPEND PIP_INSTALL_ARGUMENTS ${package_name}==${package_version})
        else()
          list(APPEND PIP_INSTALL_ARGUMENTS ${package_name})
        endif()
      else()
        # Let the parent scope know about this missing package
        set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${package_name} (Python package)" PARENT_SCOPE)
      endif()
    else()
      message(STATUS "Found Python package ${package_name}:${installed_version}")
      if(package_version AND NOT package_version STREQUAL installed_version)
        message(WARNING
          "Python package already on the system ${package_name}:${installed_version} differs from the "
          "version required by this project ${package_version}. Please resolve this situation manually "
          "as this may cause compatibility issues.")
      endif()
    endif()
    
  endforeach()

  # Install pip packages. This will only happen if INSTALL_MISSING_PYTHON is set to ON
  if(PIP_INSTALL_ARGUMENTS)
    message(STATUS "Attempting to install missing dependencies with pip...")

    set(pip_install_command ${Python_EXECUTABLE} -m pip install --upgrade ${PIP_INSTALL_ARGUMENTS})
    execute_process(
      COMMAND ${pip_install_command}
      ERROR_VARIABLE error_output
      ERROR_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE return_val
    )

    string(JOIN " " pip_install_command_str ${pip_install_command})
    if (return_val)
      message(FATAL_ERROR
        "Could not install missing dependencies (${PIP_INSTALL_ARGUMENTS}) using pip.\n"
        "This can be because:\n"
        "   - Pip isn't installed\n"
        "   - There are dependency issues due to pre-existing versions of pip packages that need to be manually resolved\n"
        "Once resolved, please re-run cmake.\n"
        "${pip_install_command_str} output:\n"
        "${error_output}"
      )
    else()
      message(STATUS "...done.")
    endif()

  endif()

endfunction()

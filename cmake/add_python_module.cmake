# Copyright (c) Quantum Brilliance Pty Ltd

# Add a python package dependency.
macro(add_python_module)

  # Iterate over all requested modules
  foreach(module ${ARGN})
    
    # Stupidly, cmake does not cache the path to the python executable between runs, but nor does it rediscover it after the first run either.
    if (NOT Python_EXECUTABLE) 
      set(Python_EXECUTABLE PYBIND11_PYTHON_EXECUTABLE_LAST)
    endif()

    # Split the module entry into module name and pip package name
    string(REPLACE ":" ";" module ${module})
    list(LENGTH module MOD_LEN)
    if(${MOD_LEN} EQUAL 1)
      set(pip_package ${module})
    else()
      list(GET module 1 pip_package)
      list(GET module 0 module)
    endif()

    # Attempt to import the module
    execute_process(COMMAND ${Python_EXECUTABLE} -c "import ${module}" 
                    RESULT_VARIABLE return_val 
                    ERROR_QUIET) 

    # If importing failed, depending on INSTALL_MISSING_PYTHON, throw an error or install the missing module using pip.
    if (return_val)
      message(STATUS "Python module ${module} not found.")    
      if (INSTALL_MISSING_PYTHON)
        message(STATUS "Attempting to install with pip...")    
        execute_process(COMMAND ${Python_EXECUTABLE} -m pip install ${pip_package} --upgrade
                        RESULT_VARIABLE return_val)
        if (return_val)
          message(FATAL_ERROR "Could not install python module ${module} using pip. Please install manually and rerun cmake.")
        else()  
          message(STATUS "...done.")
        endif()
      else()
        set(MISSING_DEPENDENCIES "${MISSING_DEPENDENCIES}" "${module} (Python module)")
      endif()
    else()
      message(STATUS "Found Python module ${module}.")    
    endif()
    
  endforeach()
  
endmacro()

# Copyright (c) Quantum Brilliance Pty Ltd

# Add a python package dependency.
macro(add_python_module)

  # Iterate over all requested modules
  foreach(module ${ARGN})
    
    # Stupidly, cmake does not cache the path to the python executable between runs, but nor does it rediscover it after the first run either.
    if (NOT Python_EXECUTABLE) 
      set(Python_EXECUTABLE PYBIND11_PYTHON_EXECUTABLE_LAST)
    endif()
    
    # Attempt to import the module
    execute_process(COMMAND ${Python_EXECUTABLE} -c "import ${module}" 
                    RESULT_VARIABLE return_val 
                    ERROR_QUIET) 

    # If importing failed, depending on INSTALL_MISSING, throw an error or install the missing module using pip.
    if (return_val)
      message(STATUS "Python module ${module} not found.")    
      if (INSTALL_MISSING)
        message(STATUS "Attempting to installl with pip...")    
        execute_process(COMMAND ${Python_EXECUTABLE} -m pip install ${module} --upgrade
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

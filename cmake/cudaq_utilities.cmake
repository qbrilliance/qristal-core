# Copyright (c) Quantum Brilliance Pty Ltd

# Macro for adding a new CUDAQ executable
macro(add_cudaq_executable TARGET_NAME BACKEND)

  # Check if the core library has been compiled with CUDAQ support
  get_directory_property(DEFS COMPILE_DEFINITIONS)
  if (NOT ";${DEFS};" MATCHES ";WITH_CUDAQ;")
    message(FATAL_ERROR "The Qristal core library was not compiled with CUDA Quantum support. To use the macro add_cudaq_executable(), please rebuild your core library with CUDA Quantum support enabled.")
  endif()

  # Prevent usage of -isystem for dependency includes
  set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)

  # Add the actual executable
  add_executable(${TARGET_NAME} ${ARGN})
  if (NOT CUDAQ_DIR) 
    set(CUDAQ_DIR "/opt/cudaq/lib/cmake")
  endif()

  find_package(CUDAQ REQUIRED)
  
  # Modify CUDAQ to support specification of backend.
  if (${BACKEND} STREQUAL "qpp")
    set(CMAKE_CUDAQ_BACKEND_COMMAND "")
  else()
    set(CMAKE_CUDAQ_BACKEND_COMMAND "--qpu ${BACKEND}")
  endif()
  set(CMAKE_CUDAQ_COMPILE_OBJECT "${CMAKE_CUDAQ_COMPILE_OBJECT} ${CMAKE_CUDAQ_BACKEND_COMMAND}")
  set(CMAKE_CUDAQ_LINK_EXECUTABLE "${CMAKE_CUDAQ_LINK_EXECUTABLE} ${CMAKE_CUDAQ_BACKEND_COMMAND}")

  # Modify CUDAQ to support extra include paths.
  # This will be available in the public release.
  set(CMAKE_INCLUDE_FLAG_CUDAQ "-I")
  set(CMAKE_CUDAQ_COMPILE_OBJECT "${CMAKE_CUDAQ_COMPILE_OBJECT} <DEFINES> <INCLUDES>")
  
  set_target_properties(${TARGET_NAME} PROPERTIES LANGUAGE CUDAQ)
   
  # Add the core's include dir with -I instead of -isystem
  target_include_directories(${TARGET_NAME} PRIVATE ${qbcore_DIR}/${qbcore_LIBDIR}/../include)

  # Intercept the link file created by the cmake generate step before it is used, and change all full-path linkages to uses of -L and -l.
  # Also add -Wl,-rpath, to avoid runtime "library not found" issues, and remove the curl library as it will clash with that brought in by CUDAQ. 
  add_custom_command(TARGET ${TARGET_NAME}
      PRE_LINK
      COMMAND sed -i "s# \\(/[^ ]*/\\)lib\\([^ /]*\\)\\.\\(a\\|so[^ ]*\\)# -Wl,-rpath,\\1 -L\\1 -l\\2#g" ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir/link.txt
      COMMAND sed -i "s#-lcurl ##"  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir/link.txt
      DEPENDS ${TARGET_NAME}
      VERBATIM
  )

endmacro()

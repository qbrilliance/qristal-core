# Copyright (c) Quantum Brilliance Pty Ltd

# Macro for adding a new QODA executable
macro(add_qoda_executable TARGET_NAME)

  # Prevent usage of -isystem for dependency includes
  set(CMAKE_NO_SYSTEM_FROM_IMPORTED ON)

  # Add the actual executable
  add_executable(${TARGET_NAME} ${ARGN})
  if (NOT QODA_DIR) 
    set(QODA_DIR "/opt/qoda/lib/cmake")
  endif()

  find_package(QODA REQUIRED)
  
  # Modify QODA to support extra include paths.
  # This will be avaialble in the public release.
  set(CMAKE_INCLUDE_FLAG_QODA "-I")
  set(CMAKE_QODA_COMPILE_OBJECT "${CMAKE_QODA_COMPILE_OBJECT} <DEFINES> <INCLUDES>")
  
  set_target_properties(${TARGET_NAME} PROPERTIES LANGUAGE QODA)
   
  # Add the core's include dir with -I instead of -isystem
  target_include_directories(${TARGET_NAME} PRIVATE ${qbcore_DIR}/lib/../include)

  # Intercept the link file created by the cmake generate step before it is used, and change all full-path linkages to uses of -L and -l.
  # (also, adding -Wl,-rpath, to avoid runtime "library not found" issues) 
  add_custom_command(TARGET ${TARGET_NAME}
      PRE_LINK
      COMMAND sed -i "s# \\(/[^ ]*/\\)lib\\([^ /]*\\)\\.\\(a\\|so[^ ]*\\)# -Wl,-rpath,\\1 -L\\1 -l\\2#g" ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir/link.txt
      DEPENDS ${TARGET_NAME}
      VERBATIM
  )

endmacro()

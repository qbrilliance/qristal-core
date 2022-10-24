find_package(Python COMPONENTS Interpreter Development)
if(Python_FOUND)
  # Upgrade conan to build standalone qiskit-aer for mock testing
  execute_process(COMMAND ${Python_EXECUTABLE} -m pip install conan --upgrade --quiet)
endif()

include(ExternalProject)
ExternalProject_Add(qiskit-aer
  GIT_REPOSITORY    https://github.com/Qiskit/qiskit-aer.git
  GIT_TAG           0.10.4
  SOURCE_DIR        "${CMAKE_BINARY_DIR}/qiskit-aer-src"
  BINARY_DIR        "${CMAKE_BINARY_DIR}/qiskit-aer-build"
  CMAKE_ARGS    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}
)

set(QB_STANDALONE_AER_EXE ${CMAKE_CURRENT_BINARY_DIR}/bin/qasm_simulator)

configure_file(fake_aer_server.py.in
  ${CMAKE_BINARY_DIR}/plugins/qb_lambda/tests/fake_aer_server.py
)

add_gtest(QBLambda)

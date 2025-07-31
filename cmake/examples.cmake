# Copyright (c) Quantum Brilliance Pty Ltd

# Add a C++ example that gets compiled when building and installed unbuilt when installing
macro(add_example NAME)

  set(flags CUDAQ)
  set(multiValueArgs SOURCES EXTRAS)
  cmake_parse_arguments(arg "${flags}" "" "${multiValueArgs}" "${ARGN}")
  if (SOURCES IN_LIST arg_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "[add_example]: SOURCES requires at least one value")
  endif()

  if (WITH_CUDAQ OR NOT arg_CUDAQ)
    add_executable(${NAME} ${arg_SOURCES})
  endif()

  if(TARGET ${NAME})
    set_property(TARGET ${NAME} PROPERTY BUILD_RPATH "${CMAKE_BUILD_DIR};${XACC_ROOT}/lib")
    target_link_libraries(${NAME} PRIVATE qristal::core)
  endif()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/${NAME}/CMakeLists.txt.in
                  ${CMAKE_BINARY_DIR}/configured_example_files/${NAME}/CMakeLists.txt
                  @ONLY)
  install(
    FILES
      ${arg_SOURCES}
      ${arg_EXTRAS}
      ${CMAKE_BINARY_DIR}/configured_example_files/${NAME}/CMakeLists.txt
    DESTINATION ${CMAKE_INSTALL_PREFIX}/examples/cpp/${NAME}
  )

endmacro()


# Build and install examples by default
set(WITH_EXAMPLES ON CACHE BOOL "Enable examples")
if(WITH_EXAMPLES)
  add_example(demo1 SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/demo1/demo1.cpp)
  add_example(bell_with_SPAM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/bell_with_SPAM/bell_with_SPAM.cpp)
  add_example(h1 SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/h1/h1.cpp)
  add_example(noise_model SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/noise_model/noise_model.cpp)
  add_example(noise_model_custom_channel SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/noise_model_custom_channel/noise_model_custom_channel.cpp)
  add_example(vqee SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/vqee/vqee_example.cpp)
  add_example(vqeeCalculator SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/vqeeCalculator/vqee_calculator.cpp)
  add_example(qb_mpdo_noisy SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qb_mpdo_noisy/qb_mpdo_noisy.cpp)
  add_example(qb_purification_noisy SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qb_purification_noisy/qb_purification_noisy.cpp)
  add_example(qb_mps_noisy SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qb_mps_noisy/qb_mps_noisy.cpp)
  add_example(parametrization SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/parametrization/parametrization_demo.cpp)
  add_example(qst_fidelity SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qst_fidelity/qst_fidelity_example.cpp)
  add_example(qst SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qst/qst_example.cpp)
  add_example(qpt_fidelity_CZ SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qpt_fidelity_CZ/qpt_fidelity_CZ_example.cpp)
  add_example(qpt_fidelity_rotation_sweep SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qpt_fidelity_rotation_sweep/qpt_fidelity_rotation_sweep_example.cpp)
  add_example(qpt SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qpt/qpt_example.cpp)
  add_example(noise_model_custom_kraus SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/noise_model_custom_kraus/noise_model_custom_kraus.cpp)
  add_example(noise_model_custom_parameterized SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/noise_model_custom_parameterized/noise_model_custom_parameterized.cpp)
  add_example(noise_model_custom_channel_qb_gateset SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/noise_model_custom_channel_qb_gateset/noise_model_custom_channel_qb_gateset.cpp)
  add_example(qft SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/qft/qft.cpp)
  if (WITH_CUDAQ)
    add_example(benchmark1_qasm SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/benchmark1_qasm/benchmark1_qasm.cpp)
    add_example(benchmark1_cudaq CUDAQ SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/benchmark1_cudaq/benchmark1_cudaq.cpp)
    add_example(cudaq_vqe_cobyla CUDAQ SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/cudaq_vqe_cobyla/vqe-cobyla-cudaq.cpp)
    add_example(cudaq_vqe_lbfgs CUDAQ SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/cudaq_vqe_lbfgs/vqe-lbfgs-cudaq.cpp)
    add_example(cudaq_vqe_hydrogens CUDAQ SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/cudaq_vqe_hydrogens/vqe-hydrogens-cudaq.cpp EXTRAS ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/cudaq_vqe_hydrogens/gen_h_chain.py)
    add_example(cudaq_qft SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/cudaq_qft/cudaq_qft.cpp)
  endif()

  if (WITH_TKET)
    add_example(noise_aware_placement_simple SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/noise_aware_placement_simple/noise_aware_placement.cpp)
    add_example(noise_aware_placement_aws SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/noise_aware_placement_aws/noise_aware_placement_aws.cpp EXTRAS ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/noise_aware_placement_aws/aws_rigetti.yaml)
  endif()

  if (WITH_PROFILING)
    add_example(runtime_profiling SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/runtime_profiling/runtime_profiling.cpp)
  endif()

  if (WITH_MPI)
    add_example(mpi_demo
        SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/mpi_demo/mpi_demo.cpp
    )
    add_example(mpi_multi_qpu_demo
      SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/mpi_multi_qpu_demo/mpi_multi_qpu_demo.cpp
      EXTRAS
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/mpi_multi_qpu_demo/docker-compose.yaml
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/mpi_multi_qpu_demo/localhost_vqpus.yaml
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/mpi_multi_qpu_demo/mpi_process_accelerators.yaml
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/mpi_multi_qpu_demo/vqpu.system_config.json
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/cpp/mpi_multi_qpu_demo/README.md
    )
  endif()

  install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/python
    DESTINATION ${CMAKE_INSTALL_PREFIX}/examples
  )

  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/README.md
    DESTINATION ${CMAKE_INSTALL_PREFIX}/examples
  )

endif()

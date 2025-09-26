# Copyright (c) Quantum Brilliance Pty Ltd

# Gateset transpiler module (to be used by emulator)
add_xacc_plugin(qb_gateset_transpiler
  SOURCES
    src/transpiler/QuantumBrillianceGateSetTranspiler.cpp
    src/backends/hardware/qb/visitor.cpp
    src/backends/hardware/qb/visitor_CZ.cpp
  DEPENDENCIES
    nlohmann::json
    Eigen3::Eigen
)

# Custom QB QObj compiler (to be used by emulator)
add_xacc_plugin(qb_qobj_compiler
  SOURCES
    src/qobj/QuantumBrillianceQobjCompiler.cpp
    src/qobj/QobjCompilersActivator.cpp
  HEADERS
    include/qristal/core/qobj/QuantumBrillianceQobjCompiler.hpp
  DEPENDENCIES
    nlohmann::json
)

if (NOT SUPPORT_EMULATOR_BUILD_ONLY)

# Amplitude estimation
add_xacc_plugin(algorithm_ae
  SOURCES
    src/algorithms/amplitude_estimation/amplitude_estimation_algo_activator.cpp
    src/algorithms/amplitude_estimation/canonical_amplitude_estimation.cpp
    src/algorithms/amplitude_estimation/ML_amplitude_estimation.cpp
  HEADERS
    include/qristal/core/algorithms/amplitude_estimation/canonical_amplitude_estimation.hpp
    include/qristal/core/algorithms/amplitude_estimation/ML_amplitude_estimation.hpp
)

# Exponential search
add_xacc_plugin(algorithm_es
  SOURCES
    src/algorithms/exponential_search/exponential_search_algo_activator.cpp
    src/algorithms/exponential_search/exponential_search.cpp
  HEADERS
    include/qristal/core/algorithms/exponential_search/exponential_search.hpp
)

# Offload to AWS Braket
add_xacc_plugin(aws_braket
  SOURCES
    src/backends/sims/aws/braket/AWSAccelerator.cpp
    src/backends/sims/aws/braket/aws_python_script.py
    src/backends/sims/aws/braket/AWSQuantumTask.cpp
  HEADERS
    include/qristal/core/backends/sims/aws/braket/AWSAccelerator.hpp
    include/qristal/core/backends/sims/aws/braket/AWSOpenQasm3Visitor.hpp
    include/qristal/core/backends/sims/aws/braket/AWSQuantumTask.hpp
    include/qristal/core/backends/sims/aws/braket/AWSVisitor.hpp
  DEPENDENCIES
    Python::Python
    pybind11::pybind11
    qristal::core
)
# Install additional runtime assets
install(
  FILES src/backends/sims/aws/braket/aws_python_script.py
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${qristal_core_LIBDIR}
)

# Circuit library
add_xacc_plugin(circuits
  SOURCES
    src/circuits/ae_to_metric.cpp
    src/circuits/amplitude_amplification.cpp
    src/circuits/canonical_amplitude_estimation.cpp
    src/circuits/comparator.cpp
    src/circuits/compare_beam_oracle.cpp
    src/circuits/compare_gt.cpp
    src/circuits/controlled_addition.cpp
    src/circuits/controlled_multiplication.cpp
    src/circuits/controlled_proper_fraction_division.cpp
    src/circuits/controlled_subtraction.cpp
    src/circuits/controlled_swap.cpp
    src/circuits/efficient_encoding.cpp
    src/circuits/equality_checker.cpp
    src/circuits/generalised_mcx.cpp
    src/circuits/init_rep_flag.cpp
    src/circuits/inverse_circuit.cpp
    src/circuits/mcu_with_ancilla.cpp
    src/circuits/mean_value_finder.cpp
    src/circuits/multiplication.cpp
    src/circuits/phase_estimation.cpp
    src/circuits/proper_fraction_division.cpp
    src/circuits/pseudo_trace_amplitude_estimation.cpp
    src/circuits/circuits_activator.cpp
    src/circuits/qd_beam_state_prep.cpp
    src/circuits/q_prime_unitary.cpp
    src/circuits/ripple_adder.cpp
    src/circuits/subtraction.cpp
    src/circuits/superposition_adder.cpp
    src/circuits/u_prime_unitary.cpp
    src/circuits/uq_prime_unitary.cpp
    src/circuits/w_prime_unitary.cpp
  HEADERS
    include/qristal/core/circuits/ae_to_metric.hpp
    include/qristal/core/circuits/amplitude_amplification.hpp
    include/qristal/core/circuits/canonical_amplitude_estimation.hpp
    include/qristal/core/circuits/comparator.hpp
    include/qristal/core/circuits/compare_beam_oracle.hpp
    include/qristal/core/circuits/compare_gt.hpp
    include/qristal/core/circuits/controlled_addition.hpp
    include/qristal/core/circuits/controlled_multiplication.hpp
    include/qristal/core/circuits/controlled_proper_fraction_division.hpp
    include/qristal/core/circuits/controlled_subtraction.hpp
    include/qristal/core/circuits/controlled_swap.hpp
    include/qristal/core/circuits/efficient_encoding.hpp
    include/qristal/core/circuits/equality_checker.hpp
    include/qristal/core/circuits/generalised_mcx.hpp
    include/qristal/core/circuits/init_rep_flag.hpp
    include/qristal/core/circuits/inverse_circuit.hpp
    include/qristal/core/circuits/mcu_with_ancilla.hpp
    include/qristal/core/circuits/mean_value_finder.hpp
    include/qristal/core/circuits/multiplication.hpp
    include/qristal/core/circuits/phase_estimation.hpp
    include/qristal/core/circuits/proper_fraction_division.hpp
    include/qristal/core/circuits/pseudo_trace_amplitude_estimation.hpp
    include/qristal/core/circuits/qd_beam_state_prep.hpp
    include/qristal/core/circuits/q_prime_unitary.hpp
    include/qristal/core/circuits/ripple_adder.hpp
    include/qristal/core/circuits/subtraction.hpp
    include/qristal/core/circuits/superposition_adder.hpp
    include/qristal/core/circuits/u_prime_unitary.hpp
    include/qristal/core/circuits/uq_prime_unitary.hpp
    include/qristal/core/circuits/w_prime_unitary.hpp
)

# Sparse simulator
add_xacc_plugin(sparse_simulator
  SOURCES
    src/backends/sims/microsoft/sparse-sim/SparseStateVecAccelerator.cpp
  HEADERS
    include/qristal/core/backends/sims/microsoft/sparse-sim/basic_quantum_state.hpp
    include/qristal/core/backends/sims/microsoft/sparse-sim/gates.h
    include/qristal/core/backends/sims/microsoft/sparse-sim/quantum_state.hpp
    include/qristal/core/backends/sims/microsoft/sparse-sim/SparseSimulator.h
    include/qristal/core/backends/sims/microsoft/sparse-sim/types.h
)

# UCSSD quantum chemistry
add_xacc_plugin(uccsd
  SOURCES
    src/uccsd/uccsd.cpp
  HEADERS
    include/qristal/core/uccsd/fermionic_excitation_generator.hpp
  DEPENDENCIES
    cppitertools::cppitertools
    Eigen3::Eigen
)

# VQE
add_xacc_plugin(vqe
  SOURCES
    src/vqe/VariationalQuantumEigenSolver.cpp
)

endif()

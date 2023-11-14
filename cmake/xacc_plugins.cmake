# Copyright (c) 2022 Quantum Brilliance Pty Ltd

# Gateset transpiler module (to be used by emulator)
add_xacc_plugin(qb_gateset_transpiler
  SOURCES
    src/transpiler/QuantumBrillianceGateSetTranspiler.cpp
    src/QuantumBrillianceRemoteVisitor.cpp
  DEPENDENCIES
    nlohmann::json
)

# Custom QB QObj compiler (to be used by emulator)
add_xacc_plugin(qb_qobj_compiler
  SOURCES
    src/qobj/QuantumBrillianceQobjCompiler.cpp
    src/qobj/QobjCompilersActivator.cpp
  HEADERS
    include/qb/core/qobj/QuantumBrillianceQobjCompiler.hpp
  DEPENDENCIES
    nlohmann::json
)

if (NOT SUPPORT_EMULATOR_BUILD_ONLY)

# optimization modules
add_xacc_plugin(optimizationModules
  SOURCES
    src/optimization/qaoa/qaoa_warmStart_activator.cpp
    src/optimization/qaoa/qaoa_warmStart_algorithm.cpp
    src/optimization/qaoa/qaoa_warmStart_circuit.cpp
  HEADERS
    include/qb/core/optimization/qaoa/qaoa_warmStart_algorithm.hpp
    include/qb/core/optimization/qaoa/qaoa_warmStart_circuit.hpp
)

# Amplitude estimation
add_xacc_plugin(algorithm_ae
  SOURCES
    src/algorithms/amplitude_estimation/amplitude_estimation_algo_activator.cpp
    src/algorithms/amplitude_estimation/canonical_amplitude_estimation.cpp
    src/algorithms/amplitude_estimation/ML_amplitude_estimation.cpp
  HEADERS
    include/qb/core/algorithms/amplitude_estimation/canonical_amplitude_estimation.hpp
    include/qb/core/algorithms/amplitude_estimation/ML_amplitude_estimation.hpp
)

# Exponential search
add_xacc_plugin(algorithm_es
  SOURCES
    src/algorithms/exponential_search/exponential_search_algo_activator.cpp
    src/algorithms/exponential_search/exponential_search.cpp
  HEADERS
    include/qb/core/algorithms/exponential_search/exponential_search.hpp
)

# Offload to AWS Braket
add_xacc_plugin(aws_braket
  SOURCES
    src/aws_braket/AWSAccelerator.cpp
    src/aws_braket/aws_python_script.py
    src/aws_braket/AWSQuantumTask.cpp
  HEADERS
    include/qb/core/aws_braket/AWSAccelerator.hpp
    include/qb/core/aws_braket/AWSOpenQasm3Visitor.hpp
    include/qb/core/aws_braket/AWSQuantumTask.hpp
    include/qb/core/aws_braket/AWSVisitor.hpp
  DEPENDENCIES
    Python::Python
    pybind11::pybind11
    qb::core
)
# Install additional runtime assets
install(
  FILES src/aws_braket/aws_python_script.py
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${qbcore_LIBDIR}
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
    include/qb/core/circuits/ae_to_metric.hpp
    include/qb/core/circuits/amplitude_amplification.hpp
    include/qb/core/circuits/canonical_amplitude_estimation.hpp
    include/qb/core/circuits/comparator.hpp
    include/qb/core/circuits/compare_beam_oracle.hpp
    include/qb/core/circuits/compare_gt.hpp
    include/qb/core/circuits/controlled_addition.hpp
    include/qb/core/circuits/controlled_multiplication.hpp
    include/qb/core/circuits/controlled_proper_fraction_division.hpp
    include/qb/core/circuits/controlled_subtraction.hpp
    include/qb/core/circuits/controlled_swap.hpp
    include/qb/core/circuits/efficient_encoding.hpp
    include/qb/core/circuits/equality_checker.hpp
    include/qb/core/circuits/generalised_mcx.hpp
    include/qb/core/circuits/init_rep_flag.hpp
    include/qb/core/circuits/inverse_circuit.hpp
    include/qb/core/circuits/mcu_with_ancilla.hpp
    include/qb/core/circuits/mean_value_finder.hpp
    include/qb/core/circuits/multiplication.hpp
    include/qb/core/circuits/phase_estimation.hpp
    include/qb/core/circuits/proper_fraction_division.hpp
    include/qb/core/circuits/pseudo_trace_amplitude_estimation.hpp
    include/qb/core/circuits/qd_beam_state_prep.hpp
    include/qb/core/circuits/q_prime_unitary.hpp
    include/qb/core/circuits/ripple_adder.hpp
    include/qb/core/circuits/subtraction.hpp
    include/qb/core/circuits/superposition_adder.hpp
    include/qb/core/circuits/u_prime_unitary.hpp
    include/qb/core/circuits/uq_prime_unitary.hpp
    include/qb/core/circuits/w_prime_unitary.hpp
)

# Offload to QB Lambda server
add_xacc_plugin(qb_lambda
  SOURCES
    src/lambda/QBLambdaRemoteAccelerator.cpp
  DEPENDENCIES
    cpr
)

# Sparse simulator
add_xacc_plugin(sparse_simulator
  SOURCES
    src/sparse_simulator/SparseStateVecAccelerator.cpp
  HEADERS
    include/qb/core/sparse_simulator/basic_quantum_state.hpp
    include/qb/core/sparse_simulator/gates.h
    include/qb/core/sparse_simulator/quantum_state.hpp
    include/qb/core/sparse_simulator/SparseSimulator.h
    include/qb/core/sparse_simulator/types.h
)

# UCSSD quantum chemistry
add_xacc_plugin(uccsd
  SOURCES
    src/uccsd/uccsd.cpp
  HEADERS
    include/qb/core/uccsd/fermionic_excitation_generator.hpp
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

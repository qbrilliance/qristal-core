// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#include "cudaq/algorithm.h"
#include "cudaq/gradients/central_difference.h"
#include "cudaq/optimizers.h"
#include "cudaq/spin_op.h"
#include "qristal/core/cudaq/ir_converter.hpp"
#include "qristal/core/session.hpp"
#include "qristal/core/circuit_builder.hpp"

#include "xacc.hpp"
#include "xacc_service.hpp"
#include <cudaq.h>
#include <gtest/gtest.h>

// Define a quantum kernel with CUDAQ at compile time.
template <std::size_t N> struct ghz {
  auto operator()() __qpu__ {
    cudaq::qreg<N> q;
    h(q[0]);
    for (int i = 0; i < N - 1; i++) {
      x<cudaq::ctrl>(q[i], q[i + 1]);
    }
    mz(q);
  }
};

TEST(CudaqTester, check_kernel_execution) {
  // And we're off!
  std::cout << "Executing C++ CUDAQ test..." << std::endl;

  // Make a Qristal session
  auto my_sim = qristal::session(false);

  // Number of qubits we want to run
  constexpr int NB_QUBITS = 20;

  // Add CUDAQ ghz kernel to the current session
  my_sim.set_cudaq_kernel(ghz<NB_QUBITS>{});
  // Use CUDAQ qpp backend
  my_sim.set_acc("cudaq:qpp");
  // Set up sensible default parameters
  my_sim.init();

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(20000);
  my_sim.set_qn(NB_QUBITS);
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  // Print the cumulative results
  std::cout << "Results:" << std::endl
            << my_sim.results()[0][0] << std::endl;
  const auto& res = my_sim.results()[0][0];
  EXPECT_EQ(res.size(), 2);
  int sum = std::accumulate(res.begin(), res.end(), 0,
             [](auto prev_sum, auto &entry) { return prev_sum + entry.second; });
  EXPECT_EQ(sum, 20000);
}

TEST(CudaqTester, check_vqe_cobyla) {
  // And we're off!
  std::cout << "Executing C++ test: Solving Deuteron's ground state energy ..."
            << std::endl;
  xacc::Initialize();
  xacc::qasm(R"(
        .compiler xasm
        .circuit deuteron_ansatz
        .parameters theta
        .qbit q
        X(q[0]);
        Ry(q[1], theta);
        CNOT(q[1],q[0]);
    )");

  std::cout << "Compiled ansatz with Qristal..." << std::endl;
  auto ansatz = xacc::getCompiled("deuteron_ansatz");
  std::cout << "QB IR:\n" << ansatz->toString() << "\n";

  qristal::cudaq_ir_converter converter(ansatz);
  std::cout << "Converted ansatz to CUDAQ (Quake IR) ..." << std::endl;
  auto &cudaq_builder = converter.get_cudaq_builder();
  std::cout << "CUDAQ QUAKE: \n" << cudaq_builder.to_quake();

  cudaq::spin_op h = 5.907 - 2.1433 * cudaq::spin::x(0) * cudaq::spin::x(1) -
                     2.1433 * cudaq::spin::y(0) * cudaq::spin::y(1) +
                     .21829 * cudaq::spin::z(0) - 6.125 * cudaq::spin::z(1);
  std::cout << "Constructed Deuteron Hamiltonian as CUDAQ spin_op: \n";
  h.dump();

  // Run VQE with the builder
  cudaq::optimizers::cobyla c_opt;
  std::cout << "Running VQE with Cobyla optimizer! \n";
  auto [opt_val, opt_params] =
      cudaq::vqe(cudaq_builder, h, c_opt, /*n_params*/ 1);

  std::cout << "Ground state energy (expected -1.74886): " << opt_val << "\n";
  EXPECT_NEAR(opt_val, -1.74886, 1e-3);
}

TEST(CudaqTester, check_vqe_lbfgs) {
  // And we're off!
  std::cout << "Executing C++ test: Solving Deuteron's ground state energy ..."
            << std::endl;
  xacc::Initialize();
  xacc::qasm(R"(
        .compiler xasm
        .circuit deuteron_ansatz
        .parameters theta
        .qbit q
        X(q[0]);
        Ry(q[1], 0.125 * theta);
        CNOT(q[1],q[0]);
    )");
  std::cout << "Compiled ansatz with Qristal..." << std::endl;

  auto ansatz = xacc::getCompiled("deuteron_ansatz");
  std::cout << "QB IR:\n" << ansatz->toString() << "\n";
  qristal::cudaq_ir_converter converter(ansatz);
  std::cout << "Converted ansatz to CUDAQ (Quake IR) ..." << std::endl;

  auto &cudaq_builder = converter.get_cudaq_builder();
  std::cout << "CUDAQ QUAKE: \n" << cudaq_builder.to_quake();

  cudaq::spin_op h = 5.907 - 2.1433 * cudaq::spin::x(0) * cudaq::spin::x(1) -
                     2.1433 * cudaq::spin::y(0) * cudaq::spin::y(1) +
                     .21829 * cudaq::spin::z(0) - 6.125 * cudaq::spin::z(1);

  std::cout << "Constructed Deuteron Hamiltonian as CUDAQ spin_op: \n";
  h.dump();
  // Run VQE with the builder

  cudaq::optimizers::lbfgs l_opt;
  cudaq::gradients::central_difference gradient(cudaq_builder);
  std::cout << "Running VQE with L-BFGS optimizer, central difference gradient "
               "calculator! \n";

  auto [opt_val, opt_params] =
      cudaq::vqe(cudaq_builder, gradient, h, l_opt, /*n_params*/ 1);

  std::cout << "Ground state energy (expected -1.74886): " << opt_val << "\n";
  EXPECT_NEAR(opt_val, -1.74886, 1e-3);
}

#ifdef ENABLE_CUDA_TESTS
TEST(CudaqTester, check_kernel_execution_custatevec) {
  // And we're off!
  std::cout << "Executing C++ CUDAQ test..." << std::endl;

  // Make a Qristal session
  auto my_sim = qristal::session(true);

  // Number of qubits we want to run
  // Large number of qubits, since we are using GPUs!
  constexpr int NB_QUBITS = 31;

  // Add CUDAQ ghz kernel to the current session
  my_sim.set_cudaq_kernel(ghz<NB_QUBITS>{});

  // Set up sensible default parameters
  my_sim.init();
  // Both custatevec_fp32 and custatevec_fp64 are okay,
  // use f32 to speed up the test.
  my_sim.set_acc("cudaq:custatevec_fp32");
  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(20000);
  my_sim.set_qn(NB_QUBITS);
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  // Print the cumulative results
  const auto& res = my_sim.results()[0][0];
  std::cout << "Results:" << std::endl << res << std::endl;
  EXPECT_EQ(res.size(), 2);
  int sum = std::accumulate(res.begin(), res.end(), 0,
             [](auto prev_sum, auto &entry) { return prev_sum + entry.second; });
  EXPECT_EQ(sum, 20000);
}
#endif

TEST(CudaqTester, check_openqasm_on_cudaq_backend) {
  // And we're off!
  std::cout << "Executing C++ CUDAQ test..." << std::endl;

  // Make a Qristal session
  auto my_sim = qristal::session(false);

  // Define the quantum program to run (aka 'quantum kernel' aka 'quantum
  // circuit')
  const std::string targetCircuit = R"(
    __qpu__ void MY_QUANTUM_CIRCUIT(qreg q)
    {
      OPENQASM 2.0;
      include "qelib1.inc";
      creg c[2];
      h q[0];
      cx q[0], q[1];
      measure q[1] -> c[1];
      measure q[0] -> c[0];
    }
    )";

  // Hand the kernel over to the sim object
  my_sim.set_instring(targetCircuit);

  // Set up sensible default parameters
  my_sim.init();

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(100);
  my_sim.set_qn(2);
  // Use CUDAQ "dm" backend
  my_sim.set_acc("cudaq:dm");
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();
  // Print the cumulative results
  const auto& res = my_sim.results()[0][0];
  std::cout << "Results:" << std::endl  << res << std::endl;
  EXPECT_EQ(res.size(), 2);
  int sum = std::accumulate(res.begin(), res.end(), 0,
             [](auto prev_sum, auto &entry) { return prev_sum + entry.second; });
  EXPECT_EQ(sum, 100);
}

TEST(CudaqTester, check_circuit_builder_on_cudaq_backend) {
  // And we're off!
  std::cout << "Executing C++ CUDAQ test..." << std::endl;

  // Make a Qristal session
  auto my_sim = qristal::session(false);

  qristal::CircuitBuilder circ;
  circ.H(0);
  circ.CNOT(0, 1);
  circ.MeasureAll(2);
  // Hand the CircuitBuilder over to the sim object
  my_sim.set_irtarget_m(circ.get());

  // Set up sensible default parameters
  my_sim.init();

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(100);
  my_sim.set_qn(2);
  // Use CUDAQ "dm" backend
  my_sim.set_acc("cudaq:dm");
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();
  // Print the cumulative results
  const auto& res = my_sim.results()[0][0];
  std::cout << "Results:" << std::endl << res << std::endl;
  EXPECT_EQ(res.size(), 2);
  int sum = std::accumulate(res.begin(), res.end(), 0,
             [](auto prev_sum, auto &entry) { return prev_sum + entry.second; });
  EXPECT_EQ(sum, 100);
}

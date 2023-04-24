// Copyright (c) 2023 Quantum Brilliance Pty Ltd
#include "cudaq/algorithm.h"
#include "cudaq/gradients/central_difference.h"
#include "cudaq/optimizers.h"
#include "cudaq/spin_op.h"
#include "qb/core/cudaq/ir_converter.hpp"
#include "qb/core/session.hpp"

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

  // Make a QB SDK session
  auto my_sim = qb::session(false);

  // Number of qubits we want to run
  constexpr int NB_QUBITS = 20;

  // Add CUDAQ ghz kernel to the current session
  my_sim.set_cudaq_kernel(ghz<NB_QUBITS>{});

  // Set up sensible default parameters
  my_sim.qb12();

  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(20000);
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  // Print the cumulative results
  std::cout << "Results:" << std::endl
            << my_sim.get_out_raws()[0][0] << std::endl;
  const auto out_counts = my_sim.get_out_counts()[0][0];
  EXPECT_EQ(out_counts.size(), 2);
  int sum = 0;
  for (const auto &[bitStr, count] : out_counts) {
    sum += count;
  }
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

  qb::cudaq_ir_converter converter(ansatz);
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
  qb::cudaq_ir_converter converter(ansatz);
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

  // Make a QB SDK session
  auto my_sim = qb::session(false);

  // Number of qubits we want to run
  // Large number of qubits, since we are using GPUs!
  constexpr int NB_QUBITS = 30;

  // Add CUDAQ ghz kernel to the current session
  my_sim.set_cudaq_kernel(ghz<NB_QUBITS>{});

  // Set up sensible default parameters
  my_sim.qb12();
  // Both custatevec_f32 and custatevec are okay,
  // use f32 to speed up the test.
  my_sim.set_acc("custatevec_f32");
  // Choose how many 'shots' to run through the circuit
  my_sim.set_sn(20000);
  std::cout << "About to run quantum program..." << std::endl;
  my_sim.run();

  // Print the cumulative results
  std::cout << "Results:" << std::endl
            << my_sim.get_out_raws()[0][0] << std::endl;
  const auto out_counts = my_sim.get_out_counts()[0][0];
  EXPECT_EQ(out_counts.size(), 2);
  int sum = 0;
  for (const auto &[bitStr, count] : out_counts) {
    sum += count;
  }
  EXPECT_EQ(sum, 20000);
}
#endif
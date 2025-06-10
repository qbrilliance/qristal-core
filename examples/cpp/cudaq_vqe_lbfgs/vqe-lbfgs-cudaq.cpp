// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/cudaq/ir_converter.hpp>
#include <qristal/core/cudaq/sim_pool.hpp>
#include <cudaq/algorithm.h>
#include <cudaq/gradients/central_difference.h>
#include <cudaq/optimizers.h>
#include <cudaq/spin_op.h>
#include <xacc.hpp>
#include <xacc_service.hpp>

int main() {
  // And we're off!
  std::cout << "Executing C++ demo: Solving Deuteron's ground state energy ..."
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

  // Load the cudaq version of qpp
  std::string be = "qpp";
  std::cout << "Connecting CUDA Quantum backend " << be << std::endl;
  qristal::load_cudaq_backend(be);

  // Run VQE with the builder
  cudaq::optimizers::lbfgs l_opt;
  cudaq::gradients::central_difference gradient(cudaq_builder);
  std::cout << "Running VQE with L-BFGS optimizer, central difference gradient "
               "calculator! \n";

  auto [opt_val, opt_params] =
      cudaq::vqe(cudaq_builder, gradient, h, l_opt, /*n_params*/ 1);

  std::cout << "Ground state energy (expected -1.74886): " << opt_val << "\n";
}

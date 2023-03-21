// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/cudaq/ir_converter.hpp"
#include "qoda/algorithm.h"
#include "qoda/optimizers.h"
#include "qoda/spin_op.h"
#include "xacc.hpp"
#include "xacc_service.hpp"

namespace cudaq = qoda;

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
}

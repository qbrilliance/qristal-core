// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/qoda/ir_converter.hpp"
#include "qoda/algorithm.h"
#include "qoda/gradients/central_difference.h"
#include "qoda/optimizers.h"
#include "qoda/spin_op.h"
#include "xacc.hpp"
#include "xacc_service.hpp"

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
  qb::qoda_ir_converter converter(ansatz);
  std::cout << "Converted ansatz to QODA (Quake IR) ..." << std::endl;

  auto &qoda_builder = converter.get_qoda_builder();
  std::cout << "QODA QUAKE: \n" << qoda_builder.to_quake();

  qoda::spin_op h = 5.907 - 2.1433 * qoda::spin::x(0) * qoda::spin::x(1) -
                    2.1433 * qoda::spin::y(0) * qoda::spin::y(1) +
                    .21829 * qoda::spin::z(0) - 6.125 * qoda::spin::z(1);

  std::cout << "Constructed Deuteron Hamiltonian as QODA spin_op: \n";
  h.dump();
  // Run VQE with the builder

  qoda::optimizers::lbfgs l_opt;
  qoda::gradients::central_difference gradient(qoda_builder);
  std::cout << "Running VQE with L-BFGS optimizer, central difference gradient "
               "calculator! \n";

  auto [opt_val, opt_params] =
      qoda::vqe(qoda_builder, gradient, h, l_opt, /*n_params*/ 1);

  std::cout << "Ground state energy (expected -1.74886): " << opt_val << "\n";
}

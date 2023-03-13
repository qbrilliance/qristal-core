// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/qoda/ir_converter.hpp"
#include "qoda/algorithm.h"
#include "qoda/gradients/central_difference.h"
#include "qoda/optimizers.h"
#include "qoda/spin_op.h"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <filesystem>

int main() {
  // Please run the "gen_h_chain.py" script to generate the binary file
  // representing the H4 Hamiltonian
  std::cout << "Loading QODA Hamiltonian in binary format..." << std::endl;
  const std::string data_file =
      std::filesystem::current_path().string() + "/../h2_185_terms_data.bin";
  qoda::binary_spin_op_reader reader;
  qoda::spin_op h = reader.read(data_file);
  std::cout << "QODA Hamiltonian:" << std::endl;
  h.dump();

  xacc::Initialize();

  std::cout << "Constructing Qristal UCCSD ansatz circuit ..." << std::endl;
  auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("UCCSD"));
  constexpr int num_electrons = 4;
  constexpr int num_spin_orbitals = 2 * num_electrons;
  uccsd->expand({{"ne", num_electrons}, {"nq", num_spin_orbitals}});
  std::cout << "QB UCCSD IR:\n" << uccsd->toString() << "\n";

  std::cout << "Converting to QODA QUAKE IR ..." << std::endl;
  qb::qoda_ir_converter converter(uccsd);
  auto &qoda_builder = converter.get_qoda_builder();
  std::cout << "QODA QUAKE: \n" << qoda_builder.to_quake();

  qoda::optimizers::lbfgs l_opt;
  qoda::gradients::central_difference gradient(qoda_builder);
  std::cout << "Running VQE for H4 with L-BFGS optimizer, central difference "
               "gradient calculator! \n";

  auto [opt_val, opt_params] = qoda::vqe(qoda_builder, gradient, h, l_opt,
                                         /*n_params*/ uccsd->nVariables());

  std::cout << "Ground state energy of H4 (expected -2.238588): " << opt_val
            << "\n";
}

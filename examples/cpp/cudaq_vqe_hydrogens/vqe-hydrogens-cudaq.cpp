// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qristal/core/cudaq/ir_converter.hpp"
#include "qristal/core/cudaq/sim_pool.hpp"
#include "cudaq/algorithm.h"
#include "cudaq/gradients/central_difference.h"
#include "cudaq/optimizers.h"
#include "cudaq/spin_op.h"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <filesystem>

int main() {
  // Please run the "gen_h_chain.py" script to generate the binary file
  // representing the H4 Hamiltonian
  std::cout << "Loading CUDAQ Hamiltonian in binary format..." << std::endl;
  const std::string data_file =
      std::filesystem::current_path().string() + "/../h2_185_terms_data.bin";
  cudaq::binary_spin_op_reader reader;
  cudaq::spin_op h = reader.read(data_file);
  std::cout << "CUDAQ Hamiltonian:" << std::endl;
  h.dump();

  xacc::Initialize();

  std::cout << "Constructing Qristal UCCSD ansatz circuit ..." << std::endl;
  auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("UCCSD"));
  constexpr int num_electrons = 4;
  constexpr int num_spin_orbitals = 2 * num_electrons;
  uccsd->expand({{"ne", num_electrons}, {"nq", num_spin_orbitals}});
  std::cout << "QB UCCSD IR:\n" << uccsd->toString() << "\n";

  std::cout << "Converting to CUDAQ QUAKE IR ..." << std::endl;
  qristal::cudaq_ir_converter converter(uccsd);
  auto &cudaq_builder = converter.get_cudaq_builder();
  std::cout << "CUDAQ QUAKE: \n" << cudaq_builder.to_quake();

  // Load the cudaq version of qpp
  std::string be = "qpp";
  std::cout << "Connecting CUDA Quantum backend " << be << std::endl;
  qristal::load_cudaq_backend(be);

  cudaq::optimizers::lbfgs l_opt;
  cudaq::gradients::central_difference gradient(cudaq_builder);
  std::cout << "Running VQE for H4 with L-BFGS optimizer, central difference "
               "gradient calculator! \n";

  auto [opt_val, opt_params] = cudaq::vqe(cudaq_builder, gradient, h, l_opt,
                                         /*n_params*/ uccsd->nVariables());

  std::cout << "Ground state energy of H4 (expected -2.238588): " << opt_val
            << "\n";
}

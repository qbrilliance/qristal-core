// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "CommonGates.hpp"
#include "ObservableTransform.hpp"
#include "Optimizer.hpp"
#include "xacc.hpp"
#include "xacc_observable.hpp"
#include "xacc_service.hpp"
#include <Eigen/Eigenvalues>
#include <cmath>
#include <gtest/gtest.h>
#include <random>
#include <sstream>

namespace {
std::string HamStringHydrogens(int nbHydrogens) {
  std::stringstream geom_ss;
  double x = 0.0;
  for (int i = 0; i < nbHydrogens; i++) {
    geom_ss << "H 0.0 0.0 " << x;
    if (i != nbHydrogens - 1) {
      geom_ss << "; ";
    }
    if (i % 2 == 0) {
      x += 0.75;
    } else {
      x += 1.125;
    }
  }
  return geom_ss.str();
}

// Generate random vector
std::vector<double> random_vector(const double l_range, const double r_range,
                                  const std::size_t size) {
  // Generate a random initial parameter set
  std::random_device rnd_device;
  std::mt19937 mersenne_engine{rnd_device()}; // Generates random integers
  std::uniform_real_distribution<double> dist{l_range, r_range};
  auto gen = [&dist, &mersenne_engine]() { return dist(mersenne_engine); };
  std::vector<double> vec(size);
  std::generate(vec.begin(), vec.end(), gen);
  return vec;
}

double minEigenVal(std::vector<xacc::SparseTriplet> &ham_mat, int nbQubits) {
  Eigen::MatrixXcd test = Eigen::MatrixXcd::Zero(1 << nbQubits, 1 << nbQubits);

  for (auto &triplet : ham_mat) {
    const auto row = triplet.row();
    const auto col = triplet.col();
    const auto val = triplet.coeff();
    test(row, col) = val;
  }
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> es;
  es.compute(test);
  //   std::cout << "The eigenvalues of H are: " << es.eigenvalues().transpose()
  //             << std::endl;

  return es.eigenvalues()(0);
}
} // namespace

TEST(VqeGenTester, checkDeuteronH2) {
  auto accelerator = xacc::getAccelerator("qpp");

  // Create the N=2 deuteron Hamiltonian
  auto H_N_2 = xacc::quantum::getObservable(
      "pauli", std::string("5.907 - 2.1433 X0X1 "
                           "- 2.1433 Y0Y1"
                           "+ .21829 Z0 - 6.125 Z1"));

  auto optimizer = xacc::getOptimizer("nlopt");
  xacc::qasm(R"(
        .compiler xasm
        .circuit deuteron_ansatz
        .parameters theta
        .qbit q
        X(q[0]);
        Ry(q[1], theta);
        CNOT(q[1],q[0]);
    )");
  auto ansatz = xacc::getCompiled("deuteron_ansatz");

  // Get the VQE Algorithm and initialize it
  auto vqe = xacc::getAlgorithm("vqe-gen");
  vqe->initialize({{"ansatz", ansatz},
                   {"observable", H_N_2},
                   {"accelerator", accelerator},
                   {"optimizer", optimizer}});

  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(2);
  vqe->execute(buffer);
  buffer->print();
  EXPECT_NEAR((*buffer)["opt-val"].as<double>(), -1.74886, 1e-4);
  EXPECT_NEAR((*buffer)["opt-params"].as<std::vector<double>>()[0], 0.594,
              1e-2);
}

TEST(VqeGenTester, checkDeuteronH3) {
  // Use Qpp accelerator
  auto accelerator = xacc::getAccelerator("qpp");
  // Create the N=3 deuteron Hamiltonian
  auto H_N_3 = xacc::quantum::getObservable(
      "pauli",
      std::string("5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1 + "
                  "9.625 - 9.625 Z2 - 3.91 X1 X2 - 3.91 Y1 Y2"));

  auto optimizer = xacc::getOptimizer("nlopt");

  // JIT map Quil QASM Ansatz to IR
  xacc::qasm(R"(
        .compiler xasm
        .circuit deuteron_ansatz_h3
        .parameters t0, t1
        .qbit q
        X(q[0]);
        exp_i_theta(q, t1, {{"pauli", "X0 Y1 - Y0 X1"}});
        exp_i_theta(q, t0, {{"pauli", "X0 Z1 Y2 - X2 Z1 Y0"}});
    )");
  auto ansatz = xacc::getCompiled("deuteron_ansatz_h3");

  // Get the VQE Algorithm and initialize it
  auto vqe = xacc::getAlgorithm("vqe-gen");
  vqe->initialize({{"ansatz", ansatz},
                   {"observable", H_N_3},
                   {"accelerator", accelerator},
                   {"optimizer", optimizer}});

  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(3);
  vqe->execute(buffer);
  buffer->print();
  // Expected result: -2.04482
  EXPECT_NEAR((*buffer)["opt-val"].as<double>(), -2.04482, 1e-4);
  EXPECT_NEAR((*buffer)["opt-params"].as<std::vector<double>>()[0],
              0.06846192759756081, 1e-3);
  EXPECT_NEAR((*buffer)["opt-params"].as<std::vector<double>>()[1],
              0.1779995542396726, 1e-3);
}

TEST(VqeGenTester, checkH2) {
  // Use Qpp accelerator
  auto accelerator = xacc::getAccelerator("qpp");

  // Create the Observable
  auto H2 = xacc::quantum::getObservable(
      "pyscf", {{"basis", "sto-3g"}, {"geometry", HamStringHydrogens(2)}});

  //   std::cout << H2->toString() << "\n";
  auto ham_sparse = xacc::getService<xacc::ObservableTransform>("jw")
                        ->transform(H2)
                        ->to_sparse_matrix();
  const auto minEnergy = minEigenVal(ham_sparse, 4);
  std::cout << "Expected ground-state energy: " << minEnergy << "\n";
  auto optimizer = xacc::getOptimizer("nlopt");

  auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
  auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
  EXPECT_TRUE(uccsd->expand({{"ne", 2}, {"nq", 4}}));

  //   std::cout << uccsd->toString() << "\n";

  // Get the VQE Algorithm and initialize it
  auto vqe = xacc::getAlgorithm("vqe-gen");
  vqe->initialize({{"ansatz", uccsd},
                   {"observable", H2},
                   {"accelerator", accelerator},
                   {"optimizer", optimizer}});

  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(4);
  vqe->execute(buffer);
  buffer->print();
  EXPECT_NEAR((*buffer)["opt-val"].as<double>(), minEnergy, 1e-4);
}

// TEST(VqeGenTester, checkH4) {
//   xacc::set_verbose(true);
//   // Use Qpp accelerator
//   auto accelerator = xacc::getAccelerator("qpp");
//   std::cout << HamStringHydrogens(4) << "\n";
//   // Create the Observable
//   auto H4 = xacc::quantum::getObservable(
//       "pyscf", {{"basis", "sto-3g"}, {"geometry", HamStringHydrogens(4)}});
//   auto ham_sparse = xacc::getService<xacc::ObservableTransform>("jw")
//                         ->transform(H4)
//                         ->to_sparse_matrix();
//   const auto minEnergy = minEigenVal(ham_sparse, 8);
//   std::cout << "Expected ground-state energy: " << minEnergy << "\n";
//   auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
//   auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
//   EXPECT_TRUE(uccsd->expand({{"ne", 4}, {"nq", 8}}));
//   auto optimizer = xacc::getOptimizer("nlopt", {{"maxeval", 600}});
//   // Get the VQE Algorithm and initialize it
//   auto vqe = xacc::getAlgorithm("vqe-gen");
//   vqe->initialize({{"ansatz", uccsd},
//                    {"observable", H4},
//                    {"accelerator", accelerator},
//                    {"optimizer", optimizer}});

//   // Allocate some qubits and execute
//   auto buffer = xacc::qalloc(8);
//   vqe->execute(buffer);
//   buffer->print();
//   EXPECT_NEAR((*buffer)["opt-val"].as<double>(), minEnergy, 1e-3);
// }

// TEST(VqeGenTester, checkH6) {
//   xacc::set_verbose(true);
//   // Use Qpp accelerator
//   auto accelerator = xacc::getAccelerator("qpp");
//   std::cout << HamStringHydrogens(6) << "\n";
//   // Create the Observable
//   auto H6 = xacc::quantum::getObservable(
//       "pyscf", {{"basis", "sto-3g"}, {"geometry", HamStringHydrogens(6)}});
//   auto ham_sparse = xacc::getService<xacc::ObservableTransform>("jw")
//                         ->transform(H6)
//                         ->to_sparse_matrix();
//   const auto minEnergy = minEigenVal(ham_sparse, 12);
//   std::cout << "Expected ground-state energy: " << minEnergy << "\n";
//   auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
//   auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
//   EXPECT_TRUE(uccsd->expand({{"ne", 6}, {"nq", 12}}));
//   auto optimizer = xacc::getOptimizer("nlopt", {{"maxeval", 1000}});
//   // Get the VQE Algorithm and initialize it
//   auto vqe = xacc::getAlgorithm("vqe-gen");
//   vqe->initialize({{"ansatz", uccsd},
//                    {"observable", H6},
//                    {"accelerator", accelerator},
//                    {"optimizer", optimizer}});

//   // Allocate some qubits and execute
//   auto buffer = xacc::qalloc(12);
//   vqe->execute(buffer);
//   buffer->print();
//   EXPECT_NEAR((*buffer)["opt-val"].as<double>(), minEnergy, 1e-3);
// }

TEST(VqeGenTester, checkH2AerDensityMatrix) {
  xacc::set_verbose(true);
  // Use AER accelerator
  auto accelerator =
      xacc::getAccelerator("aer", {{"sim-type", "density_matrix"}});

  // Create the Observable
  auto H2 = xacc::quantum::getObservable(
      "pyscf", {{"basis", "sto-3g"}, {"geometry", HamStringHydrogens(2)}});

  //   std::cout << H2->toString() << "\n";
  auto ham_sparse = xacc::getService<xacc::ObservableTransform>("jw")
                        ->transform(H2)
                        ->to_sparse_matrix();
  const auto minEnergy = minEigenVal(ham_sparse, 4);
  std::cout << "Expected ground-state energy: " << minEnergy << "\n";
  auto optimizer = xacc::getOptimizer("nlopt");

  auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
  auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
  EXPECT_TRUE(uccsd->expand({{"ne", 2}, {"nq", 4}}));

  //   std::cout << uccsd->toString() << "\n";

  // Get the VQE Algorithm and initialize it
  auto vqe = xacc::getAlgorithm("vqe-gen");
  vqe->initialize({{"ansatz", uccsd},
                   {"observable", H2},
                   {"accelerator", accelerator},
                   {"optimizer", optimizer}});

  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(4);
  vqe->execute(buffer);
  buffer->print();
  EXPECT_NEAR((*buffer)["opt-val"].as<double>(), minEnergy, 1e-4);
}

TEST(VqeGenTester, checkH2AerStateVector) {
  xacc::set_verbose(true);
  // Use AER accelerator
  auto accelerator = xacc::getAccelerator("aer", {{"sim-type", "statevector"}});

  // Create the Observable
  auto H2 = xacc::quantum::getObservable(
      "pyscf", {{"basis", "sto-3g"}, {"geometry", HamStringHydrogens(2)}});

  //   std::cout << H2->toString() << "\n";
  auto ham_sparse = xacc::getService<xacc::ObservableTransform>("jw")
                        ->transform(H2)
                        ->to_sparse_matrix();
  const auto minEnergy = minEigenVal(ham_sparse, 4);
  std::cout << "Expected ground-state energy: " << minEnergy << "\n";
  auto optimizer = xacc::getOptimizer("nlopt");

  auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
  auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
  EXPECT_TRUE(uccsd->expand({{"ne", 2}, {"nq", 4}}));

  //   std::cout << uccsd->toString() << "\n";

  // Get the VQE Algorithm and initialize it
  auto vqe = xacc::getAlgorithm("vqe-gen");
  vqe->initialize({{"ansatz", uccsd},
                   {"observable", H2},
                   {"accelerator", accelerator},
                   {"optimizer", optimizer}});

  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(4);
  vqe->execute(buffer);
  buffer->print();
  EXPECT_NEAR((*buffer)["opt-val"].as<double>(), minEnergy, 1e-4);
}

TEST(VqeGenTester, checkH2AerDensityMatrixGradients) {
  xacc::set_verbose(true);
  // Use AER accelerator
  auto accelerator =
      xacc::getAccelerator("aer", {{"sim-type", "density_matrix"}});

  // Create the Observable
  auto H2 = xacc::quantum::getObservable(
      "pyscf", {{"basis", "sto-3g"}, {"geometry", HamStringHydrogens(2)}});

  //   std::cout << H2->toString() << "\n";
  auto ham_sparse = xacc::getService<xacc::ObservableTransform>("jw")
                        ->transform(H2)
                        ->to_sparse_matrix();
  const auto minEnergy = minEigenVal(ham_sparse, 4);
  std::cout << "Expected ground-state energy: " << minEnergy << "\n";
  // Gradient-based optimizer
  auto optimizer = xacc::getOptimizer("mlpack");

  auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
  auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
  EXPECT_TRUE(uccsd->expand({{"ne", 2}, {"nq", 4}}));

  //   std::cout << uccsd->toString() << "\n";

  // Get the VQE Algorithm and initialize it
  auto vqe = xacc::getAlgorithm("vqe-gen");
  vqe->initialize({{"ansatz", uccsd},
                   {"observable", H2},
                   {"accelerator", accelerator},
                   {"optimizer", optimizer}});

  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(4);
  vqe->execute(buffer);
  buffer->print();
  EXPECT_NEAR((*buffer)["opt-val"].as<double>(), minEnergy, 1e-3);
}

TEST(VqeGenTester, checkH2AerStateVectorGradients) {
  xacc::set_verbose(true);
  // Use AER accelerator
  auto accelerator = xacc::getAccelerator("aer", {{"sim-type", "statevector"}});

  // Create the Observable
  auto H2 = xacc::quantum::getObservable(
      "pyscf", {{"basis", "sto-3g"}, {"geometry", HamStringHydrogens(2)}});

  //   std::cout << H2->toString() << "\n";
  auto ham_sparse = xacc::getService<xacc::ObservableTransform>("jw")
                        ->transform(H2)
                        ->to_sparse_matrix();
  const auto minEnergy = minEigenVal(ham_sparse, 4);
  std::cout << "Expected ground-state energy: " << minEnergy << "\n";
  auto optimizer = xacc::getOptimizer("mlpack");

  auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
  auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
  EXPECT_TRUE(uccsd->expand({{"ne", 2}, {"nq", 4}}));

  //   std::cout << uccsd->toString() << "\n";

  // Get the VQE Algorithm and initialize it
  auto vqe = xacc::getAlgorithm("vqe-gen");
  vqe->initialize({{"ansatz", uccsd},
                   {"observable", H2},
                   {"accelerator", accelerator},
                   {"optimizer", optimizer}});

  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(4);
  vqe->execute(buffer);
  buffer->print();
  EXPECT_NEAR((*buffer)["opt-val"].as<double>(), minEnergy, 1e-3);
}

TEST(VqeGenTester, checkH4Aer) {
  xacc::set_verbose(true);
  // Use AER accelerator
  auto accelerator = xacc::getAccelerator("aer", {{"sim-type",
  "statevector"}}); std::cout << HamStringHydrogens(4) << "\n";
  // Create the Observable
  auto H4 = xacc::quantum::getObservable(
      "pyscf", {{"basis", "sto-3g"}, {"geometry", HamStringHydrogens(4)}});
  auto ham_sparse = xacc::getService<xacc::ObservableTransform>("jw")
                        ->transform(H4)
                        ->to_sparse_matrix();
  const auto minEnergy = minEigenVal(ham_sparse, 8);
  std::cout << "Expected ground-state energy: " << minEnergy << "\n";
  auto tmp = xacc::getService<xacc::Instruction>("UCCSD");
  auto uccsd = std::dynamic_pointer_cast<xacc::CompositeInstruction>(tmp);
  EXPECT_TRUE(uccsd->expand({{"ne", 4}, {"nq", 8}}));
  auto optimizer = xacc::getOptimizer("nlopt", {{"maxeval", 600}});
  // Get the VQE Algorithm and initialize it
  auto vqe = xacc::getAlgorithm("vqe-gen");
  vqe->initialize({{"ansatz", uccsd},
                   {"observable", H4},
                   {"accelerator", accelerator},
                   {"optimizer", optimizer}});

  // Allocate some qubits and execute
  auto buffer = xacc::qalloc(8);
  vqe->execute(buffer);
  buffer->print();
  EXPECT_NEAR((*buffer)["opt-val"].as<double>(), minEnergy, 1e-3);
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  xacc::external::load_external_language_plugins();
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  //xacc::external::unload_external_language_plugins();
  xacc::Finalize();
  return ret;
}

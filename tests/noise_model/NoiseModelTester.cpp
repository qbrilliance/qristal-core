// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include <random>

#include "xacc.hpp"
#include "Accelerator.hpp"
#include "Eigen/Dense"
#include <unsupported/Eigen/KroneckerProduct>

#include "qristal/core/noise_model/noise_model.hpp"
#include "qristal/core/primitives.hpp"

TEST(NoiseModelTester, checkReadoutErrors)
{
  qristal::NoiseProperties noise_props;
  // Use very different values to check:
  const qristal::ReadoutError ro_1{0.1, 0.2};
  const qristal::ReadoutError ro_2{0.3, 0.4};
  noise_props.readout_errors = {{0, ro_1},
                                {1, ro_2}};
  qristal::NoiseModel noise_model(noise_props);
  // std::cout << "HOWDY:\n"
  //           << noise_model.to_json() << "\n";
  auto accelerator = xacc::getAccelerator(
      "aer", {{"noise-model", noise_model.to_json()}, {"shots", 32768}});
  auto xasmCompiler = xacc::getCompiler("xasm");

  {
    // Q0: prep 1 and measure
    auto program = xasmCompiler
                       ->compile(R"(__qpu__ void test(qbit q) {
        X(q[0]);
        Measure(q[0]);
      })",
                                 accelerator)
                       ->getComposites()[0];
    auto buffer = xacc::qalloc(1);
    accelerator->execute(buffer, program);
    // buffer->print();
    // Measurement error matched the noise property
    EXPECT_NEAR(buffer->computeMeasurementProbability("0"), ro_1.p_01, 1e-2);
  }

  {
    // Q0: prep 0 and measure
    auto program = xasmCompiler
                       ->compile(R"(__qpu__ void test(qbit q) {
        Measure(q[0]);
      })",
                                 accelerator)
                       ->getComposites()[0];
    auto buffer = xacc::qalloc(1);
    accelerator->execute(buffer, program);
    // buffer->print();
    EXPECT_NEAR(buffer->computeMeasurementProbability("1"), ro_1.p_10, 1e-2);
  }

  {
    // Q1: prep 1 and measure
    auto program = xasmCompiler
                       ->compile(R"(__qpu__ void test(qbit q) {
        X(q[1]);
        Measure(q[1]);
      })",
                                 accelerator)
                       ->getComposites()[0];
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program);
    // buffer->print();
    // Measurement error matched the noise property
    EXPECT_NEAR(buffer->computeMeasurementProbability("0"), ro_2.p_01, 1e-2);
  }

  {
    // Q1: prep 0 and measure
    auto program = xasmCompiler
                       ->compile(R"(__qpu__ void test(qbit q) {
        Measure(q[1]);
      })",
                                 accelerator)
                       ->getComposites()[0];
    auto buffer = xacc::qalloc(2);
    accelerator->execute(buffer, program);
    // buffer->print();
    EXPECT_NEAR(buffer->computeMeasurementProbability("1"), ro_2.p_10, 1e-2);
  }
}

TEST(NoiseModelTester, checkKrausNoise)
{
  qristal::NoiseModel noise_model;
  noise_model.add_gate_error(qristal::GeneralizedAmplitudeDampingChannel::Create(0, 0.25, 0.75), "id", {0});
  // The equilibrium state after infinitely many applications of the
  //  channel is:
  //  rho_eq = [[1 - p1, 0]], [0, p1]]
  // std::cout << "HOWDY:\n"
  //           << noise_model.to_json() << "\n";

  auto accelerator = xacc::getAccelerator(
      "aer", {{"noise-model", noise_model.to_json()}, {"sim-type", "density_matrix"}});
  auto xasmCompiler = xacc::getCompiler("xasm");
  // Apply many identity gates (with noise) to get to equilibrium
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test(qbit q) {
        X(q[0]);
        for (int i = 0; i < 50; i++) {
          I(q[0]);
        }
        Measure(q[0]);
      })",
                               accelerator)
                     ->getComposites()[0];
  auto buffer = xacc::qalloc(1);
  accelerator->execute(buffer, program);
  // buffer->print();
  auto dm =
      *(accelerator->getExecutionInfo<xacc::ExecutionInfo::DensityMatrixPtrType>(
          xacc::ExecutionInfo::DmKey));
  EXPECT_NEAR(dm[0][0].real(), 0.75, 1e-6);
  EXPECT_NEAR(dm[1][1].real(), 0.25, 1e-6);
  EXPECT_NEAR(dm[0][0].real() + dm[1][1].real(), 1.0, 1e-9);
}

TEST(NoiseModelTester, checkDefaultNoiseModel)
{
  // Get the 'default' noise model, simple uniform Pauli depolarizing noise.
  auto noise_model = qristal::NoiseModel("default",2);
  // std::cout << "HOWDY:\n"
  //           << noise_model.to_json() << "\n";

  auto accelerator = xacc::getAccelerator(
      "aer", {{"noise-model", noise_model.to_json()}, {"sim-type", "density_matrix"}});
  auto xasmCompiler = xacc::getCompiler("xasm");
  // Apply a CNOT gate on all 0's state: no effect on ideal sim but adding decoherence (Pauli depolarizing) with noise.
  auto program = xasmCompiler
                     ->compile(R"(__qpu__ void test(qbit q) {
        CX(q[0], q[1]);
      })",
                               accelerator)
                     ->getComposites()[0];
  auto buffer = xacc::qalloc(2);
  accelerator->execute(buffer, program);
  // buffer->print();
  auto dm =
      *(accelerator->getExecutionInfo<xacc::ExecutionInfo::DensityMatrixPtrType>(
          xacc::ExecutionInfo::DmKey));
  // for (const auto &row : dm)
  // {
  //   for (const auto &x : row)
  //   {
  //     std::cout << x << " ";
  //   }
  //   std::cout << "\n";
  // }
  // Check that we have some noise effect
  // Note: the default error rate is 99.9%
  // hence use 99.95 as the check limit.
  EXPECT_TRUE(std::abs(dm[0][0]) < 0.9995);
  EXPECT_NEAR(dm[0][0].real() + dm[1][1].real() + dm[2][2].real() + dm[3][3].real(), 1.0, 1e-9);
}

TEST(NoiseModelTester, checkNoiseModelFromDeviceProps)
{
  qristal::NoiseProperties noise_props;
  noise_props.t1_us = {{0, 1e6}};
  noise_props.t2_us = {{0, 1e3}};
  std::map<std::vector<size_t>, double> gate_time;
  gate_time[std::vector<size_t>{0}] = 10.0;
  noise_props.gate_time_us["u3"] = gate_time;
  std::map<std::vector<size_t>, double> rb_gate_error;
  rb_gate_error[std::vector<size_t>{0}] = 0.01;
  noise_props.gate_pauli_errors["u3"] = rb_gate_error;
  qristal::NoiseModel noise_model(noise_props);
  auto provider = xacc::getIRProvider("quantum");
  auto test_circ = provider->createComposite("testCircuit");
  test_circ->addInstruction(
      provider->createInstruction("U", {0},
                                  std::vector<xacc::InstructionParameter>{
                                      0.1, 0.2, 0.3}));

  auto accelerator = xacc::getAccelerator(
      "aer", {{"noise-model", noise_model.to_json()}, {"sim-type", "density_matrix"}});

  auto buffer = xacc::qalloc(1);
  accelerator->execute(buffer, test_circ);
  // buffer->print();
  auto dm =
      *(accelerator->getExecutionInfo<xacc::ExecutionInfo::DensityMatrixPtrType>(
          xacc::ExecutionInfo::DmKey));
  // for (const auto &row : dm)
  // {
  //   for (const auto &x : row)
  //   {
  //     std::cout << x << " ";
  //   }
  //   std::cout << "\n";
  // }
  EXPECT_NEAR(dm[0][0].real() + dm[1][1].real(), 1.0, 1e-9);
}

TEST(NoiseModelTester, checkKrausToChoiConversion) 
{
  Eigen::Matrix<std::complex<double>, 4, 4> choiI;
  choiI << 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1;
  const double p = []() {
    std::random_device rd;
    std::mt19937 e(rd());
    std::uniform_real_distribution<> dist(0.01, 0.99);
    return dist(e);
  }();
  // Expected Choi matrix for a depolarizing noise channel of amplitude p
  const Eigen::Matrix4cd expected_choi_mat =
      (1 - p * 4 / 3) * choiI +
      p * 4 / 3 * Eigen::Matrix<std::complex<double>, 4, 4>::Identity() / 2;

  auto depol_channel = qristal::DepolarizingChannel::Create(0, p);
  const auto choi_mat = qristal::kraus_to_choi(depol_channel);
  EXPECT_EQ(choi_mat.size(), 4);
  for (const auto &row : choi_mat) {
    EXPECT_EQ(row.size(), 4);
  }
  std::cout << "Depolarizing with p = " << p << "\n";
  std::cout << "Choi matrix:\n";
  for (const auto &row : choi_mat) {
    for (const auto &val : row) {
      std::cout << val << " ";
    }
    std::cout << "\n";
  }
  std::cout << "EXPECTED:\n" << expected_choi_mat << "\n";

  for (Eigen::Index row = 0; row < expected_choi_mat.rows(); ++row) {
    for (Eigen::Index col = 0; col < expected_choi_mat.cols(); ++col) {
      EXPECT_NEAR(std::abs(choi_mat[row][col] - expected_choi_mat(row, col)),
                  0.0, 1e-9);
    }
  }
}

TEST(NoiseModelTester, checkFidelityCalc) 
{
  const double p = []() {
    std::random_device rd;
    std::mt19937 e(rd());
    std::uniform_real_distribution<> dist(0.01, 0.99);
    return dist(e);
  }();
  std::cout << "Depolarizing with p = " << p << "\n";
  auto depol_channel = qristal::DepolarizingChannel::Create(0, p);
  const double fid = qristal::process_fidelity(depol_channel);
  std::cout << "Fidelity = " << fid << "\n";
  EXPECT_NEAR(fid, 1.0 - p, 1e-6);
}


Eigen::MatrixXcd evolve_density_process(const Eigen::MatrixXcd& process_matrix, const Eigen::MatrixXcd& density) {
  size_t n_qubits = std::log2(density.rows());
  Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> result = Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic>::Zero(density.rows(), density.cols());
  std::vector<qristal::Pauli> basis{
    qristal::Pauli::Symbol::I,
    qristal::Pauli::Symbol::X,
    qristal::Pauli::Symbol::Y,
    qristal::Pauli::Symbol::Z 
  };
  for (Eigen::Index i = 0; i < process_matrix.rows(); ++i) {
    auto left = qristal::build_up_matrix_by_Kronecker_product(i, basis, n_qubits);
    for (Eigen::Index j = 0; j < process_matrix.cols(); ++j) {
      auto right = qristal::build_up_matrix_by_Kronecker_product(j, basis, n_qubits);
      result += process_matrix(i,j) * left * density * right.adjoint(); //evolve density
    } 
  }
  return result;
}

Eigen::MatrixXcd evolve_density_choi(const Eigen::MatrixXcd& choi_matrix, const Eigen::MatrixXcd& density) {
  Eigen::MatrixXcd result = Eigen::MatrixXcd::Zero(density.rows(), density.cols());
  for (Eigen::Index i = 0; i < choi_matrix.rows(); ++i) {
    Eigen::MatrixXcd basis_i = Eigen::MatrixXcd::Zero(density.rows(), density.cols());
    basis_i(i / density.rows(), i % density.rows()) = std::complex<double>(1.0); 
    for (Eigen::Index j = 0; j < choi_matrix.cols(); ++j) {
      Eigen::MatrixXcd basis_j = Eigen::MatrixXcd::Zero(density.rows(), density.cols());
      basis_j(j / density.rows(), j % density.rows()) = std::complex<double>(1.0);
      result += choi_matrix(i,j) * basis_i.adjoint() * density * basis_j;  
    }
  }
  return result;
}

Eigen::MatrixXcd evolve_density_kraus(const std::vector<Eigen::MatrixXcd>& kraus_mats, const Eigen::MatrixXcd& density) {
  Eigen::MatrixXcd result = Eigen::MatrixXcd::Zero(density.rows(), density.cols());
  for (const auto& kraus : kraus_mats) {
    result += kraus * density * kraus.adjoint();
  }
  return result;
}

Eigen::MatrixXcd evolve_density_superop(const Eigen::MatrixXcd& superop, const Eigen::MatrixXcd& density) {
  //(1) vectorize density in column-major order
  Eigen::VectorXcd result_vec{density.reshaped()};
  //(2) evolve density by matrix-vector multiplication of superoperator representation
  result_vec = superop * result_vec;
  //(3) reshape result vector to density matrix in column major order
  Eigen::MatrixXcd result{result_vec.reshaped(density.rows(), density.cols())};
  return result;
}

TEST(NoiseChannelTester, checkProcess2Choi2Kraus) {
  const size_t n_qubits = 3; 
  std::uniform_real_distribution<double> gen(-1.0*std::numbers::pi, std::numbers::pi);
  std::default_random_engine re;
  //process to test: Rx, Ry, Rz with random angles applied to qubits 0, 1, and 2 
  double t = gen(re);
  double c = cos(t/2.0);
  double s = sin(t/2.0);
  Eigen::MatrixXcd Rx(4, 4); 
  Rx <<                                c*c, std::complex<double>(0.0, -1.0*s*c), 0.0, 0.0, 
        std::complex<double>(0.0, 1.0*s*c),                                 s*s, 0.0, 0.0,
                                       0.0,                                 0.0, 0.0, 0.0, 
                                       0.0,                                 0.0, 0.0, 0.0;
  t = gen(re);
  c = cos(t/2.0);
  s = sin(t/2.0);
  Eigen::MatrixXcd Ry(4, 4);
  Ry <<                                c*c, 0.0, std::complex<double>(0.0, -1.0*s*c), 0.0, 
                                       0.0, 0.0,                                 0.0, 0.0,
        std::complex<double>(0.0, 1.0*s*c), 0.0,                                 s*s, 0.0, 
                                       0.0, 0.0,                                 0.0, 0.0;
  t = gen(re);
  c = cos(t/2.0);
  s = sin(t/2.0);                           
  Eigen::MatrixXcd Rz(4, 4);
  Rz <<                                c*c, 0.0, 0.0, std::complex<double>(0.0, -1.0*s*c), 
                                       0.0, 0.0, 0.0,                                 0.0,
                                       0.0, 0.0, 0.0,                                 0.0,
        std::complex<double>(0.0, 1.0*s*c), 0.0, 0.0,                                 s*s; 
  Eigen::MatrixXcd process_mat = Eigen::kroneckerProduct(Rx, Rz).eval(); 
  process_mat = Eigen::kroneckerProduct(process_mat, Ry).eval();

  //transform to Choi matrix: 
  auto choi_mat = qristal::process_to_choi(process_mat);

  //transform to superoperator: 
  auto superop_1 = qristal::choi_to_superoperator(choi_mat);
  auto superop_2 = qristal::process_to_superoperator(process_mat); //also test the direct call
  auto choi_mat_2 = qristal::superoperator_to_choi(superop_1);
  EXPECT_TRUE(choi_mat.isApprox(choi_mat_2, 1e-14)); //and check back transformation to Choi

  //transform to Kraus matrices
  auto kraus_mats_1 = qristal::choi_to_kraus(choi_mat);
  auto kraus_mats_2 = qristal::process_to_kraus(process_mat); //also test the direct call

  //initialize random density 
  Eigen::Vector<std::complex<double>, Eigen::Dynamic> state = Eigen::Vector<std::complex<double>, Eigen::Dynamic>::Random(std::pow(2, n_qubits));
  state.normalize();
  Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> density = state * state.adjoint();

  //evolve density with process matrix
  auto evolved_density_process = evolve_density_process(process_mat, density);
  //evolve density with choi matrix
  auto evolved_density_choi = evolve_density_choi(choi_mat, density);
  //evolve density with superoperator matrix 
  auto evolved_density_superop_1 = evolve_density_superop(superop_1, density);
  auto evolved_density_superop_2 = evolve_density_superop(superop_2, density);
  //evolve density with kraus matrices 
  auto evolved_density_kraus_1 = evolve_density_kraus(kraus_mats_1, density);
  auto evolved_density_kraus_2 = evolve_density_kraus(kraus_mats_2, density);

  //check if they are identical
  EXPECT_TRUE(evolved_density_process.isApprox(evolved_density_choi, 1e-14));
  EXPECT_TRUE(evolved_density_process.isApprox(evolved_density_superop_1, 1e-14));
  EXPECT_TRUE(evolved_density_process.isApprox(evolved_density_superop_2, 1e-14));
  EXPECT_TRUE(evolved_density_process.isApprox(evolved_density_kraus_1, 1e-14));
  EXPECT_TRUE(evolved_density_process.isApprox(evolved_density_kraus_2, 1e-14));

  //final check: transform to kraus matrices to NoiseChannel and back to Choi matrix 
  auto choi_mat2 = qristal::kraus_to_choi(kraus_mats_1);

  EXPECT_TRUE(choi_mat.isApprox(choi_mat2, 1e-14));
}

TEST(NoiseChannelTester, testProcessMatrixSolver1Qubit) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<double> dist_angle(0.0, 2 * std::numbers::pi);
  // Choose some physically meaningful random values
  static std::uniform_real_distribution<double> dist_amp_damp(0.0, 1e-6);
  static std::uniform_real_distribution<double> dist_phase_damp(0.0, 1e-3);
  static std::uniform_real_distribution<double> dist_depol1(0.0, 1e-5);

  // Create process matrix with some chosen Euler angles {theta, phi, lambda} and random noise channel damping parameters.
  double theta = dist_angle(gen);
  double phi = dist_angle(gen);
  double lambda = dist_angle(gen);
  double gen_amp_damp_rate = dist_amp_damp(gen);
  double gen_phase_damp_rate = dist_phase_damp(gen);
  double depol_1qubit_rate = dist_depol1(gen);

  Eigen::VectorXd channel_params(3); // Factor of 3 corresponds to the 3 noise channels
  channel_params << gen_amp_damp_rate, gen_phase_damp_rate, depol_1qubit_rate;

  // Create input process matrix
  Eigen::MatrixXcd process_mat_noisy = qristal::create1QubitNoisyProcessMatrix(theta, phi, lambda, channel_params);

  // Solve noise channel damping parameters for the given input process matrix
  size_t max_iter = 1000;
  Eigen::VectorXd x = qristal::processMatrixSolver1Qubit(process_mat_noisy, theta, phi, lambda, max_iter);

  // Check that the solved damping parameters are close to their input values.
  EXPECT_NEAR(gen_amp_damp_rate, x(0), 1e-9);
  EXPECT_NEAR(gen_phase_damp_rate, x(1), 1e-6);
  EXPECT_NEAR(depol_1qubit_rate, x(2), 1e-8);

  std::cout << "\nInput values vs. solved values\n";
  std::cout << "gen_amp_damp_rate, input: " << gen_amp_damp_rate << ", solved: " << x(0)
            << ", % diff: " << std::abs(gen_amp_damp_rate - x(0)) / gen_amp_damp_rate * 100 << "\n";
  std::cout << "gen_phase_damp_rate, input: " << gen_phase_damp_rate << ", solved: " << x(1)
            << ", % diff: " << std::abs(gen_phase_damp_rate - x(1)) / gen_phase_damp_rate * 100 << "\n";
  std::cout << "depol_1qubit_rate, input: " << depol_1qubit_rate << ", solved: " << x(2)
            << ", % diff: " << std::abs(depol_1qubit_rate - x(2)) / depol_1qubit_rate * 100 << "\n";
  std::cout << "\n";

  // Reconstruct the process matrix by using the solved parameters
  Eigen::MatrixXcd reconstructed_mat =
      qristal::create1QubitNoisyProcessMatrix(theta, phi, lambda, x);
  // Check that the input process matrix and the reconstructed matrix are close.
  EXPECT_TRUE(process_mat_noisy.isApprox(reconstructed_mat, 1.0e-6));
}

TEST(NoiseChannelTester, testProcessMatrixSolverNQubit) {
  const size_t nb_qubits = 2;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<double> dist_angle(0.0, 2 * std::numbers::pi);
  // Choose some physically meaningful random values
  static std::uniform_real_distribution<double> dist_amp_damp(0.0, 1e-6);
  static std::uniform_real_distribution<double> dist_phase_damp(0.0, 1e-3);
  static std::uniform_real_distribution<double> dist_depol1(0.0, 1e-5);

  // Create process matrix with some chosen angles {theta, phi, lambda} and random noise channel damping parameters.
  std::vector<double> theta, phi, lambda, gen_amp_damp_rate_vec, gen_phase_damp_rate_vec, depol_1qubit_rate_vec;
  for (size_t i = 0; i < nb_qubits; i++) {
    theta.emplace_back(dist_angle(gen));
    phi.emplace_back(dist_angle(gen));
    lambda.emplace_back(dist_angle(gen));
    gen_amp_damp_rate_vec.emplace_back(dist_amp_damp(gen));
    gen_phase_damp_rate_vec.emplace_back(dist_phase_damp(gen));
    depol_1qubit_rate_vec.emplace_back(dist_depol1(gen));
  }

  Eigen::VectorXd gen_amp_damp_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_amp_damp_rate_vec.data(), gen_amp_damp_rate_vec.size());
  Eigen::VectorXd gen_phase_damp_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_phase_damp_rate_vec.data(), gen_phase_damp_rate_vec.size());
  Eigen::VectorXd depol_1qubit_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(depol_1qubit_rate_vec.data(), depol_1qubit_rate_vec.size());
  Eigen::VectorXd channel_params1(3); // 3 parameters from the 2 noise channels
  Eigen::VectorXd channel_params2(3); // 3 parameters from the 2 noise channels
  Eigen::VectorXd channel_paramsN(3*nb_qubits); // Factor of 3 corresponds to the 2 noise channels
  channel_params1 << gen_amp_damp_rate(0), gen_phase_damp_rate(0), depol_1qubit_rate(0);
  channel_params2 << gen_amp_damp_rate(1), gen_phase_damp_rate(1), depol_1qubit_rate(1);
  channel_paramsN << gen_amp_damp_rate, gen_phase_damp_rate, depol_1qubit_rate;

  // Create input 1-qubit process matrices
  Eigen::MatrixXcd process_mat_noisy1 = qristal::create1QubitNoisyProcessMatrix(theta[0], phi[0], lambda[0], channel_params1);
  Eigen::MatrixXcd process_mat_noisy2 = qristal::create1QubitNoisyProcessMatrix(theta[1], phi[1], lambda[1], channel_params2);
  // Create input N-qubit process matrix
  Eigen::MatrixXcd process_mat_noisyN = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta, phi, lambda, channel_paramsN);

  // Solve noise channel damping parameters for the given input process matrix
  size_t max_iter = 1000;
  Eigen::VectorXd x = qristal::processMatrixSolverNQubit({process_mat_noisy1, process_mat_noisy2},
      process_mat_noisyN, nb_qubits, theta, phi, lambda, max_iter);

  // Check that the solved damping parameters are close to their input values.
  for (size_t i = 0; i < nb_qubits; i++) {
    EXPECT_NEAR(gen_amp_damp_rate_vec[i], x(i), 1e-9);
    EXPECT_NEAR(gen_phase_damp_rate[i], x(i + nb_qubits), 1e-6);
    EXPECT_NEAR(depol_1qubit_rate[i], x(i + 2*nb_qubits), 1e-8);
  }

  std::cout << "\nInput values vs. solved values\n";
  for (size_t i = 0; i < nb_qubits; i++) {
    std::cout << "Qubit: " << i << "\n";
    std::cout << "gen_amp_damp_rate, input: " << gen_amp_damp_rate_vec[i] << ", solved: " << x(i)
              << ", % diff: " << std::abs(gen_amp_damp_rate_vec[i] - x(i)) / gen_amp_damp_rate_vec[i] * 100 << "\n";
    std::cout << "gen_phase_damp_rate, input: " << gen_phase_damp_rate_vec[i] << ", solved: " << x(i + nb_qubits)
              << ", % diff: " << std::abs(gen_phase_damp_rate_vec[i] - x(i + nb_qubits)) / gen_phase_damp_rate_vec[i] * 100 << "\n";
    std::cout << "depol_1qubit_rate, input: " << depol_1qubit_rate_vec[i] << ", solved: " << x(i + 2*nb_qubits)
              << ", % diff: " << std::abs(depol_1qubit_rate_vec[i] - x(i + 2*nb_qubits)) / depol_1qubit_rate_vec[i] * 100 << "\n";
    std::cout << "\n";
  }

  // Reconstruct the process matrix by using the solved parameters
  Eigen::MatrixXcd reconstructed_mat = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta, phi, lambda, x);
  // Check that the input process matrix and the reconstructed matrix are close.
  EXPECT_TRUE(process_mat_noisyN.isApprox(reconstructed_mat, 1.0e-6));
}

TEST(NoiseChannelTester, testProcessMatrixInterpolator) {
  const size_t nb_qubits = 2;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  // Choose some physically meaningful random values
  static std::uniform_real_distribution<double> dist_amp_damp(0.0, 1e-6);
  static std::uniform_real_distribution<double> dist_phase_damp(0.0, 1e-3);
  static std::uniform_real_distribution<double> dist_depol1(0.0, 1e-5);

  // Create angles and noise channel parameters for angles {theta1, phi1, lambda1}
  std::vector<double> theta1 = {0.1*std::numbers::pi, 0.2*std::numbers::pi};
  std::vector<double> phi1 = {0.3*std::numbers::pi, 0.4*std::numbers::pi};
  std::vector<double> lambda1 = {0.5*std::numbers::pi, 0.6*std::numbers::pi};
  std::vector<double> gen_amp_damp_rate_vec1 = {dist_amp_damp(gen), dist_amp_damp(gen)};
  std::vector<double> gen_phase_damp_rate_vec1 = {dist_phase_damp(gen), dist_phase_damp(gen)};
  std::vector<double> depol_1qubit_rate_vec1 = {dist_depol1(gen), dist_depol1(gen)};

  // Create angles and noise channel parameters for angles {theta2, phi2, lambda2}
  std::vector<double> theta2 = {0.3*std::numbers::pi, 0.4*std::numbers::pi};
  std::vector<double> phi2 = {0.5*std::numbers::pi, 0.6*std::numbers::pi};
  std::vector<double> lambda2 = {0.7*std::numbers::pi, 0.8*std::numbers::pi};
  std::vector<double> gen_amp_damp_rate_vec2 = {dist_amp_damp(gen), dist_amp_damp(gen)};
  std::vector<double> gen_phase_damp_rate_vec2 = {dist_phase_damp(gen), dist_phase_damp(gen)};
  std::vector<double> depol_1qubit_rate_vec2 = {dist_depol1(gen), dist_depol1(gen)};

  // Convert angles and noise channel parameters to Eigen
  Eigen::VectorXd gen_amp_damp_rate1 = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_amp_damp_rate_vec1.data(), gen_amp_damp_rate_vec1.size());
  Eigen::VectorXd gen_amp_damp_rate2 = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_amp_damp_rate_vec2.data(), gen_amp_damp_rate_vec2.size());
  Eigen::VectorXd gen_phase_damp_rate1 = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_phase_damp_rate_vec1.data(), gen_phase_damp_rate_vec1.size());
  Eigen::VectorXd gen_phase_damp_rate2 = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(gen_phase_damp_rate_vec2.data(), gen_phase_damp_rate_vec2.size());
  Eigen::VectorXd depol_1qubit_rate1 = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(depol_1qubit_rate_vec1.data(), depol_1qubit_rate_vec1.size());
  Eigen::VectorXd depol_1qubit_rate2 = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(depol_1qubit_rate_vec2.data(), depol_1qubit_rate_vec2.size());
  Eigen::VectorXd channel_params_qubit1_1(3), channel_params_qubit2_1(3), channel_params_qubit1_2(3), channel_params_qubit2_2(3),
      channel_params_qubitN_1(6), channel_params_qubitN_2(6);
  channel_params_qubit1_1 << gen_amp_damp_rate1(0), gen_phase_damp_rate1(0), depol_1qubit_rate1(0);
  channel_params_qubit2_1 << gen_amp_damp_rate1(1), gen_phase_damp_rate1(1), depol_1qubit_rate1(1);
  channel_params_qubitN_1 << gen_amp_damp_rate1, gen_phase_damp_rate1, depol_1qubit_rate1;
  channel_params_qubit1_2 << gen_amp_damp_rate2(0), gen_phase_damp_rate2(0), depol_1qubit_rate2(0);
  channel_params_qubit2_2 << gen_amp_damp_rate2(1), gen_phase_damp_rate2(1), depol_1qubit_rate2(1);
  channel_params_qubitN_2 << gen_amp_damp_rate2, gen_phase_damp_rate2, depol_1qubit_rate2;

  // Create input 1-qubit process matrices at angles {theta1, phi1, lambda1}
  Eigen::MatrixXcd process_mat_noisy_qubit1_1 = qristal::create1QubitNoisyProcessMatrix(theta1[0], phi1[0], lambda1[0], channel_params_qubit1_1);
  Eigen::MatrixXcd process_mat_noisy_qubit2_1 = qristal::create1QubitNoisyProcessMatrix(theta1[1], phi1[1], lambda1[1], channel_params_qubit2_1);
  // Create input N-qubit process matrix at angles {theta1, phi1, lambda1}
  Eigen::MatrixXcd process_mat_noisy_qubitN_1 = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta1, phi1, lambda1, channel_params_qubitN_1);

  // Create input 1-qubit process matrices at angles {theta2, phi2, lambda2}
  Eigen::MatrixXcd process_mat_noisy_qubit1_2 = qristal::create1QubitNoisyProcessMatrix(theta2[0], phi2[0], lambda2[0], channel_params_qubit1_2);
  Eigen::MatrixXcd process_mat_noisy_qubit2_2 = qristal::create1QubitNoisyProcessMatrix(theta2[1], phi2[1], lambda2[1], channel_params_qubit2_2);
  // Create input N-qubit process matrix at angles {theta2, phi2, lambda2}
  Eigen::MatrixXcd process_mat_noisy_qubitN_2 = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta2, phi2, lambda2, channel_params_qubitN_2);

  // Interpolate noise channel parameters to generate a process matrix at target angles
  double theta_target = 0.2*std::numbers::pi;
  double phi_target = 0.3*std::numbers::pi;
  double lambda_target = 0.4*std::numbers::pi;
  Eigen::MatrixXcd process_mat_interp = qristal::processMatrixInterpolator(nb_qubits,
      {process_mat_noisy_qubit1_1, process_mat_noisy_qubit2_1}, {process_mat_noisy_qubit1_2, process_mat_noisy_qubit2_2},
      process_mat_noisy_qubitN_1, process_mat_noisy_qubitN_2, theta1, phi1, lambda1, theta2, phi2, lambda2,
      theta_target, phi_target, lambda_target);

  // Check that interpolated process matrix have the same values as a constructed process
  // matrix with the expected arguments
  std::vector<double> theta_target_vec, phi_target_vec, lambda_target_vec;
  for (size_t i = 0; i < nb_qubits; i++) {
    theta_target_vec.emplace_back(theta_target);
    phi_target_vec.emplace_back(phi_target);
    lambda_target_vec.emplace_back(lambda_target);
  }
  Eigen::VectorXd channel_params_avg = (channel_params_qubitN_1 + channel_params_qubitN_2) / 2;
  Eigen::MatrixXcd reconstructed_process_mat_interp = qristal::createNQubitNoisyProcessMatrix(
      nb_qubits, theta_target_vec, phi_target_vec, lambda_target_vec, channel_params_avg);
  EXPECT_TRUE(reconstructed_process_mat_interp.isApprox(process_mat_interp, 1.0e-6));
}
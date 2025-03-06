// Copyright (c) Quantum Brilliance Pty Ltd
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

TEST(NoiseChannelTester, checkConversions) {
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

  //transform from choi to process:
  Eigen::MatrixXcd process_c = qristal::choi_to_process(choi_mat); 

  //transform superoperator to process: 
  Eigen::MatrixXcd process_s = qristal::superoperator_to_process(superop_2);

  //initialize random density 
  Eigen::Vector<std::complex<double>, Eigen::Dynamic> state = Eigen::Vector<std::complex<double>, Eigen::Dynamic>::Random(std::pow(2, n_qubits));
  state.normalize();
  Eigen::Matrix<std::complex<double>, Eigen::Dynamic, Eigen::Dynamic> density = state * state.adjoint();

  //evolve density with process matrix
  auto evolved_density_process = evolve_density_process(process_mat, density);
  auto evolved_density_process_c = evolve_density_process(process_c, density);
  auto evolved_density_process_s = evolve_density_process(process_s, density);
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
  EXPECT_TRUE(evolved_density_process.isApprox(evolved_density_process_c, 1e-14));
  EXPECT_TRUE(evolved_density_process.isApprox(evolved_density_process_s, 1e-14));
  
  //check transformation to process
  EXPECT_TRUE(process_c.isApprox(process_mat, 1e-14)); //check transformation from choi to process
  EXPECT_TRUE(process_s.isApprox(process_mat, 1e-14)); //check transformation from superoperator to process

  //final check: transform to kraus matrices to NoiseChannel and back to Choi matrix 
  auto choi_mat2 = qristal::kraus_to_choi(kraus_mats_1);
  EXPECT_TRUE(choi_mat.isApprox(choi_mat2, 1e-14));
}

//============================================ Process matrix interpolation methods testers ============================================
// Available channels:
// depolarization_1qubit: 1-qubit depolarization channel. Parameter: 1-qubit depolarization rate.
// depolarization_2qubit: 2-qubit depolarization channel. Parameter: 2-qubit depolarization rate.
// generalized_phase_amplitude_damping: Generalized phase and amplitude damping channel. Parameters: Phase damping rate, amplitude damping rate.
// generalized_amplitude_damping: Generalized amplitude damping channel. Parameter: Amplitude damping rate.
// amplitude_damping: Amplitude damping channel. Parameter: Amplitude damping rate.
// phase_damping: Phase damping channel. Parameter: Phase damping rate.

TEST(NoiseChannelTester, testProcessMatrixSolver1Qubit) {
  // Test 1-qubit process matrix solver for all 1-qubit noise channels.
  size_t max_iter = 1000;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<double> dist_angle(0.0, 2 * std::numbers::pi);
  // Choose some physically meaningful random values
  static std::uniform_real_distribution<double> dist_amp_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_phase_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_depol1(1e-8, 1e-1);

  // Test 4 1-qubit noise channels together with 1-qubit depolarization
  std::vector<std::vector<qristal::noiseChannelSymbol>> channel_tests_vec =
      {{qristal::generalized_phase_amplitude_damping, qristal::depolarization_1qubit},
       {qristal::generalized_amplitude_damping, qristal::depolarization_1qubit},
       {qristal::amplitude_damping, qristal::phase_damping, qristal::depolarization_1qubit}};

  // Create random input noise channel parameters for each channel in each test
  Eigen::VectorXd channel_params_test1(3), channel_params_test2(2), channel_params_test3(3);
  channel_params_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_test2 << dist_amp_damp(gen), dist_depol1(gen);
  channel_params_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  std::vector<Eigen::VectorXd> channel_params_vec = {channel_params_test1, channel_params_test2, channel_params_test3};

  // Loop over tests (different combinations of channels)
  for (size_t test_id = 0; test_id < channel_tests_vec.size(); test_id++) {
    // Create some random Euler angles {theta, phi, lambda}
    double theta = dist_angle(gen);
    double phi = dist_angle(gen);
    double lambda = dist_angle(gen);

    // Retrieve noise channels and number of channel parameters
    std::vector<qristal::noiseChannelSymbol> channel_list = channel_tests_vec[test_id];
    size_t nb_params = 0;
    for (size_t i = 0; i < channel_list.size(); i++) {
      nb_params += qristal::getNumberOfNoiseChannelParams(channel_list[i]);
    }

    // Retrieve channel parameters
    Eigen::VectorXd channel_params = channel_params_vec[test_id];

    // Create input noisy 1-qubit process matrix with angles and noise channels.
    Eigen::MatrixXcd process_mat_noisy = qristal::create1QubitNoisyProcessMatrix(theta, phi, lambda,
                                         channel_list, channel_params);

    // Solve noise channel damping parameters for the input 1-qubit noisy process process matrix
    Eigen::VectorXd x = qristal::processMatrixSolver1Qubit(process_mat_noisy, theta, phi, lambda,
                                                           channel_list, nb_params, max_iter);

    // Check that the solved damping parameters are close to their input values.
    std::cout << "Input values vs. solved values\n";
    for (size_t i = 0; i < channel_params.rows(); i++) {
      double tol = 1e-3 * channel_params(i);
      EXPECT_NEAR(channel_params(i), x(i), tol);
      std::cout << "Input param: " << channel_params(i) << ", solved: " << x(i)
                << ", % diff: " << std::abs(channel_params(i) - x(i)) / channel_params(i) * 100 << "\n";
    }
    std::cout << "\n";

    // Reconstruct the process matrix by using the solved parameters
    Eigen::MatrixXcd reconstructed_mat = qristal::create1QubitNoisyProcessMatrix(theta, phi, lambda,
                                         channel_list, x);
    // Check that the input process matrix and the reconstructed matrix are close.
    EXPECT_TRUE(process_mat_noisy.isApprox(reconstructed_mat, 1.0e-6));
  }
}

TEST(NoiseChannelTester, testProcessMatrixSolverNQubit) {
  // Test N-qubit process matrix solver for all 1-qubit noise channels.
  const size_t nb_qubits = 2;
  size_t max_iter = 1000;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<double> dist_angle(0.0, 2 * std::numbers::pi);
  // Choose some physically meaningful random values
  static std::uniform_real_distribution<double> dist_amp_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_phase_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_depol1(1e-8, 1e-1);

  // Test 4 1-qubit noise channels together with 1-qubit depolarization
  std::vector<std::vector<qristal::noiseChannelSymbol>> channel_tests_vec =
      {{qristal::generalized_phase_amplitude_damping, qristal::depolarization_1qubit},
       {qristal::generalized_amplitude_damping, qristal::depolarization_1qubit},
       {qristal::amplitude_damping, qristal::phase_damping, qristal::depolarization_1qubit}};

  // Create random input noise channel parameters for each channel in each test
  Eigen::VectorXd channel_params_q0_test1(3), channel_params_q1_test1(3), channel_params_qN_test1(6),
                  channel_params_q0_test2(2), channel_params_q1_test2(2), channel_params_qN_test2(4),
                  channel_params_q0_test3(3), channel_params_q1_test3(3), channel_params_qN_test3(6);
  channel_params_q0_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q1_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_qN_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen),
                             dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q0_test2 << dist_amp_damp(gen), dist_depol1(gen);
  channel_params_q1_test2 << dist_amp_damp(gen), dist_depol1(gen);
  channel_params_qN_test2 << dist_amp_damp(gen), dist_depol1(gen),
                             dist_amp_damp(gen), dist_depol1(gen);
  channel_params_q0_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q1_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_qN_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen),
                             dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  std::vector<std::vector<Eigen::VectorXd>> channel_params_vec =
      {{channel_params_q0_test1, channel_params_q1_test1, channel_params_qN_test1},
       {channel_params_q0_test2, channel_params_q1_test2, channel_params_qN_test2},
       {channel_params_q0_test3, channel_params_q1_test3, channel_params_qN_test3}};

  // Loop over tests (different combinations of channels)
  for (size_t test_id = 0; test_id < channel_tests_vec.size(); test_id++) {
    // Create some random Euler angles {theta, phi, lambda}
    std::vector<double> theta, phi, lambda;
    for (size_t i = 0; i < nb_qubits; i++) {
      theta.emplace_back(dist_angle(gen));
      phi.emplace_back(dist_angle(gen));
      lambda.emplace_back(dist_angle(gen));
    }

    // Retrieve noise channels and number of channel parameters
    std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
        qristal::vector_hash<std::vector<size_t>>> channel_list;
    channel_list[{1}] = channel_tests_vec[test_id];
    channel_list[{0}] = channel_tests_vec[test_id];
    std::vector<size_t> nb_params(nb_qubits);
    for (const auto &[qubit, channels] : channel_list) {
      size_t nb_params_i = 0;
      for (size_t i = 0; i < channels.size(); i++) {
        nb_params_i += qristal::getNumberOfNoiseChannelParams(channels[i]);
      }
      nb_params[qubit[0]] = nb_params_i;
    }

    // Retrieve channel parameters
    Eigen::VectorXd channel_params1 = channel_params_vec[test_id][0]; // qubit 1's channel parameters
    Eigen::VectorXd channel_params2 = channel_params_vec[test_id][1]; // qubit 2's channel parameters
    Eigen::VectorXd channel_paramsN = channel_params_vec[test_id][2]; // 2-qubit channel parameters

    // Create input noisy 1-qubit process matrix with angles and noise channels.
    Eigen::MatrixXcd process_mat_noisy1 = qristal::create1QubitNoisyProcessMatrix(theta[0], phi[0], lambda[0],
      channel_list[{0}], channel_params1);
    Eigen::MatrixXcd process_mat_noisy2 = qristal::create1QubitNoisyProcessMatrix(theta[1], phi[1], lambda[1],
      channel_list[{1}], channel_params2);
    // Create input noisy N-qubit process matrix with angles and noise channels.
    Eigen::MatrixXcd process_mat_noisyN = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta, phi, lambda,
        channel_list, channel_paramsN);

    // Solve noise channel damping parameters for the given input process matrix
    std::vector<Eigen::MatrixXcd> process_mat_noisy_1qubit = {process_mat_noisy1, process_mat_noisy2};
    Eigen::VectorXd x = qristal::processMatrixSolverNQubit(process_mat_noisy_1qubit,
        process_mat_noisyN, nb_qubits, theta, phi, lambda, channel_list, nb_params, max_iter);

    // Check that the solved damping parameters are close to their input values.
    std::cout << "Input values vs. solved values\n";
    for (size_t i = 0; i < channel_paramsN.rows(); i++) {
      double tol = 1e-3 * channel_paramsN(i);
      EXPECT_NEAR(channel_paramsN(i), x(i), tol);
      std::cout << "Input param: " << channel_paramsN(i) << ", solved: " << x(i)
                << ", % diff: " << std::abs(channel_paramsN(i) - x(i)) / channel_paramsN(i) * 100 << "\n";
    }
    std::cout << "\n";

    // Reconstruct the process matrix by using the solved parameters
    Eigen::MatrixXcd reconstructed_mat = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta, phi, lambda,
        channel_list, x);
    // Check that the input process matrix and the reconstructed matrix are close.
    EXPECT_TRUE(process_mat_noisyN.isApprox(reconstructed_mat, 1.0e-6));
  }
}

TEST(NoiseChannelTester, testProcessMatrixSolverNQubit_2qubitDepol) {
  // Test N-qubit process matrix solver for 1-qubit noise channels and 2-qubit depolarization.
  const size_t nb_qubits = 2;
  size_t max_iter = 1000;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<double> dist_angle(0.0, 2 * std::numbers::pi);
  // Choose some physically meaningful random values
  static std::uniform_real_distribution<double> dist_amp_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_phase_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_depol2(1e-8, 1e-1);

  // Test 4 1-qubit noise channels together with 2-qubit depolarization (latter is retrieved below)
  std::vector<std::vector<qristal::noiseChannelSymbol>> channel_tests_vec =
      {{qristal::generalized_phase_amplitude_damping},
       {qristal::generalized_amplitude_damping},
       {qristal::amplitude_damping, qristal::phase_damping}};

  // Create random input noise channel parameters for each channel in each test
  Eigen::VectorXd channel_params_q0_test1(2), channel_params_q1_test1(2), channel_params_qN_test1(5),
                  channel_params_q0_test2(1), channel_params_q1_test2(1), channel_params_qN_test2(3),
                  channel_params_q0_test3(2), channel_params_q1_test3(2), channel_params_qN_test3(5);
  channel_params_q0_test1 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_q1_test1 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_qN_test1 << dist_amp_damp(gen), dist_phase_damp(gen),
                             dist_amp_damp(gen), dist_phase_damp(gen),
                             dist_depol2(gen);
  channel_params_q0_test2 << dist_amp_damp(gen);
  channel_params_q1_test2 << dist_amp_damp(gen);
  channel_params_qN_test2 << dist_amp_damp(gen), dist_amp_damp(gen),
                             dist_depol2(gen);
  channel_params_q0_test3 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_q1_test3 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_qN_test3 << dist_amp_damp(gen), dist_phase_damp(gen),
                             dist_amp_damp(gen), dist_phase_damp(gen),
                             dist_depol2(gen);
  std::vector<std::vector<Eigen::VectorXd>> channel_params_vec =
      {{channel_params_q0_test1, channel_params_q1_test1, channel_params_qN_test1},
       {channel_params_q0_test2, channel_params_q1_test2, channel_params_qN_test2},
       {channel_params_q0_test3, channel_params_q1_test3, channel_params_qN_test3}};

  // Loop over tests (different combinations of channels)
  for (size_t test_id = 0; test_id < channel_tests_vec.size(); test_id++) {
    // Create some random Euler angles {theta, phi, lambda}
    std::vector<double> theta, phi, lambda;
    for (size_t i = 0; i < nb_qubits; i++) {
      theta.emplace_back(dist_angle(gen));
      phi.emplace_back(dist_angle(gen));
      lambda.emplace_back(dist_angle(gen));
    }

    // Retrieve noise channels and number of channel parameters
    std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
        qristal::vector_hash<std::vector<size_t>>> channel_list;
    channel_list[{0, 1}] = {qristal::depolarization_2qubit}; // Retrieve 2-qubit depolarization for the qubit pair
    channel_list[{1}] = channel_tests_vec[test_id]; // Qubit 2's channels
    channel_list[{0}] = channel_tests_vec[test_id]; // Qubit 1's channels
    std::vector<size_t> nb_params(channel_list.size());
    size_t channel_ctr = 0;
    for (const auto &[qubit, channels] : channel_list) {
      size_t nb_params_i = 0;
      for (size_t i = 0; i < channels.size(); i++) {
        nb_params_i += qristal::getNumberOfNoiseChannelParams(channels[i]);
      }
      nb_params[channel_ctr] = nb_params_i;
      channel_ctr++;
    }

    // Retrieve channel parameters
    Eigen::VectorXd channel_params1 = channel_params_vec[test_id][0]; // qubit 1's channel parameters
    Eigen::VectorXd channel_params2 = channel_params_vec[test_id][1]; // qubit 2's channel parameters
    Eigen::VectorXd channel_paramsN = channel_params_vec[test_id][2]; // 2-qubit channel parameters

    // Create input noisy 1-qubit process matrix with angles and noise channels.
    Eigen::MatrixXcd process_mat_noisy1 = qristal::create1QubitNoisyProcessMatrix(theta[0], phi[0], lambda[0],
      channel_list[{0}], channel_params1);
    Eigen::MatrixXcd process_mat_noisy2 = qristal::create1QubitNoisyProcessMatrix(theta[1], phi[1], lambda[1],
      channel_list[{1}], channel_params2);
    // Create input noisy N-qubit process matrix with angles and noise channels.
    Eigen::MatrixXcd process_mat_noisyN = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta, phi, lambda,
        channel_list, channel_paramsN);

    // Solve noise channel damping parameters for the given input process matrix
    std::vector<Eigen::MatrixXcd> process_mat_noisy_1qubit = {process_mat_noisy1, process_mat_noisy2};
    Eigen::VectorXd x = qristal::processMatrixSolverNQubit(process_mat_noisy_1qubit,
        process_mat_noisyN, nb_qubits, theta, phi, lambda, channel_list, nb_params, max_iter);

    // Check that the solved damping parameters are close to their input values.
    std::cout << "Input values vs. solved values\n";
    for (size_t i = 0; i < channel_paramsN.rows(); i++) {
      double tol = 1e-3 * channel_paramsN(i);
      EXPECT_NEAR(channel_paramsN(i), x(i), tol);
      std::cout << "Input param: " << channel_paramsN(i) << ", solved: " << x(i)
                << ", % diff: " << std::abs(channel_paramsN(i) - x(i)) / channel_paramsN(i) * 100 << "\n";
    }
    std::cout << "\n";

    // Reconstruct the process matrix by using the solved parameters
    Eigen::MatrixXcd reconstructed_mat = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta, phi, lambda,
        channel_list, x);
    // Check that the input process matrix and the reconstructed matrix are close.
    EXPECT_TRUE(process_mat_noisyN.isApprox(reconstructed_mat, 1.0e-6));
  }
}

TEST(NoiseChannelTester, testProcessMatrixSolverNQubit_2qubitDepol_qubitPairs) {
  // Test N-qubit process matrix solver for a 1-qubit generalized amplitude & phase damping channel 
  // and 2-qubit depolarization.
  // This test is to test for all pairs of qubits, primarily the non-adjacent qubit pair 0 and 2.
  std::vector<std::pair<size_t, size_t>> qubit_pair; // Create some qubit pairs for a 3-qubit system
  qubit_pair.emplace_back(std::make_pair(0, 1));
  qubit_pair.emplace_back(std::make_pair(0, 2));
  qubit_pair.emplace_back(std::make_pair(1, 2));

  const size_t nb_qubits = 3;
  size_t max_iter = 1000;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<double> dist_angle(0.0, 2 * std::numbers::pi);
  // Choose some physically meaningful random values
  static std::uniform_real_distribution<double> dist_amp_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_phase_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_depol2(1e-8, 1e-1);

  // Loop over qubit pairs
  for (size_t qId = 0; qId < qubit_pair.size(); qId++) {
    // Create some random Euler angles {theta, phi, lambda} and noise channel damping parameters.
    std::vector<double> theta = {dist_angle(gen), dist_angle(gen), dist_angle(gen)};
    std::vector<double> phi = {dist_angle(gen), dist_angle(gen), dist_angle(gen)};
    std::vector<double> lambda = {dist_angle(gen), dist_angle(gen), dist_angle(gen)};
    std::vector<double> amp_damp_rate_vec = {dist_amp_damp(gen), dist_amp_damp(gen), dist_amp_damp(gen)};
    std::vector<double> phase_damp_rate_vec = {dist_phase_damp(gen), dist_phase_damp(gen), dist_phase_damp(gen)};
    std::vector<double> depol_2qubit_rate_vec = {dist_depol2(gen)};

    size_t q1 = qubit_pair[qId].first;
    size_t q2 = qubit_pair[qId].second;
    std::cout << "q1:" << q1 << ", q2:" << q2 << "\n";

    // Retrieve noise channels and number of channel parameters
    std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
        qristal::vector_hash<std::vector<size_t>>> channel_list;
    channel_list[{q1, q2}] = {qristal::depolarization_2qubit}; // Retrieve 2-qubit depolarization for the qubit pair
    channel_list[{2}] = {qristal::generalized_phase_amplitude_damping}; // Qubit 3's channels
    channel_list[{1}] = {qristal::generalized_phase_amplitude_damping}; // Qubit 2's channels
    channel_list[{0}] = {qristal::generalized_phase_amplitude_damping}; // Qubit 1's channels
    std::vector<size_t> nb_params(channel_list.size());
    size_t channel_ctr = 0;
    for (const auto &[qubit, channels] : channel_list) {
      size_t nb_params_i = 0;
      for (size_t i = 0; i < channels.size(); i++) {
        nb_params_i += qristal::getNumberOfNoiseChannelParams(channels[i]);
      }
      nb_params[channel_ctr] = nb_params_i;
      channel_ctr++;
    }

    // Retrieve channel parameters
    Eigen::VectorXd amp_damp_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(amp_damp_rate_vec.data(), amp_damp_rate_vec.size());
    Eigen::VectorXd phase_damp_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(phase_damp_rate_vec.data(), phase_damp_rate_vec.size());
    Eigen::VectorXd depol_2qubit_rate = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(depol_2qubit_rate_vec.data(), depol_2qubit_rate_vec.size());
    Eigen::VectorXd channel_params1(2); // 2 parameters from generalized phase & amplitude damping channel
    Eigen::VectorXd channel_params2(2); // 2 parameters from generalized phase & amplitude damping channel
    Eigen::VectorXd channel_params3(2); // 2 parameters from generalized phase & amplitude damping channel
    Eigen::VectorXd channel_paramsN(7); // 7 parameters: 6 from generalized phase & amplitude damping channels and 1 from 2-qubit depol.
    channel_params1 << amp_damp_rate(0), phase_damp_rate(0); // qubit 1's channel parameters
    channel_params2 << amp_damp_rate(1), phase_damp_rate(1); // qubit 2's channel parameters
    channel_params3 << amp_damp_rate(2), phase_damp_rate(2); // qubit 3's channel parameters
    channel_paramsN << amp_damp_rate(0), phase_damp_rate(0), // All qubits' channel parameters
                       amp_damp_rate(1), phase_damp_rate(1),
                       amp_damp_rate(2), phase_damp_rate(2),
                       depol_2qubit_rate(0);

    // Create input noisy 1-qubit process matrix with angles and noise channels.
    Eigen::MatrixXcd process_mat_noisy1 = qristal::create1QubitNoisyProcessMatrix(theta[0], phi[0], lambda[0],
      channel_list[{0}], channel_params1);
    Eigen::MatrixXcd process_mat_noisy2 = qristal::create1QubitNoisyProcessMatrix(theta[1], phi[1], lambda[1],
      channel_list[{1}], channel_params2);
    Eigen::MatrixXcd process_mat_noisy3 = qristal::create1QubitNoisyProcessMatrix(theta[2], phi[2], lambda[2],
      channel_list[{2}], channel_params3);
    // Create input noisy N-qubit process matrix with angles and noise channels.
    Eigen::MatrixXcd process_mat_noisyN = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta, phi, lambda,
        channel_list, channel_paramsN);

    // Solve noise channel damping parameters for the given input process matrix
    std::vector<Eigen::MatrixXcd> process_mat_noisy_1qubit = {process_mat_noisy1, process_mat_noisy2, process_mat_noisy3};
    Eigen::VectorXd x = qristal::processMatrixSolverNQubit(process_mat_noisy_1qubit,
        process_mat_noisyN, nb_qubits, theta, phi, lambda, channel_list, nb_params, max_iter);

    // Check that the solved damping parameters are close to their input values.
    std::cout << "Input values vs. solved values\n";
    for (size_t i = 0; i < channel_paramsN.rows(); i++) {
      double tol = 1e-3 * channel_paramsN(i);
      EXPECT_NEAR(channel_paramsN(i), x(i), tol);
      std::cout << "Input param: " << channel_paramsN(i) << ", solved: " << x(i)
                << ", % diff: " << std::abs(channel_paramsN(i) - x(i)) / channel_paramsN(i) * 100 << "\n";
    }
    std::cout << "\n";

    // Reconstruct the process matrix by using the solved parameters
    Eigen::MatrixXcd reconstructed_mat = qristal::createNQubitNoisyProcessMatrix(nb_qubits, theta, phi, lambda,
                                        channel_list, x);
    // Check that the input process matrix and the reconstructed matrix are close.
    EXPECT_TRUE(process_mat_noisyN.isApprox(reconstructed_mat, 1.0e-6));
  }
}

TEST(NoiseChannelTester, testProcessMatrixInterpolator) {
  // Test process matrix interpolator for all 1-qubit noise channels.
  const size_t nb_qubits = 2;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  // Choose some physically meaningful random values
  static std::uniform_real_distribution<double> dist_amp_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_phase_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_depol1(1e-8, 1e-1);

  // Test 4 1-qubit noise channels together with 1-qubit depolarization
  std::vector<std::vector<qristal::noiseChannelSymbol>> channel_tests_vec =
      {{qristal::generalized_phase_amplitude_damping, qristal::depolarization_1qubit},
       {qristal::generalized_amplitude_damping, qristal::depolarization_1qubit},
       {qristal::amplitude_damping, qristal::phase_damping, qristal::depolarization_1qubit}};

  // Create random input noise channel parameters for each channel in each test
  Eigen::VectorXd channel_params_q0_angle1_test1(3), channel_params_q1_angle1_test1(3), channel_params_qN_angle1_test1(6),
                  channel_params_q0_angle2_test1(3), channel_params_q1_angle2_test1(3), channel_params_qN_angle2_test1(6),
                  channel_params_q0_angle1_test2(2), channel_params_q1_angle1_test2(2), channel_params_qN_angle1_test2(4),
                  channel_params_q0_angle2_test2(2), channel_params_q1_angle2_test2(2), channel_params_qN_angle2_test2(4),
                  channel_params_q0_angle1_test3(3), channel_params_q1_angle1_test3(3), channel_params_qN_angle1_test3(6),
                  channel_params_q0_angle2_test3(3), channel_params_q1_angle2_test3(3), channel_params_qN_angle2_test3(6);
  channel_params_q0_angle1_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q1_angle1_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_qN_angle1_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen),
                                    dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q0_angle2_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q1_angle2_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_qN_angle2_test1 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen),
                                    dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q0_angle1_test2 << dist_amp_damp(gen), dist_depol1(gen);
  channel_params_q1_angle1_test2 << dist_amp_damp(gen), dist_depol1(gen);
  channel_params_qN_angle1_test2 << dist_amp_damp(gen), dist_depol1(gen),
                                    dist_amp_damp(gen), dist_depol1(gen);
  channel_params_q0_angle2_test2 << dist_amp_damp(gen), dist_depol1(gen);
  channel_params_q1_angle2_test2 << dist_amp_damp(gen), dist_depol1(gen);
  channel_params_qN_angle2_test2 << dist_amp_damp(gen), dist_depol1(gen),
                                    dist_amp_damp(gen), dist_depol1(gen);
  channel_params_q0_angle1_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q1_angle1_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_qN_angle1_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen),
                                    dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q0_angle2_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_q1_angle2_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);
  channel_params_qN_angle2_test3 << dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen),
                                    dist_amp_damp(gen), dist_phase_damp(gen), dist_depol1(gen);

  std::vector<std::vector<Eigen::VectorXd>> channel_params_vec_angle1 =
      {{channel_params_q0_angle1_test1, channel_params_q1_angle1_test1, channel_params_qN_angle1_test1},
       {channel_params_q0_angle1_test2, channel_params_q1_angle1_test2, channel_params_qN_angle1_test2},
       {channel_params_q0_angle1_test3, channel_params_q1_angle1_test3, channel_params_qN_angle1_test3}};
  std::vector<std::vector<Eigen::VectorXd>> channel_params_vec_angle2 =
      {{channel_params_q0_angle2_test1, channel_params_q1_angle2_test1, channel_params_qN_angle2_test1},
       {channel_params_q0_angle2_test2, channel_params_q1_angle2_test2, channel_params_qN_angle2_test2},
       {channel_params_q0_angle2_test3, channel_params_q1_angle2_test3, channel_params_qN_angle2_test3}};

  // Loop over tests (different combinations of channels)
  for (size_t test_id = 0; test_id < channel_tests_vec.size(); test_id++) {
    // Retrieve noise channels and number of channel parameters
    std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
        qristal::vector_hash<std::vector<size_t>>> channel_list;
    channel_list[{1}] = channel_tests_vec[test_id];
    channel_list[{0}] = channel_tests_vec[test_id];
    std::vector<size_t> nb_params(nb_qubits);
    for (const auto &[qubit, channels] : channel_list) {
      size_t nb_params_i = 0;
      for (size_t i = 0; i < channels.size(); i++) {
        nb_params_i += qristal::getNumberOfNoiseChannelParams(channels[i]);
      }
      nb_params[qubit[0]] = nb_params_i;
    }

    // Create angles {theta1, phi1, lambda1}
    std::vector<double> theta1 = {0.1*std::numbers::pi, 0.2*std::numbers::pi};
    std::vector<double> phi1 = {0.3*std::numbers::pi, 0.4*std::numbers::pi};
    std::vector<double> lambda1 = {0.5*std::numbers::pi, 0.6*std::numbers::pi};

    // Create angles {theta2, phi2, lambda2}
    std::vector<double> theta2 = {0.3*std::numbers::pi, 0.4*std::numbers::pi};
    std::vector<double> phi2 = {0.5*std::numbers::pi, 0.6*std::numbers::pi};
    std::vector<double> lambda2 = {0.7*std::numbers::pi, 0.8*std::numbers::pi};

    // Retrieve channel parameters
    Eigen::VectorXd channel_params_qubit1_angle1 = channel_params_vec_angle1[test_id][0];
    Eigen::VectorXd channel_params_qubit2_angle1 = channel_params_vec_angle1[test_id][1];
    Eigen::VectorXd channel_params_qubitN_angle1 = channel_params_vec_angle1[test_id][2];
    Eigen::VectorXd channel_params_qubit1_angle2 = channel_params_vec_angle2[test_id][0];
    Eigen::VectorXd channel_params_qubit2_angle2 = channel_params_vec_angle2[test_id][1];
    Eigen::VectorXd channel_params_qubitN_angle2 = channel_params_vec_angle2[test_id][2];

    // Create input noisy 1-qubit process matrices with angles {theta1, phi1, lambda1} and noise channels
    Eigen::MatrixXcd process_mat_noisy_qubit1_angle1 = qristal::create1QubitNoisyProcessMatrix(
        theta1[0], phi1[0], lambda1[0], channel_list[{0}], channel_params_qubit1_angle1);
    Eigen::MatrixXcd process_mat_noisy_qubit2_angle1 = qristal::create1QubitNoisyProcessMatrix(
      theta1[1], phi1[1], lambda1[1], channel_list[{1}], channel_params_qubit2_angle1);
    // Create input N-qubit process matrix with angles {theta1, phi1, lambda1} and noise channels
    Eigen::MatrixXcd process_mat_noisy_qubitN_angle1 = qristal::createNQubitNoisyProcessMatrix(nb_qubits,
        theta1, phi1, lambda1, channel_list, channel_params_qubitN_angle1);

    // Create input 1-qubit process matrices with angles {theta2, phi2, lambda2} and noise channels
    Eigen::MatrixXcd process_mat_noisy_qubit1_angle2 = qristal::create1QubitNoisyProcessMatrix(
        theta2[0], phi2[0], lambda2[0], channel_list[{0}], channel_params_qubit1_angle2);
    Eigen::MatrixXcd process_mat_noisy_qubit2_angle2 = qristal::create1QubitNoisyProcessMatrix(
        theta2[1], phi2[1], lambda2[1], channel_list[{1}], channel_params_qubit2_angle2);
    // Create input N-qubit process matrix with angles {theta2, phi2, lambda2} and noise channels
    Eigen::MatrixXcd process_mat_noisy_qubitN_angle2 = qristal::createNQubitNoisyProcessMatrix(
        nb_qubits, theta2, phi2, lambda2, channel_list, channel_params_qubitN_angle2);

    // Interpolate noise channel parameters to generate a process matrix at target angles
    double theta_target = 0.2*std::numbers::pi;
    double phi_target = 0.3*std::numbers::pi;
    double lambda_target = 0.4*std::numbers::pi;
    std::vector<Eigen::MatrixXcd> process_mat_noisy_1qubit_angle1 = {process_mat_noisy_qubit1_angle1, process_mat_noisy_qubit2_angle1};
    std::vector<Eigen::MatrixXcd> process_mat_noisy_1qubit_angle2 = {process_mat_noisy_qubit1_angle2, process_mat_noisy_qubit2_angle2};

    //(A) obtain the noise channel params for each process matrix 
    Eigen::VectorXd params1 = qristal::processMatrixSolverNQubit(
      process_mat_noisy_1qubit_angle1, process_mat_noisy_qubitN_angle1, nb_qubits, theta1, phi1, lambda1, channel_list, nb_params
    );
    Eigen::VectorXd params2 = qristal::processMatrixSolverNQubit(
      process_mat_noisy_1qubit_angle2, process_mat_noisy_qubitN_angle2, nb_qubits, theta2, phi2, lambda2, channel_list, nb_params
    );
    //(B) construct interpolator 
    qristal::NoiseChannelInterpolator interpolator(
      {params1, params2}, //noise channels for two different angles 
      {{theta1[0], phi1[0], lambda1[0]}, {theta1[1], phi1[1], lambda1[1]}}, 
      qristal::InterpolationModel(qristal::InterpolationModel::Type::Average)
    );
    //(C) obtain interpolated noise channel parameters for target 
    auto new_channels = interpolator({theta_target, phi_target, lambda_target}); 

    //check that the interpolated channels are just the average 
    Eigen::VectorXd channel_params_avg = (channel_params_qubitN_angle1 + channel_params_qubitN_angle2) / 2;
    EXPECT_TRUE(channel_params_avg.isApprox(new_channels, 1e-6)); 
  }
}

TEST(NoiseChannelTester, testProcessMatrixInterpolator_2qubitDepol) {
  // Test process matrix interpolator for 1-qubit noise channels and 2-qubit depolarization.
  const size_t nb_qubits = 2;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  // Choose some physically meaningful random values
  static std::uniform_real_distribution<double> dist_amp_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_phase_damp(1e-8, 1e-1);
  static std::uniform_real_distribution<double> dist_depol2(1e-8, 1e-1);

  // Test 4 1-qubit noise channels together with 2-qubit depolarization (latter is retrieved below)
  std::vector<std::vector<qristal::noiseChannelSymbol>> channel_tests_vec =
      {{qristal::generalized_phase_amplitude_damping},
       {qristal::generalized_amplitude_damping},
       {qristal::amplitude_damping, qristal::phase_damping}};

  //  Create random input noise channel parameters for each channel in each test
  Eigen::VectorXd channel_params_q0_angle1_test1(2), channel_params_q1_angle1_test1(2), channel_params_qN_angle1_test1(5),
                  channel_params_q0_angle2_test1(2), channel_params_q1_angle2_test1(2), channel_params_qN_angle2_test1(5),
                  channel_params_q0_angle1_test2(1), channel_params_q1_angle1_test2(1), channel_params_qN_angle1_test2(3),
                  channel_params_q0_angle2_test2(1), channel_params_q1_angle2_test2(1), channel_params_qN_angle2_test2(3),
                  channel_params_q0_angle1_test3(2), channel_params_q1_angle1_test3(2), channel_params_qN_angle1_test3(5),
                  channel_params_q0_angle2_test3(2), channel_params_q1_angle2_test3(2), channel_params_qN_angle2_test3(5);
  channel_params_q0_angle1_test1 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_q1_angle1_test1 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_qN_angle1_test1 << dist_amp_damp(gen), dist_phase_damp(gen),
                                    dist_amp_damp(gen), dist_phase_damp(gen),
                                    dist_depol2(gen);
  channel_params_q0_angle2_test1 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_q1_angle2_test1 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_qN_angle2_test1 << dist_amp_damp(gen), dist_phase_damp(gen),
                                    dist_amp_damp(gen), dist_phase_damp(gen),
                                    dist_depol2(gen);
  channel_params_q0_angle1_test2 << dist_amp_damp(gen);
  channel_params_q1_angle1_test2 << dist_amp_damp(gen);
  channel_params_qN_angle1_test2 << dist_amp_damp(gen), dist_amp_damp(gen), dist_depol2(gen);
  channel_params_q0_angle2_test2 << dist_amp_damp(gen);
  channel_params_q1_angle2_test2 << dist_amp_damp(gen);
  channel_params_qN_angle2_test2 << dist_amp_damp(gen), dist_amp_damp(gen), dist_depol2(gen);
  channel_params_q0_angle1_test3 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_q1_angle1_test3 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_qN_angle1_test3 << dist_amp_damp(gen), dist_phase_damp(gen),
                                    dist_amp_damp(gen), dist_phase_damp(gen),
                                    dist_depol2(gen);
  channel_params_q0_angle2_test3 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_q1_angle2_test3 << dist_amp_damp(gen), dist_phase_damp(gen);
  channel_params_qN_angle2_test3 << dist_amp_damp(gen), dist_phase_damp(gen),
                                    dist_amp_damp(gen), dist_phase_damp(gen),
                                    dist_depol2(gen);

  std::vector<std::vector<Eigen::VectorXd>> channel_params_vec_angle1 =
      {{channel_params_q0_angle1_test1, channel_params_q1_angle1_test1, channel_params_qN_angle1_test1},
       {channel_params_q0_angle1_test2, channel_params_q1_angle1_test2, channel_params_qN_angle1_test2},
       {channel_params_q0_angle1_test3, channel_params_q1_angle1_test3, channel_params_qN_angle1_test3}};
  std::vector<std::vector<Eigen::VectorXd>> channel_params_vec_angle2 =
      {{channel_params_q0_angle2_test1, channel_params_q1_angle2_test1, channel_params_qN_angle2_test1},
       {channel_params_q0_angle2_test2, channel_params_q1_angle2_test2, channel_params_qN_angle2_test2},
       {channel_params_q0_angle2_test3, channel_params_q1_angle2_test3, channel_params_qN_angle2_test3}};

  // Loop over tests (different combinations of channels)
  for (size_t test_id = 0; test_id < channel_tests_vec.size(); test_id++) {
    std::unordered_map<std::vector<size_t>, std::vector<qristal::noiseChannelSymbol>,
        qristal::vector_hash<std::vector<size_t>>> channel_list;
    channel_list[{0, 1}] = {qristal::depolarization_2qubit}; // Retrieve 2-qubit depolarization for the qubit pair
    channel_list[{1}] = channel_tests_vec[test_id]; // Qubit 2's channels
    channel_list[{0}] = channel_tests_vec[test_id]; // Qubit 1's channels
    std::vector<size_t> nb_params(channel_list.size());
    size_t channel_ctr = 0;
    for (const auto &[qubit, channels] : channel_list) {
      size_t nb_params_i = 0;
      for (size_t i = 0; i < channels.size(); i++) {
        nb_params_i += qristal::getNumberOfNoiseChannelParams(channels[i]);
      }
      nb_params[channel_ctr] = nb_params_i;
      channel_ctr++;
    }

    // Create angles {theta1, phi1, lambda1}
    std::vector<double> theta1 = {0.1*std::numbers::pi, 0.2*std::numbers::pi};
    std::vector<double> phi1 = {0.3*std::numbers::pi, 0.4*std::numbers::pi};
    std::vector<double> lambda1 = {0.5*std::numbers::pi, 0.6*std::numbers::pi};

    // Create angles {theta2, phi2, lambda2}
    std::vector<double> theta2 = {0.3*std::numbers::pi, 0.4*std::numbers::pi};
    std::vector<double> phi2 = {0.5*std::numbers::pi, 0.6*std::numbers::pi};
    std::vector<double> lambda2 = {0.7*std::numbers::pi, 0.8*std::numbers::pi};

    // Retrieve channel parameters
    Eigen::VectorXd channel_params_qubit1_angle1 = channel_params_vec_angle1[test_id][0];
    Eigen::VectorXd channel_params_qubit2_angle1 = channel_params_vec_angle1[test_id][1];
    Eigen::VectorXd channel_params_qubitN_angle1 = channel_params_vec_angle1[test_id][2];
    Eigen::VectorXd channel_params_qubit1_angle2 = channel_params_vec_angle2[test_id][0];
    Eigen::VectorXd channel_params_qubit2_angle2 = channel_params_vec_angle2[test_id][1];
    Eigen::VectorXd channel_params_qubitN_angle2 = channel_params_vec_angle2[test_id][2];

    // Create input noisy 1-qubit process matrices with angles {theta1, phi1, lambda1} and noise channels
    Eigen::MatrixXcd process_mat_noisy_qubit1_angle1 = qristal::create1QubitNoisyProcessMatrix(
        theta1[0], phi1[0], lambda1[0], channel_list[{0}], channel_params_qubit1_angle1);
    Eigen::MatrixXcd process_mat_noisy_qubit2_angle1 = qristal::create1QubitNoisyProcessMatrix(
      theta1[1], phi1[1], lambda1[1], channel_list[{1}], channel_params_qubit2_angle1);
    // Create input N-qubit process matrix with angles {theta1, phi1, lambda1} and noise channels
    Eigen::MatrixXcd process_mat_noisy_qubitN_angle1 = qristal::createNQubitNoisyProcessMatrix(nb_qubits,
        theta1, phi1, lambda1, channel_list, channel_params_qubitN_angle1);

    // Create input 1-qubit process matrices with angles {theta2, phi2, lambda2} and noise channels
    Eigen::MatrixXcd process_mat_noisy_qubit1_angle2 = qristal::create1QubitNoisyProcessMatrix(
        theta2[0], phi2[0], lambda2[0], channel_list[{0}], channel_params_qubit1_angle2);
    Eigen::MatrixXcd process_mat_noisy_qubit2_angle2 = qristal::create1QubitNoisyProcessMatrix(
        theta2[1], phi2[1], lambda2[1], channel_list[{1}], channel_params_qubit2_angle2);
    // Create input N-qubit process matrix with angles {theta2, phi2, lambda2} and noise channels
    Eigen::MatrixXcd process_mat_noisy_qubitN_angle2 = qristal::createNQubitNoisyProcessMatrix(
        nb_qubits, theta2, phi2, lambda2, channel_list, channel_params_qubitN_angle2);

    // Interpolate noise channel parameters to generate a process matrix at target angles
    double theta_target = 0.2*std::numbers::pi;
    double phi_target = 0.3*std::numbers::pi;
    double lambda_target = 0.4*std::numbers::pi;
    std::vector<Eigen::MatrixXcd> process_mat_noisy_1qubit_angle1 = {process_mat_noisy_qubit1_angle1, process_mat_noisy_qubit2_angle1};
    std::vector<Eigen::MatrixXcd> process_mat_noisy_1qubit_angle2 = {process_mat_noisy_qubit1_angle2, process_mat_noisy_qubit2_angle2};

    //(A) obtain the noise channel params for each process matrix 
    Eigen::VectorXd params1 = qristal::processMatrixSolverNQubit(
      process_mat_noisy_1qubit_angle1, process_mat_noisy_qubitN_angle1, nb_qubits, theta1, phi1, lambda1, channel_list, nb_params
    );
    Eigen::VectorXd params2 = qristal::processMatrixSolverNQubit(
      process_mat_noisy_1qubit_angle2, process_mat_noisy_qubitN_angle2, nb_qubits, theta2, phi2, lambda2, channel_list, nb_params
    );
    //(B) construct interpolator 
    qristal::NoiseChannelInterpolator interpolator(
      {params1, params2}, //noise channels for two different angles 
      {{theta1[0], phi1[0], lambda1[0]}, {theta1[1], phi1[1], lambda1[1]}}, 
      qristal::InterpolationModel(qristal::InterpolationModel::Type::Average)
    );
    //(C) obatin interpolated noise channel parameters for target 
    auto new_channels = interpolator({theta_target, phi_target, lambda_target}); 

    //check that the interpolated channels are just the average 
    Eigen::VectorXd channel_params_avg = (channel_params_qubitN_angle1 + channel_params_qubitN_angle2) / 2;
    EXPECT_TRUE(channel_params_avg.isApprox(new_channels, 1e-6)); 
  }
}

TEST(NoiseChannelTester, check_basic_interpolation) {
  //trivial check for correct 1-D interpolation
  std::vector<qristal::U3Angle> angles{
    {0.0, 0.0, 0.0}, 
    {1.0, 0.0, 0.0},
    {2.0, 0.0, 0.0}, 
    {3.0, 0.0, 0.0}
  }; 

  //different noise channels for the four angles to be interpolated
  //value 1: (10, 20, 30, 40) -> average : 25.0 (constant)
  //value 2: (5, 10, 15, 20) -> linear : 5*x + 5
  //value 3: (0, 1, 4, 9) -> polynomial (degree 2) : x^2
  //value 4: (1, 2, 4, 8) -> exponential : 2^x = exp(ln(2) * x)
  Eigen::VectorXd p1(4); 
  p1 << 10.0, 5.0, 0.0, 1.0; 
  Eigen::VectorXd p2(4); 
  p2 << 20.0, 10.0, 1.0, 2.0; 
  Eigen::VectorXd p3(4); 
  p3 << 30.0, 15.0, 4.0, 4.0; 
  Eigen::VectorXd p4(4);
  p4 << 40.0, 20.0, 9.0, 8.0;

  //create interpolator with different models for each value
  qristal::NoiseChannelInterpolator interpolator(
    {p1, p2, p3, p4},
    angles,
    {
      qristal::InterpolationModel(qristal::InterpolationModel::Type::Average), 
      qristal::InterpolationModel(qristal::InterpolationModel::Type::Linear), 
      qristal::InterpolationModel(qristal::InterpolationModel::Type::Polynomial, 2), 
      qristal::InterpolationModel(qristal::InterpolationModel::Type::Exponential)
    }
  ); 

  //check correct interpolation in test range from 0.5 to 10.0
  for (double test = 0.5; test <= 10.0; test += 0.1) {
    auto new_channels = interpolator({test, 0.0, 0.0}); 
    Eigen::VectorXd correct(4); 
    correct << 25.0, 5.0*test + 5.0, std::pow(test, 2), std::exp(std::log(2.0)*test); 
    EXPECT_TRUE(correct.isApprox(new_channels, 1e-12)); 
  }
}

TEST(NoiseChannelTester, testexpandProcessMatrixSpace) {
  //In this test: Create a random (ideal) 1-qubit process matrix, expand it to up to n qubit space 
  //and check correct density evolution 
  std::uniform_real_distribution<double> gen(0, 2.0 * std::numbers::pi);
  std::default_random_engine re;
  Eigen::MatrixXcd chi = qristal::createIdealU3ProcessMatrix(gen(re), gen(re), gen(re)); 
  Eigen::MatrixXcd I_process = Eigen::MatrixXcd::Zero(4, 4);
  I_process(0, 0) = 1.0; 

  //initialize random 1-qubit density 
  Eigen::VectorXcd state = Eigen::VectorXcd::Random(2);
  state.normalize();
  Eigen::MatrixXcd density = state * state.adjoint();
  Eigen::MatrixXcd I_density = Eigen::MatrixXcd::Zero(2, 2);
  I_density(0, 0) = 1.0; 

  Eigen::MatrixXcd correct_evolution = evolve_density_process(chi, density);

  const size_t max_n_qubits = 4;
  for (size_t n_qubits = 1; n_qubits <= max_n_qubits; ++n_qubits) {
    for (size_t q_idx = 0; q_idx < n_qubits; ++q_idx) {
      //(1) build up n_qubit density matrix and n qubit evolved density matrix 
      Eigen::MatrixXcd n_density = Eigen::MatrixXcd::Ones(1, 1);
      Eigen::MatrixXcd correct_n_evolved_density = Eigen::MatrixXcd::Ones(1, 1);
      for (size_t q = 0; q < n_qubits; ++q) {
        if (q == q_idx) {
          n_density = Eigen::kroneckerProduct(n_density, density).eval();
          correct_n_evolved_density = Eigen::kroneckerProduct(correct_n_evolved_density, correct_evolution).eval();
        }
        else {
          n_density = Eigen::kroneckerProduct(n_density, I_density).eval();
          correct_n_evolved_density = Eigen::kroneckerProduct(correct_n_evolved_density, I_density).eval();
        }
      }

      //(2) evolve n_qubit density with expanded process matrix 
      Eigen::MatrixXcd n_super = qristal::expandProcessMatrixSpace({q_idx}, n_qubits, qristal::process_to_superoperator(chi)); 
      Eigen::MatrixXcd n_evolved_density = evolve_density_superop(n_super, n_density);

      //(3) check against expanded correct evolved density 
      EXPECT_TRUE(correct_n_evolved_density.isApprox(n_evolved_density, 1e-12));
    }
  }
}

TEST(NoiseChannelTester, testTraceProcessMatrix){
  std::vector<size_t> nb_qubits = {2,3,4,5}; 

  for (size_t n_id = 0; n_id < nb_qubits.size(); ++n_id){
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<double> dist_angle(0.0, 2 * std::numbers::pi);
  
    // Create nb_qubits random angles
    std::vector<double> theta, phi, lambda;
    
    for (size_t i = 0; i < nb_qubits[n_id]; ++i) {
      theta.emplace_back(dist_angle(gen));
      phi.emplace_back(dist_angle(gen));
      lambda.emplace_back(dist_angle(gen));
    }
    
    // Calculate tensor product of the nb_qubits process matrices
    Eigen::MatrixXcd process = qristal::createIdealU3ProcessMatrix(theta[0],phi[0],lambda[0]);
    for (size_t i = 1; i < nb_qubits[n_id]; ++i) {
      process = Eigen::kroneckerProduct(process,qristal::createIdealU3ProcessMatrix(theta[i],phi[i],lambda[i])).eval();
    }

    // Check partial trace with indices are all subsets of {0,.., nb_qubits-1} except empty and improper subset
    std::set<size_t> indices;
    size_t n_subsets = std::pow(2, nb_qubits[n_id]);
    for(size_t i=1; i<n_subsets-1; ++i){
      size_t r = i;
      size_t v = 0;
      indices = {};
      while(r > 0){
          if (fmod(r,2) ==1){
              indices.insert(v);
          }
          v++;
          r = size_t(r/2);
      }
      Eigen::MatrixXcd trace_keep = qristal::partialTraceProcessMatrixKeep(process, indices);
      Eigen::MatrixXcd trace_remove = qristal::partialTraceProcessMatrixRemove(process, qristal::getComplementarySet(nb_qubits[n_id], indices));
      //Calculate reduced process matrix
      Eigen::MatrixXcd reduced_process = Eigen::MatrixXcd::Ones(1, 1);
      for (auto const& i : indices) {
        reduced_process = Eigen::kroneckerProduct(reduced_process, qristal::createIdealU3ProcessMatrix(theta[i], phi[i], lambda[i])).eval();
      }
      EXPECT_TRUE(trace_keep.isApprox(reduced_process, 1e-12));
      EXPECT_TRUE(trace_remove.isApprox(reduced_process, 1e-12));
    }
  }
}


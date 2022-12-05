// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include "xacc.hpp"
#include "Accelerator.hpp"
#include "qb/core/noise_model/noise_model.hpp"
#include "qb/core/noise_model/noise_model_factory.hpp"
TEST(NoiseModelTester, checkReadoutErrors)
{
  qb::NoiseProperties noise_props;
  // Use very different values to check:
  const qb::NoiseProperties::ReadoutError ro_1{0.1, 0.2};
  const qb::NoiseProperties::ReadoutError ro_2{0.3, 0.4};
  noise_props.readout_errors = {{0, ro_1},
                                {1, ro_2}};
  qb::NoiseModel noise_model(noise_props);
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
  qb::NoiseModel noise_model;
  noise_model.add_gate_error(qb::GeneralizedAmplitudeDampingChannel::Create(0, 0.25, 0.75), "id", {0});
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

TEST(NoiseModelTester, checkNoiseModelFactory)
{
  // Get the 'default' noise model, simple uniform Pauli depolarizing noise.
  auto noise_model_factory = qb::get_noise_model_factory("default");
  EXPECT_TRUE(noise_model_factory != nullptr);
  auto noise_model = noise_model_factory->Create(2);
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
  qb::NoiseProperties noise_props;
  noise_props.t1_us = {{0, 1e6}};
  noise_props.t2_us = {{0, 1e3}};
  std::map<std::vector<size_t>, double> gate_time;
  gate_time[std::vector<size_t>{0}] = 10.0;
  noise_props.gate_time_us["u3"] = gate_time;
  std::map<std::vector<size_t>, double> rb_gate_error;
  rb_gate_error[std::vector<size_t>{0}] = 0.01;
  noise_props.gate_pauli_errors["u3"] = rb_gate_error;
  qb::NoiseModel noise_model(noise_props);
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


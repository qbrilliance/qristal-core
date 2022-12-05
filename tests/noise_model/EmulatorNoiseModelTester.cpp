// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include "xacc.hpp"
#include "Accelerator.hpp"
#include "qb/core/noise_model/noise_model.hpp"
#include "qb/core/noise_model/noise_model_factory.hpp"

// IMPORTANT: This test requires that emulator noise model library.

TEST(EmulatorNoiseModelTester, check_nm1)
{
  auto factory = qb::get_noise_model_factory("qb-nm1");
  EXPECT_TRUE(factory != nullptr);
  auto noise_model = factory->Create();
  std::set<int> qubits;
  for (const auto& [q1, q2]: noise_model.get_connectivity()) {
    qubits.emplace(q1);
    qubits.emplace(q2);
  }
  EXPECT_EQ(qubits.size(), 48);
}

TEST(EmulatorNoiseModelTester, check_nm2)
{
  auto factory = qb::get_noise_model_factory("qb-nm2");
  EXPECT_TRUE(factory != nullptr);
  auto noise_model = factory->Create();
  std::set<int> qubits;
  for (const auto& [q1, q2]: noise_model.get_connectivity()) {
    qubits.emplace(q1);
    qubits.emplace(q2);
  }
  EXPECT_EQ(qubits.size(), 64);
}



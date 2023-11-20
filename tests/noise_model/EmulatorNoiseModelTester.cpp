// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>
#include "xacc.hpp"
#include "Accelerator.hpp"
#include "qb/core/noise_model/noise_model.hpp"

// IMPORTANT: This test requires that emulator noise model library.

TEST(EmulatorNoiseModelTester, check_nm1)
{
  qb::NoiseModel noise_model("qb-nm1");
  std::set<int> qubits;
  for (const auto& [q1, q2]: noise_model.get_connectivity()) {
    qubits.emplace(q1);
    qubits.emplace(q2);
  }
  EXPECT_EQ(qubits.size(), 48);
}

TEST(EmulatorNoiseModelTester, check_nm2)
{
  qb::NoiseModel noise_model("qb-nm2");
  std::set<int> qubits;
  for (const auto& [q1, q2]: noise_model.get_connectivity()) {
    qubits.emplace(q1);
    qubits.emplace(q2);
  }
  EXPECT_EQ(qubits.size(), 64);
}

TEST(EmulatorNoiseModelTester, check_nm3)
{
  qb::NoiseModel noise_model("qb-nm3");
  std::set<int> qubits;
  for (const auto& [q1, q2]: noise_model.get_connectivity()) {
    qubits.emplace(q1);
    qubits.emplace(q2);
  }
  EXPECT_EQ(qubits.size(), 48);
}

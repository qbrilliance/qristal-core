// Copyright (c) Quantum Brilliance Pty Ltd
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

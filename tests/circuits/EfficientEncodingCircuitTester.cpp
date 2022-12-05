// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "Circuit.hpp"
#include <gtest/gtest.h>
#include <bitset>
#include <memory>

TEST(EfficientEncodingCircuitTester, noancilla) {
  // Test Efficient Encoding: prepare the entangled state |state>|score> given
  // by the scoring function In practice the scoring function may call on data
  // from a table In this example the scoring function just returns the value of
  // the state We input the state |+++>|000> We expect the output |+++>|+++> =
  // |000>|000> + ... + |111>|111>

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  std::function<int(int)> scoring_function = [&](int i) { return i; };
  int num_state_qubits = 3;
  int num_scoring_qubits = 3;
  auto efficient_encoder =
      std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EfficientEncoding"));
  const bool expand_ok =
      efficient_encoder->expand({{"scoring_function", scoring_function},
                                 {"num_state_qubits", num_state_qubits},
                                 {"num_scoring_qubits", num_scoring_qubits}});
  EXPECT_TRUE(expand_ok);

  auto circuit = gateRegistry->createComposite("sim_comp");
  for (int i = 0; i < num_state_qubits; i++) {
    circuit->addInstruction(gateRegistry->createInstruction("H", i));
  }
  circuit->addInstructions(efficient_encoder->getInstructions());
  for (int i = 0; i < num_state_qubits + num_scoring_qubits; i++) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", i));
  }

  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(num_state_qubits+num_scoring_qubits);
  acc->execute(buffer, circuit);
  buffer->print();
  int a = buffer->getMeasurementCounts()["000000"];
  int b = buffer->getMeasurementCounts()["001001"];
  int c = buffer->getMeasurementCounts()["010010"];
  int d = buffer->getMeasurementCounts()["011011"];
  int e = buffer->getMeasurementCounts()["100100"];
  int f = buffer->getMeasurementCounts()["101101"];
  int g = buffer->getMeasurementCounts()["110110"];
  int h = buffer->getMeasurementCounts()["111111"];
  EXPECT_GT(a, 0);
  EXPECT_GT(b, 0);
  EXPECT_GT(c, 0);
  EXPECT_GT(d, 0);
  EXPECT_GT(e, 0);
  EXPECT_GT(f, 0);
  EXPECT_GT(g, 0);
  EXPECT_GT(h, 0);
  EXPECT_EQ(a + b + c + d + e + f + g + h, 1024);
}

TEST(EfficientEncodingCircuitTester, ancilla) {
  // Test Efficient Encoding: prepare the entangled state |state>|score> given
  // by the scoring function In practice the scoring function may call on data
  // from a table In this example the scoring function just returns the value of
  // the state We input the state |+++>|000> We expect the output |+++>|+++> =
  // |000>|000> + ... + |111>|111>

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  std::function<int(int)> scoring_function = [&](int i) { return i; };
  int num_state_qubits = 3;
  int num_scoring_qubits = 3;
  auto efficient_encoder =
      std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EfficientEncoding"));
  const bool expand_ok =
      efficient_encoder->expand({{"scoring_function", scoring_function},
                                 {"num_state_qubits", num_state_qubits},
                                 {"num_scoring_qubits", num_scoring_qubits},
                                 {"use_ancilla", true}});
  EXPECT_TRUE(expand_ok);

  auto circuit = gateRegistry->createComposite("sim_comp");
  for (int i = 0; i < num_state_qubits; i++) {
    circuit->addInstruction(gateRegistry->createInstruction("H", i));
  }
  circuit->addInstructions(efficient_encoder->getInstructions());
  for (int i = 0; i < num_state_qubits + num_scoring_qubits; i++) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", i));
  }

  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(2*num_state_qubits+num_scoring_qubits-1);
  acc->execute(buffer, circuit);
  buffer->print();
  int a = buffer->getMeasurementCounts()["000000"];
  int b = buffer->getMeasurementCounts()["001001"];
  int c = buffer->getMeasurementCounts()["010010"];
  int d = buffer->getMeasurementCounts()["011011"];
  int e = buffer->getMeasurementCounts()["100100"];
  int f = buffer->getMeasurementCounts()["101101"];
  int g = buffer->getMeasurementCounts()["110110"];
  int h = buffer->getMeasurementCounts()["111111"];
  EXPECT_GT(a, 0);
  EXPECT_GT(b, 0);
  EXPECT_GT(c, 0);
  EXPECT_GT(d, 0);
  EXPECT_GT(e, 0);
  EXPECT_GT(f, 0);
  EXPECT_GT(g, 0);
  EXPECT_GT(h, 0);
  EXPECT_EQ(a + b + c + d + e + f + g + h, 1024);
}


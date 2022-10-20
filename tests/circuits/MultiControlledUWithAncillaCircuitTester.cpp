// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <CompositeInstruction.hpp>
#include <bitset>
#include <gtest/gtest.h>
#include <memory>

TEST(MultiControlledUWithAncillaCircuitTester, checksimple) {
  for (int i = 0; i < 64; i++) {

    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    std::vector<int> qubits_control = {0, 1, 2, 3, 4};
    std::vector<int> qubits_ancilla = {6, 7, 8, 9};
    auto U = gateRegistry->createComposite("U");
    U->addInstruction(gateRegistry->createInstruction("X", 5));
    auto mcu = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("MultiControlledUWithAncilla"));
    const bool expand_ok = mcu->expand({{"qubits_control", qubits_control},
                                        {"qubits_ancilla", qubits_ancilla},
                                        {"U", U}});
    EXPECT_TRUE(expand_ok);

    auto circuit = gateRegistry->createComposite("sim_mcu");

    std::string i_binary = std::bitset<6>(i).to_string();
    std::string i_binary_n =
        i_binary.substr(i_binary.size() < 6 ? 0 : i_binary.size() - 6);

    for (int j = 0; j < 6; j++) {
      if (i_binary_n[j] == '1') {
        circuit->addInstruction(gateRegistry->createInstruction("X", j));
      }
    }

    circuit->addInstructions(mcu->getInstructions());

    for (int bit : qubits_control) {
        circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
    }
    circuit->addInstruction(gateRegistry->createInstruction("Measure", 5));

    auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
    auto buffer = xacc::qalloc(10);
    acc->execute(buffer, circuit);
    if (i != 62 && i != 63) {
      EXPECT_EQ(buffer->getMeasurementCounts()[i_binary_n], 1024);
    }
    if (i == 62) {
      EXPECT_EQ(buffer->getMeasurementCounts()["111111"], 1024);
    }
    if (i == 63) {
      EXPECT_EQ(buffer->getMeasurementCounts()["111110"], 1024);
    }
  }
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}
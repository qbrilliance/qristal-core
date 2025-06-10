// Copyright (c) Quantum Brilliance Pty Ltd
#include <qristal/core/circuits/init_rep_flag.hpp>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <Circuit.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <bitset>


TEST(InitRepeatFlag_1, checkSimple) {
  std::cout << "InitRepeatFlagTester1:\n";
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  const std::vector<int> qubits_string = {0,1,2,3};
  const std::vector<int> qubits_init_repeat = {4,5};
  const std::vector<int> qubits_next_letter = {6,7};
  int max_key = 7;

  auto init_repeat_flag = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("InitRepeatFlag"));

  xacc::HeterogeneousMap map = {{"iteration",1},{"qubits_init_repeat", qubits_init_repeat}, {"qubits_string", qubits_string},
        {"qubits_next_letter", qubits_next_letter}};
  init_repeat_flag->expand(map);

  std::cout << "init_repeat_flag_test\n";
  auto init_flag_test = gateRegistry->createComposite("init_repeat_flag");

  for (int qindex = 0; qindex < std::size(qubits_string) ; qindex++){
      init_flag_test->addInstruction(gateRegistry->createInstruction("H", qubits_string[qindex]));
  }
  init_flag_test->addInstruction(gateRegistry->createInstruction("X", qubits_next_letter[0]));

  // Add init repeat flag
  init_flag_test->addInstructions(init_repeat_flag->getInstructions());

  // Measure evaluation qubits:
  for (int i = 0; i <= max_key; ++i) {
    init_flag_test->addInstruction(gateRegistry->createInstruction("Measure", i));
  }
  std::cout << "HOWDY: InitRepeatFlag circuit:\n";
  std::cout << init_flag_test->toString() << '\n';

  // Sim:
  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(max_key + 1);
  std::cout << "acc->execute()" << max_key << "\n" ;
  acc->execute(buffer, init_flag_test);
  buffer->print();
  int nq_next_letter = qubits_next_letter.size();
  int string_integer;
  std::cout << init_flag_test->toString() << '\n';
  for (int bits1 = 0; bits1 < 4; bits1++){
      for (int bits2 = 0; bits2 < 4; bits2++){
        string_integer = 32*bits1 + 8*bits2 + 1;
        if (bits1 == bits2){
            std::bitset<2>  bitstring(string_integer + 4);
            EXPECT_EQ(buffer->getMeasurementCounts()[bitstring.to_string()], 0);
        }
        else {
            std::bitset<2>  bitstring(string_integer);
            EXPECT_EQ(buffer->getMeasurementCounts()[bitstring.to_string()], 0);
        }
      }
  }
  EXPECT_EQ(buffer->getMeasurementCounts().size(), 16);
  buffer->print();
}


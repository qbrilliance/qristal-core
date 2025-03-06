// Copyright (c) Quantum Brilliance Pty Ltd
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "Circuit.hpp"
#include <bitset>
#include <gtest/gtest.h>

TEST(InverseCircuitCircuitTester, checksimple1) {
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("test_circ");

  int q0 = 0;
  int q1 = 1;
  int q2 = 2;
  std::vector<int> FA = {3, 4};
  std::vector<int> FB = {5, 6};
  std::vector<int> SA = {7, 8, 9, 10}; // Initialize SA = |0000>

  // Initialize FA and FB as |11>
  for (int i = 0; i < FA.size(); i++) {
    circuit->addInstruction(gateRegistry->createInstruction("X", FA[i]));
    circuit->addInstruction(gateRegistry->createInstruction("X", FB[i]));
  }

  // Beam checker
  auto beam_check = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("CompareBeamOracle"));
  const bool expand_ok = beam_check->expand(
      {{"q0", q0}, {"q1", q1}, {"q2", q2}, {"FA", FA}, {"FB", FB}, {"SA", SA}});
  EXPECT_TRUE(expand_ok);
  circuit->addInstructions(beam_check->getInstructions());

  // Undo
  auto undo = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("InverseCircuit"));
  const bool expand_ok2 = undo->expand({{"circ", beam_check}});
  EXPECT_TRUE(expand_ok2);
  circuit->addInstructions(undo->getInstructions());

  for (int i = 0; i < 11; i++) {
    circuit->addInstruction(gateRegistry->createInstruction("Measure", i));
  }

  auto acc = xacc::getAccelerator("qsim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(3 + FA.size() + FB.size() + SA.size());
  acc->execute(buffer, circuit);

  // buffer->print();
  EXPECT_EQ(buffer->getMeasurementCounts()["00011110000"], 1024);
}

TEST(InverseCircuitCircuitTester, checksimple2) {
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("test_circ");

  auto controlled_U = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("C-U"));
  auto x = gateRegistry->createComposite("x");
  x->addInstruction(gateRegistry->createInstruction("X", 0));
  std::vector<int> controls = {1, 2, 3};
  controlled_U->expand({{"U", x}, {"control-idx", controls}});
  circuit->addInstruction(controlled_U);

  // std::cout<<"circuit:\n"<<circuit->toString()<<"\n";

  // Undo
  auto circuit_inverse = gateRegistry->createComposite("test_circ");
  auto undo = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("InverseCircuit"));
  const bool expand_ok2 = undo->expand({{"circ", circuit}});
  EXPECT_TRUE(expand_ok2);
  circuit_inverse->addInstructions(undo->getInstructions());

  // std::cout<<"circuit_inverse:\n"<<circuit_inverse->toString()<<"\n";
}

TEST(InverseCircuitCircuitTester, checkiSwap) {
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  for (int i = 0; i < 4; i++) {
    auto circuit = gateRegistry->createComposite("test_circ");
    std::vector<std::complex<double>> inputvec;
    if (i == 0) {
      inputvec = {{1, 0}, {0, 0}, {0, 0}, {0, 0}};
    } else if (i == 1) {
      inputvec = {{0, 0}, {1, 0}, {0, 0}, {0, 0}};
      circuit->addInstruction(gateRegistry->createInstruction("X", 0));
    } else if (i == 2) {
      inputvec = {{0, 0}, {0, 0}, {1, 0}, {0, 0}};
      circuit->addInstruction(gateRegistry->createInstruction("X", 1));
    } else if (i == 3) {
      inputvec = {{0, 0}, {0, 0}, {0, 0}, {1, 0}};
      circuit->addInstruction(gateRegistry->createInstruction("X", 0));
      circuit->addInstruction(gateRegistry->createInstruction("X", 1));
    }

    circuit->addInstruction(gateRegistry->createInstruction("iSwap", {0, 1}));
    auto iswap = gateRegistry->createComposite("iswap");
    iswap->addInstruction(gateRegistry->createInstruction("iSwap", {0, 1}));

    // Undo
    auto undo = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("InverseCircuit"));
    const bool expand_ok2 = undo->expand({{"circ", iswap}});
    EXPECT_TRUE(expand_ok2);
    circuit->addInstructions(undo->getInstructions());

    auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
    auto buffer = xacc::qalloc(2);
    acc->execute(buffer, circuit);

    auto exeInfo = acc->getExecutionInfo();
    auto waveFn = acc->getExecutionInfo<xacc::ExecutionInfo::WaveFuncPtrType>(
        xacc::ExecutionInfo::WaveFuncKey);
    std::vector<std::complex<double>> waveFnvec;
    for (const auto &elem : *waveFn) {
      // std::cout << "elem " << elem << "\n";
      waveFnvec.emplace_back(elem);
    }
    //   for (int j = 0; j < 4; j++) {
    //       std::cout << "input " << inputvec[j] << "\n";
    //   }
    for (int j = 0; j < 4; j++) {
      assert(std::abs(inputvec[j] - waveFnvec[j]) < 1e-9);
    }
  }
}

TEST(InverseCircuitCircuitTester, checkfSim) {
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("test_circ");

  circuit->addInstruction(gateRegistry->createInstruction("H", 0));
  circuit->addInstruction(gateRegistry->createInstruction("H", 1));

  circuit->addInstruction(gateRegistry->createInstruction("fSim", {0, 1}));
  auto fSim = gateRegistry->createComposite("fSim");
  fSim->addInstruction(gateRegistry->createInstruction("fSim", {0, 1}));

  // Undo
  auto undo = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("InverseCircuit"));
  const bool expand_ok2 = undo->expand({{"circ", fSim}});
  EXPECT_TRUE(expand_ok2);
  circuit->addInstructions(undo->getInstructions());

  circuit->addInstruction(gateRegistry->createInstruction("H", 0));
  circuit->addInstruction(gateRegistry->createInstruction("H", 1));

  auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
  auto buffer = xacc::qalloc(2);
  acc->execute(buffer, circuit);

  auto exeInfo = acc->getExecutionInfo();
  auto waveFn = acc->getExecutionInfo<xacc::ExecutionInfo::WaveFuncPtrType>(
      xacc::ExecutionInfo::WaveFuncKey);
  std::vector<std::complex<double>> waveFnvec;
  for (const auto &elem : *waveFn) {
    // std::cout << "elem " << elem << "\n";
    waveFnvec.emplace_back(elem);
  }
  assert(std::abs(waveFnvec[0] - std::complex<double>{1, 0}) < 1e-9);
  for (int j = 1; j < 4; j++) {
    assert(std::abs(waveFnvec[j]) < 1e-9);
  }
}

TEST(InverseCircuitCircuitTester, checkU) {
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  const auto theta =
      xacc::linspace(-xacc::constants::pi, xacc::constants::pi, 10);
  const auto phi =
      xacc::linspace(-xacc::constants::pi, xacc::constants::pi, 10);
  const auto lambda =
      xacc::linspace(-xacc::constants::pi, xacc::constants::pi, 10);

  for (size_t t = 0; t < theta.size(); ++t) {
    for (size_t p = 0; p < phi.size(); ++p) {
      for (size_t l = 0; l < lambda.size(); ++l) {
        auto circuit = gateRegistry->createComposite("test_circ");
        circuit->addInstruction(gateRegistry->createInstruction("H", 0));

        circuit->addInstruction(gateRegistry->createInstruction(
            "U", {0}, {theta[t], phi[p], lambda[l]}));
        auto U = gateRegistry->createComposite("U");
        U->addInstruction(gateRegistry->createInstruction(
            "U", {0}, {theta[t], phi[p], lambda[l]}));

        // Undo
        auto undo = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("InverseCircuit"));
        const bool expand_ok2 = undo->expand({{"circ", U}});
        EXPECT_TRUE(expand_ok2);
        circuit->addInstructions(undo->getInstructions());

        circuit->addInstruction(gateRegistry->createInstruction("H", 0));

        //std::cout << "circ:\n" << circuit->toString() << "\n";

        auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}});
        auto buffer = xacc::qalloc(2);
        acc->execute(buffer, circuit);

        auto exeInfo = acc->getExecutionInfo();
        auto waveFn =
            acc->getExecutionInfo<xacc::ExecutionInfo::WaveFuncPtrType>(
                xacc::ExecutionInfo::WaveFuncKey);
        std::vector<std::complex<double>> waveFnvec;
        for (const auto &elem : *waveFn) {
          //std::cout << "elem " << elem << "\n";
          waveFnvec.emplace_back(elem);
        }
        assert(std::abs(waveFnvec[0] - std::complex<double>{1, 0}) < 1e-9);
        for (int j = 1; j < 4; j++) {
          assert(std::abs(waveFnvec[j]) < 1e-9);
        }
      }
    }
  }
}


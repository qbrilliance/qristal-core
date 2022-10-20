#include "Circuit.hpp"
#include "qbos_circuit_builder.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>
#include <memory>
////////////////////////
// Other include statements
////////////////////////

TEST(TestCase, checksimple) {

  //////////////////////////////////////
  // Define circuit
  //////////////////////////////////////

  // Define the circuit we want to run
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto circuit = gateRegistry->createComposite("circuit");

  auto cswap = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("ControlledSwap"));
  cswap->expand({{"qubits_a", std::vector<int>{0}},
                 {"qubits_b", std::vector<int>{1}},
                 {"flags_on", std::vector<int>{2}}});

  auto controlled_U = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("C-U"));
  controlled_U->expand({{"U", cswap}, {"control-idx", std::vector<int>{3}}});

  std::cout << "controlled swap:\n" << cswap->toString() << "\n";
  std::cout << "C-cswap:\n" << controlled_U->toString() << "\n";
  
  // Create the state |10> that we will perform the conditional swap operation on |10>->|01>
  circuit->addInstruction(gateRegistry->createInstruction("X", 0));
  
  // Turn on both control bits (q2 and q3) so the whole state is |1011>
  circuit->addInstruction(gateRegistry->createInstruction("X", 2));
  //circuit->addInstructions(cswap->getInstructions());
  circuit->addInstruction(gateRegistry->createInstruction("X", 3));

  // Conditional on q2 and q3, swap q0 and q1
  // Expected output is |0111>
  circuit->addInstruction(controlled_U);

  for (int bit = 0; bit < 4; bit++) {
      circuit->addInstruction(gateRegistry->createInstruction("Measure", bit));
  }


  auto accelerator = xacc::getAccelerator("sparse-sim", {{"shots", 1024}});
  auto buffer = xacc::qalloc(4);
  accelerator->execute(buffer, circuit);
  buffer->print();
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}
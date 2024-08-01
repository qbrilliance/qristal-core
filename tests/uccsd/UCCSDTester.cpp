#include <gtest/gtest.h>
#include "Instruction.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include "qristal/core/uccsd/fermionic_excitation_generator.hpp"
using namespace xacc;

TEST(UCCSDTester, generateFermionicExcitations) {
  {
    auto test = generate_fermionic_excitations(1, 6, std::make_pair(1, 1));
    EXPECT_EQ(excitationsToString(test),
              "[((0,), (1,)), ((0,), (2,)), ((3,), (4,)), ((3,), (5,))]");
  }
  {
    auto test = generate_fermionic_excitations(1, 6, std::make_pair(1, 1), true,
                                               true, true);
    std::cout << excitationsToString(test) << "\n";
    EXPECT_EQ(excitationsToString(test),
              "[((0,), (1,)), ((0,), (2,)), ((1,), (2,)), ((3,), (4,)), ((3,), "
              "(5,)), ((4,), (5,))]");
  }
  {
    auto test = generate_fermionic_excitations(1, 8, std::make_pair(2, 2));
    std::cout << test.size() << "\n";
    EXPECT_EQ(excitationsToString(test),
              "[((0,), (2,)), ((0,), (3,)), ((1,), (2,)), ((1,), (3,)), ((4,), "
              "(6,)), ((4,), (7,)), ((5,), (6,)), ((5,), (7,))]");
  }
  {
    auto test = generate_fermionic_excitations(2, 8, std::make_pair(2, 2));
    std::cout << test.size() << "\n";
    EXPECT_EQ(
        excitationsToString(test),
        "[((0, 1), (2, 3)), ((0, 4), (2, 6)), ((0, 4), (2, 7)), ((0, 5), (2, "
        "6)), ((0, 5), (2, 7)), ((0, 4), (3, 6)), ((0, 4), (3, 7)), ((0, 5), "
        "(3, 6)), ((0, 5), (3, 7)), ((1, 4), (2, 6)), ((1, 4), (2, 7)), ((1, "
        "5), (2, 6)), ((1, 5), (2, 7)), ((1, 4), (3, 6)), ((1, 4), (3, 7)), "
        "((1, 5), (3, 6)), ((1, 5), (3, 7)), ((4, 5), (6, 7))]");
  }
}

TEST(UCCSDTester, checkUCCSDH4) {
  auto tmp = xacc::getService<Instruction>("UCCSD");
  auto uccsd = std::dynamic_pointer_cast<CompositeInstruction>(tmp);
  EXPECT_TRUE(uccsd->expand({{"ne", 4}, {"nq", 8}}));
  //   std::cout << uccsd->toString() << "\n";
  std::cout << "Number variables = " << uccsd->nVariables() << "\n";
  EXPECT_EQ(uccsd->nVariables(), 26);
}


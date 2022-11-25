// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <bitset>
#include <gtest/gtest.h>
#include <heterogeneous.hpp>
#include <numeric>
#include <string>
#include <vector>

TEST(CanonicalExponentialSearchAlgorithmAltTester, checkSimplefunc) {
  // Testing a simple maximum search using quantum exponential search
  const std::vector<int> dataset{1, 2, 0, 3, 0, 0, 1, 1,
                                 2, 1, 0, 1, 1, 2, 2, 1};
  // Set inputs
  const int BestScore = 0;
  // register
  std::vector<int> trial_score_qubits = {0,1};
  std::vector<int> trial_qubits = {2,3,4,5};
  int flag_qubit = 6;
  std::vector<int> best_score_qubits = {7,8,9,10};
  std::vector<int> ancilla_qubits = {11,12};

  // oracle
  std::function<std::shared_ptr<xacc::CompositeInstruction>(int)> oracle_ =
      [&](int BestScore) {
        auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
        int qubit_flag = ancilla_qubits[0];
        int c_in = ancilla_qubits[1];
        int n = best_score_qubits.size();

        // Initialize comparator oracle circuit
        auto oracle = gateRegistry->createComposite("oracle");

        // Encode BestScore as a bitstring
        std::string BestScore_binary = std::bitset<sizeof(BestScore)>(BestScore).to_string();
        std::string BestScore_binary_n = BestScore_binary.substr(
            BestScore_binary.size() < n ? 0 : BestScore_binary.size() - n);

        // Prepare |BestScore>
        for (int i = 0; i < n; i++) {
          if (BestScore_binary_n[i] == '1') {
            oracle->addInstruction(gateRegistry->createInstruction("X", best_score_qubits[i]));
          }
        }
        // Phase kickback method
        oracle->addInstruction(gateRegistry->createInstruction("X", qubit_flag));
        oracle->addInstruction(gateRegistry->createInstruction("H", qubit_flag));

        auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("CompareGT"));
        xacc::HeterogeneousMap options{{"qubits_a", trial_qubits},
                                       {"qubits_b", best_score_qubits},
                                       {"qubit_flag", flag_qubit},
                                       {"qubit_ancilla", c_in},
                                       {"is_LSB", true}};
        const bool expand_ok = comp->expand(options);
        assert(expand_ok);
        oracle->addInstruction(comp);
        oracle->addInstruction(gateRegistry->createInstruction("H", qubit_flag));
        oracle->addInstruction(gateRegistry->createInstruction("X", qubit_flag));
        return oracle;
      };

  // state prep
  std::function<std::shared_ptr<xacc::CompositeInstruction>(
      std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>)>
      state_prep_ = [&](std::vector<int> trial_qubits,
                        std::vector<int> trial_score_qubits,
                        std::vector<int> a,
                        std::vector<int> b,
                        std::vector<int> c) {
        const std::vector<int> dataset{1, 2, 0, 3, 0, 0, 1, 1,
                                       2, 1, 0, 1, 1, 2, 2, 1};
        const int num_scoring_qubits = 2;
        const int num_string_qubits = 4;
        auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
        auto state_prep = gateRegistry->createComposite("state_prep");
        for (int q = 0; q < num_string_qubits; ++q) {
          state_prep->addInstruction(gateRegistry->createInstruction("H", trial_qubits[q]));
        }
        for (int q = 0; q < dataset.size(); q++) {
          std::string string_binary = std::bitset<sizeof(num_string_qubits)>(q).to_string();
          std::string string = string_binary.substr(
              string_binary.size() < num_string_qubits ? 0 : string_binary.size() - num_string_qubits);
          std::string score_binary = std::bitset<sizeof(num_scoring_qubits)>(dataset[q]).to_string();
          std::string score_string = score_binary.substr(
              score_binary.size() < num_scoring_qubits ? 0 : score_binary.size() - num_scoring_qubits);

          for (int bit = 0; bit < num_string_qubits; bit++) {
            if (string[bit] == '0') {
              state_prep->addInstruction(gateRegistry->createInstruction("X", trial_qubits[bit]));
            }
          }

          for (int bit = 0; bit < num_scoring_qubits; bit++) {
            if (score_string[bit] == '1') {
              std::vector<int> control_bits = trial_qubits;
              auto x_gate = gateRegistry->createComposite("x_gate");
              auto temp_gate = gateRegistry->createInstruction("X", trial_score_qubits[bit]);
              temp_gate->setBufferNames({"q"});
              x_gate->addInstruction(temp_gate);
              auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
                  xacc::getService<xacc::Instruction>("C-U"));
              mcx->expand({{"U", x_gate}, {"control-idx", control_bits}});
              state_prep->addInstruction(mcx);
            }
          }

        for (int bit = 0; bit < (int)trial_qubits.size(); bit++) {
            if (string[bit] == '0') {
              state_prep->addInstruction(gateRegistry->createInstruction("X", trial_qubits[bit]));
            }
          }
        }
        return state_prep;
      };
      auto state_prep_circ = state_prep_(trial_qubits, trial_score_qubits, {}, {}, {});

  // scoring
  std::function<int(int)> f_score = [&](int score) { return score; };

  int total_num_qubits = 1 + trial_qubits.size() + trial_score_qubits.size()
                        + best_score_qubits.size() + ancilla_qubits.size();

  // Validate the success probs:
  constexpr int N_TRIALS = 1;
  int nSuccess = 0;
  auto acc = xacc::getAccelerator("qsim", {{"shots", 1}});
  for (int runCount = 0; runCount < N_TRIALS; ++runCount) {
    auto exp_search_algo = xacc::getAlgorithm(
      "exponential-search", {{"method", "canonical"},
                             {"state_preparation_circuit", state_prep_circ},
                             {"oracle_circuit", oracle_},
                             {"best_score", BestScore},
                             {"f_score", f_score},
                             {"total_num_qubits", total_num_qubits},
                             {"qubits_string", trial_qubits},
                             {"total_metric", trial_score_qubits},
                             {"qpu", acc}});
    auto buffer = xacc::qalloc(total_num_qubits);
    exp_search_algo->execute(buffer);
    auto info = buffer->getInformation();
    if (info.find("best-score") != info.end()) {
      ++nSuccess;
    }
  }
  std::cout << "Total success: " << nSuccess << "\n";
  // Pm >= 1/4 (https://arxiv.org/pdf/quant-ph/9605034.pdf)
  EXPECT_GT(nSuccess, 0.25);
}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

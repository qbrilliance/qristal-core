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

TEST(CanonicalExponentialSearchAlgorithmTester, checkSimplefunc) {
  // Testing a simple maximum search using quantum exponential search
  const std::vector<int> dataset{1, 2, 0, 3, 0, 0, 1, 1,
                                 2, 1, 0, 1, 1, 2, 2, 1};
  // Set inputs
  const int BestScore = 0;
  // register
  std::vector<int> trial_score_qubits = {0,1};
  std::vector<int> trial_qubits = {2,3,4,5};
  std::vector<int> next_letter = {};
  std::vector<int> next_score = {};
  int flag_qubit = 6;
  std::vector<int> best_score_qubits = {7, 8};
  std::vector<int> ancilla_qubits = {9, 10, 11, 12, 13};

  // oracle
  std::function<std::shared_ptr<xacc::CompositeInstruction>(
      int, int, std::vector<int>, int, std::vector<int>, std::vector<int>)>
      oracle_ = [&](int BestScore, int num_scoring_qubits,
                    std::vector<int> trial_score_qubits, int flag_qubit,
                    std::vector<int> best_score_qubits,
                    std::vector<int> ancilla_qubits) {
        auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
        auto oracle = gateRegistry->createComposite("oracle");
        oracle->addInstruction(
            gateRegistry->createInstruction("X", flag_qubit));
        oracle->addInstruction(
            gateRegistry->createInstruction("H", flag_qubit));
        auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("Comparator"));
        xacc::HeterogeneousMap options{
            {"BestScore", BestScore},
            {"num_scoring_qubits", num_scoring_qubits},
            {"trial_score_qubits", trial_score_qubits},
            {"flag_qubit", flag_qubit},
            {"best_score_qubits", best_score_qubits},
            {"ancilla_qubits", ancilla_qubits},
            {"as_oracle", true},
            {"is_LSB", false}};
        const bool expand_ok = comp->expand(options);
        assert(expand_ok);
        oracle->addInstructions(comp->getInstructions());
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
          state_prep->addInstruction(
              gateRegistry->createInstruction("H", trial_qubits[q]));
        }
        for (int q = 0; q < dataset.size(); q++) {
          std::string string_binary =
              std::bitset<sizeof(num_string_qubits)>(q).to_string();
          std::string string = string_binary.substr(
              string_binary.size() < num_string_qubits
                  ? 0
                  : string_binary.size() - num_string_qubits);
          std::string score_binary =
              std::bitset<sizeof(num_scoring_qubits)>(dataset[q]).to_string();
          std::string score_string = score_binary.substr(
              score_binary.size() < num_scoring_qubits
                  ? 0
                  : score_binary.size() - num_scoring_qubits);

          for (int bit = 0; bit < num_string_qubits; bit++) {
            if (string[bit] == '0') {
              state_prep->addInstruction(
                  gateRegistry->createInstruction("X", trial_qubits[bit]));
            }
          }

          for (int bit = 0; bit < num_scoring_qubits; bit++) {
            if (score_string[bit] == '1') {
              std::vector<int> control_bits = trial_qubits;
              auto x_gate = gateRegistry->createComposite("x_gate");
              auto temp_gate =
                  gateRegistry->createInstruction("X", trial_score_qubits[bit]);
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
              state_prep->addInstruction(
                  gateRegistry->createInstruction("X", trial_qubits[bit]));
            }
          }
        }
        return state_prep;
      };

  // scoring
  std::function<int(int)> f_score = [&](int score) { return score; };

  // Validate the success probs:
  constexpr int N_TRIALS = 1;
  int nSuccess = 0;
  auto acc = xacc::getAccelerator("qsim", {{"shots", 1}});
  for (int runCount = 0; runCount < N_TRIALS; ++runCount) {
    auto exp_search_algo = xacc::getAlgorithm(
        "exponential-search", {{"method", "canonical"},
                               {"state_preparation_circuit", state_prep_},
                               {"oracle_circuit", oracle_},
                               {"best_score", BestScore},
                               {"f_score", f_score},
                               {"qubit_flag", flag_qubit},
                               {"qubits_metric", trial_score_qubits},
                               {"qubits_string", trial_qubits},
                               {"qubits_next_letter", next_letter},
                               {"qubits_next_metric", next_score},
                               {"qubits_best_score", best_score_qubits},
                               {"qubits_ancilla_oracle", ancilla_qubits},
                               {"qpu", acc}});

    auto buffer = xacc::qalloc(14);
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

// TEST(ExponentialSearchCanonicalQAEAlgorithmTester, checkSimplefunc) {
//   // Testing a simple maximum search using quantum exponential search
//   const std::vector<int> dataset{1,2,0,3,0,0,1,1,2,1,0,1,1,2,2,1};
//   // Set inputs
//   const int BestScore = 0;
//   int num_evaluation_qubits = 10;
//   // register
//   std::vector<int> trial_score_qubits = {0,1};
//   std::vector<int> trial_qubits = {2,3,4,5};
//   int flag_qubit = 6;
//   std::vector<int> best_score_qubits = {7,8};
//   std::vector<int> ancilla_qubits = {9,10,11,12};
//     std::vector<int> next_letter = {};
//   std::vector<int> next_score = {};

//   // oracle
//   std::function<std::shared_ptr<xacc::CompositeInstruction>(int,int,std::vector<int>,int,std::vector<int>,std::vector<int>)> oracle_ = [&](int BestScore, int num_scoring_qubits, std::vector<int> trial_score_qubits, int flag_qubit, std::vector<int> best_score_qubits, std::vector<int> ancilla_qubits) {
//     auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("Comparator"));
//     const bool expand_ok = comp->expand(
//       {{"BestScore", BestScore}, {"num_scoring_qubits", num_scoring_qubits},
//       {"trial_score_qubits", trial_score_qubits}, {"flag_qubit", flag_qubit},
//       {"best_score_qubits", best_score_qubits},
//       {"ancilla_qubits",ancilla_qubits}, {"as_oracle", true}, {"is_LSB",
//       false}});
//     return comp;
//   };

//   // state prep
//   std::function<std::shared_ptr<xacc::CompositeInstruction>(int, std::vector<int>, std::vector<int>, std::vector<int>)> state_prep_ = [&](int num_string_qubits, std::vector<int> trial_score_qubits, std::vector<int> trial_qubits, std::vector<int> trial_ancilla) {
//     const std::vector<int> dataset{1,2,0,3,0,0,1,1,2,1,0,1,1,2,2,1};
//     const int num_scoring_qubits = 2;
//     auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
//     auto state_prep = gateRegistry->createComposite("state_prep");
//     for (int q = 0; q < num_string_qubits; ++q) {
//       state_prep->addInstruction(gateRegistry->createInstruction("H", trial_qubits[q]));
//     }
//     for (int q = 0; q < dataset.size(); q++) {
//       std::string string_binary = std::bitset<sizeof(num_string_qubits)>(q).to_string();
//       std::string string = string_binary.substr(string_binary.size() < num_string_qubits ? 0 : string_binary.size() - num_string_qubits);
//       std::string score_binary = std::bitset<sizeof(num_scoring_qubits)>(dataset[q]).to_string();
//       std::string score_string = score_binary.substr(score_binary.size() < num_scoring_qubits ? 0 : score_binary.size() - num_scoring_qubits);

//         for (int bit = 0; bit < num_string_qubits; bit++) {
//             if (string[bit] == '0') {
//                 state_prep->addInstruction(gateRegistry->createInstruction("X", trial_qubits[bit]));
//             }
//         }

//         for (int bit = 0; bit < num_scoring_qubits; bit++) {
//             if (score_string[bit] == '1') {
//                 std::vector<int> control_bits = trial_qubits;
//                 auto x_gate = gateRegistry->createComposite("x_gate");
//                 auto temp_gate = gateRegistry->createInstruction("X", trial_score_qubits[bit]);
//                 temp_gate->setBufferNames({"q"});
//                 x_gate->addInstruction(temp_gate);
//                 auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//                   xacc::getService<xacc::Instruction>("C-U"));
//                 mcx->expand({{"U", x_gate}, {"control-idx", control_bits}});
//                 state_prep->addInstructions(mcx->getInstructions());
//             }
//         }

//         for (int bit = 0; bit < num_string_qubits; bit++) {
//             if (string[bit] == '0') {
//                 state_prep->addInstruction(gateRegistry->createInstruction("X", trial_qubits[bit]));
//             }
//         }

//     }
//     return state_prep;
//   };

//   // scoring
//   std::function<int(int)> f_score = [&](int score) { return score; };

//   // Validate the success probs:
//   constexpr int N_TRIALS = 10;
//   int nSuccess = 0;
//   for (int runCount = 0; runCount < N_TRIALS; ++runCount) {
//     auto exp_search_algo = xacc::getAlgorithm(
//         "exponential-search", {{"method", "CQAE"},
//                                {"state_preparation_circuit", state_prep_},
//                                {"oracle_circuit", oracle_},
//                                {"best_score_", BestScore},
//                                {"f_score", f_score},
//                                {"qubit_flag", flag_qubit},
//                                {"qubits_metric", trial_score_qubits},
//                                {"qubits_string", trial_qubits},
//                                {"qubits_next_letter", next_letter},
//                                {"qubits_next_metric", next_score},
//                                {"qubits_best_score", best_score_qubits},
//                                {"qubits_ancilla_oracle", ancilla_qubits},
//                                {"CQAE_num_evaluation_qubits", num_evaluation_qubits}});

//     auto buffer = xacc::qalloc(23);
//     exp_search_algo->execute(buffer);
//     auto info = buffer->getInformation();
//     if (info.find("best-score") != info.end()) {
//       ++nSuccess;
//     }
//   }
//   std::cout << "Total success: " << nSuccess << "\n";
//   // Pm >= 1/4
//   EXPECT_GT(nSuccess, 2.5);
// }

// TEST(ExponentialSearchMLQAEAlgorithmTester, checkSimplefunc) {
//   // Testing a simple maximum search using quantum exponential search
//   const std::vector<int> dataset{1,2,0,3,0,0,1,1,2,1,0,1,1,2,2,1};
//   // Set inputs
//   const int BestScore = 0;
//   int num_runs = 4;
//   int shots = 100;
//   // register
//   std::vector<int> trial_score_qubits = {0,1};
//   std::vector<int> trial_qubits = {2,3,4,5};
//   int flag_qubit = 6;
//   std::vector<int> best_score_qubits = {7,8};
//   std::vector<int> ancilla_qubits = {9,10,11,12};

//   // oracle
//   std::function<std::shared_ptr<xacc::CompositeInstruction>(int,int,std::vector<int>,int,std::vector<int>,std::vector<int>)> oracle_ = [&](int BestScore, int num_scoring_qubits, std::vector<int> trial_score_qubits, int flag_qubit, std::vector<int> best_score_qubits, std::vector<int> ancilla_qubits) {
//     auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//       xacc::getService<xacc::Instruction>("Comparator"));
//     const bool expand_ok = comp->expand(
//       {{"BestScore", BestScore}, {"num_scoring_qubits", num_scoring_qubits},
//       {"trial_score_qubits", trial_score_qubits}, {"flag_qubit", flag_qubit},
//       {"best_score_qubits", best_score_qubits},
//       {"ancilla_qubits",ancilla_qubits}, {"as_oracle", true}, {"is_LSB",
//       false}});
//     return comp;
//   };

//   // state prep
//   std::function<std::shared_ptr<xacc::CompositeInstruction>(int, std::vector<int>, std::vector<int>, std::vector<int>)> state_prep_ = [&](int num_string_qubits, std::vector<int> trial_score_qubits, std::vector<int> trial_qubits, std::vector<int> trial_ancilla) {
//     const std::vector<int> dataset{1,2,0,3,0,0,1,1,2,1,0,1,1,2,2,1};
//     const int num_scoring_qubits = 2;
//     auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
//     auto state_prep = gateRegistry->createComposite("state_prep");
//     for (int q = 0; q < num_string_qubits; ++q) {
//       state_prep->addInstruction(gateRegistry->createInstruction("H", trial_qubits[q]));
//     }
//     for (int q = 0; q < dataset.size(); q++) {
//       std::string string_binary = std::bitset<sizeof(num_string_qubits)>(q).to_string();
//       std::string string = string_binary.substr(string_binary.size() < num_string_qubits ? 0 : string_binary.size() - num_string_qubits);
//       std::string score_binary = std::bitset<sizeof(num_scoring_qubits)>(dataset[q]).to_string();
//       std::string score_string = score_binary.substr(score_binary.size() < num_scoring_qubits ? 0 : score_binary.size() - num_scoring_qubits);

//         for (int bit = 0; bit < num_string_qubits; bit++) {
//             if (string[bit] == '0') {
//                 state_prep->addInstruction(gateRegistry->createInstruction("X", trial_qubits[bit]));
//             }
//         }

//         for (int bit = 0; bit < num_scoring_qubits; bit++) {
//             if (score_string[bit] == '1') {
//                 std::vector<int> control_bits = trial_qubits;
//                 auto x_gate = gateRegistry->createComposite("x_gate");
//                 auto temp_gate = gateRegistry->createInstruction("X", trial_score_qubits[bit]);
//                 temp_gate->setBufferNames({"q"});
//                 x_gate->addInstruction(temp_gate);
//                 auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
//                   xacc::getService<xacc::Instruction>("C-U"));
//                 mcx->expand({{"U", x_gate}, {"control-idx", control_bits}});
//                 state_prep->addInstructions(mcx->getInstructions());
//             }
//         }

//         for (int bit = 0; bit < num_string_qubits; bit++) {
//             if (string[bit] == '0') {
//                 state_prep->addInstruction(gateRegistry->createInstruction("X", trial_qubits[bit]));
//             }
//         }

//     }
//     return state_prep;
//   };

//   // scoring
//   std::function<int(int)> f_score = [&](int score) { return score; };

//   // is in good subspace
//   std::function<int(std::string,int)> MLQAE_is_in_good_subspace = [&] (std::string str, int bestscore) {
//       int val = 0;
//       for (int i = 0 ; i < str.length() ; i++) {
//           auto s = s[i];
//           if (s == '0') {int temp = 0;}
//           if (s == '1') {int temp = 1;}
//           val = val + (temp * std::pow(2,str.length()-1-i));
//       if (val > bestscore) {return 1;}
//       else {return 0;}
//   // Validate the success probs:
//   constexpr int N_TRIALS = 1;
//   int nSuccess = 0;
//   for (int runCount = 0; runCount < N_TRIALS; ++runCount) {
//       std::cout << "run starting!" << "\n";
//     auto exp_search_algo = xacc::getAlgorithm(
//         "exponential-search", {{"method", "MLQAE"},
//                                {"state_preparation_circuit", state_prep_},
//                                {"oracle_circuit", oracle_},
//                                {"best_score_", BestScore},
//                                {"f_score", f_score},
//                                {"qubit_flag", flag_qubit},
//                                {"qubits_metric", trial_score_qubits},
//                                {"qubits_string", trial_qubits},
//                                {"qubits_next_letter", {}},
//                                {"qubits_next_metric", {}},
//                                {"qubits_best_score", best_score_qubits},
//                                {"qubits_ancilla_oracle", ancilla_qubits},
//                                {"MLQAE_num_runs", num_runs},
//                                {"MLQAE_num_shots", shots},
//                                {"MLQAE_is_in_good_subspace", MLQAE_is_in_good_subspace}});

//     auto buffer = xacc::qalloc(13);
//     exp_search_algo->execute(buffer);
//     auto info = buffer->getInformation();
//     if (info.find("best-score") != info.end()) {
//       ++nSuccess;
//     }
//   }
//   std::cout << "Total success: " << nSuccess << "\n";
//   // Pm >= 1/4
//   EXPECT_GT(nSuccess, 0.25);
// }


int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

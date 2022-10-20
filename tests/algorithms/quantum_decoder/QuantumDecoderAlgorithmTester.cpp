// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <gtest/gtest.h>
#include <numeric>

TEST(QuantumDecoderCanonicalAlgorithmTester, checkSimple) {
  //Initial state parameters:
  std::vector<std::string> alphabet = {"-","a"};

  //Rows represent time step, while columns represent alphabets.
  //For each row, the sum of probabilities across the columns must add up to 1.
  std::vector<std::vector<float>> probability_table{{1.0, 0.0}, {1.0, 0.0}};

  int L = probability_table.size(); // string length = number of rows of probability_table (number of columns is probability_table[0].size())
  int S = 1; // number of qubits per letter, ceiling(log2(|Sigma|))
  int m = 2;//2; // metric precision
  int k = std::round(0.5 + std::log2(1 + L*(std::pow(2,m) - 1))); // number of qubits in total_metric
  int k2 = k*(k+1)/2; // number of precision qubits needed for ae
  int k3 = k + L; // number of qubits needed for qubits_beam_metric

  //First, the qubits that aren't ancilla / can't be re-used:
  std::vector<int> qubits_metric;
  for (int i = 0; i < L*m; i++) {qubits_metric.emplace_back(i);} // L*m
  std::vector<int> qubits_string;
  for (int i = L*m; i < L*m + L*S; i++) {qubits_string.emplace_back(i);} // L*S
  std::vector<int> qubits_init_null;
  for (int i = L*m+L*S; i < L*(m+S+1); i++) {qubits_init_null.emplace_back(i);} // L
  std::vector<int> qubits_init_repeat;
  for (int i = L*(m+S+1); i < L*(m+S+2); i++) {qubits_init_repeat.emplace_back(i);} // L
  std::vector<int> qubits_superfluous_flags; 
  for (int i = L*(m+S+2); i < L*(m+S+3); i++) {qubits_superfluous_flags.emplace_back(i);} // L

  std::vector<int> qubits_total_metric_copy;
  for (int i = L*(m+2*S+6); i < L*(m+2*S+6) + k; i++) {qubits_total_metric_copy.emplace_back(i);} // k

  std::vector<int> qubits_ancilla_adder;
  for (int i = L*(m+2*S+6)+k; i < L*(m+2*S+6)+2*k-m; i++) {qubits_ancilla_adder.emplace_back(i);} //We need total_metric to have k = log2(1 + L*(2**m - 1)) qubits, so qubits_ancilla_adder.size() = k - m
  std::vector<int> qubits_beam_metric;
  for (int i = L*(m+2*S+6)+2*k-m; i < L*(m+2*S+6)+3*k-m+L; i++) {qubits_beam_metric.emplace_back(i);} // k + L 
  std::vector<int> qubits_best_score;
  for (int i = L*(m+2*S+6)+3*k-m+L; i < L*(m+2*S+6)+4*k-m+2*L; i++) {qubits_best_score.emplace_back(i);} //Same size as qubits_beam_metric

  std::vector<int> precision_bits; 
  for (int i = 1; i <= k; i++) {precision_bits.emplace_back(i);} //THESE ARE NOT QUBITS! {2^0, 2^1, 2^2, ...}
  std::vector<int> evaluation_qubits;
   for (int i = L*(m+2*S+6)+4*k-m+2*L; i < L*(m+2*S+6)+4*k-m+2*L+k2; i++) {evaluation_qubits.emplace_back(i);} //k2

  //The remaining qubits can be drawn from an 'ancilla pool'. The size of this ancilla
  //pool needs to be the maximum number of ancilla required at any one time.
  int last_qubit_non_ancilla = evaluation_qubits[evaluation_qubits.size() - 1] + 1;//qubits_best_score[qubits_best_score.size() - 1] + 1;
  std::vector<int> qubits_ancilla_pool;
  int num_qubits_ancilla_pool = std::max({S+m, k-m, 3, 3*k3});
  for (int i = 0; i < num_qubits_ancilla_pool; i++) {
	qubits_ancilla_pool.push_back(last_qubit_non_ancilla + i);
  }
  std::cout<<"num qubits = " << L*(m+3*S+6) + 4*k - m + 2*L + k2 + num_qubits_ancilla_pool  << "\n";

  const int BestScore = 0; //Initial best score
  int N_TRIALS = 1; //Number of decoder iterations

//  const int iteration = probability_table[0].size(); //Number of columns of probability_table
  const int iteration = probability_table.size(); //Number of rows of probability_table
  const int num_next_letter_qubits = S;
  const int num_next_metric_qubits = m;

  ////////////////////////////////////////////////////////////////////////////////////////

  //Exponential search parameters
  //Set inputs
  const int num_scoring_qubits = qubits_metric.size();
  auto acc = xacc::getAccelerator("sparse-sim", {{"shots", 1}});
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  auto quantum_decoder_algo = xacc::getAlgorithm(
    "quantum-decoder", {{"iteration", iteration},
                        {"probability_table", probability_table},
                        {"qubits_metric", qubits_metric},
                        {"qubits_string", qubits_string},
                        {"method", "canonical"},
                        {"BestScore", BestScore},
                        {"N_TRIALS", N_TRIALS},
                        {"qubits_ancilla_adder", qubits_ancilla_adder},
                        {"qubits_init_null", qubits_init_null},
                        {"qubits_init_repeat", qubits_init_repeat},
                        {"qubits_superfluous_flags", qubits_superfluous_flags},
                        {"qubits_beam_metric", qubits_beam_metric},
                        {"qubits_ancilla_pool", qubits_ancilla_pool},
                        {"qubits_best_score", qubits_best_score},
                        {"qubits_total_metric_copy", qubits_total_metric_copy},
                        {"evaluation_bits", evaluation_qubits},
                        {"precision_bits", precision_bits},
                        {"qpu", acc}});

  auto buffer = xacc::qalloc(1 + (int)qubits_string.size() + (int)qubits_total_metric_copy.size() + (int)qubits_metric.size()
                               + (int)qubits_best_score.size() + (int)qubits_ancilla_adder.size()
                               + (int)qubits_init_null.size() + (int)qubits_init_repeat.size()
                               + (int)qubits_superfluous_flags.size() + qubits_beam_metric.size()
                               + (int)qubits_ancilla_pool.size() +(int)evaluation_qubits.size());
  quantum_decoder_algo->execute(buffer);
  auto info = buffer->getInformation();
//  buffer->print();
//  EXPECT_GT(BestScore, 0);

}

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}


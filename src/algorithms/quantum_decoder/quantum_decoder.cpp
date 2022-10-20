// Copyright (c) 2022 Quantum Brilliance Pty Ltd

#include "qb/core/algorithms/quantum_decoder/quantum_decoder.hpp"

#include "Algorithm.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"

#include <assert.h>
#include <bitset>
#include <iomanip>
#include <memory>

namespace qbOS {
bool QuantumDecoder::initialize(const xacc::HeterogeneousMap &parameters) {

  // W prime unitary parameters
  iteration = parameters.get<int>("iteration");

  probability_table = {};
  if (parameters.keyExists<std::vector<std::vector<float>>>(
          "probability_table")) {
    probability_table =
        parameters.get<std::vector<std::vector<float>>>("probability_table");
  }

  //////////////////////////////////////////////////////////////////////////////////////

  //U prime unitary parameters
  qubits_metric = {};
  if (parameters.keyExists<std::vector<int>>("qubits_metric")) {
    qubits_metric = parameters.get<std::vector<int>>("qubits_metric");
  }

  qubits_string = {};
  if (parameters.keyExists<std::vector<int>>("qubits_string")) {
    qubits_string = parameters.get<std::vector<int>>("qubits_string");
  }

  //////////////////////////////////////////////////////////////////////////////////////

  //Parameters for comparator oracle in exponential search
  BestScore = parameters.get_or_default("BestScore", 0);

  qubits_best_score = {};
  if (parameters.keyExists<std::vector<int>>("qubits_best_score")) {
    qubits_best_score = parameters.get<std::vector<int>>("qubits_best_score");
  }

  //////////////////////////////////////////////////////////////////////////////////////

  //Parameters for adder
  qubits_ancilla_adder = {};
  if (parameters.keyExists<std::vector<int>>("qubits_ancilla_adder")) {
    qubits_ancilla_adder =
        parameters.get<std::vector<int>>("qubits_ancilla_adder");
  }

  //////////////////////////////////////////////////////////////////////////////////////

  //Parameters for exponential search
  N_TRIALS = parameters.get<int>("N_TRIALS");

  //////////////////////////////////////////////////////////////////////////////////////

  // Parameters for decoder kernel
  if (!parameters.keyExists<std::vector<int>>("qubits_init_null")) {
    return false;
  }
  qubits_init_null = parameters.get<std::vector<int>>("qubits_init_null");

  if (!parameters.keyExists<std::vector<int>>("qubits_init_repeat")) {
    return false;
  }
  qubits_init_repeat = parameters.get<std::vector<int>>("qubits_init_repeat");

  if (!parameters.keyExists<std::vector<int>>("evaluation_bits")) {
    return false;
  }
  evaluation_bits = parameters.get<std::vector<int>>("evaluation_bits");

  if (!parameters.keyExists<std::vector<int>>("precision_bits")) {
    return false;
  }
  precision_bits = parameters.get<std::vector<int>>("precision_bits");

  qubits_ancilla_pool = {};
  if (parameters.keyExists<std::vector<int>>("qubits_ancilla_pool"))
  {
    qubits_ancilla_pool = parameters.get<std::vector<int>>("qubits_ancilla_pool");
  }

  if (!parameters.keyExists<std::vector<int>>("qubits_superfluous_flags"))
  {
    return false;
  }
  qubits_superfluous_flags = parameters.get<std::vector<int>>("qubits_superfluous_flags");

  if (!parameters.keyExists<std::vector<int>>("qubits_total_metric_copy"))
  {
    return false;
  }
  qubits_total_metric_copy = parameters.get<std::vector<int>>("qubits_total_metric_copy");

  qubits_beam_metric = {};
  if (parameters.keyExists<std::vector<int>>("qubits_beam_metric"))
  {
    qubits_beam_metric = parameters.get<std::vector<int>>("qubits_beam_metric");
  }

  //////////////////////////////////////////////////////////////////////////////////////

  //Initialize qpu accelerator
  qpu_ = nullptr;
  if (parameters.stringExists("qpu")) {
    static auto acc =
        xacc::getAccelerator(parameters.getString("qpu"), {{"shots", 1}});
    qpu_ = acc.get();
  } else if (parameters.pointerLikeExists<xacc::Accelerator>("qpu")) {
    qpu_ = parameters.getPointerLike<xacc::Accelerator>("qpu");
  }

  if (!qpu_) {
    static auto qpp = xacc::getAccelerator("qpp", {{"shots", 1}});
    // Default to qpp if none provided
    qpu_ = qpp.get();
  }

  return true;
} //QuantumDecoder::initialize

/////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<std::string> QuantumDecoder::requiredParameters() const {
  return {"probability_table", "iteration", "qubits_metric", "qubits_string",
          "method", "BestScore", "qubits_beam_metric", "qubits_superfluous_flags",
          "num_scoring_qubits", "qubits_init_null", "qubits_init_repeat",
          "qubits_best_score", "qubits_ancilla_oracle", "N_TRIALS"};
}

/////////////////////////////////////////////////////////////////////////////////////////////

void QuantumDecoder::execute(
    const std::shared_ptr<xacc::AcceleratorBuffer> buffer) const {
  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  // qubits_next_letter and qubits_next_metric required at the same time
  std::vector<int> qubits_next_letter; // S
  std::vector<int> qubits_next_metric; // m
  int L = qubits_init_null.size();
  int S = qubits_string.size()/L;
  int m = qubits_metric.size()/L;
  int k = std::round(0.5 + std::log2(1 + L*(std::pow(2,m) - 1))); // number of qubits in total_metric
  int k2 = k*(k+1)/2; // number of precision qubits needed for ae
  int k3 = k + L; // number of qubits needed for qubits_beam_metric

  for (int i = 0; i < S; i++) {
  qubits_next_letter.push_back(qubits_ancilla_pool[i]);
  }
  for (int i = 0; i < m; i++) {
  qubits_next_metric.push_back(qubits_ancilla_pool[S+i]);
  }

  // qubit_flag and qubits_ancilla_oracle required at the same time
  int qubit_flag = qubits_ancilla_pool[0];
  std::vector<int> qubits_ancilla_oracle; // 3k2 - 1
  for (int i = 0; i < 3*k3-1; i++) {
  qubits_ancilla_oracle.push_back(qubits_ancilla_pool[1+i]);
  }

  // Take the logarithm of the probability table
//   std::vector<std::vector<float>> log_prob_table;
//   for (auto i : probability_table) {
//       std::vector<float> log_i;
//       for (auto j : i) {
//           float log_j = std::log2(j);
//           log_i.push_back(log_j);
//       }
//       log_prob_table.push_back(log_i);
//   }
//   probability_table = log_prob_table;

  //State preparation: Prepare initial state using unitaries for the exponential search.
  std::function<std::shared_ptr<xacc::CompositeInstruction>(
    std::vector<int>, std::vector<int>, std::vector<int>,
    std::vector<int>, std::vector<int>)>
    state_prep_ = [&](std::vector<int> qubits_string, std::vector<int> qubits_metric,
                      std::vector<int> qubits_next_letter, std::vector<int> qubits_next_metric ,
                      std::vector<int> qubits_ancilla_adder) {

    //Initialize state preparation circuit
    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    auto state_prep = gateRegistry->createComposite("state_prep");

    /////////////////////////////////////////////////////////////////////////////////////////////

    //Loop over rows of the probability table (i.e. over string length)
    for (int it = 0; it < iteration; it++) {
      //Initialize W prime unitary
      auto w_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("WPrime"));

      //Merge qubit register for W prime unitary into a heterogenous map
      xacc::HeterogeneousMap w_map = {{"iteration", it},
                                      {"qubits_next_letter", qubits_next_letter},
                                      {"qubits_next_metric", qubits_next_metric},
                                      {"probability_table", probability_table},
                                      {"qubits_init_null", qubits_init_null},
                                      {"flag_integer", 0}};

      //Add qubit register to W prime
      w_prime->expand(w_map);

      //Add W prime unitary to state preparation circuit
      state_prep->addInstructions(w_prime->getInstructions());

      /////////////////////////////////////////////////////////////////////////////////////////////

      // Initialize repetition flags
      if (it > 0) {
        auto init_repeat =
            std::dynamic_pointer_cast<xacc::CompositeInstruction>(
                xacc::getService<xacc::Instruction>("InitRepeatFlag"));
        xacc::HeterogeneousMap rep_map = {
            {"iteration", it},
            {"qubits_string", qubits_string},
            {"qubits_next_letter", qubits_next_letter},
            {"qubits_init_repeat", qubits_init_repeat}};
        init_repeat->expand(rep_map);
        // Add marking of repeat symbols to state preparation circuit
        state_prep->addInstructions(init_repeat->getInstructions());
      }

      /////////////////////////////////////////////////////////////////////////////////////////////

      //Initialize U prime unitary
      auto u_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("UPrime"));

      //Merge qubit register for U prime unitary into a heterogenous map
      xacc::HeterogeneousMap u_map = {{"iteration", it},
                                      {"qubits_next_letter", qubits_next_letter},
                                      {"qubits_next_metric", qubits_next_metric},
                                      {"qubits_string", qubits_string},
                                      {"qubits_metric", qubits_metric}};

      //Add qubit register to U prime
      u_prime->expand(u_map);

      //Add U prime unitary to state preparation circuit
      state_prep->addInstructions(u_prime->getInstructions());

      /////////////////////////////////////////////////////////////////////////////////////////////

      //Initialize Q prime unitary
      auto q_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("QPrime"));

      //Merge qubit register for Q prime unitary into a heterogenous map
      xacc::HeterogeneousMap q_map = {{"iteration", it},
                                      {"qubits_next_letter", qubits_next_letter},
                                      {"qubits_next_metric", qubits_next_metric},
                                      {"qubits_string", qubits_string},
                                      {"qubits_metric", qubits_metric}};

      //Add qubit register to Q prime
      q_prime->expand(q_map);

      //Add Q prime unitary to state preparation circuit
      state_prep->addInstructions(q_prime->getInstructions());
    }// Loop over string length

    /////////////////////////////////////////////////////////////////////////////////////////////

    //The comparator oracle takes in the total metric. Therefore we need to use an adder to
    //sum up the individual scores and form total metric.
    int m = qubits_next_metric.size(); //Size of the qubits_next_metric is constant and fixed at the initizalization of the program.
    int c_in = qubits_ancilla_pool[0];//qubits_ancilla_adder[0]; //Carry over
    std::vector<int> total_metric;

    //Insert first iteration's qubits into total_metric and use it in the following for-loop to be summed iteratively with other qubits_next_metric iterations
    for (int i = 0; i < m; i++)
      total_metric.push_back(qubits_metric[i]);

    for (int i = 0; i < qubits_ancilla_adder.size(); i++)//for (int i = 1; i < qubits_ancilla_adder.size(); i++)
      total_metric.push_back(qubits_ancilla_adder[i]);

    for (int it = 1; it < iteration; it++){
      std::vector<int> metrics; //Vector to store elements of the metric at each iteration.
      int start = it*m;
      int end = (it+1)*m;

      for (int i = start; i < end; i++)
        metrics.push_back(qubits_metric[i]);

      for (int i = 0; i < total_metric.size()-1-m; i++) {
        metrics.push_back(qubits_ancilla_pool[i+1]);//metrics.push_back(qubits_ancilla_oracle[i]);
      }

      //Use ripple adder to add the qubits_metric at iteration 'it' to the total metric vector
      auto adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("RippleCarryAdder"));
      bool expand_ok = adder->expand({{"adder_bits", metrics}, {"sum_bits", total_metric}, {"c_in", c_in}});
      assert(expand_ok);

      //Add total metric to state preparation circuit
      state_prep->addInstructions(adder->getInstructions());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////

    // Now we apply the decoder kernel to form beam equivalence classes
    auto decoder_kernel =
    std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("DecoderKernel"));
    bool expand_ok = decoder_kernel->expand({
            {"qubits_string", qubits_string},
            {"qubits_metric", qubits_metric},
            {"qubits_ancilla_adder", qubits_ancilla_adder},
            {"qubits_init_null", qubits_init_null},
            {"qubits_init_repeat", qubits_init_repeat},
            {"qubits_beam_metric", qubits_beam_metric},
            {"qubits_superfluous_flags", qubits_superfluous_flags},
            {"qubits_ancilla_pool", qubits_ancilla_pool},
            {"total_metric", total_metric},
            {"total_metric_copy", qubits_total_metric_copy},
            {"evaluation_bits", evaluation_bits},//CAN USE SOME OF qubits_ancilla_pool?
            {"precision_bits", precision_bits}});
    assert(expand_ok);
    state_prep->addInstructions(decoder_kernel->getInstructions());
    return state_prep;
  };
  auto state_prep_circ = state_prep_(qubits_string, qubits_metric, qubits_next_letter,
                                     qubits_next_metric, qubits_ancilla_adder);

  /////////////////////////////////////////////////////////////////////////////////////////////

  //Comparator oracle
  std::function<std::shared_ptr<xacc::CompositeInstruction>(
    int,int,std::vector<int>,int,std::vector<int>,std::vector<int>)>
    oracle_ = [&](int BestScore, int num_scoring_qubits, std::vector<int> qubits_beam_metric,//std::vector<int> total_metric,
                  int qubit_flag, std::vector<int> qubits_best_score,
                  std::vector<int> qubits_ancilla_oracle) {

    //Initialize comparator oracle circuit
    auto oracle = gateRegistry->createComposite("oracle");
//    oracle->addInstruction(gateRegistry->createInstruction("X", qubit_flag));
//    oracle->addInstruction(gateRegistry->createInstruction("H", qubit_flag));
    auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("Comparator"));

    //Create list of input parameters
    xacc::HeterogeneousMap options{{"BestScore", BestScore},
                                   {"num_scoring_qubits", (int)qubits_best_score.size()},
                                   {"qubits_beam_metric",qubits_beam_metric},//{"trial_score_qubits", total_metric},
                                   {"flag_qubit", qubit_flag},
                                   {"best_score_qubits", qubits_best_score},
                                   {"ancilla_qubits", qubits_ancilla_oracle},
                                   {"as_oracle", true}};

    //Add input parameter list to comparator and check if initializing is okay
    const bool expand_ok = comp->expand(options);
    assert(expand_ok);

    //Add comparator to oracle circuit
    oracle->addInstructions(comp->getInstructions());
std::cout << "num gates oracle: " << oracle->nInstructions() << "\n";
    return oracle;
  };

  /////////////////////////////////////////////////////////////////////////////////////////////

  //Initiate scoring function
  std::function<int(int)> f_score = [&](int score) { return score; };

  // Validate the success probabilities. This is done by iterating the
  // exponential search N_TRIALS times until the best score is obtained.
  int nSuccess = 0;

  /////////////////////////////////////////////////////////////////////////////////////////////

  //Have to re-initiate total_metric here for getAlgorithm("exponential-search") below
  std::vector<int> total_metric;

  // Insert first iteration's qubits into total_metric and use it in the
  // following for loop to be summed iteratively with other qubits_next_metric
  // iteration
  for (int i = 0; i < m; i++)
    total_metric.push_back(qubits_metric[i]);

  for (int i = 0; i < qubits_ancilla_adder.size();
       i++) // for (int i = 1; i < qubits_ancilla_adder.size(); i++)
    total_metric.push_back(qubits_ancilla_adder[i]);

  /////////////////////////////////////////////////////////////////////////////////////////////

  int current_best_score = BestScore;
  int max_best_score = current_best_score;
  std::string best_string;
  int total_num_qubits = L*(m+3*S+6) + 4*k - m + 2*L + k2 + qubits_ancilla_pool.size();

  std::cout<< "Total number qubits = " << total_num_qubits << "\n";

  for (int runCount = 0; runCount < N_TRIALS; ++runCount) {
    std::cout << "Decoder iteration: " << runCount + 1
              << ", initial best score: " << current_best_score << std::endl;
    auto exp_search_algo = xacc::getAlgorithm(
      "exponential-search", {{"method", "canonical"},
                             {"state_preparation_circuit", state_prep_circ},//{"state_preparation_circuit", state_prep_},
                             {"oracle_circuit", oracle_},
                             {"best_score", current_best_score},
                             {"f_score", f_score},
                             {"qubit_flag", qubit_flag},
                             {"qubits_metric", qubits_metric},
                             {"qubits_string", qubits_string},
                             {"qubits_next_letter", qubits_next_letter},
                             {"qubits_next_metric", qubits_next_metric},
                             {"qubits_best_score", qubits_best_score},
                             {"qubits_ancilla_adder", qubits_ancilla_adder},
                             {"qubits_ancilla_oracle", qubits_ancilla_oracle},
                             {"qubits_beam_metric", qubits_beam_metric},//{"total_metric", total_metric},
                             {"total_num_qubits", total_num_qubits},
                             {"qubits_init_null", qubits_init_null},
                             {"qubits_init_repeat", qubits_init_repeat},
                             {"qubits_superfluous_flags", qubits_superfluous_flags},
                             {"evaluation_bits", evaluation_bits},
                             {"total_metric_copy", qubits_total_metric_copy},
                             {"qpu", qpu_}});

    auto buffer = xacc::qalloc(1 + (int)qubits_string.size() + (int)qubits_total_metric_copy.size() + (int)qubits_metric.size()
                               + (int)qubits_best_score.size() + (int)qubits_ancilla_adder.size()
                               + (int)qubits_init_null.size() + (int)qubits_init_repeat.size()
                               + (int)qubits_superfluous_flags.size() + qubits_beam_metric.size()
                               + (int)qubits_ancilla_pool.size() +  (int)precision_bits.size());
    exp_search_algo->execute(buffer);
    auto info = buffer->getInformation();
    //    std::cout << buffer->toString() << std::endl;
    int bs = info.at("best-score").as<int>();

    int previous_best_score = current_best_score;
    current_best_score = bs; // Set best score to that of the current best score
                             // for the subsequent loop.

    if (current_best_score > previous_best_score) {
      std::cout << "New best score: " << current_best_score << std::endl;
      best_string = info.at("best-string").as<std::string>();
    }
    if (current_best_score > max_best_score)
      max_best_score = current_best_score;
    // if (current_best_score <= previous_best_score && previous_best_score > 0)
    // {
    //   std::cout << std::endl;
    //   std::cout << "--------------------------------------------------" <<
    //   std::endl; std::cout << "Maximum score achieved at decoder iteration "
    //   << runCount
    //             << ", with score: " << previous_best_score
    //             << ", and string: " << best_string << std::endl;
    //   std::cout <<  "Exiting decoder." << std::endl;
    //   std::cout << "--------------------------------------------------" <<
    //   std::endl; std::cout << std::endl; break;
    // }
    std::cout << "--------------------------------------------------"
              << std::endl;
    std::cout << std::endl;
  }
  assert(max_best_score >= BestScore);

} // QuantumDecoder::execute
} // namespace qbOS
REGISTER_PLUGIN(qbOS::QuantumDecoder, xacc::Algorithm)

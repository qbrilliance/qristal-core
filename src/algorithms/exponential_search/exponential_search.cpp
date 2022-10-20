// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "qb/core/algorithms/exponential_search/exponential_search.hpp"
#include <chrono>
#include <ctime>

namespace {
int uniform_integer_random_sampling(int max) {
  static std::random_device rd;
  static std::mt19937 rng(rd());
  std::uniform_int_distribution<int> uni(0, max);

  int random_integer = uni(rng);
  return random_integer;
}

bool is_msb_func(xacc::Accelerator *qpu_) {
  if (qpu_->name() == "qsim") {
    return false;
  } else {
    const bool is_msb = qpu_->getBitOrder() == xacc::Accelerator::BitOrder::MSB;
    return is_msb;
  }
}
} // namespace

namespace qbOS {
bool ExponentialSearch::initialize(const xacc::HeterogeneousMap &parameters) {
  // Get the method
  if (parameters.keyExists<std::string>("method")) {
    method = parameters.get<std::string>("method");
  } else {
    method = "canonical";
  }

  // Get the generator
  if (!parameters
           .keyExists<std::function<std::shared_ptr<xacc::CompositeInstruction>(
               int, int, std::vector<int>, int, std::vector<int>,
               std::vector<int>)>>("oracle_circuit")) {
    return false;
  }

  oracle_circuit_gen_ =
      parameters.get<std::function<std::shared_ptr<xacc::CompositeInstruction>(
          int, int, std::vector<int>, int, std::vector<int>,
          std::vector<int>)>>("oracle_circuit");

  if (!parameters.keyExists<std::function<int(int)>>("f_score")) {
    return false;
  }
  f_score_ = parameters.get<std::function<int(int)>>("f_score");

  state_prep_circuit_ = nullptr;
  if (parameters.pointerLikeExists<xacc::CompositeInstruction>(
          "state_preparation_circuit")) {
    state_prep_circuit_ = parameters.getPointerLike<xacc::CompositeInstruction>(
        "state_preparation_circuit");
    assert(state_prep_circuit_->nInstructions() > 0);
  }
  if (!state_prep_circuit_) {

    // Get the generator
    if (!parameters.keyExists<
            std::function<std::shared_ptr<xacc::CompositeInstruction>(
                std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>)>>(
            "state_preparation_circuit")) {
      return false;
    }
    state_prep_circuit_gen_ =
        parameters
            .get<std::function<std::shared_ptr<xacc::CompositeInstruction>(
                std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>)>>(
                "state_preparation_circuit");
  }

  best_score_ = parameters.get_or_default("best_score", 0);

  if (!parameters.pointerLikeExists<xacc::Accelerator>("qpu")) {
    static auto qpp = xacc::getAccelerator("qpp", {{"shots", 1}});
    // Default to qpp if none provided
    qpu_ = qpp.get();
  } else {
    qpu_ = parameters.getPointerLike<xacc::Accelerator>("qpu");
    qpu_->updateConfiguration({{"shots", 1}});
  }

  if (method == "MLQAE") {
    if (!parameters.keyExists<std::function<int(std::string, int)>>(
            "MLQAE_is_in_good_subspace")) {
      return false;
    }
    MLQAE_is_in_good_subspace = parameters.get<std::function<int(std::string, int)>>(
        "MLQAE_is_in_good_subspace");
    MLQAE_num_runs = parameters.get_or_default("MLQAE_num_runs", 4);
    MLQAE_num_shots = parameters.get_or_default("MLQAE_num_shots", 100);
  }

  if (method == "CQAE") {
    if (!parameters.keyExists<int>("CQAE_num_evaluation_qubits")) {
      return false;
    }
    CQAE_num_evaluation_qubits = parameters.get<int>("CQAE_num_evaluation_qubits");
  }

  // If qubit indices are not provided, the default qubit register will be
  // |trial_ancilla>|qubits_metric>|qubits_string>|qubit_flag>|qubits_best_score>|qubits_ancilla_oracle>

  if (!parameters.keyExists<std::vector<int>>("qubits_metric")) {
    return false;
  }
  qubits_metric = parameters.get<std::vector<int>>("qubits_metric");
  int num_qubits_metric = qubits_metric.size();

  if (!parameters.keyExists<std::vector<int>>("qubits_best_score")) {
    return false;
  }
  qubits_best_score = parameters.get<std::vector<int>>("qubits_best_score");
  int num_qubits_best_score = qubits_best_score.size();

  if (!parameters.keyExists<std::vector<int>>("qubits_string")) {
    return false;
  }
  qubits_string = parameters.get<std::vector<int>>("qubits_string");
  int num_qubits_string = qubits_string.size();

  if (!parameters.keyExists<std::vector<int>>("qubits_ancilla_oracle")) {
    return false;
  }
  qubits_ancilla_oracle = parameters.get<std::vector<int>>("qubits_ancilla_oracle");
  int num_qubits_ancilla_oracle = qubits_ancilla_oracle.size();

  if (!parameters.keyExists<std::vector<int>>("qubits_next_letter")) {
    return false;
  }
  qubits_next_letter = parameters.get<std::vector<int>>("qubits_next_letter");
  int num_qubits_next_letter = qubits_next_letter.size();

  if (!parameters.keyExists<std::vector<int>>("qubits_next_metric")) {
    return false;
  }
  qubits_next_metric = parameters.get<std::vector<int>>("qubits_next_metric");
  int num_qubits_next_metric = qubits_next_metric.size();

  if (!parameters.keyExists<int>("qubit_flag")) {
    return false;
  }
  qubit_flag = parameters.get<int>("qubit_flag");
  int num_qubit_flag = 1;

  if (!parameters.keyExists<std::vector<int>>("qubits_ancilla_adder")) {
    qubits_ancilla_adder = {};
  } else {
    qubits_ancilla_adder = parameters.get<std::vector<int>>("qubits_ancilla_adder");
  }
  int num_qubits_ancilla_adder = qubits_ancilla_adder.size();

  if (parameters.keyExists<std::vector<int>>("total_metric")) {
    total_metric = parameters.get<std::vector<int>>("total_metric");
  } else {
    total_metric = parameters.get<std::vector<int>>("qubits_metric");
  }
  if (total_metric.size() == 0) {
    total_metric = parameters.get<std::vector<int>>("qubits_metric");
  }

  int total_num_qubits = num_qubit_flag +
                         num_qubits_ancilla_oracle + num_qubits_best_score +
                         num_qubits_metric + num_qubits_next_letter +
                         num_qubits_next_metric + num_qubits_string +
                         num_qubits_ancilla_adder;
  return true;
}

const std::vector<std::string> ExponentialSearch::requiredParameters() const {
  return {"method",
          "state_preparation_circuit",
          "oracle_circuit",
          "best_score",
          "f_score",
          "qubits_string",
          "qubits_metric",
          "qubits_next_letter",
          "qubits_next_metric",
          "qubit_flag",
          "qubits_best_score",
          "qubits_ancilla_oracle"};
}

void ExponentialSearch::execute(
    const std::shared_ptr<xacc::AcceleratorBuffer> buffer) const {

  int num_qubits_metric = qubits_metric.size();

  int num_qubits_best_score = qubits_best_score.size();

  int num_qubits_string = qubits_string.size();

  int num_qubits_ancilla_oracle = qubits_ancilla_oracle.size();

  int num_qubits_next_letter = qubits_next_letter.size();

  int num_qubits_next_metric = qubits_next_metric.size();

  int num_qubits_ancilla_adder = qubits_ancilla_adder.size();

  int num_total_metric = total_metric.size();

  int num_qubit_flag = 1;

  int total_num_qubits = num_qubit_flag +
                         num_qubits_ancilla_oracle + num_qubits_best_score +
                         num_qubits_metric + num_qubits_next_letter +
                         num_qubits_next_metric + num_qubits_string +
                         num_qubits_ancilla_adder;

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  std::shared_ptr<xacc::CompositeInstruction> state_prep;
  bool sp_composite_instruction;
  if (state_prep_circuit_) {
    sp_composite_instruction = true;
    state_prep = xacc::as_shared_ptr(state_prep_circuit_);
  } else {
    sp_composite_instruction = false;
    state_prep = state_prep_circuit_gen_(qubits_string,
                                         qubits_metric, qubits_next_letter, qubits_next_metric,
                                         qubits_ancilla_adder);
  }

  auto oracle = gateRegistry->createComposite("oracle");
  oracle->addInstruction(
      oracle_circuit_gen_(best_score_, num_total_metric, total_metric,
                          qubit_flag, qubits_best_score, qubits_ancilla_oracle));

  int iterations;

  if (method == "MLQAE") {
    // Amplitude Estimation
    auto temp_buffer = xacc::qalloc(buffer->size());
    auto ae_algo = xacc::getAlgorithm(
        "ML-ae", {{"state_preparation_circuit", state_prep},
                  {"oracle_circuit", oracle},
                  {"is_in_good_subspace", MLQAE_is_in_good_subspace},
                  {"num_runs", MLQAE_num_runs},
                  {"shots", MLQAE_num_shots},
                  {"score_qubits", total_metric},
                  {"bestScore", best_score_},
                  {"total_num_qubits", total_num_qubits},
                  {"qpu", qpu_}});
    ae_algo->execute(temp_buffer);

    double amplitude_estimate =
        temp_buffer["amplitude-estimation"].as<double>();

    int optimal_iterations = ceil(M_PI / (4 * amplitude_estimate));

    iterations = optimal_iterations;
  }

  if (method == "CQAE") {
    std::vector<int> all_trial_qubits = {};
    for (int i = 0; i < num_qubits_metric; i++) {
      all_trial_qubits.push_back(qubits_metric[i]);
    }
    for (int i = 0; i < num_qubits_string; i++) {
      all_trial_qubits.push_back(qubits_string[i]);
    }
    all_trial_qubits.push_back(qubit_flag);
    for (int i = 0; i < num_qubits_best_score; i++) {
      all_trial_qubits.push_back(qubits_best_score[i]);
    }
    for (int i = 0; i < num_qubits_ancilla_oracle; i++) {
      all_trial_qubits.push_back(qubits_ancilla_oracle[i]);
    }
    for (int i = 0; i < num_qubits_next_letter; i++) {
      all_trial_qubits.push_back(qubits_next_letter[i]);
    }
    for (int i = 0; i < num_qubits_next_metric; i++) {
      all_trial_qubits.push_back(qubits_next_metric[i]);
    }
    for (int i = 0; i < num_total_metric; i++) {
      all_trial_qubits.push_back(total_metric[i]);
    }

    int total_num_qubits_temp = all_trial_qubits.size();
    std::vector<int> evaluation_qubits = {};
    for (int i = 0; i < CQAE_num_evaluation_qubits; i++) {
      evaluation_qubits.push_back(total_num_qubits_temp + i);
    }

    // Amplitude Estimation
    auto temp_buffer = xacc::qalloc(buffer->size());
    auto ae_algo = xacc::getAlgorithm(
        "canonical-ae",
        {{"state_preparation_circuit", state_prep},
         {"oracle", oracle},
         {"trial_qubits", all_trial_qubits},
         {"evaluation_qubits", evaluation_qubits},
         {"num_evaluation_qubits", CQAE_num_evaluation_qubits},
         {"num_state_qubits", num_qubits_string + num_qubits_metric +
                              num_qubits_next_letter + num_total_metric},
         {"num_trial_qubits", total_num_qubits_temp},
         {"qpu", qpu_}});
    ae_algo->execute(temp_buffer);

    double amplitude_estimate =
        temp_buffer["amplitude-estimation"].as<double>();

    int optimal_iterations = ceil(M_PI / (4 * amplitude_estimate));

    iterations = optimal_iterations;
  }

  int m = 1;
  constexpr double lambda = 6.0 / 5.0;
  const auto n = num_qubits_string;
  const auto N = 1ULL << n;
  int loops = 1;
  if (method == "canonical") {
    // number of maximum iterations we may run:
    loops = std::floor(std::sqrt(N) * 9.0 / 4.0);
  }
  std::cout << "Maximum exponential search iterations = " << loops << std::endl;
  std::cout << std::endl;

  int i = 0;
  while (i < loops) {
    std::cout << "Current exponential search iteration = " << i+1 << std::endl;
    if (method == "canonical") {
      iterations = std::max(uniform_integer_random_sampling(m), 1);
    }
    auto exp_search_circuit =
        gateRegistry->createComposite("__TEMP__EXP__SEARCH__");

    // amplitude amplification
    if (state_prep_circuit_) {
      auto state_prep = state_prep_circuit_;
      exp_search_circuit->addInstructions(
          state_prep_circuit_->getInstructions());
    } else {
      auto state_prep = state_prep_circuit_gen_(
           qubits_string, qubits_metric, qubits_next_letter,qubits_next_metric,qubits_ancilla_adder);
      exp_search_circuit->addInstructions(state_prep->getInstructions());
    }

    for (int count = 0; count < iterations; ++count) {
      xacc::InstructionIterator it(oracle);
      while (it.hasNext()) {
        auto nextInst = it.next();
        if (nextInst->isEnabled() && !nextInst->isComposite()) {
          exp_search_circuit->addInstruction(nextInst->clone());
        }
      }
      std::vector<xacc::InstPtr> state_prep_gates;
      if (state_prep_circuit_) {
        auto state_prep = state_prep_circuit_;
        xacc::InstructionIterator it(xacc::as_shared_ptr(state_prep));
        while (it.hasNext()) {
          auto nextInst = it.next();
          if (nextInst->isEnabled() && !nextInst->isComposite()) {
            state_prep_gates.emplace_back(nextInst->clone());
          }
        }
      } else {
        auto state_prep = state_prep_circuit_gen_(
            qubits_string, qubits_metric, qubits_next_letter, qubits_next_metric, qubits_ancilla_adder);
        xacc::InstructionIterator it(state_prep);
        while (it.hasNext()) {
          auto nextInst = it.next();
          if (nextInst->isEnabled() && !nextInst->isComposite()) {
            state_prep_gates.emplace_back(nextInst->clone());
          }
        }
      }

      std::vector<xacc::InstPtr> temp_inverse_state_prep_gates;
      for (const auto &inst : state_prep_gates) {
        temp_inverse_state_prep_gates.emplace_back(inst->clone());
      }
      std::reverse(temp_inverse_state_prep_gates.begin(),
                   temp_inverse_state_prep_gates.end());

      std::vector<xacc::InstPtr> inverse_state_prep_gates;
      auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
      for (auto &inst : temp_inverse_state_prep_gates) {
        // Parametric gates:
        if (inst->name() == "Rx" || inst->name() == "Ry" ||
            inst->name() == "Rz" || inst->name() == "CPhase" ||
            inst->name() == "U1" || inst->name() == "CRZ") {
          inst->setParameter(0, -inst->getParameter(0).template as<double>());
          inverse_state_prep_gates.emplace_back(inst);
        }
        // Handles T and S gates, etc... => T -> Tdg
        else if (inst->name() == "T") {
          auto tdg = gateRegistry->createInstruction("Tdg", inst->bits()[0]);
          inverse_state_prep_gates.emplace_back(tdg);
        } else if (inst->name() == "S") {
          auto sdg = gateRegistry->createInstruction("Sdg", inst->bits()[0]);
          inverse_state_prep_gates.emplace_back(sdg);
        } else if (inst->name() == "Tdg") {
          auto t = gateRegistry->createInstruction("T", inst->bits()[0]);
          inverse_state_prep_gates.emplace_back(t);
        } else if (inst->name() == "Sdg") {
          auto s = gateRegistry->createInstruction("S", inst->bits()[0]);
          inverse_state_prep_gates.emplace_back(s);
        } else {
          inverse_state_prep_gates.emplace_back(inst);
        }
      }
      assert(inverse_state_prep_gates.size() == state_prep_gates.size());

      exp_search_circuit->addInstructions(inverse_state_prep_gates);

      std::vector<int> qubits_state_prep;
      if (sp_composite_instruction) {
        auto bits = state_prep->uniqueBits();
        for (auto it = bits.begin(); it != bits.end(); it++) {
          std::cout << *it << "\n";
          qubits_state_prep.push_back(*it);
        }
      } else {
        for (int i = 0; i < num_qubits_string; ++i) {
          qubits_state_prep.emplace_back(qubits_string[i]);
        }
        for (int i = 0; i < num_qubits_metric; ++i) {
          qubits_state_prep.emplace_back(qubits_metric[i]);
        }
        for (int i = 0; i < num_qubits_next_letter; ++i) {
          qubits_state_prep.emplace_back(qubits_next_letter[i]);
        }
        for (int i = 0; i < num_qubits_next_metric; ++i) {
          qubits_state_prep.emplace_back(qubits_next_metric[i]);
        }
        for (int i = 0; i < num_qubits_ancilla_adder; ++i) {
          qubits_state_prep.emplace_back(qubits_ancilla_adder[i]);
        }
      }

      for (int q = 0; q < qubits_state_prep.size(); ++q) {//for (int q = 0; q < num_qubits_string; ++q) {
        exp_search_circuit->addInstruction(
            gateRegistry->createInstruction("X", qubits_state_prep[q]));//gateRegistry->createInstruction("X", qubits_string[q]));
      }
      auto z_gate = gateRegistry->createComposite("z_gate");
      auto temp_gate = gateRegistry->createInstruction("Z", qubits_state_prep[(int)qubits_state_prep.size() - 1]);
      temp_gate->setBufferNames({"q"});
      z_gate->addInstruction(temp_gate);
      auto mcz = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      std::vector<int> controlled_bits;
      for (int i = 0; i < qubits_state_prep.size() - 1; i++) {
        controlled_bits.push_back(qubits_state_prep[i]);
      }
      mcz->expand({{"U", z_gate}, {"control-idx", controlled_bits}});
      exp_search_circuit->addInstruction(mcz);

      for (int q = 0; q < qubits_state_prep.size(); ++q) {//for (int q = 0; q < num_qubits_string; ++q) {
        exp_search_circuit->addInstruction(
            gateRegistry->createInstruction("X", qubits_state_prep[q]));//gateRegistry->createInstruction("X", qubits_string[q]));
      }

      exp_search_circuit->addInstructions(state_prep_gates);
    }

    std::vector<int> measured_indices;
    for (int q = 0; q < num_qubits_string; ++q){
      measured_indices.push_back(qubits_string[q]);
    }
    for (int q = 0; q < num_total_metric; ++q) {
      measured_indices.push_back(total_metric[q]);
    }

    std::vector<int> measured_indices_sorted;
    std::sort(measured_indices.begin(), measured_indices.end());
    for (int q = 0; q < measured_indices.size(); q++) {
      measured_indices_sorted.push_back(measured_indices[q]);
    }

    for (int q = 0; q < measured_indices.size(); ++q) {
      exp_search_circuit->addInstruction(
        gateRegistry->createInstruction("Measure", measured_indices_sorted[q]));
    }

    auto temp_buffer_2 = xacc::qalloc(total_num_qubits);
    srand((unsigned)time(0));
    auto seed = rand();
    qpu_->updateConfiguration({{"shots", 1}, {"seed", seed}});
    qpu_->execute(temp_buffer_2, exp_search_circuit);
    assert(temp_buffer_2->getMeasurements().size() == 1);
    bool is_msb = is_msb_func(qpu_);//const bool is_msb = qpu_->getBitOrder() == xacc::Accelerator::BitOrder::MSB;
    const std::string rawBitString = temp_buffer_2->getMeasurements().front();
    std::string BitString = is_msb ? std::string(rawBitString.rbegin(), rawBitString.rend()) : rawBitString;

    std::string BitString_metric;
    std::string BitString_string;
    for (int q = 0; q < measured_indices_sorted.size(); q++) {
      if (std::find(total_metric.begin(), total_metric.end(), measured_indices_sorted[q]) != total_metric.end()) {
        BitString_metric.push_back(BitString[q]);
      }
      if (std::find(qubits_string.begin(), qubits_string.end(), measured_indices_sorted[q]) != qubits_string.end()) {
        BitString_string.push_back(BitString[q]);
      }
    }
    std::string bitString_metric_msb = std::string(BitString_metric.rbegin(), BitString_metric.rend());
    const int x_value_binary = atoi(bitString_metric_msb.c_str());//const int x_value = std::stoi(bitString_metric_msb, 0, 2);
    int x_value = 0;
    int base = 1; //Initialize base value to 1 = 2^0
    int temp = x_value_binary;
    while (temp) {
        int last_digit = temp % 10;
        temp /= 10;
        x_value += last_digit * base;
        base *= 2;
    }

    std::cout << "Measure: " << rawBitString
              << ", qubits_metric: " << BitString_metric
              << ", qubits_string: " << BitString_string
              << ", score = " << f_score_(x_value) << "\n";
    std::cout << std::endl;
    if (f_score_(x_value) > best_score_) {
      buffer->addExtraInfo("best-score", f_score_(x_value));
      buffer->addExtraInfo("best-string", BitString_string);
      break;
    } else {
      buffer->addExtraInfo("best-score", best_score_);
      buffer->addExtraInfo("best-string", BitString_string);
    }
    m = std::round(m * lambda);
    ++i;

    if (i == loops) {
      std::cout << "Maximum search iterations reached. Exiting exponential search." << std::endl;
    }
  }
}
} // namespace qbOS

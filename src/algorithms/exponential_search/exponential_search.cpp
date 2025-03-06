// Copyright (c) Quantum Brilliance Pty Ltd
#include "qristal/core/algorithms/exponential_search/exponential_search.hpp"
#include <chrono>
#include <ctime>
//#include <memory>

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

namespace qristal {
bool ExponentialSearch::initialize(const xacc::HeterogeneousMap &parameters) {
  std::cout << "Intiialise ExponentialSearch" << std::endl;
  // Get the method
  if (parameters.keyExists<std::string>("method")) {
    method = parameters.get<std::string>("method");
  } else {
    method = "canonical";
  }
  std::cout << "method:" << method << std::endl;

  // Get the generator
  if (!parameters.keyExists<
          std::function<std::shared_ptr<xacc::CompositeInstruction>(int)>>(
          "oracle_circuit")) {
    std::cout << "oracle_circuit" << std::endl;
    return false;
  }

  oracle_circuit_gen_ =
      parameters
          .get<std::function<std::shared_ptr<xacc::CompositeInstruction>(int)>>(
              "oracle_circuit");

  if (!parameters.keyExists<std::function<int(int)>>("f_score")) {
    std::cout << "f_score" << std::endl;
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
                std::vector<int>, std::vector<int>, std::vector<int>,
                std::vector<int>, std::vector<int>)>>(
            "state_preparation_circuit")) {
      std::cout << "state_prep_circuit" << std::endl;
      return false;
    }
    state_prep_circuit_gen_ =
        parameters
            .get<std::function<std::shared_ptr<xacc::CompositeInstruction>(
                std::vector<int>, std::vector<int>, std::vector<int>,
                std::vector<int>, std::vector<int>)>>(
                "state_preparation_circuit");
  }
  std::cout << "Parameters input" << std::endl;
  best_score_ = parameters.get_or_default("best_score", 0);
  std::cout << "qpu" << std::endl;

  if (!parameters.pointerLikeExists<xacc::Accelerator>("qpu")) {
    static auto qpp = xacc::getAccelerator("qpp", {{"shots", 1}});
    // Default to qpp if none provided
    qpu_ = qpp.get();
  } else {
    qpu_ = parameters.getPointerLike<xacc::Accelerator>("qpu");
    qpu_->updateConfiguration({{"shots", 1}});
  }

  std::cout << "methods" << std::endl;
  if (method == "MLQAE") {
    if (!parameters.keyExists<std::function<int(std::string, int)>>(
            "MLQAE_is_in_good_subspace")) {
      return false;
    }
    MLQAE_is_in_good_subspace =
        parameters.get<std::function<int(std::string, int)>>(
            "MLQAE_is_in_good_subspace");
    MLQAE_num_runs = parameters.get_or_default("MLQAE_num_runs", 4);
    MLQAE_num_shots = parameters.get_or_default("MLQAE_num_shots", 100);
  }

  if (method == "CQAE") {
    if (!parameters.keyExists<int>("CQAE_num_evaluation_qubits")) {
      return false;
    }
    CQAE_num_evaluation_qubits =
        parameters.get<int>("CQAE_num_evaluation_qubits");
  }
//   if (parameters.keyExists<std::vector<int>>("total_metric")) {
//     total_metric = parameters.get<std::vector<int>>("total_metric");
//   } else {
//     total_metric = parameters.get<std::vector<int>>("qubits_metric");
//   }
//   if (total_metric.size() == 0) {
//     total_metric = parameters.get<std::vector<int>>("qubits_metric");
//   }

  if (!parameters.keyExists<int>("total_num_qubits")) {
      std::cout << "total_num_qubits" << std::endl;
      return false;
  }
 total_num_qubits = parameters.get<int>("total_num_qubits");
  assert(total_num_qubits > 0);

  if (!parameters.keyExists<std::vector<int>>("qubits_string")) {
      return false;
  }
  qubits_string = parameters.get<std::vector<int>>("qubits_string");

  if (!parameters.keyExists<std::vector<int>>("total_metric")) {
      std::cout << "total_metric" << std::endl;
      return false;
  }
  total_metric = parameters.get<std::vector<int>>("total_metric");
  std::cout << "Initialised" << std::endl;

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

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
  std::shared_ptr<xacc::CompositeInstruction> state_prep;
  if (state_prep_circuit_) {
    state_prep = xacc::as_shared_ptr(state_prep_circuit_);
  } else {
    state_prep = state_prep_circuit_gen_(qubits_string, total_metric, {}, {}, {});
  }

  auto oracle = gateRegistry->createComposite("oracle");
  oracle->addInstruction(oracle_circuit_gen_(best_score_));

  int iterations;
  int m = 1;
  constexpr double lambda = 6.0 / 5.0;
  const auto n = qubits_string.size();
  const auto N = 1ULL << n;
  int loops = 1;
  if (method == "canonical") {
    loops = std::floor(std::sqrt(N) * 9.0 / 4.0);
  }
  std::cout << "Maximum exponential search iterations = " << loops << std::endl;
  std::cout << std::endl;

  int i = 0;
  while (i < loops) {
    std::cout << "Current exponential search iteration = " << i + 1
              << std::endl;
    if (method == "canonical") {
      iterations = std::max(uniform_integer_random_sampling(m), 1);
    }
    auto exp_search_circuit =
        gateRegistry->createComposite("__TEMP__EXP__SEARCH__");

    exp_search_circuit->addInstruction(state_prep);

    auto inv_sp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("InverseCircuit"));
    const bool expand_ok_inv_sp = inv_sp->expand({{"circ", state_prep}});
    assert(expand_ok_inv_sp);

    std::vector<int> qubits_state_prep;
    auto qubits_state_prep_set = qristal::uniqueBitsQD(state_prep);
    for (auto bit : qubits_state_prep_set) {
      qubits_state_prep.push_back(bit);
    }

    auto zero_reflection = gateRegistry->createComposite("ZR");
    for (int q = 0; q < qubits_state_prep.size(); ++q) {
      zero_reflection->addInstruction(
          gateRegistry->createInstruction("X", qubits_state_prep[q]));
    }
    auto z_gate = gateRegistry->createComposite("z_gate");
    auto temp_gate = gateRegistry->createInstruction(
        "Z", qubits_state_prep[(int)qubits_state_prep.size() - 1]);
    temp_gate->setBufferNames({"q"});
    z_gate->addInstruction(temp_gate);
    auto mcz = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    std::vector<int> controlled_bits;
    for (int i = 0; i < qubits_state_prep.size() - 1; i++) {
      controlled_bits.push_back(qubits_state_prep[i]);
    }
    mcz->expand({{"U", z_gate}, {"control-idx", controlled_bits}});
    zero_reflection->addInstruction(mcz);

    for (int q = 0; q < qubits_state_prep.size(); ++q) {
      zero_reflection->addInstruction(
          gateRegistry->createInstruction("X", qubits_state_prep[q]));
    }

    // amplitude amplification
    for (int count = 0; count < iterations; ++count) {

      std::shared_ptr<xacc::CompositeInstruction> state_prep_clone =
          xacc::ir::asComposite(state_prep->clone());
      std::shared_ptr<xacc::CompositeInstruction> oracle_clone =
          xacc::ir::asComposite(oracle->clone());
      std::shared_ptr<xacc::CompositeInstruction> inv_sp_clone =
          xacc::ir::asComposite(inv_sp->clone());
      std::shared_ptr<xacc::CompositeInstruction> zr_clone =
          xacc::ir::asComposite(zero_reflection->clone());

      exp_search_circuit->addInstruction(oracle_clone);
      exp_search_circuit->addInstruction(inv_sp_clone);
      exp_search_circuit->addInstruction(zr_clone);
      exp_search_circuit->addInstruction(state_prep_clone);
    }

    std::vector<int> measured_indices;
    for (int q = 0; q < qubits_string.size(); ++q) {
      measured_indices.push_back(qubits_string[q]);
    }
    for (int q = 0; q < total_metric.size(); ++q) {
      measured_indices.push_back(total_metric[q]);
    }

    std::vector<int> measured_indices_sorted;
    std::sort(measured_indices.begin(), measured_indices.end());
    for (int q = 0; q < measured_indices.size(); q++) {
      measured_indices_sorted.push_back(measured_indices[q]);
    }

    for (int q = 0; q < measured_indices.size(); ++q) {
      exp_search_circuit->addInstruction(gateRegistry->createInstruction(
          "Measure", measured_indices_sorted[q]));
    }

    auto temp_buffer_2 = xacc::qalloc(total_num_qubits);
    srand((unsigned)time(0));
    auto seed = rand();
    qpu_->updateConfiguration({{"shots", 1}, {"seed", seed}});
    qpu_->execute(temp_buffer_2, exp_search_circuit);
//    temp_buffer_2->print();
    assert(temp_buffer_2->getMeasurements().size() == 1);
    bool is_msb = is_msb_func(qpu_); // const bool is_msb = qpu_->getBitOrder()
                                     // == xacc::Accelerator::BitOrder::MSB;
    const std::string rawBitString = temp_buffer_2->getMeasurements().front();
    std::string BitString =
        is_msb ? std::string(rawBitString.rbegin(), rawBitString.rend())
               : rawBitString;

    std::string BitString_metric;
    std::string BitString_string;
    for (int q = 0; q < measured_indices_sorted.size(); q++) {
      if (std::find(total_metric.begin(), total_metric.end(),
                    measured_indices_sorted[q]) != total_metric.end()) {
        BitString_metric.push_back(BitString[q]);
      }
      if (std::find(qubits_string.begin(), qubits_string.end(),
                    measured_indices_sorted[q]) != qubits_string.end()) {
        BitString_string.push_back(BitString[q]);
      }
    }
    std::string bitString_metric_msb =
        std::string(BitString_metric.rbegin(), BitString_metric.rend());
    const int x_value_binary = atoi(
        bitString_metric_msb.c_str()); 
    int x_value = 0;
    int base = 1; 
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
      std::cout
          << "Maximum search iterations reached. Exiting exponential search."
          << std::endl;
    }
  }
}
}

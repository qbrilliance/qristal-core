// Copyright (c) Quantum Brilliance Pty Ltd

#include "qristal/core/circuit_builder.hpp"

namespace qristal
{

  CircuitBuilder::CircuitBuilder()
      : gate_provider_(xacc::getService<xacc::IRProvider>("quantum")),
        circuit_(gate_provider_->createComposite("QBSDK_circuit")) {}

  CircuitBuilder::CircuitBuilder(std::shared_ptr<xacc::CompositeInstruction> &composite,
                  bool copy_nodes)
      : gate_provider_(xacc::getService<xacc::IRProvider>("quantum")) {
    if (copy_nodes) {
      circuit_ = gate_provider_->createComposite("QBSDK_circuit");
      circuit_->addInstructions(composite->getInstructions());
      circuit_->addVariables(composite->getVariables());
    } else {
      circuit_ = composite;
    }
    xacc::InstructionIterator it(composite);
    while (it.hasNext()) {
      auto next_inst = it.next();
      add_instruction_params_to_list(next_inst);
    }
  }

  void CircuitBuilder::append(CircuitBuilder &other) {
    xacc::InstructionIterator it(other.get());
    while (it.hasNext()) {
      auto next_inst = it.next();
      if (next_inst->isEnabled() && !next_inst->isComposite()) {
        std::shared_ptr<xacc::Instruction> new_inst = next_inst->clone();
        add_instruction_params_to_list(new_inst);
        circuit_->addInstruction(new_inst);
      }
    }
  }

  const std::vector<double> CircuitBuilder::param_map_to_vec(const std::map<std::string, double> &param_map) const {
    std::vector<double> param_vec(param_map.size());
    // if (param_vec.size() < param_map.size()) param_vec.resize(param_map.size());
    for (auto el: param_map) {
      std::string param_name = el.first;
      auto param_it = std::find(free_params_.begin(), free_params_.end(), param_name);
      if (param_it != free_params_.end()) {
        size_t idx = param_it - free_params_.begin();
        param_vec[idx] = el.second;
      }
      else std::cout << "Detected an unused parameter " << param_name << std::endl;
    }
    return param_vec;
  }

  void CircuitBuilder::add_gate_with_free_parameters(std::string gate_name, std::vector<size_t> qubits,
                           std::vector<std::string> param_names)
  {
    is_parametrized_ = true;
    std::vector<xacc::InstructionParameter> gate_params;
    for (auto param_name: param_names) {
      gate_params.emplace_back(param_name);
      if (std::find(free_params_.begin(), free_params_.end(), param_name) == free_params_.end()) {
        circuit_->addVariable(param_name);
        free_params_.emplace_back(param_name);
      }
    }
    auto gate =
        gate_provider_->createInstruction(gate_name, qubits, gate_params);
    circuit_->addInstruction(gate);
  }

  void CircuitBuilder::add_instruction_params_to_list(std::shared_ptr<xacc::Instruction> inst) {
    if (!inst->isParameterized()) return;
    is_parametrized_ = true;
    std::vector<xacc::InstructionParameter> params = inst->getParameters();
    for (auto param: params) {
      if (!(param.which() == 2)) continue; // Not named (string) parameter
      std::string param_name = param.toString();
      bool param_in_vector = std::find(free_params_.begin(), free_params_.end(), param_name) != free_params_.end();
      if (!param_in_vector) {
        free_params_.emplace_back(param_name);
        circuit_->addVariable(param_name);
      }
    }
  }

  void CircuitBuilder::H(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("H", idx));
  }

  void CircuitBuilder::X(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("X", idx));
  }

  void CircuitBuilder::Y(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Y", idx));
  }

  void CircuitBuilder::Z(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Z", idx));
  }

  void CircuitBuilder::T(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("T", idx));
  }

  void CircuitBuilder::S(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("S", idx));
  }

  void CircuitBuilder::Tdg(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Tdg", idx));
  }

  void CircuitBuilder::Sdg(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Sdg", idx));
  }

  void CircuitBuilder::RX(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Rx", {idx}, {theta}));
  }

  void CircuitBuilder::RX(size_t idx, std::string param_name) {
    std::string gate_name = "Rx";
    add_gate_with_free_parameters(gate_name, {idx}, {param_name});
  }

  void CircuitBuilder::RY(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Ry", {idx}, {theta}));
  }

  void CircuitBuilder::RY(size_t idx, std::string param_name) {
    std::string gate_name = "Ry";
    add_gate_with_free_parameters(gate_name, {idx}, {param_name});
  }

  void CircuitBuilder::RZ(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Rz", {idx}, {theta}));
  }

  void CircuitBuilder::RZ(size_t idx, std::string param_name) {
    std::string gate_name = "Rz";
    add_gate_with_free_parameters(gate_name, {idx}, {param_name});
  }

  void CircuitBuilder::U1(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("U1", {idx}, {theta}));
  }

  void CircuitBuilder::U1(size_t idx, std::string param_name) {
    std::string gate_name = "U1";
    add_gate_with_free_parameters(gate_name, {idx}, {param_name});
  }
  void CircuitBuilder::U3(size_t idx, std::string param1_name, std::string param2_name, std::string param3_name) {
    std::string gate_name = "U";
    add_gate_with_free_parameters(gate_name, {idx},
                                  {param1_name, param2_name, param3_name});
  }

  void CircuitBuilder::U3(size_t idx, double theta, double phi, double lambda) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("U", {idx}, {theta, phi, lambda}));
  }

  void CircuitBuilder::CNOT(size_t ctrl_idx, size_t target_idx) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("CNOT", {ctrl_idx, target_idx}));
  }

  void CircuitBuilder::MCX(const std::vector<int> &ctrl_inds, size_t target_idx) {
    auto x_gate = gate_provider_->createComposite("temp_X");
    auto temp_gate = gate_provider_->createInstruction("X", target_idx);
    temp_gate->setBufferNames({"q"});
    x_gate->addInstruction(temp_gate);
    auto controlled_U = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    controlled_U->expand({{"U", x_gate}, {"control-idx", ctrl_inds}});
    circuit_->addInstruction(controlled_U);
  }

  void CircuitBuilder::CU(CircuitBuilder &circ, std::vector<int> ctrl_inds) {
    auto controlled_U = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    controlled_U->expand({{"U", circ.get()}, {"control-idx", ctrl_inds}});
    circuit_->addInstruction(controlled_U);
  }

  void CircuitBuilder::CZ(size_t ctrl_idx, size_t target_idx) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("CZ", {ctrl_idx, target_idx}));
  }

  void CircuitBuilder::CH(size_t ctrl_idx, size_t target_idx) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("CH", {ctrl_idx, target_idx}));
  }

  void CircuitBuilder::CPhase(size_t ctrl_idx, size_t target_idx, double theta) {
    circuit_->addInstruction(gate_provider_->createInstruction(
        "CPhase", {ctrl_idx, target_idx}, {theta}));
  }

  void CircuitBuilder::CPhase(size_t ctrl_idx, size_t target_idx, std::string param_name) {
    std::string gate_name = "CPhase";
    add_gate_with_free_parameters(gate_name, {ctrl_idx, target_idx}, {param_name});
  }

  void CircuitBuilder::CRZ(size_t ctrl_idx, size_t target_idx, double theta) {
    circuit_->addInstruction(gate_provider_->createInstruction(
        "CRZ", {ctrl_idx, target_idx}, {theta}));
  }

  void CircuitBuilder::CRZ(size_t ctrl_idx, size_t target_idx, std::string param_name) {
    add_gate_with_free_parameters("CRZ", {ctrl_idx, target_idx}, {param_name});
  }

  void CircuitBuilder::CRX(size_t ctrl_idx, size_t target_idx, double theta) {
    this->RY(target_idx, -1.0 * std::numbers::pi / 2.0);
    this->CRZ(ctrl_idx, target_idx, theta);
    this->RY(target_idx, std::numbers::pi / 2.0);
  }

  void CircuitBuilder::CRX(size_t ctrl_idx, size_t target_idx, std::string param_name) {
    this->RY(target_idx, -1.0 * std::numbers::pi / 2.0);
    this->CRZ(ctrl_idx, target_idx, param_name);
    this->RY(target_idx, std::numbers::pi / 2.0);
  }

  void CircuitBuilder::CRY(size_t ctrl_idx, size_t target_idx, double theta) {
    this->RX(target_idx, std::numbers::pi / 2.0);
    this->CRZ(ctrl_idx, target_idx, theta);
    this->RX(target_idx, -1.0 * std::numbers::pi / 2.0);
  }

  void CircuitBuilder::CRY(size_t ctrl_idx, size_t target_idx, std::string param_name) {
    this->RX(target_idx, std::numbers::pi / 2.0);
    this->CRZ(ctrl_idx, target_idx, param_name);
    this->RX(target_idx, -1.0 * std::numbers::pi / 2.0);
  }

  void CircuitBuilder::SWAP(size_t q1, size_t q2) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Swap", {q1, q2}));
  }

  void CircuitBuilder::Measure(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Measure", idx));
  }

  void CircuitBuilder::MeasureAll(int NUM_QUBITS) {
    int nbQubits;
    if (NUM_QUBITS < 0) {
        auto qubits_set = uniqueBitsQD(circuit_);
        nbQubits = 0;
        for (int q: qubits_set) {
            if (q+1 > nbQubits) {
                nbQubits = q+1;
            }
        }
    } else {
        nbQubits = NUM_QUBITS;
    }
    for (int idx = 0; idx < nbQubits; ++idx) {
      circuit_->addInstruction(
          gate_provider_->createInstruction("Measure", (size_t)idx));
    }
  }

  void CircuitBuilder::QFT(const std::vector<int> &qubit_idxs) {
    auto qft = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("qft"));
    qft->expand({{"nq", static_cast<int>(qubit_idxs.size())}});

    // Need to remap qubit operands of these instruction to the
    // evaluation_qubits register range
    for (auto &inst : qft->getInstructions()) {
      const auto bits = inst->bits();
      std::vector<size_t> new_bits;
      for (const auto &bit : bits) {
        new_bits.emplace_back(qubit_idxs[bit]);
      }
      auto new_inst = inst->clone();
      new_inst->setBits(new_bits);
      circuit_->addInstruction(new_inst);
    }
  }

  void CircuitBuilder::IQFT(const std::vector<int> &qubit_idxs) {
    auto qft = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("iqft"));
    qft->expand({{"nq", static_cast<int>(qubit_idxs.size())}});

    // Need to remap qubit operands of these instruction to the
    // evaluation_qubits register range
    for (auto &inst : qft->getInstructions()) {
      const auto bits = inst->bits();
      std::vector<size_t> new_bits;
      for (const auto &bit : bits) {
        new_bits.emplace_back(qubit_idxs[bit]);
      }
      auto new_inst = inst->clone();
      new_inst->setBits(new_bits);
      circuit_->addInstruction(new_inst);
    }
  }

  void CircuitBuilder::QPE(CircuitBuilder &oracle, int num_evaluation_qubits,
            std::vector<int> trial_qubits, std::vector<int> evaluation_qubits) {
    auto qpe = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("PhaseEstimation"));
    assert(qpe);
    const bool expand_ok =
        qpe->expand({{"unitary", oracle.get()},
                      {"num_evaluation_qubits", num_evaluation_qubits},
                      {"trial_qubits", trial_qubits},
                      {"evaluation_qubits", evaluation_qubits}});
    circuit_->addInstructions(qpe->getInstructions());
  }

  void CircuitBuilder::CanonicalAmplitudeEstimation(CircuitBuilder &state_prep,
                                    CircuitBuilder &grover_op,
                                    int num_evaluation_qubits,
                                    int num_state_qubits, int num_trial_qubits,
                                    std::vector<int> trial_qubits,
                                    std::vector<int> evaluation_qubits, bool no_state_prep) {
    auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
    assert(ae);
    const bool expand_ok =
        ae->expand({{"state_preparation_circuit", state_prep.get()},
                    {"grover_op_circuit", grover_op.get()},
                    {"num_evaluation_qubits", num_evaluation_qubits},
                    {"num_state_qubits", num_state_qubits},
                    {"trial_qubits", trial_qubits},
                    {"evaluation_qubits", evaluation_qubits},
                    {"num_trial_qubits", num_trial_qubits},
                    {"no_state_prep", no_state_prep}});
    circuit_->addInstructions(ae->getInstructions());
  }

  void CircuitBuilder::MultiControlledUWithAncilla(CircuitBuilder &U,
                                    std::vector<int> qubits_control,
                                    std::vector<int> qubits_ancilla) {
    auto amcu = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("MultiControlledUWithAncilla"));
    assert(amcu);
    const bool expand_ok = amcu->expand({{"U", U.get()},
                                          {"qubits_control", qubits_control},
                                          {"qubits_ancilla", qubits_ancilla}});
    assert(expand_ok);
    circuit_->addInstructions(amcu->getInstructions());
  }

  std::string CircuitBuilder::RunCanonicalAmplitudeEstimation(
      CircuitBuilder &state_prep, CircuitBuilder &grover_op,
      int num_evaluation_qubits, int num_state_qubits, int num_trial_qubits,
      std::vector<int> trial_qubits, std::vector<int> evaluation_qubits,
      std::string acc_name) {
    auto acc = xacc::getAccelerator(acc_name);
    auto buffer = xacc::qalloc(num_evaluation_qubits + num_trial_qubits);
    auto ae_algo = xacc::getAlgorithm(
        "canonical-ae", {{"state_preparation_circuit", state_prep.get()},
                          {"grover_op_circuit", grover_op.get()},
                          {"num_evaluation_qubits", num_evaluation_qubits},
                          {"num_state_qubits", num_state_qubits},
                          {"trial_qubits", trial_qubits},
                          {"evaluation_qubits", evaluation_qubits},
                          {"num_trial_qubits", num_trial_qubits},
                          {"qpu", acc}});
    ae_algo->execute(buffer);
    return buffer->toString();
  }

  std::string CircuitBuilder::RunCanonicalAmplitudeEstimationWithOracle(
      CircuitBuilder &state_prep, CircuitBuilder &oracle,
      int num_evaluation_qubits, int num_state_qubits, int num_trial_qubits,
      std::vector<int> evaluation_qubits, std::vector<int> trial_qubits,
      std::string acc_name) {
    auto acc = xacc::getAccelerator(acc_name);
    auto buffer = xacc::qalloc(num_evaluation_qubits + num_trial_qubits);
    auto ae_algo = xacc::getAlgorithm(
        "canonical-ae", {{"state_preparation_circuit", state_prep.get()},
                          {"oracle", oracle.get()},
                          {"num_evaluation_qubits", num_evaluation_qubits},
                          {"num_state_qubits", num_state_qubits},
                          {"trial_qubits", trial_qubits},
                          {"evaluation_qubits", evaluation_qubits},
                          {"num_trial_qubits", num_trial_qubits},
                          {"qpu", acc}});

    ae_algo->execute(buffer);
    return buffer->toString();
  }

  std::string CircuitBuilder::RunMLAmplitudeEstimation(
      CircuitBuilder &state_prep, CircuitBuilder &oracle,
      std::function<int(std::string, int)> is_in_good_subspace,
      std::vector<int> score_qubits, int total_num_qubits, int num_runs,
      int shots, std::string acc_name) {
    auto buffer = xacc::qalloc(total_num_qubits);
    auto acc = xacc::getAccelerator(acc_name);
    auto ae_algo = xacc::getAlgorithm(
        "ML-ae", {{"state_preparation_circuit", state_prep.get()},
                  {"oracle_circuit", oracle.get()},
                  {"is_in_good_subspace", is_in_good_subspace},
                  {"score_qubits", score_qubits},
                  {"num_runs", num_runs},
                  {"shots", shots},
                  {"qpu", acc}});

    ae_algo->execute(buffer);
    return buffer->toString();
  }

  void CircuitBuilder::AmplitudeAmplification(CircuitBuilder &oracle,
                              CircuitBuilder &state_prep, int power) {
    auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("AmplitudeAmplification"));
    assert(ae);
    const bool expand_ok =
        ae->expand({{"oracle", oracle.get()},
                    {"state_preparation", state_prep.get()},
                    {"power", power}});
    assert(expand_ok);
    circuit_->addInstructions(ae->getInstructions());
  }

  void CircuitBuilder::QPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
              int &nb_qubits_next_letter_probabilities,
              int &nb_qubits_next_letter) {
    auto qprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("QPrime"));
    qprime->expand({{"nb_qubits_ancilla_metric", nb_qubits_ancilla_metric},
                    {"nb_qubits_ancilla_letter", nb_qubits_ancilla_letter},
                    {"nb_qubits_next_letter_probabilities",
                      nb_qubits_next_letter_probabilities},
                    {"nb_qubits_next_letter", nb_qubits_next_letter}});
    circuit_->addInstructions(qprime->getInstructions());
  }

  void CircuitBuilder::UPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
              int &nb_qubits_next_letter_probabilities,
              int &nb_qubits_next_letter) {
    auto uprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("UPrime"));
    uprime->expand({{"nb_qubits_ancilla_metric", nb_qubits_ancilla_metric},
                    {"nb_qubits_ancilla_letter", nb_qubits_ancilla_letter},
                    {"nb_qubits_next_letter_probabilities",
                      nb_qubits_next_letter_probabilities},
                    {"nb_qubits_next_letter", nb_qubits_next_letter}});
    circuit_->addInstructions(uprime->getInstructions());
  }

  void CircuitBuilder::WPrime(int iteration, std::vector<int> qubits_next_metric,
              std::vector<int> qubits_next_letter, std::vector<std::vector<float>> probability_table,
              std::vector<int> qubits_init_null, int null_integer, bool use_ancilla,
              std::vector<int> qubits_ancilla) {
    auto wprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("WPrime"));
    wprime->expand({{"probability_table", probability_table},
                    {"iteration", iteration},
                    {"qubits_next_metric", qubits_next_metric},
                    {"qubits_next_letter", qubits_next_letter},
                    {"qubits_init_null", qubits_init_null},
                    {"null_integer", null_integer},
                    {"use_ancilla", use_ancilla},
                    {"ancilla_qubits", qubits_ancilla}});
    circuit_->addInstructions(wprime->getInstructions());
  }

  void CircuitBuilder::UQPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
                int &nb_qubits_next_letter_probabilities,
                int &nb_qubits_next_letter) {
    auto uqprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("UQPrime"));
    uqprime->expand({{"nb_qubits_ancilla_metric", nb_qubits_ancilla_metric},
                      {"nb_qubits_ancilla_letter", nb_qubits_ancilla_letter},
                      {"nb_qubits_next_letter_probabilities",
                      nb_qubits_next_letter_probabilities},
                      {"nb_qubits_next_letter", nb_qubits_next_letter}});
    circuit_->addInstructions(uqprime->getInstructions());
  }

  void CircuitBuilder::RippleAdd(const std::vector<int> &a, const std::vector<int> &b,
                  int carry_bit) {
    auto adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("RippleCarryAdder"));
    adder->expand({{"adder_bits", a}, {"sum_bits", b}, {"c_in", carry_bit}});
    circuit_->addInstructions(adder->getInstructions());
  }

  void CircuitBuilder::Comparator_as_Oracle(int BestScore, int num_scoring_qubits,
                            std::vector<int> trial_score_qubits, int flag_qubit,
                            std::vector<int> best_score_qubits,
                            std::vector<int> ancilla_qubits, bool is_LSB,
                            std::vector<int> controls_on, std::vector<int> controls_off) {
    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Comparator"));
    assert(comp);
    const bool expand_ok =
        comp->expand({{"BestScore", BestScore},
                      {"num_scoring_qubits", num_scoring_qubits},
                      {"trial_score_qubits", trial_score_qubits},
                      {"flag_qubit", flag_qubit},
                      {"best_score_qubits", best_score_qubits},
                      {"ancilla_qubits", ancilla_qubits},
                      {"as_oracle", true},
                      {"is_LSB", is_LSB},
                      {"controls_on", controls_on},
                      {"controls_off", controls_off}});
    circuit_->addInstructions(comp->getInstructions());
  }

  void CircuitBuilder::Comparator(int BestScore, int num_scoring_qubits,
                  std::vector<int> trial_score_qubits, int flag_qubit,
                  std::vector<int> best_score_qubits,
                  std::vector<int> ancilla_qubits, bool is_LSB,
                  std::vector<int> controls_on, std::vector<int> controls_off) {
    auto comp = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Comparator"));
    assert(comp);
    const bool expand_ok =
        comp->expand({{"BestScore", BestScore},
                      {"num_scoring_qubits", num_scoring_qubits},
                      {"trial_score_qubits", trial_score_qubits},
                      {"flag_qubit", flag_qubit},
                      {"best_score_qubits", best_score_qubits},
                      {"ancilla_qubits", ancilla_qubits},
                      {"is_LSB", is_LSB},
                      {"controls_on", controls_on},
                      {"controls_off", controls_off}});
    circuit_->addInstructions(comp->getInstructions());
  }

  void CircuitBuilder::EfficientEncoding(std::function<int(int)> scoring_function,
                          int num_state_qubits, int num_scoring_qubits,
                          std::vector<int> state_qubits,
                          std::vector<int> scoring_qubits, bool is_LSB, bool use_ancilla,
                          std::vector<int> qubits_init_flag, int flag_integer) {
    auto ee = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("EfficientEncoding"));
    assert(ee);
    const bool expand_ok =
        ee->expand({{"scoring_function", scoring_function},
                    {"num_state_qubits", num_state_qubits},
                    {"num_scoring_qubits", num_scoring_qubits},
                    {"state_qubits", state_qubits},
                    {"scoring_qubits", scoring_qubits},
                    {"is_LSB", is_LSB},
                    {"use_ancilla", use_ancilla},
                    {"qubits_init_flag", qubits_init_flag},
                    {"flag_integer", flag_integer}});
    circuit_->addInstructions(ee->getInstructions());
  }

  void CircuitBuilder::EqualityChecker(std::vector<int> qubits_a, std::vector<int> qubits_b,
                        int flag, bool use_ancilla,
                        std::vector<int> qubits_ancilla,
                        std::vector<int> controls_on,
                        std::vector<int> controls_off) {
    auto ec = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("EqualityChecker"));
    assert(ec);
    const bool expand_ok = ec->expand({{"qubits_a", qubits_a},
                                        {"qubits_b", qubits_b},
                                        {"flag", flag},
                                        {"use_ancilla", use_ancilla},
                                        {"qubits_ancilla", qubits_ancilla},
                                        {"controls_on", controls_on},
                                        {"controls_off", controls_off}});
    circuit_->addInstructions(ec->getInstructions());
  }

  void CircuitBuilder::ControlledSwap(std::vector<int> qubits_a, std::vector<int> qubits_b,
                        std::vector<int> flags_on, std::vector<int> flags_off) {
    auto cs = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ControlledSwap"));
    assert(cs);
    const bool expand_ok = cs->expand({{"qubits_a", qubits_a},
                                        {"qubits_b", qubits_b},
                                        {"flags_on", flags_on},
                                        {"flags_off", flags_off}});
    circuit_->addInstructions(cs->getInstructions());
  }

  void CircuitBuilder::ControlledAddition(std::vector<int> qubits_adder, std::vector<int> qubits_sum, int c_in,
                        std::vector<int> flags_on, std::vector<int> flags_off, bool no_overflow) {
    auto ca = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ControlledAddition"));
    assert(ca);
    const bool expand_ok = ca->expand({{"qubits_adder", qubits_adder},
                                        {"qubits_sum", qubits_sum},
                                        {"c_in", c_in},
                                        {"flags_on", flags_on},
                                        {"flags_off", flags_off},
                                        {"no_overflow", no_overflow}});
    circuit_->addInstructions(ca->getInstructions());
  }

  void CircuitBuilder::GeneralisedMCX(int target, std::vector<int> controls_on,
                        std::vector<int> controls_off) {
    auto gmcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("GeneralisedMCX"));
    assert(gmcx);
    const bool expand_ok = gmcx->expand({{"target", target},
                                        {"controls_on", controls_on},
                                        {"controls_off", controls_off}});
    circuit_->addInstructions(gmcx->getInstructions());
  }

  void CircuitBuilder::CompareBeamOracle(int q0, int q1, int q2, std::vector<int> FA,
                        std::vector<int> FB, std::vector<int> SA, std::vector<int> SB, bool simplified) {
    auto cbo = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("CompareBeamOracle"));
    assert(cbo);
    const bool expand_ok = cbo->expand({{"q0", q0},
                                        {"q1", q1},
                                        {"q2", q2},
                                        {"FA", FA},
                                        {"FB", FB},
                                        {"SA", SA},
                                        {"SB", SB},
                                        {"simplified", simplified}});
    circuit_->addInstructions(cbo->getInstructions());
  }

  void CircuitBuilder::SuperpositionAdder(int q0, int q1, int q2,
                          std::vector<int> qubits_flags, std::vector<int> qubits_string,
                          std::vector<int> qubits_metric,
                          CircuitBuilder &ae_state_prep_circ,
                          std::vector<int> qubits_ancilla,
                          std::vector<int> qubits_beam_metric) {
    auto sa = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("SuperpositionAdder"));
    assert(sa);
    const bool expand_ok = sa->expand({
      {"q0", q0}, {"q1", q1}, {"q2", q2},
      {"qubits_flags", qubits_flags},
      {"qubits_string", qubits_string},
      {"qubits_metric", qubits_metric},
      {"ae_state_prep_circ", ae_state_prep_circ.get()},
      {"qubits_ancilla", qubits_ancilla},
      {"qubits_beam_metric", qubits_beam_metric}});
    circuit_->addInstructions(sa->getInstructions());
  }

  void CircuitBuilder::InverseCircuit(CircuitBuilder &circ) {
    auto is = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("InverseCircuit"));
    const bool expand_ok = is->expand({{"circ", circ.get()}});
    circuit_->addInstructions(is->getInstructions());
  }

  void CircuitBuilder::Subtraction(std::vector<int> qubits_larger,
                    std::vector<int> qubits_smaller, bool is_LSB, int qubit_ancilla) {
    auto s = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Subtraction"));
    const bool expand_ok = s->expand({{"qubits_larger", qubits_larger},
                                      {"qubits_smaller", qubits_smaller},
                                      {"qubit_ancilla", qubit_ancilla},
                                      {"is_LSB", is_LSB}});
    circuit_->addInstructions(s->getInstructions());
  }

  void CircuitBuilder::ControlledSubtraction(std::vector<int> qubits_larger,
                              std::vector<int> qubits_smaller,
                              std::vector<int> controls_on,
                              std::vector<int> controls_off, bool is_LSB, int qubit_ancilla) {
    auto cs = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ControlledSubtraction"));
    const bool expand_ok = cs->expand({{"qubits_larger", qubits_larger},
                                        {"qubits_smaller", qubits_smaller},
                                        {"is_LSB", is_LSB},
                                        {"qubit_ancilla", qubit_ancilla},
                                        {"controls_on", controls_on},
                                        {"controls_off", controls_off}});
    circuit_->addInstructions(cs->getInstructions());
  }

  void CircuitBuilder::ProperFractionDivision(std::vector<int> qubits_numerator,
                              std::vector<int> qubits_denominator,
                              std::vector<int> qubits_fraction,
                              std::vector<int> qubits_ancilla, bool is_LSB) {
    auto PFD = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ProperFractionDivision"));
    const bool expand_ok =
        PFD->expand({{"qubits_numerator", qubits_numerator},
                      {"qubits_denominator", qubits_denominator},
                      {"qubits_fraction", qubits_fraction},
                      {"qubits_ancilla", qubits_ancilla},
                      {"is_LSB", is_LSB}});
    circuit_->addInstructions(PFD->getInstructions());
  }

  void CircuitBuilder::ControlledProperFractionDivision(std::vector<int> qubits_numerator,
                                        std::vector<int> qubits_denominator,
                                        std::vector<int> qubits_fraction,
                                        std::vector<int> qubits_ancilla,
                                        std::vector<int> controls_on,
                                        std::vector<int> controls_off,
                                        bool is_LSB) {
    auto cPFD = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>(
            "ControlledProperFractionDivision"));
    const bool expand_ok =
        cPFD->expand({{"qubits_numerator", qubits_numerator},
                      {"qubits_denominator", qubits_denominator},
                      {"qubits_fraction", qubits_fraction},
                      {"qubits_ancilla", qubits_ancilla},
                      {"controls_on", controls_on},
                      {"controls_off", controls_off},
                      {"is_LSB", is_LSB}});
    circuit_->addInstructions(cPFD->getInstructions());
  }

  void CircuitBuilder::CompareGT(std::vector<int> qubits_a,
                              std::vector<int> qubits_b,
                              int qubit_flag,
                              int qubit_ancilla, bool is_LSB) {
    auto cgt = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("CompareGT"));
    const bool expand_ok =
        cgt->expand({{"qubits_a", qubits_a},
                      {"qubits_b", qubits_b},
                      {"qubit_flag", qubit_flag},
                      {"qubit_ancilla", qubit_ancilla},
                      {"is_LSB", is_LSB}});
    circuit_->addInstructions(cgt->getInstructions());
  }

  void CircuitBuilder::Multiplication(std::vector<int> qubits_a, std::vector<int> qubits_b,
                      std::vector<int> qubits_result, int qubit_ancilla, bool is_LSB) {
    auto multiplication = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Multiplication"));
    assert(multiplication);
    const bool expand_ok = multiplication->expand({
        {"qubit_ancilla", qubit_ancilla},
        {"qubits_a", qubits_a},
        {"qubits_b", qubits_b},
        {"qubits_result", qubits_result},
        {"is_LSB", is_LSB}});
    circuit_->addInstructions(multiplication->getInstructions());
  }

  void CircuitBuilder::ControlledMultiplication(std::vector<int> qubits_a, std::vector<int> qubits_b,
                        std::vector<int> qubits_result, int qubit_ancilla, bool is_LSB, std::vector<int> controls_on, std::vector<int> controls_off) {
    auto multiplication = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("ControlledMultiplication"));
    assert(multiplication);
    const bool expand_ok = multiplication->expand({
        {"qubit_ancilla", qubit_ancilla},
        {"qubits_a", qubits_a},
        {"qubits_b", qubits_b},
        {"qubits_result", qubits_result},
        {"is_LSB", is_LSB},
        {"controls_on", controls_on},
        {"controls_off", controls_off}});
    circuit_->addInstructions(multiplication->getInstructions());
  }

  int CircuitBuilder::ExponentialSearch(
      std::string method, StatePrepFuncCType state_prep_circ, OracleFuncCType oracle_func,
      int best_score, std::function<int(int)> f_score, int total_num_qubits,
      std::vector<int> qubits_string, std::vector<int> total_metric,
      std::string acc_name) {
    auto acc = xacc::getAccelerator(acc_name);
    auto exp_search_algo = xacc::getAlgorithm(
      "exponential-search", {{"method", method},
                              {"state_preparation_circuit", state_prep_circ},
                              {"oracle_circuit", oracle_func},
                              {"best_score", best_score},
                              {"f_score", f_score},
                              {"total_num_qubits", total_num_qubits},
                              {"qubits_string", qubits_string},
                              {"total_metric", total_metric},
                              {"qpu", acc}});
    auto buffer = xacc::qalloc(total_num_qubits);
    exp_search_algo->execute(buffer);
    auto info = buffer->getInformation();
    if (info.find("best-score") != info.end()) {
      return info.at("best-score").as<int>();
    }
    return best_score;
  }

}

// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#pragma once

#include "CompositeInstruction.hpp"
#include "IRProvider.hpp"
#include "InstructionIterator.hpp"
//#include <assert.h>
//#include <iostream>
#include "Accelerator.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <assert.h>
#include "Circuit.hpp"
#include "GateModifier.hpp"
namespace qb {
using StatePrepFuncCType =
    std::function<std::shared_ptr<xacc::CompositeInstruction>(
        std::vector<int>, std::vector<int>,
        std::vector<int>, std::vector<int>, std::vector<int>)>;
using OracleFuncCType =
    std::function<std::shared_ptr<xacc::CompositeInstruction>(
        int, int, std::vector<int>, int, std::vector<int>, std::vector<int>)>;

inline std::set<std::size_t> uniqueBitsQD(std::shared_ptr<xacc::CompositeInstruction> &circ) {
    std::set<std::size_t> uniqueBits;
    xacc::InstructionIterator iter(circ);
    while (iter.hasNext()) {
      auto next = iter.next();
      if (!next->isComposite()) {
        for (auto &b : next->bits()) {
          uniqueBits.insert(b);
        }
      } else if (next->isComposite() && next->name() == "C-U") {
        auto *asControlledBlock =
            dynamic_cast<xacc::quantum::ControlModifier *>(next.get());
        if (asControlledBlock) {
          const auto controlQubits = asControlledBlock->getControlQubits();
          auto baseCircuit = asControlledBlock->getBaseInstruction();
          assert(baseCircuit->isComposite());
          auto asComp = xacc::ir::asComposite(baseCircuit);
          auto targetUniqueBits = asComp->uniqueBits();
          for (const auto &[reg, qIdx] : controlQubits) {
              uniqueBits.insert(qIdx);
          }
          for (const auto &qIdx : targetUniqueBits) {
              uniqueBits.insert(qIdx);
          }
        }
      }
    }
    return uniqueBits;
  }

class CircuitBuilder {
private:
  std::shared_ptr<xacc::IRProvider> gate_provider_;
  std::shared_ptr<xacc::CompositeInstruction> circuit_;

public:
  CircuitBuilder()
      : gate_provider_(xacc::getService<xacc::IRProvider>("quantum")) {
    circuit_ = gate_provider_->createComposite("QBSDK_circuit");
  }

  CircuitBuilder(std::shared_ptr<xacc::CompositeInstruction> &composite) : gate_provider_(xacc::getService<xacc::IRProvider>("quantum")) {
      circuit_ = gate_provider_->createComposite("QBSDK_circuit");
      circuit_->addInstructions(composite->getInstructions());
  }

  std::shared_ptr<xacc::CompositeInstruction> get() { return circuit_; }
  void print() { std::cout << circuit_->toString() << std::endl; }
  void append(CircuitBuilder &other) {
    xacc::InstructionIterator it(other.circuit_);
    while (it.hasNext()) {
      auto nextInst = it.next();
      if (nextInst->isEnabled() && !nextInst->isComposite()) {
        circuit_->addInstruction(nextInst->clone());
      }
    }
  }

  // Gates:
  void H(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("H", idx));
  }
  void X(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("X", idx));
  }
  void Y(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Y", idx));
  }
  void Z(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Z", idx));
  }

  void T(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("T", idx));
  }
  void S(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("S", idx));
  }
  void Tdg(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Tdg", idx));
  }
  void Sdg(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Sdg", idx));
  }

  void RX(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Rx", {idx}, {theta}));
  }
  void RY(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Ry", {idx}, {theta}));
  }
  void RZ(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Rz", {idx}, {theta}));
  }

  void U1(size_t idx, double theta) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("U1", {idx}, {theta}));
  }
  void U3(size_t idx, double theta, double phi, double lambda) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("U", {idx}, {theta, phi, lambda}));
  }

  void CNOT(size_t ctrl_idx, size_t target_idx) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("CNOT", {ctrl_idx, target_idx}));
  }

  void MCX(const std::vector<int> &ctrl_inds, size_t target_idx) {
    auto x_gate = gate_provider_->createComposite("temp_X");
    auto temp_gate = gate_provider_->createInstruction("X", target_idx);
    temp_gate->setBufferNames({"q"});
    x_gate->addInstruction(temp_gate);
    auto controlled_U = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    controlled_U->expand({{"U", x_gate}, {"control-idx", ctrl_inds}});
    circuit_->addInstruction(controlled_U);
  }

  void CU(CircuitBuilder &circ, std::vector<int> ctrl_inds) {
    auto controlled_U = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    controlled_U->expand({{"U", circ.circuit_}, {"control-idx", ctrl_inds}});
    circuit_->addInstruction(controlled_U);
  }

  void CZ(size_t ctrl_idx, size_t target_idx) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("CZ", {ctrl_idx, target_idx}));
  }

  void CH(size_t ctrl_idx, size_t target_idx) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("CH", {ctrl_idx, target_idx}));
  }

  // CPhase == CU1
  void CPhase(size_t ctrl_idx, size_t target_idx, double theta) {
    circuit_->addInstruction(gate_provider_->createInstruction(
        "CPhase", {ctrl_idx, target_idx}, {theta}));
  }

  void SWAP(size_t q1, size_t q2) {
    circuit_->addInstruction(
        gate_provider_->createInstruction("Swap", {q1, q2}));
  }

  void Measure(size_t idx) {
    circuit_->addInstruction(gate_provider_->createInstruction("Measure", idx));
  }

void MeasureAll(int NUM_QUBITS) {
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
    // std::cout << "nbQubits " << nbQubits << "\n";
    for (int idx = 0; idx < nbQubits; ++idx) {
      circuit_->addInstruction(
          gate_provider_->createInstruction("Measure", (size_t)idx));
    }
  }

  void QFT(const std::vector<int> &qubit_idxs) {
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

  void IQFT(const std::vector<int> &qubit_idxs) {
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

  // Quantum Phase Estimation
  void QPE(CircuitBuilder &oracle, int num_evaluation_qubits,
           std::vector<int> trial_qubits, std::vector<int> evaluation_qubits) {
    auto qpe = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("PhaseEstimation"));
    assert(qpe);
    const bool expand_ok =
        qpe->expand({{"unitary", oracle.circuit_},
                     {"num_evaluation_qubits", num_evaluation_qubits},
                     {"trial_qubits", trial_qubits},
                     {"evaluation_qubits", evaluation_qubits}});
    circuit_->addInstructions(qpe->getInstructions());
  }

  void CanonicalAmplitudeEstimation(CircuitBuilder &state_prep,
                                    CircuitBuilder &grover_op,
                                    int num_evaluation_qubits,
                                    int num_state_qubits, int num_trial_qubits,
                                    std::vector<int> trial_qubits,
                                    std::vector<int> evaluation_qubits, bool no_state_prep) {
    auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));
    assert(ae);
    const bool expand_ok =
        ae->expand({{"state_preparation_circuit", state_prep.circuit_},
                    {"grover_op_circuit", grover_op.circuit_},
                    {"num_evaluation_qubits", num_evaluation_qubits},
                    {"num_state_qubits", num_state_qubits},
                    {"trial_qubits", trial_qubits},
                    {"evaluation_qubits", evaluation_qubits},
                    {"num_trial_qubits", num_trial_qubits},
                    {"no_state_prep", no_state_prep}});
    circuit_->addInstructions(ae->getInstructions());
  }

  void MultiControlledUWithAncilla(CircuitBuilder &U,
                                   std::vector<int> qubits_control,
                                   std::vector<int> qubits_ancilla) {
    auto amcu = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("MultiControlledUWithAncilla"));
    assert(amcu);
    const bool expand_ok = amcu->expand({{"U", U.circuit_},
                                         {"qubits_control", qubits_control},
                                         {"qubits_ancilla", qubits_ancilla}});
    assert(expand_ok);
    circuit_->addInstructions(amcu->getInstructions());
  }

  std::string RunCanonicalAmplitudeEstimation(
      CircuitBuilder &state_prep, CircuitBuilder &grover_op,
      int num_evaluation_qubits, int num_state_qubits, int num_trial_qubits,
      std::vector<int> trial_qubits, std::vector<int> evaluation_qubits,
      std::string acc_name) {
    auto acc = xacc::getAccelerator(acc_name);
    auto buffer = xacc::qalloc(num_evaluation_qubits + num_trial_qubits);
    auto ae_algo = xacc::getAlgorithm(
        "canonical-ae", {{"state_preparation_circuit", state_prep.circuit_},
                         {"grover_op_circuit", grover_op.circuit_},
                         {"num_evaluation_qubits", num_evaluation_qubits},
                         {"num_state_qubits", num_state_qubits},
                         {"trial_qubits", trial_qubits},
                         {"evaluation_qubits", evaluation_qubits},
                         {"num_trial_qubits", num_trial_qubits},
                         {"qpu", acc}});
    ae_algo->execute(buffer);
    return buffer->toString();
  }

  std::string RunCanonicalAmplitudeEstimationWithOracle(
      CircuitBuilder &state_prep, CircuitBuilder &oracle,
      int num_evaluation_qubits, int num_state_qubits, int num_trial_qubits,
      std::vector<int> evaluation_qubits, std::vector<int> trial_qubits,
      std::string acc_name) {
    auto acc = xacc::getAccelerator(acc_name);
    auto buffer = xacc::qalloc(num_evaluation_qubits + num_trial_qubits);
    auto ae_algo = xacc::getAlgorithm(
        "canonical-ae", {{"state_preparation_circuit", state_prep.circuit_},
                         {"oracle", oracle.circuit_},
                         {"num_evaluation_qubits", num_evaluation_qubits},
                         {"num_state_qubits", num_state_qubits},
                         {"trial_qubits", trial_qubits},
                         {"evaluation_qubits", evaluation_qubits},
                         {"num_trial_qubits", num_trial_qubits},
                         {"qpu", acc}});

    ae_algo->execute(buffer);
    return buffer->toString();
  }

  std::string RunMLAmplitudeEstimation(
      CircuitBuilder &state_prep, CircuitBuilder &oracle,
      std::function<int(std::string, int)> is_in_good_subspace,
      std::vector<int> score_qubits, int total_num_qubits, int num_runs,
      int shots, std::string acc_name) {
    auto buffer = xacc::qalloc(total_num_qubits);
    auto acc = xacc::getAccelerator(acc_name);
    auto ae_algo = xacc::getAlgorithm(
        "ML-ae", {{"state_preparation_circuit", state_prep.circuit_},
                  {"oracle_circuit", oracle.circuit_},
                  {"is_in_good_subspace", is_in_good_subspace},
                  {"score_qubits", score_qubits},
                  {"num_runs", num_runs},
                  {"shots", shots},
                  {"qpu", acc}});

    ae_algo->execute(buffer);
    return buffer->toString();
  }

  void AmplitudeAmplification(CircuitBuilder &oracle,
                              CircuitBuilder &state_prep, int power) {
    auto ae = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("AmplitudeAmplification"));
    assert(ae);
    const bool expand_ok =
        ae->expand({{"oracle", oracle.circuit_},
                    {"state_preparation", state_prep.circuit_},
                    {"power", power}});
    assert(expand_ok);
    circuit_->addInstructions(ae->getInstructions());
  }

  void QPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
              int &nb_qubits_next_letter_probabilities,
              int &nb_qubits_next_letter) {
    auto qprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("QPrime"));
    // assert(qprime);
    // const bool expand_ok =
    qprime->expand({{"nb_qubits_ancilla_metric", nb_qubits_ancilla_metric},
                    {"nb_qubits_ancilla_letter", nb_qubits_ancilla_letter},
                    {"nb_qubits_next_letter_probabilities",
                     nb_qubits_next_letter_probabilities},
                    {"nb_qubits_next_letter", nb_qubits_next_letter}});
    // assert(expand_ok);
    circuit_->addInstructions(qprime->getInstructions());
  }

  void UPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
              int &nb_qubits_next_letter_probabilities,
              int &nb_qubits_next_letter) {
    auto uprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("UPrime"));
    // assert(qprime);
    // const bool expand_ok =
    uprime->expand({{"nb_qubits_ancilla_metric", nb_qubits_ancilla_metric},
                    {"nb_qubits_ancilla_letter", nb_qubits_ancilla_letter},
                    {"nb_qubits_next_letter_probabilities",
                     nb_qubits_next_letter_probabilities},
                    {"nb_qubits_next_letter", nb_qubits_next_letter}});
    // assert(expand_ok);
    circuit_->addInstructions(uprime->getInstructions());
  }

  void WPrime(int iteration, std::vector<int> qubits_next_metric,
              std::vector<int> qubits_next_letter, std::vector<std::vector<float>> probability_table,
              std::vector<int> qubits_init_null, int null_integer, bool use_ancilla,
              std::vector<int> qubits_ancilla) {
    auto wprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("WPrime"));
    // assert(wprime);
    // const bool expand_ok =
    wprime->expand({{"probability_table", probability_table},
                    {"iteration", iteration},
                    {"qubits_next_metric", qubits_next_metric},
                    {"qubits_next_letter", qubits_next_letter},
                    {"qubits_init_null", qubits_init_null},
                    {"null_integer", null_integer},
                    {"use_ancilla", use_ancilla},
                    {"ancilla_qubits", qubits_ancilla}});
    // assert(expand_ok);
    // std::cout << wprime->toString() << "circuit_builder\n";
    circuit_->addInstructions(wprime->getInstructions());
  }

  void UQPrime(int &nb_qubits_ancilla_metric, int &nb_qubits_ancilla_letter,
               int &nb_qubits_next_letter_probabilities,
               int &nb_qubits_next_letter) {
    auto uqprime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("UQPrime"));
    // assert(uqprime);
    // const bool expand_ok =
    uqprime->expand({{"nb_qubits_ancilla_metric", nb_qubits_ancilla_metric},
                     {"nb_qubits_ancilla_letter", nb_qubits_ancilla_letter},
                     {"nb_qubits_next_letter_probabilities",
                      nb_qubits_next_letter_probabilities},
                     {"nb_qubits_next_letter", nb_qubits_next_letter}});
    // assert(expand_ok);
    circuit_->addInstructions(uqprime->getInstructions());
  }

  void RippleAdd(const std::vector<int> &a, const std::vector<int> &b,
                 int carry_bit) {
    auto adder = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("RippleCarryAdder"));
    adder->expand({{"adder_bits", a}, {"sum_bits", b}, {"c_in", carry_bit}});
    circuit_->addInstructions(adder->getInstructions());
  }

  // Comparator
  void Comparator_as_Oracle(int BestScore, int num_scoring_qubits,
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
  void Comparator(int BestScore, int num_scoring_qubits,
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

  void EfficientEncoding(std::function<int(int)> scoring_function,
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

  void EqualityChecker(std::vector<int> qubits_a, std::vector<int> qubits_b,
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

    void ControlledSwap(std::vector<int> qubits_a, std::vector<int> qubits_b,
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

    void ControlledAddition(std::vector<int> qubits_adder, std::vector<int> qubits_sum, int c_in,
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

    void GeneralisedMCX(int target, std::vector<int> controls_on,
                       std::vector<int> controls_off) {
    auto gmcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("GeneralisedMCX"));
    assert(gmcx);
    const bool expand_ok = gmcx->expand({{"target", target},
                                        {"controls_on", controls_on},
                                        {"controls_off", controls_off}});
    circuit_->addInstructions(gmcx->getInstructions());
  }

  void CompareBeamOracle(int q0, int q1, int q2, std::vector<int> FA,
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

  void InverseCircuit(CircuitBuilder &circ) {
    auto is = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("InverseCircuit"));
    const bool expand_ok = is->expand({{"circ", circ.circuit_}});
    circuit_->addInstructions(is->getInstructions());
  }

  void Subtraction(std::vector<int> qubits_larger,
                   std::vector<int> qubits_smaller, bool is_LSB, int qubit_ancilla) {
    auto s = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("Subtraction"));
    const bool expand_ok = s->expand({{"qubits_larger", qubits_larger},
                                      {"qubits_smaller", qubits_smaller},
                                      {"qubit_ancilla", qubit_ancilla},
                                      {"is_LSB", is_LSB}});
    circuit_->addInstructions(s->getInstructions());
  }

  void ControlledSubtraction(std::vector<int> qubits_larger,
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

  void ProperFractionDivision(std::vector<int> qubits_numerator,
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

  void ControlledProperFractionDivision(std::vector<int> qubits_numerator,
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

  void CompareGT(std::vector<int> qubits_a,
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

  void Multiplication(std::vector<int> qubits_a, std::vector<int> qubits_b,
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

    void ControlledMultiplication(std::vector<int> qubits_a, std::vector<int> qubits_b,
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

  // Exponential Search: returns a better score if found, otherwise returns
  // current best score
  int ExponentialSearch(
      std::string method, OracleFuncCType oracle_gen,
      StatePrepFuncCType state_prep_gen, std::function<int(int)> f_score,
      int best_score, std::vector<int> qubits_string,
      std::vector<int> qubits_metric, std::vector<int> qubits_next_letter,
      std::vector<int> qubits_next_metric, int qubit_flag,
      std::vector<int> qubits_best_score,
      std::vector<int> qubits_ancilla_oracle, std::vector<int> qubits_ancilla_adder, std::vector<int> total_metric,
      int CQAE_num_evaluation_qubits,
      std::function<int(std::string, int)> MLQAE_is_in_good_subspace,
      int MLQAE_num_runs, int MLQAE_num_shots, std::string acc_name) {
    auto acc = xacc::getAccelerator(acc_name);
    auto exp_search_algo = xacc::getAlgorithm(
        "exponential-search",
        {{"method", method},
         {"oracle_circuit", oracle_gen},
         {"state_preparation_circuit", state_prep_gen},
         {"f_score", f_score},
         {"best_score", best_score},
         {"qubits_metric", qubits_metric},
         {"qubit_flag", qubit_flag},
         {"qubits_best_score", qubits_best_score},
         {"qubits_next_letter", qubits_next_letter},
         {"qubits_next_metric", qubits_next_metric},
         {"qubits_ancilla_oracle", qubits_ancilla_oracle},
         {"qubits_ancilla_adder", qubits_ancilla_adder},
         {"total_metric", total_metric},
         {"qubits_string", qubits_string},
         {"CQAE_num_evaluation_qubits", CQAE_num_evaluation_qubits},
         {"MLQAE_is_in_good_subspace", MLQAE_is_in_good_subspace},
         {"MLQAE_num_runs", MLQAE_num_runs},
         {"MLQAE_num_shots", MLQAE_num_shots},
         {"qpu", acc}});
    const auto nb_qubits = qubits_metric.size() + qubits_best_score.size() +
                           qubits_string.size() + qubits_ancilla_oracle.size() +
                           qubits_next_letter.size() + qubits_ancilla_adder.size() +
                           qubits_next_metric.size() + 1;
    auto buffer = xacc::qalloc(nb_qubits);
    exp_search_algo->execute(buffer);
    auto info = buffer->getInformation();
    if (info.find("best-score") != info.end()) {
      return info.at("best-score").as<int>();
    }
    return best_score;
  }
};
} // namespace qb

/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/circuits/mean_value_finder.hpp>
#include <IRProvider.hpp>
#include <xacc.hpp>
#include <xacc_service.hpp>
#include <assert.h>
#include <memory>

namespace qristal {
bool MeanValueFinder::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  ////////////////////////////////////////////////////////
  // Get expected inputs or define any required variables
  ////////////////////////////////////////////////////////

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_superposition")) {
    return false;
  }
  std::vector<int> qubits_superposition =
      runtimeOptions.get<std::vector<int>>("qubits_superposition");

  if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
          "qubits_superposition_state_prep")) {
    return false;
  }
  auto qubits_superposition_state_prep =
      runtimeOptions.getPointerLike<xacc::CompositeInstruction>(
          "qubits_superposition_state_prep");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_mean")) {
    return false;
  }
  std::vector<int> qubits_mean =
      runtimeOptions.get<std::vector<int>>("qubits_mean");

  assert(qubits_mean.size() >= qubits_superposition.size());

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla")) {
    return false;
  }
  std::vector<int> qubits_ancilla =
      runtimeOptions.get<std::vector<int>>("qubits_ancilla");

  int qubit_indicator = -1;
  if (runtimeOptions.keyExists<int>("qubit_indicator")) {
    qubit_indicator = runtimeOptions.get<int>("qubit_indicator");
  }

  if (qubit_indicator >= 0 &&
      !runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>(
          "qubit_indicator_state_prep")) {
    return false;
  }
  xacc::CompositeInstruction *qubit_indicator_state_prep;
  if (qubit_indicator >= 0) {
    qubit_indicator_state_prep =
        runtimeOptions.getPointerLike<xacc::CompositeInstruction>(
            "qubit_indicator_state_prep");
  }

  bool is_LSB = true;
  if (runtimeOptions.keyExists<bool>("is_LSB")) {
    is_LSB = runtimeOptions.get<bool>("is_LSB");
  }

  int ones_idx = qubits_superposition.size();
  if (runtimeOptions.keyExists<int>("ones_idx")) {
      ones_idx = runtimeOptions.get<int>("ones_idx");
  }

  //std::cout << "initialise ok\n";

  ////////////////////////////////////////////////////////
  // Add instructions
  ////////////////////////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  if (qubit_indicator >= 0) {

    auto to_be_undone = gateRegistry->createComposite("to_be_undone");
    std::vector<int> ancilla_in_use;

    auto qae0 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));

    int num_evaluation_qubits0 = qubits_superposition.size();
    std::vector<int> evaluation_qubits0;
    for (int i = 0; i < num_evaluation_qubits0; i++) {
      evaluation_qubits0.push_back(qubits_ancilla[i]);
      ancilla_in_use.push_back(qubits_ancilla[i]);
    }

    std::shared_ptr<xacc::CompositeInstruction> ae_state_prep_circ_clone0 =
        xacc::ir::asComposite(qubit_indicator_state_prep->clone());

    auto trial_qubits_set0 = qristal::uniqueBitsQD(ae_state_prep_circ_clone0);
    std::vector<int> trial_qubits0;
    for (int bit : trial_qubits_set0) {
      trial_qubits0.push_back(bit);
    }
    int num_trial_qubits0 = trial_qubits0.size();

    auto oracle0 = gateRegistry->createComposite("oracle0");
    oracle0->addInstruction(
        gateRegistry->createInstruction("Z", qubit_indicator));

    const bool expand_ok_qae0 =
        qae0->expand({{"state_preparation_circuit", ae_state_prep_circ_clone0},
                      {"num_evaluation_qubits", num_evaluation_qubits0},
                      {"evaluation_qubits", evaluation_qubits0},
                      {"num_trial_qubits", num_trial_qubits0},
                      {"trial_qubits", trial_qubits0},
                      {"oracle", oracle0},
                      {"no_state_prep", true}});
    assert(expand_ok_qae0);
    addInstruction(qae0);
    to_be_undone->addInstruction(qae0);

    std::vector<int> division_evaluation_qubits;
    for (int q = 0; q < qubits_superposition.size(); q++) {
      auto qae1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("CanonicalAmplitudeEstimation"));

      std::shared_ptr<xacc::CompositeInstruction> ae_state_prep_circ_clone =
          xacc::ir::asComposite(qubits_superposition_state_prep->clone());

      int num_evaluation_qubits = q + 1;
      std::vector<int> evaluation_qubits;
      while (evaluation_qubits.size() < num_evaluation_qubits) {
          for (int i = 0; i < qubits_ancilla.size(); i++) {
              if (std::find(ancilla_in_use.begin(), ancilla_in_use.end(), qubits_ancilla[i]) == ancilla_in_use.end()) {
                  evaluation_qubits.push_back(qubits_ancilla[i]);
                  ancilla_in_use.push_back(qubits_ancilla[i]);
                  division_evaluation_qubits.push_back(qubits_ancilla[i]);
                  break;
              }
          }
      }

      auto trial_qubits_set = qristal::uniqueBitsQD(ae_state_prep_circ_clone);
      std::vector<int> trial_qubits;
      for (int bit : trial_qubits_set) {
        trial_qubits.push_back(bit);
      }
      int num_trial_qubits = trial_qubits.size();

      auto oracle = gateRegistry->createComposite("oracle");
      oracle->addInstruction(gateRegistry->createInstruction(
          "CZ", {static_cast<unsigned long>(qubit_indicator),
                 static_cast<unsigned long>(qubits_superposition[q])}));

      const bool expand_ok_qae1 =
          qae1->expand({{"state_preparation_circuit", ae_state_prep_circ_clone},
                        {"num_evaluation_qubits", num_evaluation_qubits},
                        {"evaluation_qubits", evaluation_qubits},
                        {"num_trial_qubits", num_trial_qubits},
                        {"trial_qubits", trial_qubits},
                        {"oracle", oracle},
                        {"no_state_prep", true}});
      assert(expand_ok_qae1);
      addInstruction(qae1);
      to_be_undone->addInstruction(qae1);

      auto inv_grover_op = gateRegistry->createComposite("grover_op");

      auto inv_state_prep =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("InverseCircuit"));
      const bool expand_ok_isp =
          inv_state_prep->expand({{"circ", ae_state_prep_circ_clone}});
      assert(expand_ok_isp);

      auto zero_reflection = gateRegistry->createComposite("zero_reflection");
      for (int i = 0; i < trial_qubits.size(); ++i) {
        zero_reflection->addInstruction(
            gateRegistry->createInstruction("X", trial_qubits[i]));
      }

      if (trial_qubits.size() == 1) {
        zero_reflection->addInstruction(
            gateRegistry->createInstruction("Z", trial_qubits[0]));
      } else {
        zero_reflection->addInstruction(
            gateRegistry->createInstruction("H", trial_qubits[0]));

        // Multi-controlled X:
        std::vector<int> controlled_bits;
        for (int i = 1; i < trial_qubits.size(); ++i) {
          controlled_bits.emplace_back(trial_qubits[i]);
        }
        auto x_gate = gateRegistry->createComposite("x_gate");
        auto temp_gate =
            gateRegistry->createInstruction("X", trial_qubits[0]);
        temp_gate->setBufferNames({"q"});
        x_gate->addInstruction(temp_gate);
        auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("C-U"));
        mcx->expand({{"U", x_gate}, {"control-idx", controlled_bits}});
        zero_reflection->addInstruction(mcx);
        //=================================

        zero_reflection->addInstruction(
            gateRegistry->createInstruction("H", trial_qubits[0]));
      }
      for (int i = 0; i < trial_qubits.size(); ++i) {
        zero_reflection->addInstruction(
            gateRegistry->createInstruction("X", trial_qubits[i]));
      }

      inv_grover_op->addInstruction(inv_state_prep);
      inv_grover_op->addInstruction(zero_reflection);
      inv_grover_op->addInstruction(ae_state_prep_circ_clone);
      inv_grover_op->addInstruction(oracle);

      auto qae1_clone =  xacc::ir::asComposite(qae1->clone());

      qubits_superposition_state_prep->addInstruction(qae1_clone);

      int num_grover_ops = std::pow(2, num_evaluation_qubits) - 1;
      for (int i = 0; i < num_grover_ops; i++) {
        auto igo_clone = xacc::ir::asComposite(inv_grover_op->clone());
        addInstruction(igo_clone);
        to_be_undone->addInstruction(igo_clone);
        qubits_superposition_state_prep->addInstruction(igo_clone);
      }
    }

    std::vector<int> aetm_evaluation_qubits;
    auto to_be_undone_div = gateRegistry->createComposite("to_be_undone_div");
    for (int q = 0; q < qubits_superposition.size(); q++) {
        auto division = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("ProperFractionDivision"));
        std::vector<int> qubits_numerator;
      int num_evaluation_qubits = q + 1;
      int eval_qubits_so_far = 0;
      for (int i = 0; i < q; i++) {
        eval_qubits_so_far += i + 1;
      }
      for (int i = eval_qubits_so_far; i < eval_qubits_so_far + num_evaluation_qubits; i++) {
        qubits_numerator.push_back(division_evaluation_qubits[i]);
      }
      std::vector<int> qubits_fraction;
      while (qubits_fraction.size() < num_evaluation_qubits) {
          for (int i = 0; i < qubits_ancilla.size(); i++) {
              if (std::find(ancilla_in_use.begin(), ancilla_in_use.end(), qubits_ancilla[i]) == ancilla_in_use.end()) {
                  qubits_fraction.push_back(qubits_ancilla[i]);
                  ancilla_in_use.push_back(qubits_ancilla[i]);
                  aetm_evaluation_qubits.push_back(qubits_ancilla[i]);
                  break;
              }
          }
      }

      std::vector<int> division_ancilla;
      std::vector<int> temp_ancilla_in_use;
      while (division_ancilla.size() < 2*num_evaluation_qubits + 1) {
          for (int i = 0; i < qubits_ancilla.size(); i++) {
              if (std::find(ancilla_in_use.begin(), ancilla_in_use.end(), qubits_ancilla[i]) == ancilla_in_use.end()\
              && std::find(temp_ancilla_in_use.begin(), temp_ancilla_in_use.end(), qubits_ancilla[i]) == temp_ancilla_in_use.end()) {
                  division_ancilla.push_back(qubits_ancilla[i]);
                  temp_ancilla_in_use.push_back(qubits_ancilla[i]);
                  break;
              }
          }
      }

      while (qubits_numerator.size() < evaluation_qubits0.size()) {
          for (int i = 0; i < qubits_ancilla.size(); i++) {
              if (std::find(ancilla_in_use.begin(), ancilla_in_use.end(), qubits_ancilla[i]) == ancilla_in_use.end()
              && std::find(temp_ancilla_in_use.begin(), temp_ancilla_in_use.end(), qubits_ancilla[i]) == temp_ancilla_in_use.end()) {
                  qubits_numerator.push_back(qubits_ancilla[i]);
                  temp_ancilla_in_use.push_back(qubits_ancilla[i]);
                  break;
              }
          }
      }

        const bool expand_ok_div = division->expand({{"qubits_numerator",qubits_numerator},
                                                    {"qubits_denominator", evaluation_qubits0},
                                                    {"qubits_fraction", qubits_fraction},
                                                    {"qubits_ancilla", division_ancilla},
                                                    {"is_LSB", true}});
        assert(expand_ok_div);
        addInstruction(division);
        to_be_undone_div->addInstruction(division);
    }

    //std::cout << "begin undone\n";
    auto undo = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("InverseCircuit"));
    const bool expand_ok_undo = undo->expand({{"circ", to_be_undone}});
    assert(expand_ok_undo);
    addInstruction(undo);
    //std::cout << "end undone\n";

    for (auto it = ancilla_in_use.begin(); it != ancilla_in_use.end(); it++) {
        if (std::find(aetm_evaluation_qubits.begin(), aetm_evaluation_qubits.end(), *it) != aetm_evaluation_qubits.end()) {
            break;
        } else {
            ancilla_in_use.erase(it);
        }
    }


    auto aetm = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("AEtoMetric"));
    std::vector<int> precision_bits;
    for (int q = 0; q < qubits_superposition.size(); q++) {
        precision_bits.push_back(q+1);
    }
    std::vector<int> aetm_ancilla;
    while (aetm_ancilla.size() < 3* (int)*std::max_element(precision_bits.begin(), precision_bits.end()) + 1 + qubits_mean.size()) {
        for (int q = 0; q < qubits_ancilla.size(); q++) {
            if (std::find(ancilla_in_use.begin(), ancilla_in_use.end(), qubits_ancilla[q]) == ancilla_in_use.end()
            && std::find(aetm_ancilla.begin(), aetm_ancilla.end(), qubits_ancilla[q]) == aetm_ancilla.end()) {
                aetm_ancilla.push_back(qubits_ancilla[q]);
                break;
            }
        }
    }

    // for (auto bit : qubits_mean) {
    //     std::cout << "qubits mean " << bit << "\n";
    // }

    // for (auto bit : aetm_evaluation_qubits) {
    //     std::cout << "qubits eval " << bit << "\n";
    // }


    const bool expand_ok_aetm = aetm->expand({{"evaluation_bits", aetm_evaluation_qubits},
                                                {"precision_bits", precision_bits},
                                                {"qubits_beam_metric", qubits_mean},
                                                {"qubits_ancilla", aetm_ancilla},
                                                {"qubits_beam_metric_ones_idx", ones_idx}});
    assert(expand_ok_aetm);
    auto c_aetm = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    const bool expand_ok_c_aetm = c_aetm->expand({{"U", aetm}, {"control-idx", qubit_indicator}});
    assert(expand_ok_c_aetm);
    addInstruction(c_aetm);

    std::shared_ptr<xacc::CompositeInstruction> to_be_undone_clone =
        xacc::ir::asComposite(to_be_undone->clone());

    addInstruction(to_be_undone_clone);

    //std::cout << "begin inv div\n";
    auto inv_div = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("InverseCircuit"));
    const bool expand_ok_inv_div = inv_div->expand({{"circ", to_be_undone_div}});
    assert(expand_ok_inv_div);
    addInstruction(inv_div);
    //std::cout << "end inv div\n";

    std::shared_ptr<xacc::CompositeInstruction> undo_clone =
        xacc::ir::asComposite(undo->clone());

    addInstruction(undo_clone);

  } else {
      // TO BE DONE LATER
      // NOT NECESSARY FOR DECODER
  }

  //std::cout << "mvf ok\n";
  return true;
}

const std::vector<std::string> MeanValueFinder::requiredKeys() { return {}; }
}

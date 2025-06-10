#include <qristal/core/circuits/qd_beam_state_prep.hpp>
#include <memory>

namespace qristal {
bool BeamStatePrep::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  ////////////////////////////////////////////////////////
  // Get expected inputs or define any required variables
  ////////////////////////////////////////////////////////

  if (!runtimeOptions.keyExists<std::vector<std::vector<float>>>(
          "probability_table")) {
    return false;
  }
  std::vector<std::vector<float>> probability_table =
      runtimeOptions.get<std::vector<std::vector<float>>>("probability_table");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_string")) {
    return false;
  }
  std::vector<int> qubits_string =
      runtimeOptions.get<std::vector<int>>("qubits_string");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_metric")) {
    return false;
  }
  std::vector<int> qubits_metric =
      runtimeOptions.get<std::vector<int>>("qubits_metric");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_iteration")) {
    return false;
  }
  std::vector<int> qubits_iteration =
      runtimeOptions.get<std::vector<int>>("qubits_iteration");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_is_occupied")) {
    return false;
  }
  std::vector<int> qubits_is_occupied =
      runtimeOptions.get<std::vector<int>>("qubits_is_occupied");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_null")) {
    return false;
  }
  std::vector<int> qubits_null =
      runtimeOptions.get<std::vector<int>>("qubits_null");

  if (!runtimeOptions.keyExists<int>("qubit_is_null")) {
    return false;
  }
  int qubit_is_null = runtimeOptions.get<int>("qubit_is_null");

  if (!runtimeOptions.keyExists<int>("qubit_is_used")) {
    return false;
  }
  int qubit_is_used = runtimeOptions.get<int>("qubit_is_used");

  if (!runtimeOptions.keyExists<int>("qubit_is_repetition")) {
    return false;
  }
  int qubit_is_repetition = runtimeOptions.get<int>("qubit_is_repetition");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_current_iteration")) {
    return false;
  }
  std::vector<int> qubits_current_iteration =
      runtimeOptions.get<std::vector<int>>("qubits_current_iteration");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_next_metric")) {
    return false;
  }
  std::vector<int> qubits_next_metric =
      runtimeOptions.get<std::vector<int>>("qubits_next_metric");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_next_letter")) {
    return false;
  }
  std::vector<int> qubits_next_letter =
      runtimeOptions.get<std::vector<int>>("qubits_next_letter");

  if (!runtimeOptions.keyExists<std::vector<int>>(
          "qubits_ancilla_state_prep")) {
    return false;
  }
  std::vector<int> qubits_ancilla_state_prep =
      runtimeOptions.get<std::vector<int>>("qubits_ancilla_state_prep");

  ////////////////////////////////////////////////////////
  // Helper Functions
  ////////////////////////////////////////////////////////
  std::function<std::string(int, int)> binary = [&](int i, int num_qubits) {
    std::string i_binary = std::bitset<8 * sizeof(i)>(i).to_string();
    std::string i_binary_n = i_binary.substr(
        i_binary.size() < num_qubits ? 0 : i_binary.size() - num_qubits);
    reverse(i_binary_n.begin(), i_binary_n.end());
    return i_binary_n;
  };

  std::function<std::vector<int>(std::string, std::string)>
      different_bits_indices =
          [&](std::string bitstring1, std::string bitstring2) {
            assert(bitstring1.size() == bitstring2.size());
            std::vector<int> different_bits;
            for (int i = 0; i < bitstring1.size(); i++) {
              if (bitstring1[i] != bitstring2[i]) {
                different_bits.push_back(i);
              }
            }
            return different_bits;
          };
  ////////////////////////////////////////////////////////
  // Add Instructions
  ////////////////////////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  // qubits_current_iteration initialised to 1
  int qubits_current_iteration_int = 1;
  addInstruction(
      gateRegistry->createInstruction("X", qubits_current_iteration[0]));

  // loop from qubits_current_iteration = 1 to qubits_current_iteration = Length
  // of String
  int L = qubits_string.size() / qubits_next_letter.size();
  while (qubits_current_iteration_int <= L) {

    // Use W' to copy data from prob table to next letter and next metric qubits
    auto w_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("WPrime"));
    xacc::HeterogeneousMap w_map = {
        {"iteration", qubits_current_iteration_int - 1},
        {"qubits_next_letter", qubits_next_letter},
        {"qubits_next_metric", qubits_next_metric},
        {"probability_table", probability_table}};
    w_prime->expand(w_map);
    addInstructions(w_prime->getInstructions());

    // mark nulls
    auto mark_null = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("GeneralisedMCX"));
    xacc::HeterogeneousMap mark_null_map = {
        {"controls_off", qubits_next_letter}, {"target", qubit_is_null}};
    mark_null->expand(mark_null_map);
    addInstructions(mark_null->getInstructions());

    // add nulls to the end of the string
    for (int pos = L - 1; pos >= 0; pos--) {
      int qubits_per_letter = qubits_next_letter.size();
      int qubits_per_metric = qubits_next_metric.size();
      int qubits_per_iteration = qubits_current_iteration.size();
      std::vector<int> qubits_pos_letter;
      for (int i = 0; i < qubits_per_letter; i++) {
        qubits_pos_letter.push_back(qubits_string[pos * qubits_per_letter + i]);
      }
      std::vector<int> qubits_pos_metric;
      for (int i = 0; i < qubits_per_metric; i++) {
        qubits_pos_metric.push_back(qubits_metric[pos * qubits_per_metric + i]);
      }
      std::vector<int> qubits_pos_iteration;
      for (int i = 0; i < qubits_per_iteration; i++) {
        qubits_pos_iteration.push_back(
            qubits_iteration[pos * qubits_per_iteration + i]);
      }
      addInstruction(gateRegistry->createInstruction("X", qubit_is_used));
      addInstruction(
          gateRegistry->createInstruction("X", qubits_is_occupied[pos]));
      auto add_null = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      auto U_add_null = gateRegistry->createComposite("U_add_null");
      for (int q = 0; q < qubits_per_letter; q++) {
        U_add_null->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_next_letter[q]),
                      static_cast<unsigned long>(qubits_pos_letter[q])}));
      }
      for (int q = 0; q < qubits_per_metric; q++) {
        U_add_null->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_next_metric[q]),
                      static_cast<unsigned long>(qubits_pos_metric[q])}));
      }
      for (int q = 0; q < qubits_per_iteration; q++) {
        U_add_null->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_current_iteration[q]),
                      static_cast<unsigned long>(qubits_pos_iteration[q])}));
      }
      xacc::HeterogeneousMap add_null_map = {
          {"U", U_add_null},
          {"control-idx", std::vector<int>{qubit_is_used, qubit_is_null,
                                           qubits_is_occupied[pos]}}};
      add_null->expand(add_null_map);
      addInstructions(add_null->getInstructions());
      addInstruction(
          gateRegistry->createInstruction("X", qubits_is_occupied[pos]));
      addInstruction(gateRegistry->createInstruction("X", qubit_is_used));

      auto null_switch1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap null_switch1_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"controls_on", std::vector<int>{qubit_is_null}},
          {"flag", qubits_is_occupied[pos]}};
      null_switch1->expand(null_switch1_map);
      addInstructions(null_switch1->getInstructions());

      auto null_switch2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap null_switch2_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"controls_on", std::vector<int>{qubit_is_null}},
          {"flag", qubit_is_used}};
      null_switch2->expand(null_switch2_map);
      addInstructions(null_switch2->getInstructions());

      auto null_switch3 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap null_switch3_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"controls_on", std::vector<int>{qubit_is_null}},
          {"flag", qubits_null[pos]}};
      null_switch3->expand(null_switch3_map);
      addInstructions(null_switch3->getInstructions());
    }

    // Now for non-null characters. Repeats are added to the back, otherwise
    // added to the front
    int qubits_per_letter = qubits_next_letter.size();
    int qubits_per_metric = qubits_next_metric.size();
    int qubits_per_iteration = qubits_current_iteration.size();
    std::vector<int> qubits_first_letter;
    for (int i = 0; i < qubits_per_letter; i++) {
      qubits_first_letter.push_back(qubits_string[i]);
    }
    std::vector<int> qubits_first_metric;
    for (int i = 0; i < qubits_per_metric; i++) {
      qubits_first_metric.push_back(qubits_metric[i]);
    }
    std::vector<int> qubits_first_iteration;
    for (int i = 0; i < qubits_per_iteration; i++) {
      qubits_first_iteration.push_back(qubits_iteration[i]);
    }

    addInstruction(gateRegistry->createInstruction("X", qubits_is_occupied[0]));
    addInstruction(gateRegistry->createInstruction("X", qubit_is_used));
    addInstruction(gateRegistry->createInstruction("X", qubit_is_null));
    auto add_first = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("C-U"));
    auto U_add_first = gateRegistry->createComposite("U_add_first");
    for (int q = 0; q < qubits_per_letter; q++) {
      U_add_first->addInstruction(gateRegistry->createInstruction(
          "CX", std::vector<size_t>{
                    static_cast<unsigned long>(qubits_next_letter[q]),
                    static_cast<unsigned long>(qubits_first_letter[q])}));
    }
    for (int q = 0; q < qubits_per_metric; q++) {
      U_add_first->addInstruction(gateRegistry->createInstruction(
          "CX", std::vector<size_t>{
                    static_cast<unsigned long>(qubits_next_metric[q]),
                    static_cast<unsigned long>(qubits_first_metric[q])}));
    }
    for (int q = 0; q < qubits_per_iteration; q++) {
      U_add_first->addInstruction(gateRegistry->createInstruction(
          "CX", std::vector<size_t>{
                    static_cast<unsigned long>(qubits_current_iteration[q]),
                    static_cast<unsigned long>(qubits_first_iteration[q])}));
    }
    xacc::HeterogeneousMap add_first_map = {
        {"U", U_add_first},
        {"control-idx", std::vector<int>{qubit_is_used, qubit_is_null,
                                         qubits_is_occupied[0]}}};
    add_first->expand(add_first_map);
    addInstructions(add_first->getInstructions());
    addInstruction(gateRegistry->createInstruction("X", qubits_is_occupied[0]));
    addInstruction(gateRegistry->createInstruction("X", qubit_is_used));
    addInstruction(gateRegistry->createInstruction("X", qubit_is_null));

    auto first_switch1 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("EqualityChecker"));
    xacc::HeterogeneousMap first_switch1_map = {
        {"qubits_a", qubits_first_iteration},
        {"qubits_b", qubits_current_iteration},
        {"controls_off", std::vector<int>{qubit_is_null}},
        {"flag", qubits_is_occupied[0]}};
    first_switch1->expand(first_switch1_map);
    addInstructions(first_switch1->getInstructions());

    auto first_switch2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("EqualityChecker"));
    xacc::HeterogeneousMap first_switch2_map = {
        {"qubits_a", qubits_first_iteration},
        {"qubits_b", qubits_current_iteration},
        {"controls_off", std::vector<int>{qubit_is_null}},
        {"flag", qubit_is_used}};
    first_switch2->expand(first_switch2_map);
    addInstructions(first_switch2->getInstructions());

    for (int pos = 1; pos < L; pos++) {
      int ancilla1 = qubits_ancilla_state_prep[0];
      int ancilla2 = qubits_ancilla_state_prep[1];

      std::vector<int> qubits_pos_letter;
      for (int i = 0; i < qubits_per_letter; i++) {
        qubits_pos_letter.push_back(qubits_string[pos * qubits_per_letter + i]);
      }
      std::vector<int> qubits_pos_metric;
      for (int i = 0; i < qubits_per_metric; i++) {
        qubits_pos_metric.push_back(qubits_metric[pos * qubits_per_metric + i]);
      }
      std::vector<int> qubits_pos_iteration;
      for (int i = 0; i < qubits_per_iteration; i++) {
        qubits_pos_iteration.push_back(
            qubits_iteration[pos * qubits_per_iteration + i]);
      }

      std::vector<int> qubits_last_letter;
      for (int i = 0; i < qubits_per_letter; i++) {
        qubits_last_letter.push_back(
            qubits_string[(pos - 1) * qubits_per_letter + i]);
      }
      std::vector<int> qubits_last_metric;
      for (int i = 0; i < qubits_per_metric; i++) {
        qubits_last_metric.push_back(
            qubits_metric[(pos - 1) * qubits_per_metric + i]);
      }
      std::vector<int> qubits_last_iteration;
      for (int i = 0; i < qubits_per_iteration; i++) {
        qubits_last_iteration.push_back(
            qubits_iteration[(pos - 1) * qubits_per_iteration + i]);
      }

      auto check_string = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap check_string_map = {
          {"qubits_a", qubits_last_letter},
          {"qubits_b", qubits_next_letter},
          {"flag", ancilla1}};
      check_string->expand(check_string_map);
      addInstructions(check_string->getInstructions());

      int c_in = qubits_ancilla_state_prep[2];
      std::vector<int> adder_bits;
      for (int i = 0; i < qubits_last_iteration.size() - 1; i++) {
        adder_bits.push_back(qubits_ancilla_state_prep[3 + i]);
      }
      addInstruction(gateRegistry->createInstruction("X", adder_bits[0]));
      auto check_iteration_adder =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("RippleCarryAdder"));
      xacc::HeterogeneousMap check_iteration_adder_map = {
          {"c_in", c_in},
          {"adder_bits", adder_bits},
          {"sum_bits", qubits_last_iteration}};
      check_iteration_adder->expand(check_iteration_adder_map);
      addInstructions(check_iteration_adder->getInstructions());
      auto check_iteration =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap check_iteration_map = {
          {"qubits_a", qubits_last_iteration},
          {"qubits_b", qubits_current_iteration},
          {"flag", ancilla2}};
      check_iteration->expand(check_iteration_map);
      addInstructions(check_iteration->getInstructions());

      auto check_repeat = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("GeneralisedMCX"));
      check_repeat->expand(
          {{"target", qubit_is_repetition},
           {"controls_on", std::vector<int>{ancilla1, ancilla2}},
           {"controls_off", std::vector<int>{qubits_is_occupied[pos],
                                             qubit_is_used, qubit_is_null}}});
      addInstructions(check_repeat->getInstructions());

      auto reset_repeat_next =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
      auto U_reset_repeat_next =
          gateRegistry->createComposite("U_reset_repeat_next");
      for (int q = 0; q < qubits_per_letter; q++) {
        U_reset_repeat_next->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_last_letter[q]),
                      static_cast<unsigned long>(qubits_next_letter[q])}));
      }
      reset_repeat_next->expand(
          {{"U", U_reset_repeat_next}, {"control-idx", qubit_is_repetition}});
      addInstruction(reset_repeat_next);

      auto undo_check_iteration =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      undo_check_iteration->expand(check_iteration_map);
      addInstructions(undo_check_iteration->getInstructions());
      auto undo_check_iteration_adder =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("InverseCircuit"));
      xacc::HeterogeneousMap undo_check_iteration_adder_map = {
          {"circ", check_iteration_adder}};
      undo_check_iteration_adder->expand(undo_check_iteration_adder_map);
      addInstructions(undo_check_iteration_adder->getInstructions());
      addInstruction(gateRegistry->createInstruction("X", adder_bits[0]));

      auto undo_check_string =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      undo_check_string->expand(check_string_map);
      addInstructions(undo_check_string->getInstructions());
      addInstruction(gateRegistry->createInstruction(
          "CX", {static_cast<unsigned long>(qubit_is_repetition),
                 static_cast<unsigned long>(ancilla1)}));

      addInstruction(gateRegistry->createInstruction("X", qubit_is_used));
      addInstruction(gateRegistry->createInstruction("X", qubit_is_null));
      addInstruction(gateRegistry->createInstruction("X", qubit_is_repetition));
      addInstruction(
          gateRegistry->createInstruction("X", qubits_is_occupied[pos]));
      auto add_letter = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      auto U_add_letter = gateRegistry->createComposite("U_add_letter");
      for (int q = 0; q < qubits_per_letter; q++) {
        U_add_letter->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_next_letter[q]),
                      static_cast<unsigned long>(qubits_pos_letter[q])}));
      }
      for (int q = 0; q < qubits_per_metric; q++) {
        U_add_letter->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_next_metric[q]),
                      static_cast<unsigned long>(qubits_pos_metric[q])}));
      }
      for (int q = 0; q < qubits_per_iteration; q++) {
        U_add_letter->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_current_iteration[q]),
                      static_cast<unsigned long>(qubits_pos_iteration[q])}));
      }
      xacc::HeterogeneousMap add_letter_map = {
          {"U", U_add_letter},
          {"control-idx",
           std::vector<int>{qubit_is_used, qubit_is_null, qubit_is_repetition,
                            qubits_is_occupied[pos]}}};
      add_letter->expand(add_letter_map);
      addInstructions(add_letter->getInstructions());
      addInstruction(gateRegistry->createInstruction("X", qubit_is_used));
      addInstruction(gateRegistry->createInstruction("X", qubit_is_null));
      addInstruction(gateRegistry->createInstruction("X", qubit_is_repetition));
      addInstruction(
          gateRegistry->createInstruction("X", qubits_is_occupied[pos]));

      auto letter_switch1 =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap letter_switch1_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"controls_off",
           std::vector<int>{qubit_is_null, qubit_is_repetition}},
          {"flag", qubits_is_occupied[pos]}};
      letter_switch1->expand(letter_switch1_map);
      addInstructions(letter_switch1->getInstructions());

      auto letter_switch2 =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap letter_switch2_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"controls_off",
           std::vector<int>{qubit_is_null, qubit_is_repetition}},
          {"flag", qubit_is_used}};
      letter_switch2->expand(letter_switch2_map);
      addInstructions(letter_switch2->getInstructions());
    }

    for (int pos = L - 1; pos >= 0; pos--) {
      std::vector<int> qubits_pos_metric;
      for (int i = 0; i < qubits_per_metric; i++) {
        qubits_pos_metric.push_back(qubits_metric[pos * qubits_per_metric + i]);
      }
      std::vector<int> qubits_pos_iteration;
      for (int i = 0; i < qubits_per_iteration; i++) {
        qubits_pos_iteration.push_back(
            qubits_iteration[pos * qubits_per_iteration + i]);
      }
      addInstruction(gateRegistry->createInstruction("X", qubit_is_used));
      addInstruction(gateRegistry->createInstruction("X", qubit_is_null));
      addInstruction(
          gateRegistry->createInstruction("X", qubits_is_occupied[pos]));
      auto add_repeat = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      auto U_add_repeat = gateRegistry->createComposite("U_add_repeat");
      for (int q = 0; q < qubits_per_metric; q++) {
        U_add_repeat->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_next_metric[q]),
                      static_cast<unsigned long>(qubits_pos_metric[q])}));
      }
      for (int q = 0; q < qubits_per_iteration; q++) {
        U_add_repeat->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_current_iteration[q]),
                      static_cast<unsigned long>(qubits_pos_iteration[q])}));
      }
      xacc::HeterogeneousMap add_repeat_map = {
          {"U", U_add_repeat},
          {"control-idx",
           std::vector<int>{qubit_is_used, qubit_is_null, qubit_is_repetition,
                            qubits_is_occupied[pos]}}};
      add_repeat->expand(add_repeat_map);
      addInstructions(add_repeat->getInstructions());
      addInstruction(gateRegistry->createInstruction("X", qubit_is_used));
      addInstruction(gateRegistry->createInstruction("X", qubit_is_null));
      addInstruction(
          gateRegistry->createInstruction("X", qubits_is_occupied[pos]));

      auto repeat_switch1 =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap repeat_switch1_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"controls_off", std::vector<int>{qubit_is_null}},
          {"controls_on", std::vector<int>{qubit_is_repetition}},
          {"flag", qubits_is_occupied[pos]}};
      repeat_switch1->expand(repeat_switch1_map);
      addInstructions(repeat_switch1->getInstructions());

      auto repeat_switch2 =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap repeat_switch2_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"controls_off", std::vector<int>{qubit_is_null}},
          {"controls_on", std::vector<int>{qubit_is_repetition}},
          {"flag", qubit_is_used}};
      repeat_switch2->expand(repeat_switch2_map);
      addInstructions(repeat_switch2->getInstructions());
    }

    // reset is_used flag
    addInstruction(gateRegistry->createInstruction("X", qubit_is_used));

    // reset next letter and next metric qubits
    for (int pos = 0; pos < L; pos++) {
      std::vector<int> qubits_pos_letter;
      for (int i = 0; i < qubits_per_letter; i++) {
        qubits_pos_letter.push_back(qubits_string[pos * qubits_per_letter + i]);
      }
      std::vector<int> qubits_pos_metric;
      for (int i = 0; i < qubits_per_metric; i++) {
        qubits_pos_metric.push_back(qubits_metric[pos * qubits_per_metric + i]);
      }
      std::vector<int> qubits_pos_iteration;
      for (int i = 0; i < qubits_per_iteration; i++) {
        qubits_pos_iteration.push_back(
            qubits_iteration[pos * qubits_per_iteration + i]);
      }
      int ancilla = qubits_ancilla_state_prep[0];
      auto check_iteration =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap check_iteration_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"flag", ancilla}};
      check_iteration->expand(check_iteration_map);
      addInstructions(check_iteration->getInstructions());

      auto reset_next = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      auto U_reset_next = gateRegistry->createComposite("U_reset_next");
      for (int q = 0; q < qubits_per_metric; q++) {
        U_reset_next->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_pos_metric[q]),
                      static_cast<unsigned long>(qubits_next_metric[q])}));
      }
      for (int q = 0; q < qubits_per_letter; q++) {
        U_reset_next->addInstruction(gateRegistry->createInstruction(
            "CX", std::vector<size_t>{
                      static_cast<unsigned long>(qubits_pos_letter[q]),
                      static_cast<unsigned long>(qubits_next_letter[q])}));
      }
      reset_next->expand({{"U", U_reset_next}, {"control-idx", ancilla}});
      addInstructions(reset_next->getInstructions());

      auto undo_check_iteration =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      undo_check_iteration->expand(check_iteration_map);
      addInstructions(undo_check_iteration->getInstructions());
    }

    // reset is_null flag
    for (int pos = 0; pos < L; pos++) {

      int ancilla1 = qubits_ancilla_state_prep[0];

      std::vector<int> qubits_pos_iteration;
      for (int i = 0; i < qubits_per_iteration; i++) {
        qubits_pos_iteration.push_back(
            qubits_iteration[pos * qubits_per_iteration + i]);
      }
      auto check_iteration =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap check_iteration_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"flag", ancilla1}};
      check_iteration->expand(check_iteration_map);
      addInstructions(check_iteration->getInstructions());

      auto reset_null = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("C-U"));
      auto U_reset_null = gateRegistry->createComposite("U_reset_null");
      U_reset_null->addInstruction(gateRegistry->createInstruction(
          "CX", {static_cast<unsigned long>(qubits_null[pos]),
                 static_cast<unsigned long>(qubit_is_null)}));
      reset_null->expand({{"U", U_reset_null}, {"control-idx", ancilla1}});
      addInstruction(reset_null);

      auto undo_check_iteration =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      undo_check_iteration->expand(check_iteration_map);
      addInstructions(undo_check_iteration->getInstructions());
    }

    // reset is_repetition flag
    for (int pos = 0; pos < L; pos++) {
      int ancilla1 = qubits_ancilla_state_prep[0];
      int ancilla2 = qubits_ancilla_state_prep[1];

      std::vector<int> qubits_pos_letter;
      for (int i = 0; i < qubits_per_letter; i++) {
        qubits_pos_letter.push_back(qubits_string[pos * qubits_per_letter + i]);
      }
      std::vector<int> qubits_pos_iteration;
      for (int i = 0; i < qubits_per_iteration; i++) {
        qubits_pos_iteration.push_back(
            qubits_iteration[pos * qubits_per_iteration + i]);
      }
      auto check_iteration =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      xacc::HeterogeneousMap check_iteration_map = {
          {"qubits_a", qubits_pos_iteration},
          {"qubits_b", qubits_current_iteration},
          {"flag", ancilla1}};
      check_iteration->expand(check_iteration_map);
      addInstructions(check_iteration->getInstructions());

      auto reset_repeat = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("GeneralisedMCX"));
      std::vector<int> controls_off = {qubits_null[pos]};
      for (int q = 0; q < qubits_pos_letter.size(); q++) {
        controls_off.push_back(qubits_pos_letter[q]);
      }
      reset_repeat->expand({{"controls_on", std::vector<int>{ancilla1}},
                            {"controls_off", controls_off},
                            {"target", qubit_is_repetition}});
      addInstructions(reset_repeat->getInstructions());

      auto undo_check_iteration =
          std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("EqualityChecker"));
      undo_check_iteration->expand(check_iteration_map);
      addInstructions(undo_check_iteration->getInstructions());
    }

    // add 1 to current iteration
    std::string current_iteration_bin = binary(
        qubits_current_iteration_int, (int)qubits_current_iteration.size());
    int next_iteration_int;
    if (qubits_current_iteration_int == L) {
        next_iteration_int = 0;
    } else {
        next_iteration_int = qubits_current_iteration_int + 1;
    }
    std::string next_iteration_bin =
        binary(next_iteration_int, (int)qubits_current_iteration.size());
    std::vector<int> different_bits =
        different_bits_indices(current_iteration_bin, next_iteration_bin);
    for (int q : different_bits) {
      addInstruction(
          gateRegistry->createInstruction("X", qubits_current_iteration[q]));
    }
    qubits_current_iteration_int++;
  }

  for (int q : qubits_is_occupied) {
      addInstruction(gateRegistry->createInstruction("X", q));
  }

  return true;
}

const std::vector<std::string> BeamStatePrep::requiredKeys() { return {}; }

}

#include "qb/core/circuit_builder.hpp"
#include "qb/core/circuits/decoder_kernel.hpp"
#include <CompositeInstruction.hpp>
#include <Instruction.hpp>
#include <memory>
#include "xacc.hpp"
namespace qbOS {

bool DecoderKernel::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  ////////////////////////////////////////////////////////
  // Define helper functions
  ////////////////////////////////////////////////////////

  // Flip a bitstring
  std::function<std::string(std::string)> flip_bitstring =
      [&](std::string bitstring) {
        std::string flipped;
        for (int i = 0; i < bitstring.size(); i++) {
          if (bitstring[i] == '1') {
            flipped.push_back('0');
          }
          if (bitstring[i] == '0') {
            flipped.push_back('1');
          }
        }
        return flipped;
      };

  // int to binary function
  std::function<std::string(int, int)> binary = [&](int i, int num_qubits) {
    std::string i_binary = std::bitset<8 * sizeof(i)>(i).to_string();
    std::string i_binary_n = i_binary.substr(
        i_binary.size() < num_qubits ? 0 : i_binary.size() - num_qubits);
    return i_binary_n;
  };

  // binary to gray code function
  std::function<std::string(std::string)> gray_code = [&](std::string binary) {
    char first_bit = binary[0];
    std::string gray_code;
    if (first_bit == '1') {
      gray_code = "1";
    }
    if (first_bit == '0') {
      gray_code = '0';
    }
    for (int i = 0; i < binary.size() - 1; i++) {
      char ex_or = ((binary[i] + binary[i + 1]) % 2 == 1) ? '1' : '0';
      gray_code = gray_code + ex_or;
    }
    return gray_code;
  };

  // Find the bit that differs
  std::function<int(std::string, std::string)> different_bit_index =
      [&](std::string bitstring1, std::string bitstring2) {
        assert(bitstring1.size() == bitstring2.size());
        for (int i = 0; i < bitstring1.size(); i++) {
          if (bitstring1[i] != bitstring2[i]) {
            return i;
          }
        }
        return -1;
      };

  ////////////////////////////////////////////////////////
  // Get expected inputs or define any required variables
  ////////////////////////////////////////////////////////

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

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla_adder")) {
    return false;
  }
  std::vector<int> qubits_ancilla_adder =
      runtimeOptions.get<std::vector<int>>("qubits_ancilla_adder");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_init_null")) {
    return false;
  }
  std::vector<int> qubits_init_null =
      runtimeOptions.get<std::vector<int>>("qubits_init_null");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_init_repeat")) {
    return false;
  }
  std::vector<int> qubits_init_repeat =
      runtimeOptions.get<std::vector<int>>("qubits_init_repeat");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_superfluous_flags")) {
    return false;
  }
  std::vector<int> qubits_superfluous_flags =
      runtimeOptions.get<std::vector<int>>("qubits_superfluous_flags");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_beam_metric")) {
    return false;
  }
  std::vector<int> qubits_beam_metric =
      runtimeOptions.get<std::vector<int>>("qubits_beam_metric");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla_pool")) {
    return false;
  }
  std::vector<int> qubits_ancilla_pool =
      runtimeOptions.get<std::vector<int>>("qubits_ancilla_pool");

  if (!runtimeOptions.keyExists<std::vector<int>>("total_metric")) {
    return false;
  }
  std::vector<int> total_metric = runtimeOptions.get<std::vector<int>>("total_metric");

  if (!runtimeOptions.keyExists<std::vector<int>>("evaluation_bits")) {
    return false;
  }
  std::vector<int> evaluation_bits = runtimeOptions.get<std::vector<int>>("evaluation_bits");

  if (!runtimeOptions.keyExists<std::vector<int>>("precision_bits")) {
    return false;
  }
  std::vector<int> precision_bits = runtimeOptions.get<std::vector<int>>("precision_bits");

  std::vector<int> total_metric_exponent;
  if (runtimeOptions.keyExists<std::vector<int>>("total_metric_exponent")) {
      total_metric_exponent = runtimeOptions.get<std::vector<int>>("total_metric_exponenet");
  }

  if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("metric_state_prep")) {
    return false;
  }
  auto metric_state_prep = runtimeOptions.getPointerLike<xacc::CompositeInstruction>("metric_state_prep");

  std::vector<int> qubits_next_letter; // S
  std::vector<int> qubits_next_metric; // m
  int L = qubits_init_null.size();
  int S = qubits_string.size()/L;
  int m = qubits_metric.size()/L;
  for (int i = 0; i < S; i++) {
  qubits_next_letter.push_back(qubits_ancilla_pool[i]);
  }
  for (int i = 0; i < m; i++) {
  qubits_next_metric.push_back(qubits_ancilla_pool[S+i]);
  }

  std::vector<int> qubits_total_metric;
  for (int i = 0; i < m; i++) {
    qubits_total_metric.push_back(qubits_metric[i]);
  }
  for (int i = 0; i < qubits_ancilla_adder.size(); i++) {
    qubits_total_metric.push_back(qubits_ancilla_adder[i]);
  }

  ////////////////////////////////////////////////////////
  // Add instructions
  ////////////////////////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  ///
  // Take the exponenet of the string total metric register
  ///

//   if (total_metric_exponent.size() > 0) {
//     const xacc::HeterogeneousMap &map_exp = {
//         {"qubits_log", qubits_total_metric},
//         {"qubits_exponent", total_metric_exponent}};
//     qbOS::Exponent build_exp;
//     const bool expand_ok = build_exp.expand(map_exp);
//     auto exponent = build_exp.get();
//     addInstructions(exponent->getInstructions());
//   }

  ///
  // Form equivalence classes
  ///

  int q0 = qubits_ancilla_pool[0];
  int q1 = qubits_ancilla_pool[1];
  int q2 = qubits_ancilla_pool[2];

  // flag superfluous symbols and mark for swap

  for (int i = L - 1; i >= 0; i--) {
    std::vector<int> letter;
    for (int j = 0; j < qubits_next_letter.size(); j++) {
      letter.push_back(qubits_string[i * (int)qubits_next_letter.size() + j]);
    }

    // flag last symbol if it is null or repeat
    if (i == L - 1) {
      addInstruction(
          gateRegistry->createInstruction("X", qubits_superfluous_flags[i]));
      auto untoffoli = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("GeneralisedMCX"));
      std::vector<int> off;
      off.push_back(qubits_init_null[i]);
      off.push_back(qubits_init_repeat[i]);
      untoffoli->expand(
          {{"controls_off", off}, {"target", qubits_superfluous_flags[i]}});
      addInstructions(untoffoli->getInstructions());
      metric_state_prep->addInstructions(untoffoli->getInstructions());
    }

    // loop from second last symbol to first symbol
    else {
      // flag if it is a repeat or a null
      addInstruction(
          gateRegistry->createInstruction("X", qubits_superfluous_flags[i]));
      metric_state_prep->addInstruction(
          gateRegistry->createInstruction("X", qubits_superfluous_flags[i]));
      auto untoffoli = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("GeneralisedMCX"));
      std::vector<int> off;
      off.push_back(qubits_init_null[i]);
      off.push_back(qubits_init_repeat[i]);
      untoffoli->expand(
          {{"controls_off", off}, {"target", qubits_superfluous_flags[i]}});
      addInstructions(untoffoli->getInstructions());
      metric_state_prep->addInstructions(untoffoli->getInstructions());

      // flip control-swap qubit according to whether that symbol is a repeat or
      // a null
      int qubit_control_swap = qubits_ancilla_pool[0];
      addInstruction(gateRegistry->createInstruction(
          "CX", {static_cast<unsigned long>(qubits_superfluous_flags[i]),
                 static_cast<unsigned long>(qubit_control_swap)}));
      metric_state_prep->addInstruction(gateRegistry->createInstruction(
          "CX", {static_cast<unsigned long>(qubits_superfluous_flags[i]),
                 static_cast<unsigned long>(qubit_control_swap)}));

      // loop from current symbol to the end
      for (int j = i; j < L - 1; j++) {
        std::vector<int> current_letter;
        std::vector<int> next_letter;
        std::vector<int> current_flag = {qubits_superfluous_flags[j]};
        std::vector<int> next_flag = {qubits_superfluous_flags[j + 1]};

        for (int k = 0; k < qubits_next_letter.size(); k++) {
          current_letter.push_back(
              qubits_string[j * qubits_next_letter.size() + k]);
        }
        for (int k = 0; k < qubits_next_letter.size(); k++) {
          next_letter.push_back(
              qubits_string[(j + 1) * qubits_next_letter.size() + k]);
        }

        // swap flagged symbol (swap conditional on control-swap) with next one
        auto c_swap_letter =
            std::dynamic_pointer_cast<xacc::CompositeInstruction>(
                xacc::getService<xacc::Instruction>("ControlledSwap"));
        std::vector<int> flags_on = {qubit_control_swap};
        xacc::HeterogeneousMap options_letter{{"qubits_a", current_letter},
                                              {"qubits_b", next_letter},
                                              {"flags_on", flags_on}};
        const bool expand_ok_letter = c_swap_letter->expand(options_letter);
        assert(expand_ok_letter);
        addInstructions(c_swap_letter->getInstructions());
        metric_state_prep->addInstructions(c_swap_letter->getInstructions());

        // swap superfluous flag (swap conditional on control-swap) with next
        // one
        auto c_swap_flag =
            std::dynamic_pointer_cast<xacc::CompositeInstruction>(
                xacc::getService<xacc::Instruction>("ControlledSwap"));
        xacc::HeterogeneousMap options_flag{{"qubits_a", current_flag},
                                            {"qubits_b", next_flag},
                                            {"flags_on", flags_on}};
        const bool expand_ok_flag = c_swap_flag->expand(options_flag);
        assert(expand_ok_flag);
        addInstructions(c_swap_flag->getInstructions());
        metric_state_prep->addInstructions(c_swap_flag->getInstructions());
      }

      // flip control-swap qubit back according to whether that symbol is a
      // repeat or a null
      addInstruction(gateRegistry->createInstruction("X", qubit_control_swap));
      metric_state_prep->addInstruction(gateRegistry->createInstruction("X", qubit_control_swap));
      auto untoffoli2 = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("GeneralisedMCX"));
      std::vector<int> off2;
      off2.push_back(qubits_init_null[i]);
      off2.push_back(qubits_init_repeat[i]);
      untoffoli2->expand(
          {{"controls_off", off2}, {"target", qubit_control_swap}});
      addInstructions(untoffoli2->getInstructions());
      metric_state_prep->addInstructions(untoffoli2->getInstructions());
    }
  }

  auto temp = xacc::as_shared_ptr(metric_state_prep);
  auto state_qubits_set = qbOS::uniqueBitsQD(temp);
  std::vector<int> state_qubits;
  for (int bit : state_qubits_set) {
    state_qubits.push_back(bit);
  }

  std::vector<int> qubits_ancilla;
  for (int i = 0; i < qubits_superfluous_flags.size(); i++) {
      qubits_ancilla.push_back(qubits_ancilla_pool[3+i]);
  }

  std::vector<int> ancilla;
  for (int i = 0; i < qubits_beam_metric.size(); i++) {
    ancilla.push_back(qubits_ancilla_pool[3+i+qubits_superfluous_flags.size()]);
  }

  auto add_metrics = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
      xacc::getService<xacc::Instruction>("SuperpositionAdder"));
  xacc::HeterogeneousMap options_adder{
      {"q0", q0}, {"q1", q1}, {"q2", q2},
      {"qubits_flags", qubits_superfluous_flags},
      {"qubits_string", qubits_string},
      {"qubits_metric", total_metric},
      {"precision_bits", precision_bits},
      {"evaluation_qubits", evaluation_bits},
      {"ae_state_prep_circ", metric_state_prep},
      {"state_qubits", state_qubits},
      {"qubits_ancilla", qubits_ancilla},
      {"qubits_ancilla_aetm", ancilla},
      {"qubits_beam_metric", qubits_beam_metric}};
  const bool expand_ok_add = add_metrics->expand(options_adder);
  assert(expand_ok_add);
  addInstructions(add_metrics->getInstructions());

  return true;
}

const std::vector<std::string> DecoderKernel::requiredKeys() { return {}; }

} // namespace qbOS

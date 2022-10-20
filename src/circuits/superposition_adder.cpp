/***
 *** Copyright (c) 2022 Quantum Brilliance Pty Ltd
 ***/

#include "qb/core/circuits/superposition_adder.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <assert.h>
#include <memory>
#include "xacc.hpp"

namespace qbOS {
bool SuperpositionAdder::expand(const xacc::HeterogeneousMap &runtimeOptions) {
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

  if (!runtimeOptions.keyExists<int>("q0")) {
    return false;
  }
  int q0 = runtimeOptions.get<int>("q0");

  if (!runtimeOptions.keyExists<int>("q1")) {
    return false;
  }
  int q1 = runtimeOptions.get<int>("q1");

  if (!runtimeOptions.keyExists<int>("q2")) {
    return false;
  }
  int q2 = runtimeOptions.get<int>("q2");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_flags")) {
    return false;
  }
  std::vector<int> qubits_flags = runtimeOptions.get<std::vector<int>>("qubits_flags");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_string")) {
    return false;
  }
  std::vector<int> qubits_string = runtimeOptions.get<std::vector<int>>("qubits_string");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_metric")) {
    return false;
  }
  std::vector<int> qubits_metric = runtimeOptions.get<std::vector<int>>("qubits_metric");

  if (!runtimeOptions.pointerLikeExists<xacc::CompositeInstruction>("ae_state_prep_circ")) {
    return false;
  }
  auto ae_state_prep_circ =
      runtimeOptions.getPointerLike<xacc::CompositeInstruction>("ae_state_prep_circ");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla")) {
    return false;
  }
  std::vector<int> qubits_ancilla = runtimeOptions.get<std::vector<int>>("qubits_ancilla");

  if (!runtimeOptions.keyExists<std::vector<int>>("qubits_beam_metric")) {
    return false;
  }
  std::vector<int> qubits_beam_metric = runtimeOptions.get<std::vector<int>>("qubits_beam_metric");

  ////////////////////////////////////////////////////////
  // Add instructions
  ////////////////////////////////////////////////////////

  auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");

  // Loop over all possible string configurations
  for (int i = 0; i < std::pow(2, (int)qubits_string.size()); i++) {
    std::string bin_i = binary(i, (int)qubits_string.size());
    std::string current_state = flip_bitstring(gray_code(bin_i));

    std::vector<int> qubits_oracle_flags;
    for (int j = 0; j < qubits_flags.size(); j++) {
      qubits_oracle_flags.push_back(qubits_ancilla[j]);
    }

    std::vector<int> qubits_oracle_string;
    for (int j = 0; j < qubits_string.size(); j++) {
        qubits_oracle_string.push_back(qubits_ancilla[j+qubits_flags.size()]);
    }

    // for (auto bit : qubits_oracle_flags) {
    //     std::cout << "oracle flags " << bit << "\n";
    // }
    // for (auto bit : qubits_oracle_string) {
    //     std::cout << "oracle string " << bit << "\n";
    // }

    int num_qubits_per_letter = (int)qubits_string.size() / (int)qubits_flags.size();
    int num_letters = qubits_flags.size();

    int num_superfluous = 0;

    for (int j = 0; j < num_letters; j++) {
        auto current_letter = current_state.substr(j*num_qubits_per_letter, num_qubits_per_letter);

        bool null = true;
        for (int k = 0; k < num_qubits_per_letter; k++) {
            if (current_letter[k] == '1') {
                null = false;
            }
        }

        bool repeat = false;
        if (j > 0) {
            auto previous_letter = current_state.substr((j-1)*num_qubits_per_letter, num_qubits_per_letter);
            int total_matches = 0;
            for (int k = 0; k < num_qubits_per_letter; k++) {
                if (current_letter[k] == previous_letter[k]) {
                    total_matches++;
                }
            }
            if (total_matches == num_qubits_per_letter) {
                repeat = true;
            }
        }

        if (null || repeat) {
            num_superfluous++;
        } else {
            for (int k = 0; k < num_qubits_per_letter; k++) {
                if (current_letter[k] == '1') {
                    addInstruction(gateRegistry->createInstruction("X", qubits_oracle_string[(j-num_superfluous)*num_qubits_per_letter+k]));
                }
            }
        }
    }

    for (int j = 0; j < num_superfluous; j++) {
        addInstruction(gateRegistry->createInstruction("X", qubits_oracle_flags[num_letters-1-j]));
    }

    std::vector<int> qubits_ancilla_mean;
    for (int j = qubits_flags.size() + qubits_string.size(); j < qubits_ancilla.size(); j++) {
      qubits_ancilla_mean.push_back(qubits_ancilla[j]);
    }

      // Compare string configuration (qubits_string) to |111...1> state.
      auto compare_beam = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("CompareBeamOracle"));
      xacc::HeterogeneousMap options_cb{{"q0", q0}, {"q1", q1}, {"q2", q2},
                                        {"FA", qubits_flags},
                                        {"FB", qubits_oracle_flags},
                                        {"SA", qubits_string},
                                        {"SB", qubits_oracle_string},
                                        {"simplified", false}};
      const bool expand_ok_cb = compare_beam->expand(options_cb);
      assert(expand_ok_cb);
      addInstruction(compare_beam);

      std::shared_ptr<xacc::CompositeInstruction> ae_state_prep_circ_clone =
          xacc::ir::asComposite(ae_state_prep_circ->clone());
      std::shared_ptr<xacc::CompositeInstruction> compare_beam_clone =
          xacc::ir::asComposite(compare_beam->clone());

      // Add amplitude of the metric register if q0 = 1.
      // Pass state_preparation_circuit and oracle into the mean value finder to
      // calculate mean of amplitude of each metric qubit
      auto mean = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("MeanValueFinder"));
      xacc::HeterogeneousMap options_mean{
          {"qubits_superposition", qubits_metric},
          {"qubits_superposition_state_prep", ae_state_prep_circ_clone},
          {"qubits_mean", qubits_beam_metric},
          {"qubits_ancilla", qubits_ancilla_mean},
          {"qubit_indicator", q0},
          {"qubit_indicator_state_prep", compare_beam_clone}};
      const bool expand_ok_mean = mean->expand(options_mean);
      assert(expand_ok_mean);
      addInstruction(mean);

     // if (i == 4) {break;}

      // Undo compare beam oracle
      auto inverse_oracle = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("InverseCircuit"));
      xacc::HeterogeneousMap options_inverse_oracle{{"circ", compare_beam}};
      const bool expand_ok_inverse_oracle = inverse_oracle->expand(options_inverse_oracle);
      addInstruction(inverse_oracle);

    int num_superfluous_undo = 0;
    for (int j = 0; j < num_letters; j++) {
        auto current_letter = current_state.substr(j*num_qubits_per_letter, num_qubits_per_letter);

        bool null = true;
        for (int k = 0; k < num_qubits_per_letter; k++) {
            if (current_letter[k] == '1') {
                null = false;
            }
        }

        bool repeat = false;
        if (j > 0) {
            auto previous_letter = current_state.substr((j-1)*num_qubits_per_letter, num_qubits_per_letter);
            int total_matches = 0;
            for (int k = 0; k < num_qubits_per_letter; k++) {
                if (current_letter[k] == previous_letter[k]) {
                    total_matches++;
                }
            }
            if (total_matches == num_qubits_per_letter) {
                repeat = true;
            }
        }

        if (null || repeat) {
            num_superfluous_undo++;
        } else {
            for (int k = 0; k < num_qubits_per_letter; k++) {
                if (current_letter[k] == '1') {
                    addInstruction(gateRegistry->createInstruction("X", qubits_oracle_string[(j-num_superfluous_undo)*num_qubits_per_letter+k]));
                }
            }
        }
    }

    for (int j = 0; j < num_superfluous_undo; j++) {
        addInstruction(gateRegistry->createInstruction("X", qubits_oracle_flags[num_letters-1-j]));
    }

    // Flip bit so that the next new string is the current |111...1> string.
    if (i == std::pow(2, (int)qubits_string.size()) - 1) {
      //addInstruction(gateRegistry->createInstruction("X", qubits_string[0]));
    } else {
      std::string bin_ip1 = binary(i + 1, (int)qubits_string.size());
      std::string next_state = flip_bitstring(gray_code(bin_ip1));
      int bit_to_flip = different_bit_index(current_state, next_state);
      bool is_string_qubit = false;
      if (bit_to_flip < qubits_string.size()) {
        is_string_qubit = true;
      }
      // Update string (qubits_string) to be compared with |111...1> in the next loop iteration.
      if (is_string_qubit) {
        //addInstruction(gateRegistry->createInstruction("X", qubits_string[bit_to_flip]));
      }
    }
  }

  return true;
}

const std::vector<std::string> SuperpositionAdder::requiredKeys() {
  return {"q0", "q1", "q2", "qubits_falgs", "qubits_string", "qubits_metric",
  "ae_state_prep_circ", "qubits_ancilla", "qubits_beam_metric"};
}
} //namespace qbOS

/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include "qristal/core/circuits/efficient_encoding.hpp"
#include "CommonGates.hpp"
#include "IRProvider.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <bitset>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace qristal {
bool EfficientEncoding::expand(const xacc::HeterogeneousMap &runtimeOptions) {

  // Inputs:
  if (!runtimeOptions.keyExists<std::function<int(int)>>("scoring_function")) {
    return false;
  }
  std::function<int(int)> scoring_function =
      runtimeOptions.get<std::function<int(int)>>("scoring_function");

  if (!runtimeOptions.keyExists<int>("num_state_qubits")) {
    return false;
  }
  int num_state_qubits = runtimeOptions.get<int>("num_state_qubits");
  assert(num_state_qubits > 0);

  if (!runtimeOptions.keyExists<int>("num_scoring_qubits")) {
    return false;
  }
  int num_scoring_qubits = runtimeOptions.get<int>("num_scoring_qubits");
  assert(num_scoring_qubits > 0);

  std::vector<int> state_qubits;
  std::vector<int> scoring_qubits;

  if (runtimeOptions.keyExists<std::vector<int>>("state_qubits")) {
    state_qubits = runtimeOptions.get<std::vector<int>>("state_qubits");
  }
  if (runtimeOptions.keyExists<std::vector<int>>("scoring_qubits")) {
    scoring_qubits = runtimeOptions.get<std::vector<int>>("scoring_qubits");
  }

  if (state_qubits.size() > 0 || scoring_qubits.size() > 0) {
    assert(state_qubits.size() == num_state_qubits);
    assert(scoring_qubits.size() == num_scoring_qubits);
  }

  if (state_qubits.size() == 0) {
    for (int i = 0; i < num_state_qubits; i++) {
      state_qubits.push_back(i);
    }

  }
  assert(state_qubits.size() == num_state_qubits);

  if (scoring_qubits.size() == 0) {
    for (int i = 0; i < num_scoring_qubits; i++) {
      scoring_qubits.push_back(num_state_qubits + i);
    }
  }
  assert(scoring_qubits.size() == num_scoring_qubits);


  // For setting the initial flags
  std::vector<int> qubits_init_flag;
  if (runtimeOptions.keyExists<std::vector<int>>("qubits_init_flag")) {
    qubits_init_flag = runtimeOptions.get<std::vector<int>>("qubits_init_flag");
  }
  int flag_integer = runtimeOptions.get_or_default("flag_integer", 0);

  bool is_LSB = true;
  if (runtimeOptions.keyExists<bool>("is_LSB")) {
    is_LSB = runtimeOptions.get<bool>("is_LSB");
  }

  bool use_ancilla = false;
  if (runtimeOptions.keyExists<bool>("use_ancilla")) {
    use_ancilla = runtimeOptions.get<bool>("use_ancilla");
  }

  std::vector<int> ancilla_bits;
  if (use_ancilla == true && runtimeOptions.keyExists<std::vector<int>>("ancilla_qubits")) {
      ancilla_bits = runtimeOptions.get<std::vector<int>>("ancilla_qubits");
  }

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
    std::string i_binary = std::bitset<8*sizeof(i)>(i).to_string();
    std::string i_binary_n = i_binary.substr(
        i_binary.size() < num_qubits ? 0 : i_binary.size() - num_qubits);
    reverse(i_binary_n.begin(), i_binary_n.end());
    return i_binary_n;
  };

  // binary to int function
  std::function<int(std::string)> integer = [&](std::string str) {
    if (is_LSB) {
      reverse(str.begin(), str.end());
    }
    int integer = std::stoi(str, 0, 2);
    return integer;
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

  // Perform encoding
  if (!use_ancilla) {
    //std::cout << "Mark initial position of null (no ancilla) " << qubits_init_flag << "\n" ;
    for (int i = 0; i < std::pow(2, num_state_qubits); i++) {
      auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
      std::string bin_i = binary(i, num_state_qubits);
      reverse(bin_i.begin(), bin_i.end());
      std::string current_state = flip_bitstring(gray_code(bin_i));
      reverse(bin_i.begin(), bin_i.end());
      int k = integer(current_state);
      // Mark initial flags if specified
      if (k == flag_integer and !qubits_init_flag.empty()) {
        std::vector<int> control_bits = state_qubits;
        auto x_gate = gateRegistry->createComposite("x_gate");
        //auto temp_gate =
          //  gateRegistry->createInstruction("X", qubits_init_flag); //null_qubit
        //temp_gate->setBufferNames({"q"});
        for (int qubit_flag : qubits_init_flag){
          x_gate->addInstruction(gateRegistry->createInstruction("X", qubit_flag));
        }
        auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("C-U"));
        mcx->expand({{"U", x_gate}, {"control-idx", control_bits}});
        addInstruction(mcx);
      }
      int score = scoring_function(k);
      std::string score_binary = binary(score, num_scoring_qubits);
      for (int j = 0; j < num_scoring_qubits; j++) {
        if (score_binary[j] == '1') {
          std::vector<int> control_bits = state_qubits;
          auto x_gate = gateRegistry->createComposite("x_gate");
          auto temp_gate =
              gateRegistry->createInstruction("X", scoring_qubits[j]);
          temp_gate->setBufferNames({"q"});
          x_gate->addInstruction(temp_gate);
          auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
              xacc::getService<xacc::Instruction>("C-U"));
          mcx->expand({{"U", x_gate}, {"control-idx", control_bits}});
          addInstruction(mcx);
        }
      }
      if (i == std::pow(2, num_state_qubits) - 1) {
        addInstruction(gateRegistry->createInstruction("X", state_qubits[0]));
      } else {
        std::string bin_ip1 = binary(i + 1, num_state_qubits);
        reverse(bin_ip1.begin(), bin_ip1.end());
        std::string next_state = flip_bitstring(gray_code(bin_ip1));
        reverse(bin_ip1.begin(), bin_ip1.end());
        int bit_to_flip = different_bit_index(current_state, next_state);
        addInstruction(
            gateRegistry->createInstruction("X", state_qubits[bit_to_flip]));
      }
    }
  }
  if (use_ancilla) {
    for (int i = 0; i < std::pow(2, num_state_qubits); i++) {
      auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
      std::string bin_i = binary(i, num_state_qubits);
      reverse(bin_i.begin(), bin_i.end());
      std::string current_state = flip_bitstring(gray_code(bin_i));
      reverse(bin_i.begin(), bin_i.end());
      int k = integer(current_state);
      // Mark initial position of null
      std::cout << "Mark initial flags (ancilla) " << qubits_init_flag.size() << "\n" ;
      if (k == flag_integer and !qubits_init_flag.empty()) {
        std::vector<int> control_bits = state_qubits;
        auto x_gate = gateRegistry->createComposite("x_gate");
        for (int qubit_flag : qubits_init_flag){
          x_gate->addInstruction(gateRegistry->createInstruction("X", qubit_flag));
        }
        auto mcx = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
            xacc::getService<xacc::Instruction>("C-U"));
        mcx->expand({{"U", x_gate}, {"control-idx", control_bits}});
        addInstruction(mcx);
      }
      int score = scoring_function(k);
      std::string score_binary = binary(score, num_scoring_qubits);
      auto U = gateRegistry->createComposite("U");
      for (int j = 0; j < num_scoring_qubits; j++) {
        if (score_binary[j] == '1') {
          U->addInstruction(
              gateRegistry->createInstruction("X", scoring_qubits[j]));
        }
      }
      std::vector<int> control_bits = state_qubits;
      int temp_index = 0;
      while ((int)ancilla_bits.size() < (int)control_bits.size() - 1) {
        if (std::find(state_qubits.begin(), state_qubits.end(), temp_index) !=
                state_qubits.end() ||
            std::find(scoring_qubits.begin(), scoring_qubits.end(),
                      temp_index) != scoring_qubits.end()) {
          temp_index = temp_index + 1;
        } else {
          ancilla_bits.push_back(temp_index);
          temp_index = temp_index + 1;
        }
      }
      auto mcu = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
          xacc::getService<xacc::Instruction>("MultiControlledUWithAncilla"));
      xacc::HeterogeneousMap options{{"qubits_control", control_bits},
                                     {"qubits_ancilla", ancilla_bits},
                                     {"U", U}};
      const bool expand_ok = mcu->expand(options);
      assert(expand_ok);
      addInstructions(mcu->getInstructions());
      if (i == std::pow(2, num_state_qubits) - 1) {
        addInstruction(gateRegistry->createInstruction("X", state_qubits[0]));
      } else {
        std::string bin_ip1 = binary(i + 1, num_state_qubits);
        reverse(bin_ip1.begin(), bin_ip1.end());
        std::string next_state = flip_bitstring(gray_code(bin_ip1));
        reverse(bin_ip1.begin(), bin_ip1.end());
        int bit_to_flip = different_bit_index(current_state, next_state);
        addInstruction(
            gateRegistry->createInstruction("X", state_qubits[bit_to_flip]));
      }
    }
  }

  return true;
}

const std::vector<std::string> EfficientEncoding::requiredKeys() {
  return {"scoring_function", "num_state_qubits", "num_scoring_qubits"};
}

}

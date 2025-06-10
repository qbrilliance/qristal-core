/***
 *** Copyright (c) Quantum Brilliance Pty Ltd
 ***/

#include <CompositeInstruction.hpp>
#include <CommonGates.hpp>
#include <Algorithm.hpp>
#include <xacc.hpp>
#include <xacc_plugin.hpp>
#include <xacc_service.hpp>
#include <qristal/core/circuits/w_prime_unitary.hpp>
#include <IRProvider.hpp>
#include <iostream>

namespace qristal{
bool WPrime::expand(const xacc::HeterogeneousMap &runtimeOptions) {
    //std::cout << "WPrime.expand()\n";

    if (!runtimeOptions.keyExists<
      std::vector<std::vector<float>>>("probability_table")) {
        return false;
    }
    std::vector<std::vector<float>> probability_table_ = runtimeOptions.get<
      std::vector<std::vector<float>>>("probability_table");

    if (!runtimeOptions.keyExists<int>("iteration")) {
        return false;
    }
    int iteration_ = runtimeOptions.get<int>("iteration");
//    std::cout << "iteration_" << iteration_ << "\n";

    if (!runtimeOptions.keyExists<std::vector<int>>("qubits_next_letter")){
        return false;
    }
    std::vector<int> qubits_next_letter_ = runtimeOptions.get<std::vector<int>>("qubits_next_letter");

    if (!runtimeOptions.keyExists<std::vector<int>>("qubits_next_metric")){
        return false;
    }
    std::vector<int> qubits_next_metric_ = runtimeOptions.get<std::vector<int>>("qubits_next_metric");

    // For setting the initial null flags
     std::vector<int> qubits_init_null_ ;
    if (runtimeOptions.keyExists<std::vector<int>>("qubits_init_null")){
        qubits_init_null_ = runtimeOptions.get<std::vector<int>>("qubits_init_null");
    }

    int null_integer = runtimeOptions.get_or_default("null_integer", 0);

    bool use_ancilla = false;
    if (runtimeOptions.keyExists<bool>("use_ancilla")) {
        use_ancilla = runtimeOptions.get<bool>("use_ancilla");
    }

    std::vector<int> ancilla_qubits = {};
    if (use_ancilla && runtimeOptions.keyExists<std::vector<int>>("ancilla_qubits")) {
        ancilla_qubits = runtimeOptions.get<std::vector<int>>("ancilla_qubits");
    }

    const int nq_qubits_metric_ = qubits_next_metric_.size();
    const int nq_qubits_letter_ = qubits_next_letter_.size();

    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    for (int qubit : qubits_next_letter_) {
        addInstruction(gateRegistry->createInstruction("H", qubit));
    }
    std::vector<float> probability_column = probability_table_[iteration_];
    int num_state_qubits = nq_qubits_letter_;
    int num_scoring_qubits = nq_qubits_metric_;
    std::vector<int> state_qubits = qubits_next_letter_;
    std::vector<int> scoring_qubits = qubits_next_metric_;

    std::function<int(int)> scoring_function = [&](int symbol) {
      int temp = num_scoring_qubits;
      float flt = probability_column[symbol];
      std::string binary;
      if (flt == 1.0) {
        for (int i = 0; i < num_scoring_qubits; i++) {
          binary.push_back('1');
        }
      } else if (flt == 0.0) {
          for (int i = 0; i < num_scoring_qubits; i++) {
              binary.push_back('0');
            }
        } else {
        while (temp) {
          flt *= 2;
          int fract_bit = flt;
          if (fract_bit == 1) {
            flt -= fract_bit;
            binary.push_back('1');
          } else {
            binary.push_back('0');
          }
          --temp;
        }
      }
      int score = std::stoi(binary, 0, 2);
      return score;
    };
    std::vector<int> flag;
    if (!qubits_init_null_.empty()) {flag = {qubits_init_null_[iteration_]};}

    auto ee = std::dynamic_pointer_cast<xacc::CompositeInstruction>(
        xacc::getService<xacc::Instruction>("EfficientEncoding"));
    ee->expand({{"scoring_function", scoring_function},
                {"num_state_qubits", num_state_qubits},
                {"num_scoring_qubits", num_scoring_qubits},
                {"state_qubits", state_qubits},
                {"scoring_qubits", scoring_qubits},
                {"qubits_init_flag", flag},
                {"flag_integer", null_integer},
                {"is_LSB", true},
                {"use_ancilla", use_ancilla},
                {"ancilla_qubits", ancilla_qubits}});
    addInstructions(ee->getInstructions());
    return true;
}

const std::vector<std::string> WPrime::requiredKeys() {
    // Two params: Um (state preparation) and oracle (comparator)
    return {"iteration","qubits_next_metric", "qubits_next_letter", "qubits_init_null"};    // state_prep
}

}

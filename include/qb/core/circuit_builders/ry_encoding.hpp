// Copyright (c) 2022 Quantum Brilliance Pty Ltd

/*
Quantum Decoder Mark III
Algorithm:
1. Encodes strings with prob. amplitudes proportional to the square root of their
probabilities
2. Perform the measurement

Outcome:
- Both strings and beams should be chosen in proportion to their total probability

Breakdown of algorithm:
A. Encode symbols with correct amplitude - Loop through alphabet at each timestep
   1. Flip on the control qubit
   2. Encode symbol using ry operations with probability-determined angle controlled by the control qubit
   3. Flip the control unit off again

B. Measure results
   - The final quantum operation is to measure the register and process the
     obtained string. 
   - This string now requires some classical processing to obtain the correct
     output beam, meaning the process of contracting repetitions and then
     removing null symbols. This is not difficult classically and only needs to
     be done once

Worklog: https://www.notion.so/quantumbrilliance/220815-Worklog-Decoder-Mark-II-01a41031ecee4f5a999986924fb4d528
*/

#pragma once
//#include "qb/core/qbos_circuit_builder.hpp"
#include "CompositeInstruction.hpp"
#include "Circuit.hpp"
#include "IRProvider.hpp"
#include "InstructionIterator.hpp"
#include "Accelerator.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <math.h>
#include <iostream>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>
#include <bitset>

namespace qbOS {
  class RyEncoding : public qbOS::CircuitBuilder {
  public:
    RyEncoding() : qbOS::CircuitBuilder() {} ;

    const std::vector<std::string> requiredKeys() {
      return {"probability_table", "qubits_string"};
    }

    bool expand(const xacc::HeterogeneousMap &runtimeOptions) {
      //////////////////////////////////////////////////////////////////////////////////////

      // Load qubits
      if (!runtimeOptions.keyExists<std::vector<std::vector<float>>>("probability_table")) {
        return false;
      }
      std::vector<std::vector<float>> probability_table = runtimeOptions.get<std::vector<std::vector<float>>>("probability_table");

      if (!runtimeOptions.keyExists<std::vector<int>>("qubits_string")) {
        return false;
      }
      std::vector<int> qubits_string = runtimeOptions.get<std::vector<int>>("qubits_string");

      //auto runtimeKeys = runtimeOptions.KeyList();
      
      bool is_LSB = true;
      if (runtimeOptions.keyExists<bool>("is_LSB")) {
        is_LSB = runtimeOptions.get<bool>("is_LSB");
      }



      //////////////////////////////////////////////////////////////////////////////////////
      std::vector<float> probability_column;
      int string_length = probability_table.size(); // Number of rows of probability_table. The rows represent the string length or time steps.
      const int alphabet_size = probability_table[0].size(); //Number of columns of probability_table. Columns represent alphabets size.
      int nb_qubits_letter = qubits_string.size()/string_length;
      int nq_symbols = std::ceil(std::log2(alphabet_size));
      assert((string_length * nq_symbols) == qubits_string.size());
      
      float probability_remaining; 

      //////////////////////////////////////////////////////////////////////////////////////

      
      // Parameters for EfficientEncoding()
      //bool is_LSB = true;
      std::vector<int> qubits_init_flag = {};
      int flag_integer = 0;
      std::vector<int> qubits_letter;
      std::vector<int> qubits_label;
      for (int it = 0; it < string_length; it++) {
        //std::cout << "Timestep:" << it << std::endl;
        probability_remaining = 1.0;
        probability_column = probability_table[it];   

        qubits_letter.assign(qubits_string.begin() + it*nb_qubits_letter,qubits_string.begin() + (it+1)*nb_qubits_letter);
        
        // Deal with special case of only one symbol qubit
        if (nb_qubits_letter == 1) {
            this->RY(qubits_letter[0],asin(sqrt(probability_column[1]))*2);  
            continue;
        }
                        
        // Loop over symbols and magnify appropriately
        std::vector<int> qubits_control = qubits_letter; 
        float next_letter_prob; 
        
        for (int qubit : qubits_letter){
            this->X(qubit);
        }        
        // Rotate qubits_letter without overlapping other states
        for (int symbol = alphabet_size - 1; symbol > 0; symbol--) {
            next_letter_prob = probability_column[symbol];
            bool first_qubit = true;  // Only ry on first qubit
            
            // Rotate only qubits in symbol
            std::vector<int> qubits_reset;
            for (int qindex = 0; qindex < qubits_letter.size(); qindex++) {
                int qubit = qubits_letter[qindex]; 
                
                qubits_control.erase(qubits_control.begin());  // The zeroeth element is the qubit being written to
                if ((symbol & ((int) std::pow(2,qindex))) != 0 ) {//} ((int) std::pow(2,qindex))) {
                    this->X(qubit);
                    if (first_qubit) {
                        CircuitBuilder ry ;
                        ry.RY(qubit,asin(sqrt(next_letter_prob/probability_remaining))*2);  
                        this->CU(ry, qubits_control);  
                        first_qubit = false;
                    }
                    else {
                        this->MCX(qubits_control,qubit);
                    }
                    qubits_reset.push_back(qubit);
                }
                qubits_control.push_back(qubit);  // = qubits_letter.assign(qubits_letter.begin() + qindex,qubits_letter.begin() + qindex+1);
            }
            probability_remaining -= next_letter_prob ;

            // Reset qubits
            for (int qubit : qubits_reset){
                this->X(qubit);
            }

            if (probability_remaining == 0.0){
                break;
            }
        }
        for (int qubit : qubits_letter){
            this->X(qubit);
        }
        
      }    


      return true;
    }; // bool expand
  }; // class RyEncoding
} // namespace qbOS

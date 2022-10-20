// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#pragma once

#include "qb/core/circuit_builder.hpp"
#include "CompositeInstruction.hpp"
#include "Circuit.hpp"
#include "IRProvider.hpp"
#include "InstructionIterator.hpp"
#include "Accelerator.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

namespace qbOS {

    /// Exponential (base 2) circuit:
    /// Builds a circuit which finds the exponent base 2
    /// of a given value. Needed to convert log_2 of
    /// string metric to actual metric for addition

    class Exponent : public qbOS::CircuitBuilder {
    public:
      Exponent() : qbOS::CircuitBuilder() {} ;

      int nb_qubits_log = 0;
      int nb_qubits_exp = 0;

      const std::vector<std::string> requiredKeys() {
        return {"qubits_log"};
      }

      bool expand(const xacc::HeterogeneousMap &runtimeOptions) {
        // Inputs:
        // qubits_log: [vector<int>] of qubits holding the number we seek the exponent base 2 of
        // qubits_exponent: [vector<int>] of qubits to hold the exponent base 2
        // qubits_ancilla: [vector<int> of qubits to hold copy of qubits_log]
        // is_LSB: [bool] is the input trial score in LSB qubit ordering? (default True)
        bool is_LSB = true;
        if (runtimeOptions.keyExists<bool>("is_LSB")) {
            is_LSB = runtimeOptions.get<bool>("is_LSB");
        }

        if (!runtimeOptions.keyExists<std::vector<int>>("qubits_log")) {
            return false;
        }
        std::vector<int> qubits_log = runtimeOptions.get<std::vector<int>>("qubits_log");
        //std::cout << is_LSB << std::endl;
        nb_qubits_log = qubits_log.size();

        std::vector<int> qubits_exponent;
        if (runtimeOptions.keyExists<std::vector<int>>("qubits_exponent")) {
            std::vector<int> qubits_exponent = runtimeOptions.get<std::vector<int>>("qubits_exponent");
            // Check that qubits_exponent overlaps correctly with qubits_log
            for (int qindex=0; qindex < nb_qubits_log; qindex++){
              //std::cout << "qubits_exponent:" << qubits_exponent[qindex] << " qubits_log:" << qubits_log[qindex] << std::endl;
              assert(qubits_exponent[qindex] == qubits_log[qindex]);
            }
        }

        int min_significance = 1;
        if (runtimeOptions.keyExists<int>("min_significance")) {
            min_significance = runtimeOptions.get<int>("min_significance");
        }
        //std::cout << "min_significance:" << min_significance << " nb_qubits_log:" << nb_qubits_log << std::endl;
        if (nb_qubits_log < min_significance) {
            std::cout << "nb_qubits_log:" << nb_qubits_log << " min_significance:" << min_significance << std::endl;
            return false;
        }
        assert(nb_qubits_log >= 0);
        nb_qubits_exp = pow(2, pow(2,nb_qubits_log-1)) - (min_significance - 1);
        if (nb_qubits_exp < nb_qubits_log) {
          std::cout << "nb_qubits_exp return:" << nb_qubits_exp << std::endl;
          return false;
        }

        std::vector<int> qubits_ancilla;
        if (!runtimeOptions.keyExists<std::vector<int>>("qubits_ancilla")) {
          if (nb_qubits_exp > 0) {
            if (is_LSB) {
              //std::cout << "qubits_ancilla: " ;
              for (int qindex = 0; qindex < nb_qubits_log; qindex++){
                qubits_ancilla.push_back( nb_qubits_log + nb_qubits_exp + (min_significance - 1) -1 - qindex);
                //std::cout << nb_qubits_log + nb_qubits_exp + (min_significance - 1) -1 - qindex << ", " ;
              }
              //std::cout << std::endl;
            }
            else {
              for (int qindex = 0; qindex < nb_qubits_log; qindex++){
                qubits_ancilla.push_back(qubits_log[qindex] + nb_qubits_exp);
              }
            }
          }
        }
        else {
          qubits_ancilla = runtimeOptions.get<std::vector<int>>("qubits_ancilla");
        }

        if (is_LSB) {
            for (int qindex =0; qindex < nb_qubits_log; qindex++){
                this->CNOT(qubits_log[qindex], qubits_ancilla[qindex]);    // Replace with max(qubits_exponent)
                //std::cout << qindex << " nb_qubits_exp:" << nb_qubits_exp << " CNOT:" << qubits_log[qindex] << " " << qubits_ancilla[qindex] << std::endl;
            }
        }
        else {
              for (int qindex = 0; qindex < nb_qubits_log; qindex++){
                this->CNOT(qubits_log[qindex], qubits_ancilla[qindex]);    // Replace with max(qubits_exponent)
                //std::cout << "CNOT:" << qubits_log[qindex] << " " << qubits_ancilla[qindex] << std::endl;
              }

        }

        // Expand logarithms quantum register to contain the exponent
        if (qubits_exponent.size() == 0) {
            if (is_LSB) {
                for (int i = 0; i < nb_qubits_log; i++) {
                    qubits_exponent.push_back(qubits_log[i]);
                    //std::cout << i << " " << qubits_exponent.size() << std::endl;
                }
                for (int i = 0; i < nb_qubits_exp + min_significance - 1 - nb_qubits_log; i++) {
                    if (i<0) {
                        continue;
                    }
                    qubits_exponent.push_back(qubits_log.back() - 1 - i);
                    //std::cout << qubits_log.back() - 1 - i << " on top " << qubits_exponent.size() << std::endl;
                }
            }
            else {
                for (int i = 0; i < nb_qubits_log; i++) {
                    qubits_exponent.push_back(qubits_log[i]);
                    //std::cout << i << " " << qubits_exponent.size() << std::endl;
                }
                for (int i = 0; i < nb_qubits_exp - nb_qubits_log; i++) {
                    if (i<0) {
                        continue;
                    }
                    qubits_exponent.push_back(qubits_log.back() + 1 + i);
                    //std::cout << qubits_log.back() + 1 + i << " on top " << qubits_exponent.size() << std::endl;
                }
            }

            if (nb_qubits_exp>0) {
                for (int i = 0; i < min_significance-1 ; i++){
                    if (i >= nb_qubits_log) {
                        break;
                    }
                    this->CNOT(qubits_ancilla[i], qubits_log[i]);
                    //std::cout << "CNOT i:" << qubits_log[i] << " control:" << qubits_ancilla[i] << " nb_qubits_exp:" << nb_qubits_exp+i << std::endl;  //*max_element(qubits_exponent.begin(),qubits_exponent.end()) + 1 +i
                }
                if (min_significance > 1) {
                    for (int i = 0; i < (nb_qubits_log - min_significance + 1); i++ ) {
                        if (i+min_significance < nb_qubits_exp) {
                            this->SWAP(qubits_exponent[i], qubits_exponent[i+min_significance-1]);
                            //std::cout << "init swap: " << qubits_exponent[i] << ", " << qubits_exponent[i+min_significance-1] << std::endl;
                        }
                    }
                }
            }
        }
        //std::cout << "nb_qubits_exp:" <<  nb_qubits_exp << " qubits_exponent.size():" << qubits_exponent.size() << " min_significance:" << min_significance << std::endl ;

        // Begin construction of circuit
        //std::cout << "nb_qubits_exp:" << nb_qubits_exp << std::endl;
        for (int significance = nb_qubits_log; significance > 0; significance--){
            //std::cout << "significance:" << significance << std::endl;
            for (int qindex = 0; qindex < (nb_qubits_exp + min_significance-1 - significance - pow(2,significance)); qindex++ ){
                if (nb_qubits_exp < (1 + qindex + significance)){
                    continue;
                }
                this->ControlledSwap({qubits_exponent[nb_qubits_exp - 1 - qindex]}, {qubits_exponent[nb_qubits_exp - 1 - (qindex+significance)]},
                {qubits_ancilla[significance-1]}, {} );
                //std::cout << "c-swap:" << qubits_exponent[nb_qubits_exp - 1 - qindex] << ", " << qubits_exponent[nb_qubits_exp - 1 - (qindex+significance)] << ", " << qubits_ancilla[significance-1] << std::endl;
                if (significance >= min_significance) {
                  this->MCX({qubits_ancilla[significance-1],qubits_exponent[nb_qubits_exp - 1 - qindex]}, qubits_exponent[significance - min_significance]);
                  //std::cout << "MCX:" << qubits_ancilla[significance-1] << ", " << qubits_exponent[nb_qubits_exp - 1 - qindex] << ", " << qubits_exponent[significance-min_significance] << std::endl;
                }
            }

            if (significance >= min_significance) {
                if ((significance - min_significance) != (pow(2,significance-1)- min_significance+2)) {    // significance-1 != pow(2,significance-1)- min_significance+1
                  this->SWAP(qubits_exponent[significance-min_significance], qubits_exponent[pow(2,significance-1)- min_significance+1]);
                  //std::cout << "swap:" << qubits_exponent[significance-min_significance] << ", " << qubits_exponent[pow(2,significance-1) - min_significance + 1] << std::endl;
                }
            }
            //else {
                //std::cout << "log-swap skipped for significance " << significance << " out of " << nb_qubits_log << std::endl;
            //}
        }

        return true;
      };
    };
}

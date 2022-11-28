// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "qb/core/circuit_builders/ry_encoding.hpp"
#include "qb/core/circuit_builder.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <bitset>
#include <random>
#include <bitset>
#include <string>
#include <iterator>
#include <map>

std::map<std::string, int> check_encoding(std::vector<std::vector<float>> prob_table,
                           std::vector<int> qubits_string) {

    qbOS::CircuitBuilder circ;
    
    const xacc::HeterogeneousMap &map = {
        {"probability_table", prob_table},
        {"qubits_string", qubits_string}};

  
    qbOS::RyEncoding build;
    const bool expand_ok = build.expand(map);
    circ.append(build);

    // Measure
    for (int i = 0; i < qubits_string.size(); i++) {
       circ.Measure(qubits_string[i]);
    }

    // Construct the full circuit including preparation of input trial score
    auto circuit = circ.get();

    // Run circuit
    //std::cout << circuit->toString() << '\n';
    auto acc = xacc::getAccelerator("qpp", {{"shots", 100000}});
    auto buffer = xacc::qalloc(qubits_string.size());
    acc->execute(buffer, circuit);
    auto measurements = buffer->getMeasurementCounts();
    std::string output_string = buffer->toString() ;

    const int nq_string = qubits_string.size();
    const int nb_timesteps = prob_table.size();
    const int nq_symbol = nq_string/nb_timesteps;

    
    std::map<std::string, int>::iterator iter;
    const int nb_symbols = prob_table[0].size() ;
    const int nq_symbols = std::ceil(std::log2(nb_symbols));
    for (int step = 0; step < nb_timesteps; step++) {
        std::vector<float> probability_column = prob_table[step];
        int shot_tally = 0;
        for (int symbol = 0; symbol < nb_symbols; symbol++) {
            std::string bitstring = std::bitset<2>(symbol).to_string();
            std::reverse(bitstring.begin(),bitstring.end()); 
            bitstring = bitstring.substr(0,nq_symbols);
            for (iter=measurements.begin(); iter != measurements.end(); iter++) {
                std::string check_key = iter->first;
                if (check_key.substr(step*nq_symbols,nq_symbols) == bitstring) {
                    shot_tally += iter->second;
                }
            }
            shot_tally = round(shot_tally/1000.0);
            //buffer->print();
            //std::cout << "shot_tally:" << shot_tally << " probability:" << 100* probability_column[symbol] << std::endl;
            assert(shot_tally == round(100*probability_column[symbol]));
        }
    }
    //std::cout << measurements.size() << std::endl;
    return measurements;
}

TEST(RyEncodingTester_1, checkSimple) {
  std::vector<std::vector<float>> probability_table = {{0.0, 0.5, 0.5, 0.0}, {0.25, 0.25, 0.25, 0.25}};  
  std::vector<int> qubits_string = {0,1,2,3};

  std::map<std::string, int> measurements = check_encoding(probability_table,qubits_string);
  //std::cout << output_string << std::endl;
  //buffer_->print();
}

TEST(RyEncodingTester_2, checkSimple) {
  std::vector<std::vector<float>> probability_table = {{0.75, 0.15, 0.05, 0.05}, {0.05, 0.10, 0.25, 0.6}}; 
  std::vector<int> qubits_string = {0,1,2,3};

  std::map<std::string, int> measurements = check_encoding(probability_table,qubits_string);
  //std::cout << output_string << std::endl;
  //buffer_->print();
}

TEST(RyEncodingTester_3, checkSimple) {
  std::vector<std::vector<float>> probability_table = {{0.9999, 0.0001},
                                                       {0.001, 0.999}};
  std::vector<int> qubits_string = {0,1};

  std::map<std::string, int> measurements = check_encoding(probability_table,qubits_string);
  //std::cout << output_string << std::endl;
  //buffer_->print();
}


TEST(RyEncodingTester_4, checkSimple) {
  std::vector<std::vector<float>> probability_table = {{0.5,0.25,0.25},{0.1,0.7,0.2},{0.5,0.5,0.0}};
  std::vector<int> qubits_string = {0,1,2,3,4,5};

  std::map<std::string, int> measurements = check_encoding(probability_table,qubits_string);
  //std::cout << output_string << std::endl;
  //buffer_->print();
}


int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

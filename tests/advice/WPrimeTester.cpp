// Copyright (c) 2022 Quantum Brilliance Pty Ltd
#include "Circuit.hpp"
#include "xacc.hpp"
#include "xacc_service.hpp"
#include <iostream>
#include <bitset>
#include <math.h>
#include "IRProvider.hpp"
#include "../w_prime_unitary.hpp"
//#include <array>
#include <gtest/gtest.h>


TEST(WPrimeTester_1, checkSimple) {
    std::cout << "WPrimeTester1:\n";
    auto gateRegistry = xacc::getService<xacc::IRProvider>("quantum");
    //auto oracle = gateRegistry->createComposite("T_oracle");
    //oracle->addInstruction(gateRegistry->createInstruction("T", 0));
    // Initialise qubits, no ancilla yet
    std::vector<int> qubits_control = {0};
    //std::vector<int> key = qubits_control;
    std::vector<int> qubits_score = {1,2};
    //key.insert(key.end(),qubits_score.begin(),qubits_score.end());
    std::vector<int> qubits_string = {3,4,5,6,7,8};
    //key.insert(key.end(),qubits_string.begin(),qubits_string.end());
    std::vector<int> qubits_next_letter_metric = {9,10};
    //key.insert(key.end(),qubits_next_letter_metric.begin(),qubits_next_letter_metric.end());
    std::vector<int> qubits_next_letter = {11,12};
    //key.insert(key.end(),qubits_next_letter.begin(),qubits_next_letter.end());
    int key[13];
    key[0] = qubits_control[0];
    for (int qindex = 0; qindex < 2; qindex++){
        key[qindex+1] = qubits_score[qindex]; 
        key[qindex+9] = qubits_next_letter_metric[qindex];
        key[qindex+11] = qubits_next_letter[qindex];
    }
    for (int qindex = 0; qindex < 6; qindex++){     // 2 qubits times 3 symbols
        key[qindex+3] = qubits_string[qindex];
    }

    const int *min_key = std::min_element(std::begin(key), std::end(key));
    const int *max_key = std::max_element(std::begin(key), std::end(key));
    std::cout << *min_key << *max_key << "\n";
    const int offset_key = *max_key + *min_key;

    std::vector<std::string> alphabet = {"a","b","c"};
    //std::vector<std::string> Grammar = {"aa"};

    std::vector<std::vector<float>> probability_table{{.5,0.25,0.25},{0.1,0.7,0.2},{0.0,0.5,0.5}};    //[0.0,0.0,0.0],
        
    //std::cout << "w_prime\n";
    //auto w_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("WPrime"));   // xacc::getService<xacc::IRProvider>("quantum");  
    for (int iteration_ = 0; iteration_ < 3 ; iteration_++) {
        std::shared_ptr<xacc::quantum::WPrime> w_prime = std::make_shared<xacc::quantum::WPrime>();
        xacc::HeterogeneousMap map = {{"iteration",iteration_}, {"qubits_control", qubits_control}, {"probability_table",probability_table},
            {"qubits_next_letter_metric", qubits_next_letter_metric},{"qubits_next_letter", qubits_next_letter}};
        w_prime->expand(map);
        
        // Simulation test:
        // Construct the full circuit, include state prep (eigen state of |1>);
        //std::cout << "w_prime_test create\n";
        auto w_prime_test = gateRegistry->createComposite("sim_wprime");
        int qubit_ancilla_prob;
        int qubit_next_letter_probabilities;
        
        //std::cout << "w_prime.getInstructions()\n";
        w_prime_test->addInstructions(w_prime->getInstructions());
        // Measure evaluation qubits:
        for (int i = 0; i < *max_key + 1; ++i) {
            w_prime_test->addInstruction(gateRegistry->createInstruction("Measure", i));
        }
        std::cout << "HOWDY: WPrime circuit:\n";
        //std::cout << w_prime_test->toString() << '\n';
        // Sim:
        auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}, {"seed", 1234}});
        auto buffer = xacc::qalloc(*max_key + 1);
        acc->execute(buffer, w_prime_test);
        buffer->print();

        std::vector<float> probability_column = probability_table[iteration_];

        EXPECT_EQ(buffer->getMeasurementCounts().size(), 4);
        switch (iteration_) {
            case 0:
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000011"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000011"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["0000000001010"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000001010"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["0000000001001"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000001001"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000100"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000100"], 290);
                break;
            case 1:
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000011"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000011"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000110"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000110"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000001"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000001"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000000"], 290);
                break;
            case 2:
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000011"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000011"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000110"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000110"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000101"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000101"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["0000000000000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["0000000000000"], 290);
                break;
            default:
                std::cout << "iteration should be one of 0, 1, 2 not " << iteration_ << std::endl;
                //EXPECT_TRUE(false);
                throw;
        }

    }
    
    
}



int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  xacc::Finalize();
  return ret;
}

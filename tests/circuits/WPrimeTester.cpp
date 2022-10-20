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
    // Initialise qubits,
    //std::vector<int> key = qubits_control;
    std::vector<int> qubits_score = {0,1};
    //key.insert(key.end(),qubits_score.begin(),qubits_score.end());
    std::vector<int> qubits_string = {2,3,4,5,6,7};
    //key.insert(key.end(),qubits_string.begin(),qubits_string.end());
    std::vector<int> qubits_next_letter_metric = {8,9};
    //key.insert(key.end(),qubits_next_letter_metric.begin(),qubits_next_letter_metric.end());
    std::vector<int> qubits_next_letter = {10,11};
    //key.insert(key.end(),qubits_next_letter.begin(),qubits_next_letter.end());
    std::vector<int> qubits_init_null = {12,13,14};

    std::vector<std::string> alphabet = {"a","b","c"};
    //std::vector<std::string> Grammar = {"aa"};

    std::vector<std::vector<float>> probability_table{{.5,0.25,0.25},{0.1,0.7,0.2},{0.0,0.5,0.5}};    //[0.0,0.0,0.0],
        
    //std::cout << "w_prime\n";
    for (int iteration_ = 0; iteration_ < 3 ; iteration_++) {
        auto w_prime = std::dynamic_pointer_cast<xacc::CompositeInstruction>(xacc::getService<xacc::Instruction>("WPrime"));   // xacc::getService<xacc::IRProvider>("quantum");  
        xacc::HeterogeneousMap map = {{"iteration",iteration_},  {"probability_table",probability_table},
            {"qubits_next_metric", qubits_next_letter_metric},{"qubits_next_letter", qubits_next_letter},
            {"qubits_init_null", qubits_init_null}};
        const bool expand_ok = w_prime->expand(map);
        assert(expand_ok);
        
        // Simulation test:
        // Construct the full circuit, include state prep (eigen state of |1>);
        //std::cout << "w_prime_test create\n";
        auto w_prime_test = gateRegistry->createComposite("sim_wprime");
        int qubit_ancilla_prob;
        int qubit_next_letter_probabilities;
        
        //std::cout << "w_prime.getInstructions()\n";
        w_prime_test->addInstructions(w_prime->getInstructions());
        // Measure evaluation qubits:
        for (int i = 0; i < 15; ++i) {
            w_prime_test->addInstruction(gateRegistry->createInstruction("Measure", i));
        }

        // Sim:
        auto acc = xacc::getAccelerator("qpp", {{"shots", 1024}, {"seed", 1234}});
        auto buffer = xacc::qalloc(15);
        acc->execute(buffer, w_prime_test);
       // buffer->print();

        std::vector<float> probability_column = probability_table[iteration_];

        EXPECT_EQ(buffer->getMeasurementCounts().size(), 4);
        switch (iteration_) {
            case 0:
                EXPECT_GT(buffer->getMeasurementCounts()["000000000011000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000011000"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["000000001010000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000001010000"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["000000001001000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000001001000"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["000000000100100"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000100100"], 290);
                break;
            case 1:
                EXPECT_GT(buffer->getMeasurementCounts()["000000000011000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000011000"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["000000000110000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000110000"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["000000000001000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000001000"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["000000000000010"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000000010"], 290);
                break;
            case 2:
                EXPECT_GT(buffer->getMeasurementCounts()["000000000011000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000011000"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["000000000110000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000110000"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["000000000101000"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000101000"], 290);
                EXPECT_GT(buffer->getMeasurementCounts()["000000000000001"], 210);
                EXPECT_LT(buffer->getMeasurementCounts()["000000000000001"], 290);
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

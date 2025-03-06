// Copyright (c) Quantum Brilliance Pty Ltd
#include <gtest/gtest.h>

#define ZIP_VIEW_INJECT_STD_VIEWS_NAMESPACE //to add zip to the std namespace
#include "qristal/core/tools/zip_tool.hpp"
#include "qristal/core/benchmark/workflows/PyGSTiBenchmark.hpp"

using namespace qristal::benchmark;

TEST(PyGSTiBenchmarkTester, check_circuit_readin) {
    const size_t n_qubits = 11;

    //define session  
    qristal::session sim(false); 
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(1000);
    sim.set_qn(n_qubits);

    //circuit strings to test
    std::vector<std::string> circuit_list{
        "{}@(0)",
        "Gxpi2:0@(0)",
        "Gypi2:0@(0)",
        "Gzpi2:0@(0)",
        "Gxpi4:0Gypi4:0Gzpi4:0@(0)",
        "Gn:0@(0)",
        "Gcnot:0:1@(0,1)",
        "Gcz:0:1@(0,1)",
        "Gcphase:0:1@(0,1)",
        "Gxx:0:1@(0,1)",
        "Gyy:0:1@(0,1)",
        "Gzz:0:1@(0,1)",
        "Gxxpi2:0:1Gyypi2:0:1Gzzpi2:0:1@(0,1)",
        "Gxx:9:10Gxpi2:7Gcz:0:2Gzzpi2:4:3@(0,1,2,3,4,5,6,7,8,9,10)"
    };

    //List of correctly assembled circuits (to compare to)
    std::vector<qristal::CircuitBuilder> correct_circuits; 
    {
        qristal::CircuitBuilder cb; 
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RX(0, std::numbers::pi / 2.0);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RY(0, std::numbers::pi / 2.0);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RZ(0, std::numbers::pi / 2.0);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RX(0, std::numbers::pi / 4.0);
        cb.RY(0, std::numbers::pi / 4.0);
        cb.RZ(0, std::numbers::pi / 4.0);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RX(0, std::numbers::pi / 2.0);
        cb.RY(0, sqrt(3.0) / 2.0);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.CNOT(0, 1);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.CZ(0, 1);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.CPhase(0, 1, std::numbers::pi);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RY(0, std::numbers::pi / 2.0); 
        cb.X(0);
        cb.CZ(0, 1); 
        cb.RX(1, -1.0 * std::numbers::pi);
        cb.CZ(0, 1); 
        cb.RY(0, std::numbers::pi / 2.0);
        cb.X(0);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RX(0, std::numbers::pi / 2.0); 
        cb.RX(1, -1.0 * std::numbers::pi / 2.0); 
        cb.RY(1, -1.0 * std::numbers::pi / 2.0);
        cb.CZ(0, 1); 
        cb.RX(1, -1.0 * std::numbers::pi);
        cb.CZ(0, 1); 
        cb.RX(0, -1.0 * std::numbers::pi / 2.0);
        cb.RY(1, std::numbers::pi / 2.0);
        cb.RX(1, std::numbers::pi / 2.0);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RY(1, std::numbers::pi / 2.0); 
        cb.X(1);
        cb.CZ(0, 1); 
        cb.RX(1, -1.0 * std::numbers::pi);
        cb.CZ(0, 1); 
        cb.RY(1, std::numbers::pi / 2.0);
        cb.X(1);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RY(0, std::numbers::pi / 2.0); 
        cb.X(0);
        cb.CZ(0, 1); 
        cb.RX(1, std::numbers::pi / 2.0);
        cb.CZ(0, 1); 
        cb.RY(0, std::numbers::pi / 2.0);
        cb.X(0);

        cb.RX(0, std::numbers::pi / 2.0); 
        cb.RX(1, -1.0 * std::numbers::pi / 2.0); 
        cb.RY(1, -1.0 * std::numbers::pi / 2.0);
        cb.CZ(0, 1); 
        cb.RX(1, std::numbers::pi / 2.0);
        cb.CZ(0, 1); 
        cb.RX(0, -1.0 * std::numbers::pi / 2.0);
        cb.RY(1, std::numbers::pi / 2.0);
        cb.RX(1, std::numbers::pi / 2.0);

        cb.RY(1, std::numbers::pi / 2.0); 
        cb.X(1);
        cb.CZ(0, 1); 
        cb.RX(1, std::numbers::pi / 2.0);
        cb.CZ(0, 1); 
        cb.RY(1, std::numbers::pi / 2.0);
        cb.X(1);
        correct_circuits.push_back(cb);
    }
    {
        qristal::CircuitBuilder cb; 
        cb.RY(9, std::numbers::pi / 2.0); 
        cb.X(9);
        cb.CZ(9, 10); 
        cb.RX(10, -1.0 * std::numbers::pi);
        cb.CZ(9, 10); 
        cb.RY(9, std::numbers::pi / 2.0);
        cb.X(9);

        cb.RX(7, std::numbers::pi / 2.0);

        cb.CZ(0, 2);

        cb.RY(3, std::numbers::pi / 2.0); 
        cb.X(3);
        cb.CZ(4, 3); 
        cb.RX(3, std::numbers::pi / 2.0);
        cb.CZ(4, 3); 
        cb.RY(3, std::numbers::pi / 2.0);
        cb.X(3);
        correct_circuits.push_back(cb);
    }

    //Now construct workflow from std::vector<std::string> and obtain circuits
    PyGSTiBenchmark workflow(circuit_list, sim);
    auto assembled_circuits = workflow.get_circuits();
    
    //and compare
    for (auto const & [correct, assembled] : std::ranges::views::zip(correct_circuits, assembled_circuits)) {
        EXPECT_EQ(correct.get()->toString(), assembled.get()->toString());
    }

}
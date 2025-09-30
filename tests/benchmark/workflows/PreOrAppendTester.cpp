// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include <qristal/core/session.hpp>
#include <qristal/core/circuit_builder.hpp>
#include <qristal/core/benchmark/workflows/PreOrAppendWorkflow.hpp>
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/SPAMBenchmark.hpp>

// STL
#include <iostream>

// Gtest
#include <gtest/gtest.h>

using namespace qristal::benchmark;

TEST(PreOrAppendTester, check_circuit_construction) {
    std::set<size_t> qubits{0, 1};
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = 1000;
    sim.qn = qubits.size();

    //dummy test workflows 
    //(Option 1) SimpleCircuitExecution workflow
    qristal::CircuitBuilder cb; 
    cb.H(0);
    cb.CNOT(0, 1);
    SimpleCircuitExecution workflow_1(cb, sim);
    //(Option 2) SPAM
    SPAMBenchmark workflow_2(qubits, sim);
    //(Option 3) RotationSweep
    RotationSweep workflow_3({'X', 'Z'}, -180, 180, 3, sim);

    //Create pre- or appendable circuits
    //(Option A) Standard circuits
    qristal::CircuitBuilder IX; 
    IX.RY(1, 0.25);
    qristal::CircuitBuilder YX; 
    YX.RX(0, -1.3);
    YX.RY(1, 2.34);
    std::vector<qristal::CircuitBuilder> circuits_A {IX, YX};
    //(Option B) Pauli primitives 
    std::vector<std::vector<qristal::Pauli>> circuits_B {
        {qristal::Pauli::Symbol::I, qristal::Pauli::Symbol::X}, 
        {qristal::Pauli::Symbol::Y, qristal::Pauli::Symbol::X}
    };
    //(Option C) BlochSphereUnitState primitives 
    std::vector<std::vector<qristal::BlochSphereUnitState>> circuits_C {
        {qristal::BlochSphereUnitState::Symbol::Zm, qristal::BlochSphereUnitState::Symbol::Xp}, 
        {qristal::BlochSphereUnitState::Symbol::Zp, qristal::BlochSphereUnitState::Symbol::Ym}
    };

    //Correct circuits (for comparison)
    std::vector<std::vector<qristal::CircuitBuilder>> correct_circuits; 
    {   //1A
        qristal::CircuitBuilder base; 
        base.H(0);
        base.CNOT(0, 1);
        qristal::CircuitBuilder c1 = base.copy(); 
        c1.RY(1, 0.25);
        qristal::CircuitBuilder c2 = base.copy(); 
        c2.RX(0, -1.3);
        c2.RY(1, 2.34);
        correct_circuits.push_back({c1, c2});
    }
    {   //2B
        qristal::CircuitBuilder II;
        qristal::CircuitBuilder XI;
        XI.X(0);
        qristal::CircuitBuilder IX;
        IX.X(1);
        qristal::CircuitBuilder XX;
        XX.X(0);
        XX.X(1);
        std::vector<qristal::CircuitBuilder> bases {II, XI, IX, XX};
        std::vector<qristal::CircuitBuilder> circuits;
        for (const auto& base : bases) {
            qristal::CircuitBuilder c1 = base.copy();
            c1.RY(1, -1.0 * std::numbers::pi / 2.0);
            qristal::CircuitBuilder c2 = base.copy();
            circuits.push_back(c1);
            c2.RX(0, std::numbers::pi / 2.0);
            c2.RY(1, -1.0 * std::numbers::pi / 2.0);
            circuits.push_back(c2);
        }
        correct_circuits.push_back(circuits);
    }
    {   //3C
        qristal::CircuitBuilder rm;
        rm.RX(0, -1.0 * std::numbers::pi);
        rm.RZ(1, -1.0 * std::numbers::pi);
        qristal::CircuitBuilder r;
        r.RX(0, 0.0);
        r.RZ(1, 0.0);
        qristal::CircuitBuilder rp;
        rp.RX(0, 1.0 * std::numbers::pi);
        rp.RZ(1, 1.0 * std::numbers::pi);
        std::vector<qristal::CircuitBuilder> bases {rm, r, rp};
        std::vector<qristal::CircuitBuilder> circuits;
        for (const auto& base : bases) {
            qristal::CircuitBuilder c1; 
            c1.X(0); 
            c1.RY(1, std::numbers::pi / 2.0);
            c1.append(base);
            circuits.push_back(c1);
            qristal::CircuitBuilder c2; 
            c2.RX(1, std::numbers::pi / 2.0);
            c2.append(base);
            circuits.push_back(c2);
        }
        correct_circuits.push_back(circuits);
    }

    //Wrap workflows in PreOrAppendWorkflow and check circuit construction
    //1A
    PreOrAppendWorkflow<SimpleCircuitExecution> wworkflow_1(workflow_1, circuits_A, Placement::Append);
    for (size_t i = 0; i < correct_circuits[0].size(); ++i) {
        EXPECT_EQ(correct_circuits[0][i].get()->toString(), wworkflow_1.get_circuits()[i].get()->toString());
    }
    //2B
    PreOrAppendWorkflow<SPAMBenchmark> wworkflow_2(workflow_2, circuits_B, Placement::Append);
    for (size_t i = 0; i < correct_circuits[1].size(); ++i) {
        EXPECT_EQ(correct_circuits[1][i].get()->toString(), wworkflow_2.get_circuits()[i].get()->toString());
    }
    //3C
    PreOrAppendWorkflow<RotationSweep> wworkflow_3(workflow_3, circuits_C, Placement::Prepend);
    for (size_t i = 0; i < correct_circuits[2].size(); ++i) {
        EXPECT_EQ(correct_circuits[2][i].get()->toString(), wworkflow_3.get_circuits()[i].get()->toString());
    }
}
// Copyright (c) Quantum Brilliance Pty Ltd

// Qristal
#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/WorkflowAddins.hpp>
#include <qristal/core/benchmark/metrics/CircuitFidelity.hpp>
#include <qristal/core/benchmark/metrics/QuantumStateFidelity.hpp>
#include <qristal/core/benchmark/metrics/QuantumProcessFidelity.hpp>
#include <qristal/core/benchmark/DataLoaderGenerator.hpp>

// STL
#include <iostream>
#include <random>
#include <thread>
#include <chrono>

// Gtest
#include <gtest/gtest.h>

using namespace qristal::benchmark;

inline ComplexMatrix KroneckerExpand(const std::vector<ComplexMatrix>& densities) {
  ComplexMatrix temp = ComplexMatrix::Ones(1,1);
  for (auto const& density : densities) {
    temp = Eigen::kroneckerProduct(temp, density).eval();
  }
  return temp;
}

// --- Helper classes for circuit primitives ---

class GHZ {
  private: 
    qristal::CircuitBuilder circuit_; 
  public:
    GHZ(const size_t n = 2) {
      circuit_.H(0);
      for (size_t i = 0; i < n - 1; ++i) {
        circuit_.CNOT(i, i+1);
      }
    }

    qristal::CircuitBuilder& get_circuit() {
      return circuit_;
    }
};

class SingleBitstringCircuit {
  private: 
    const boost::dynamic_bitset<> bitset_;
    const size_t n_qubits_;
    qristal::CircuitBuilder circuit_;
  public: 
    SingleBitstringCircuit(const size_t n_qubits, const size_t bit) : n_qubits_(n_qubits), bitset_(n_qubits, bit) {
      for (size_t i = 0; i < bitset_.size(); ++i) {
        if (bitset_[i]) {
          circuit_.X(i);
        }
      }
    } 

    qristal::CircuitBuilder& get_circuit() {
      return circuit_;
    }

    std::vector<ComplexMatrix> Build1QDensities() const {
      std::vector<ComplexMatrix> densities;
      for (size_t i = 0; i < n_qubits_; ++i) {
        Eigen::VectorXcd state(2); 
        if (bitset_[bitset_.size() - 1 - i]) {
          state << 0, 1;
        }
        else {
          state << 1, 0;
        }
        densities.push_back(state * state.adjoint());
      }
      return densities;
    }

    std::vector<ComplexMatrix> Build1QProcesses() const {
      std::vector<ComplexMatrix> processes; 
      for (size_t i = 0; i < n_qubits_; ++i) {
        ComplexMatrix process = ComplexMatrix::Zero(4, 4);
        if (bitset_[bitset_.size() - 1 - i]) {
          process(1,1) = 1.0;
        }
        else {
          process(0,0) = 1.0;
        }
        processes.push_back(process);
      }
      return processes;
    }
};

class ParallelU3Circuit {
  private: 
    const std::vector<double> u3angles_;
    size_t n_qubits_;
    qristal::CircuitBuilder circuit_;
  public: 
    ParallelU3Circuit(const std::vector<double>& u3angles) : u3angles_(u3angles) {
      n_qubits_ = static_cast<size_t>(u3angles_.size() / 3);
      for (size_t i = 0; i < n_qubits_; ++i) {
        circuit_.H(i);
        circuit_.U3(i, u3angles_[3*i], u3angles_[3*i + 1], u3angles_[3*i + 2]);
      }
    } 

    qristal::CircuitBuilder& get_circuit() {
      return circuit_;
    }

    std::vector<ComplexMatrix> Build1QDensities() {
      std::vector<ComplexMatrix> densities;
      for (int i = n_qubits_ - 1; i >= 0; --i) {
        Eigen::VectorXcd state(2); 
        double c = cos(u3angles_[3*i] / 2.0);
        double s = sin(u3angles_[3*i] / 2.0);
        auto eiphi = 
            std::complex<double>(cos(u3angles_[3*i+1]), sin(u3angles_[3*i+1]));
        auto eilambda =
            std::complex<double>(cos(u3angles_[3*i+2]), sin(u3angles_[3*i+2]));
        state << c - eilambda * s, eiphi*s + eiphi*eilambda*c;

        densities.push_back(0.5 * state * state.adjoint());
      }
      return densities;
    }
};

// --- Unit tests ---

TEST(WorkflowAddinsTester, checkIdealCountsParallelU3) {
  const size_t n_tests = 10;
  const size_t n_layers = 4;
  const size_t n_qubits = 3; 
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(-2.0 * std::numbers::pi, 2.0 * std::numbers::pi);

  //(0) Define session 
  qristal::session sim; 
  sim.sn = 10000; 
  sim.qn = n_qubits;
  sim.acc = "qpp";
  
  //(1) Build random HEA circuits
  std::vector<qristal::CircuitBuilder> circuits;
  for (size_t test = 0; test < n_tests; ++test) {
    std::vector<double> random_values;
    for (int i = 0; i < 3 * n_qubits; ++i) {
      random_values.push_back(dis(gen));
    }
    circuits.push_back(ParallelU3Circuit(random_values).get_circuit());
  }

  //(2) Wrap into SimpleCircuitExecution and add in ideal counts prediction 
  typedef AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealCounts> Workflow;
  Workflow workflow(SimpleCircuitExecution(circuits, sim));

  //(3) Compute Circuit Fidelity and check against identity
  CircuitFidelity<Workflow> metric(workflow);
  std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
  for (auto const & t2v : results) {
      //check all fidelities
      for (auto const & f : t2v.second) {
          EXPECT_NEAR(f, 1.0, 5e-2);
      }
  }
}

TEST(WorkflowAddinsTester, checkIdealDensityGHZ) {
  const std::vector<size_t> qubits{2, 3, 4, 5};

  for (auto const & n_qubits : qubits) {
    //(0) Define session 
    qristal::session sim; 
    sim.sn = 1; 
    sim.qn = n_qubits;
    sim.acc = "qpp";

    //(1) Build GHZ circuit 
    qristal::CircuitBuilder circuit = GHZ(n_qubits).get_circuit();

    //(2) Wrap into SimpleCircuitExecution and add in ideal density prediction 
    typedef AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealDensity> Workflow;
    Workflow workflow(SimpleCircuitExecution(circuit, sim));

    //(3) Get ideal density and compare to expected result
    std::time_t t = workflow.execute({Task::IdealDensity});
    DataLoaderGenerator dlg(workflow.get_identifier(), std::vector<Task>{Task::IdealDensity});
    dlg.set_timestamps(std::vector<std::time_t>{t});
    ComplexMatrix density = dlg.obtain_ideal_densities()[0][0];
    auto last = density.rows() - 1;
    for (Eigen::Index i = 0; i < density.rows(); ++i) {
      for (Eigen::Index j = 0; j < density.rows(); ++j) {
        if ((i == 0 || i == last) && (j == 0 || j == last)) {
          EXPECT_NEAR(density(i,j).real(), 0.5, 1e-12);
          EXPECT_NEAR(density(i,j).imag(), 0.0, 1e-12);
        }
        else {
          EXPECT_NEAR(density(i,j).real(), 0.0, 1e-12);
          EXPECT_NEAR(density(i,j).imag(), 0.0, 1e-12);
        }
      }
    }
  }
}

TEST(WorkflowAddinsTester, checkIdealDensityBitstring) {  
  std::vector<size_t> n_qubits_list{1, 2, 3};

  for (auto const & n_qubits : n_qubits_list) {
    //(0) Define session 
    qristal::session sim; 
    sim.sn = 1; //single (ideal) shot is sufficient here
    sim.qn = n_qubits;
    sim.acc = "qpp";

    //(1) Build bitstring circuits
    std::vector<ComplexMatrix> ref_densities; 
    std::vector<qristal::CircuitBuilder> circuits;
    for (size_t bit = 1; bit < std::pow(2, n_qubits); ++bit) {
      auto circuit = SingleBitstringCircuit(n_qubits, bit);
      circuits.push_back(circuit.get_circuit());
      ref_densities.push_back(KroneckerExpand(circuit.Build1QDensities()));
    }

    //(2) Wrap into SimpleCircuitExecution and add in ideal density prediction 
    typedef AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealDensity> Workflow;
    Workflow workflow(SimpleCircuitExecution(circuits, sim));

    //(3) First test: Get ideal density and compare to expected result
    std::time_t t = workflow.execute({Task::IdealDensity});
    DataLoaderGenerator dlg(workflow.get_identifier(), std::vector<Task>{Task::IdealDensity});
    dlg.set_timestamps(std::vector<std::time_t>{t});
    std::vector<ComplexMatrix> densities = dlg.obtain_ideal_densities()[0];
    for (size_t i = 0; i < densities.size(); ++i) {
      EXPECT_TRUE(ref_densities[i].isApprox(densities[i], 1e-12));
    }

    //(4) Second test: Compute state fidelity (ideal single shot QST) and check against 1.0 
    std::this_thread::sleep_for(std::chrono::seconds(1)); //make sure generated timestamps are different
    typedef QuantumStateTomography<Workflow> QST; 
    QST qst_workflow(workflow);
    QuantumStateFidelity<QST> metric(qst_workflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 1e-6);
        }
    }
  }
}

TEST(WorkflowAddinsTester, checkIdealDensityParallelU3) {  
  std::vector<size_t> n_qubits_list{1, 2, 3};
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(-2.0 * std::numbers::pi, 2.0 * std::numbers::pi);

  for (auto const & n_qubits : n_qubits_list) {
    //(0) Define session 
    qristal::session sim; 
    sim.sn = 10000;
    sim.qn = n_qubits;
    sim.acc = "qpp";

    //(1) Build circuit
    std::vector<double> random_values;
    for (int i = 0; i < 3 * n_qubits; ++i) {
      random_values.push_back(dis(gen));
    }
    auto circuit = ParallelU3Circuit(random_values);

    //(2) Wrap into SimpleCircuitExecution and add in ideal density prediction 
    typedef AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealDensity> Workflow;
    Workflow workflow(SimpleCircuitExecution(circuit.get_circuit(), sim));

    //(3) First test: Get ideal density and compare to expected result
    std::time_t t = workflow.execute({Task::IdealDensity});
    DataLoaderGenerator dlg(workflow.get_identifier(), std::vector<Task>{Task::IdealDensity});
    dlg.set_timestamps(std::vector<std::time_t>{t});
    ComplexMatrix density = dlg.obtain_ideal_densities()[0][0];
    ComplexMatrix ref_density = KroneckerExpand(circuit.Build1QDensities());
    EXPECT_TRUE(ref_density.isApprox(density, 1e-12));
    
    //(4) Second test: Compute state fidelity and check against 1.0 
    std::this_thread::sleep_for(std::chrono::seconds(1)); //make sure generated timestamps are different
    typedef QuantumStateTomography<Workflow> QST; 
    QST qst_workflow(workflow);
    QuantumStateFidelity<QST> metric(qst_workflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 5e-2);
        }
    }
  }
}

TEST(WorkflowAddinsTester, checkIdealProcessGHZ) {
  const std::vector<size_t> qubits{2};

  for (auto const & n_qubits : qubits) {
    //(0) Define session 
    qristal::session sim; 
    sim.sn = 10000; 
    sim.qn = n_qubits;
    sim.acc = "qpp";

    //(1) Build GHZ circuit 
    qristal::CircuitBuilder circuit = GHZ(n_qubits).get_circuit();

    //(2) Wrap into SimpleCircuitExecution and add in ideal process prediction 
    typedef AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealProcess> Workflow;
    Workflow workflow(SimpleCircuitExecution(circuit, sim));

    //(3) Compute process fidelity and check against 1.0 
    typedef QuantumStateTomography<Workflow> QST;
    QST qst_workflow(workflow);
    typedef QuantumProcessTomography<QST> QPT; 
    QPT qpt_workflow(qst_workflow);
    QuantumProcessFidelity<QPT> metric(qpt_workflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 5e-2);
        }
    }
  }
}


TEST(WorkflowAddinsTester, checkIdealProcessBitstring) {
  std::vector<size_t> n_qubits_list{1, 2};

  for (auto const & n_qubits : n_qubits_list) {
    //(0) Define session 
    qristal::session sim; 
    sim.sn = 10000;
    sim.qn = n_qubits;
    sim.acc = "qpp";

    //(1) Build bitstring circuits
    std::vector<ComplexMatrix> ref_processes; 
    std::vector<qristal::CircuitBuilder> circuits;
    for (size_t bit = 1; bit < std::pow(2, n_qubits); ++bit) {
      auto circuit = SingleBitstringCircuit(n_qubits, bit);
      circuits.push_back(circuit.get_circuit());
      ref_processes.push_back(KroneckerExpand(circuit.Build1QProcesses()));
    }

    //(2) Wrap into SimpleCircuitExecution and add in ideal process prediction 
    typedef AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealProcess> Workflow;
    Workflow workflow(SimpleCircuitExecution(circuits, sim));

    //(3) First test: Get ideal process and compare to expected result
    std::time_t t = workflow.execute({Task::IdealProcess});
    DataLoaderGenerator dlg(workflow.get_identifier(), std::vector<Task>{Task::IdealProcess});
    dlg.set_timestamps(std::vector<std::time_t>{t});
    std::vector<ComplexMatrix> processes = dlg.obtain_ideal_processes()[0];
    for (size_t i = 0; i < processes.size(); ++i) {
      EXPECT_TRUE(ref_processes[i].isApprox(processes[i], 1e-12));
    }

    //(4) Second test: Compute process fidelity and check against 1.0 
    std::this_thread::sleep_for(std::chrono::seconds(1)); //make sure generated timestamps are different
    typedef QuantumStateTomography<Workflow> QST;
    QST qst_workflow(workflow);
    typedef QuantumProcessTomography<QST> QPT; 
    QPT qpt_workflow(qst_workflow);
    QuantumProcessFidelity<QPT> metric(qpt_workflow);
    std::map<std::time_t, std::vector<double>> results = metric.evaluate(true); //optional bool will force new execution
    for (auto const & t2v : results) {
        //check all fidelities
        for (auto const & f : t2v.second) {
            EXPECT_NEAR(f, 1.0, 5e-2);
        }
    }
  }
}
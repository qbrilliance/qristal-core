#include "qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp"
#include "qristal/core/benchmark/workflows/QuantumStateTomography.hpp"
#include "qristal/core/benchmark/metrics/QuantumStateDensity.hpp"

using namespace qristal::benchmark;

int main() {
    const size_t n_qubits = 2;
    const size_t n_shots = 1000;

    //(1) define session  
    qristal::session sim(false); 
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(n_shots);
    sim.set_qn(n_qubits);

    //(2) define workflow
    qristal::CircuitBuilder circuit; 
    circuit.H(0);
    circuit.CNOT(0, 1);
    SimpleCircuitExecution workflow(
        std::vector<qristal::CircuitBuilder>{circuit},
        sim
    );
    QuantumStateTomography<SimpleCircuitExecution> qstworkflow(workflow, true); //wrap into QST object and enable maximum likelihood estimation

    //(3) pass to metric
    QuantumStateDensity<QuantumStateTomography<SimpleCircuitExecution>> metric(qstworkflow);

    //(4) evaluate and print
    auto results = metric.evaluate();
    for (const auto& [timestamp, densities] : results) {
        std::cout << "Evaluated metric from " << "UTC: " << std::put_time(std::gmtime(&timestamp), "%c %Z") 
                  << "(local: " << std::put_time(std::localtime(&timestamp), "%c %Z") << "):" << std::endl;
        for (size_t i = 0; i < densities.size(); ++i) {
            std::cout << "Quantum state density of circuit " << i << ":" << std::endl; 
            std::cout << densities[i] << std::endl;
        }
    }
}
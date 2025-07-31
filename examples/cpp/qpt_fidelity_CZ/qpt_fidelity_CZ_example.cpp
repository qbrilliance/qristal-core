#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/QuantumStateTomography.hpp>
#include <qristal/core/benchmark/workflows/QuantumProcessTomography.hpp>
#include <qristal/core/benchmark/workflows/WorkflowAddins.hpp>
#include <qristal/core/benchmark/metrics/QuantumProcessFidelity.hpp>

#include <time.h>

using namespace qristal::benchmark;

int main() {

    const size_t n_qubits = 2;

    //(1) set up session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = 1000;
    sim.qn = n_qubits;

    //(2) define workflow: CZ gate execution
    qristal::CircuitBuilder circuit; 
    circuit.CZ(0, 1);
    //wrap circuit in SimpleCircuitExecution and tell qristal::benchmark to 
    //evaluate ideal process matrix from ideal simulation
    using Workflow = AddinFromIdealSimulation<SimpleCircuitExecution, Task::IdealProcess>;
    Workflow workflow(SimpleCircuitExecution(circuit, sim));
    using QST = QuantumStateTomography<Workflow>;
    QST qstworkflow(workflow); //wrap workflow into QST object
    using QPT = QuantumProcessTomography<QST>;
    QPT qptworkflow(qstworkflow); //wrap QST into QPT object

    //(3) pass to metric
    QuantumProcessFidelity<QPT> metric(qptworkflow);

    //(4) evaluate and print
    auto results = metric.evaluate();
    for (const auto& [timestamp, fidelities] : results) {
        std::cout << "Evaluated metric from " << "UTC: " << std::put_time(std::gmtime(&timestamp), "%c %Z")
                  << "(local: " << std::put_time(std::localtime(&timestamp), "%c %Z") << "): " << std::endl;
        std::cout << "Process fidelities: [";
        for (size_t i = 0; i < fidelities.size(); ++i) {
            std::cout << (i == 0 ? "" : ", ") << fidelities[i];
        }
        std::cout << "]" << std::endl;
        std::cout << "Average gate fidelities: [";
        for (size_t i = 0; i < fidelities.size(); ++i) {
            std::cout << (i == 0 ? "" : ", ") << calculate_average_gate_fidelity(fidelities[i], n_qubits);
        }
        std::cout << "]" << std::endl;
    }
}

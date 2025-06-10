#include <qristal/core/benchmark/workflows/SimpleCircuitExecution.hpp>
#include <qristal/core/benchmark/workflows/QuantumStateTomography.hpp>
#include <qristal/core/benchmark/workflows/QuantumProcessTomography.hpp>
#include <qristal/core/benchmark/metrics/QuantumProcessMatrix.hpp>
#include <qristal/core/benchmark/DataLoaderGenerator.hpp>

using namespace qristal::benchmark;

int main() {
    const size_t n_qubits = 1;
    const size_t n_shots = 1000;

    //(1) define session
    qristal::session sim;
    sim.acc = "qpp";
    sim.sn = n_shots;
    sim.qn = n_qubits;

    //(2) define workflow
    qristal::CircuitBuilder circuit;
    circuit.RX(0, std::numbers::pi / 2.0);
    SimpleCircuitExecution workflow(
        std::vector<qristal::CircuitBuilder>{circuit},
        sim
    );
    using QST = QuantumStateTomography<SimpleCircuitExecution>;
    QST qstworkflow(workflow); //wrap circuit execution into QST object
    using QPT = QuantumProcessTomography<QST>;
    QPT qptworkflow(qstworkflow); //wrap QST into QPT object

    //(3) pass to metric
    QuantumProcessMatrix<QPT> metric(qptworkflow);

    //(4) evaluate and print
    auto results = metric.evaluate();
    for (const auto& [timestamp, processes] : results) {
        std::cout << "Evaluated metric from " << "UTC: " << std::put_time(std::gmtime(&timestamp), "%c %Z")
                  << "(local: " << std::put_time(std::localtime(&timestamp), "%c %Z") << "):" << std::endl;
        for (size_t i = 0; i < processes.size(); ++i) {
            std::cout << "Quantum process matrix of circuit " << i << ":" << std::endl;
            std::cout << processes[i] << std::endl;
        }
    }
}

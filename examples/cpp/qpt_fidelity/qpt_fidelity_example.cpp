#include <qristal/core/benchmark/workflows/RotationSweep.hpp>
#include <qristal/core/benchmark/workflows/QuantumStateTomography.hpp>
#include <qristal/core/benchmark/workflows/QuantumProcessTomography.hpp>
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

    //(2) define workflow
    const std::vector<char> rotations_per_qubit{'X', 'I'}; //apply Rx rotations on q0 and Ry rotation on q1
    const int start_degree = -180;
    const int end_degree = +180;
    const size_t n_points = 5;
    RotationSweep workflow(rotations_per_qubit, start_degree, end_degree, n_points, sim);
    using QST = QuantumStateTomography<RotationSweep>;
    QST qstworkflow(workflow); //wrap workflow into QST object
    using QPT = QuantumProcessTomography<QST>;
    QPT qptworkflow(qstworkflow); //wrap QST into QPT object

    //(3) pass to metric
    QuantumProcessFidelity<QPT> metric(qptworkflow);

    //(4) evaluate and print
    auto results = metric.evaluate();
    for (const auto& [timestamp, fidelities] : results) {
        std::cout << "Evaluated metric from " << "UTC: " << std::put_time(std::gmtime(&timestamp), "%c %Z")
                  << "(local: " << std::put_time(std::localtime(&timestamp), "%c %Z") << "): [";
        for (size_t i = 0; i < fidelities.size(); ++i) {
            std::cout << (i == 0 ? "" : ", ") << fidelities[i];
        }
        std::cout << "]" << std::endl;
    }
}

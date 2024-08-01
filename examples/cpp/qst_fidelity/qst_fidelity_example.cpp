#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/workflows/QuantumStateTomography.hpp"
#include "qristal/core/benchmark/metrics/QuantumStateFidelity.hpp"

#include <time.h>

using namespace qristal::benchmark;

int main() {

    const std::set<size_t> qubits{0, 1};

    //(1) set up session  
    qristal::session sim(false); 
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(100);
    sim.set_qn(qubits.size());

    //(2) define workflow
    SPAMBenchmark workflow(qubits, sim);
    QuantumStateTomography<SPAMBenchmark> qstworkflow(workflow);

    //(3) pass to metric
    QuantumStateFidelity<QuantumStateTomography<SPAMBenchmark>> metric(qstworkflow);

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
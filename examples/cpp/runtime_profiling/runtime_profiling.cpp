#include "qristal/core/benchmark/workflows/RuntimeAnalyzer.hpp"
#include "qristal/core/benchmark/workflows/RotationSweep.hpp"
#include "qristal/core/benchmark/workflows/SPAMBenchmark.hpp"
#include "qristal/core/benchmark/metrics/CircuitFidelity.hpp"

using namespace qristal::benchmark;

int main() {
    const std::vector<size_t> qubits{0, 1, 2, 3, 4};

    //(1) set up session  
    qristal::session sim(false); 
    sim.init();
    sim.set_acc("qpp");
    sim.set_sn(1000000);
    sim.set_qn(qubits.size());

    //(2) define workflow
    RotationSweep workflow(
        std::vector<char>{'X', 'Y', 'X', 'Z', 'X'},
        -90,
        90,
        50, 
        sim
    );
    // wrap into RuntimeAnalyzer and set profiling interval
    const size_t profiling_interval_ms = 500;
    RuntimeAnalyzer<RotationSweep> wrapped_workflow(workflow, profiling_interval_ms);

    //(3) pass to metric
    CircuitFidelity<RuntimeAnalyzer<RotationSweep>> metric(wrapped_workflow);

    // //(4) evaluate and print
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